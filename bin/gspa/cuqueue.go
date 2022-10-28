package gspa

import (
	"math/big"
	"runtime"
	"strings"
	"sync"
	"time"
	"unicode/utf16"
	"unsafe"

	"github.com/gammazero/deque"
	"github.com/google/uuid"
)

//These error codes are defined at C/C++ basic error code file definebase.h
const (
	ERROR_UNKNOWN               int = (0xffffffff - 0x100000000)
	BAD_DESERIALIZATION         int = (0xAAAA0000 - 0x100000000)
	SERIALIZATION_NOT_SUPPORTED int = (0xAAAA0001 - 0x100000000)
	BAD_OPERATION               int = (0xAAAA0002 - 0x100000000)
	BAD_INPUT                   int = (0xAAAA0003 - 0x100000000)
	NOT_SUPPORTED               int = (0xAAAA0004 - 0x100000000)
	STL_EXCEPTION               int = (0xAAAA0005 - 0x100000000)
	UNKNOWN_EXCEPTION           int = (0xAAAA0006 - 0x100000000)
	QUEUE_FILE_NOT_AVAILABLE    int = (0xAAAA0007 - 0x100000000)
	ALREADY_DEQUEUED            int = (0xAAAA0008 - 0x100000000)
	ROUTEE_DISCONNECTED         int = (0xAAAA0009 - 0x100000000)
	REQUEST_ABORTED             int = (0xAAAA000A - 0x100000000)
)

const (
	INITIAL_BUFFER_SIZE  uint32 = 4096
	INITIAL_BLOCK_SIZE   uint32 = 4096
	STRING_NULL_END      uint32 = 0xffffffff
	MAX_BUFFER_SIZE      uint32 = STRING_NULL_END
	MAX_UINT64_PLUS1_STR string = "18446744073709551616"
)

var (
	csBuffer    = sync.Mutex{}
	qBufferPool = deque.Deque[*CUQueue]{}
)

//The following structure comes from windows C/C++ DECIMAL
type DECIMAL struct {
	wReserved uint16
	Scale     byte
	Sign      byte
	Hi32      uint32
	Lo64      uint64
}

var uMax64Plus1 *big.Int

func (d *DECIMAL) String() string {
	bi := big.NewInt(int64(d.Hi32))
	if uMax64Plus1 == nil {
		uMax64Plus1, _ = big.NewInt(0).SetString(MAX_UINT64_PLUS1_STR, 10)
	}
	bi = bi.Mul(bi, uMax64Plus1)
	bi = bi.Add(bi, big.NewInt(0).SetUint64(d.Lo64))
	if d.Sign > 0 {
		bi = bi.Neg(bi)
	}
	s := bi.String()
	if d.Scale > 0 {
		pos := len(s) - int(d.Scale)
		ns := s[:pos]
		ss := s[pos:]
		s = ns + "." + ss
	}
	return s
}

func (d *DECIMAL) SetString(s string) bool {
	var scale byte
	pos := strings.Index(s, ".")
	if pos == -1 {
		scale = 0
	} else {
		scale = byte(len(s) - pos - 1)
		s = s[0:pos] + s[pos+1:]
	}
	var bi big.Int
	_, ok := bi.SetString(s, 10)
	if !ok {
		return false
	}
	d.Scale = scale
	if bi.Sign() < 0 {
		d.Sign = 0x80
		bi.Abs(&bi)
	} else {
		d.Sign = 0
	}
	d.Lo64 = bi.Uint64()
	if uMax64Plus1 == nil {
		uMax64Plus1, _ = big.NewInt(0).SetString(MAX_UINT64_PLUS1_STR, 10)
	}
	z := bi.Div(&bi, uMax64Plus1)
	d.Hi32 = uint32(z.Uint64())
	d.wReserved = 0
	return true
}

type VarType uint16

//These type consts come from windows VARTYPEs
const (
	Varray VarType = 0x2000
	Vempty VarType = 0
	Vnull  VarType = 1
	Vint16 VarType = 2
	//always 4-byte int32
	Vint     VarType = 3
	Vfloat32 VarType = 4
	Vfloat64 VarType = 5
	//money, currency = int64/10000.0
	Vcy VarType = 6
	//datetime having accuracy to 100-nanosecond
	Vdate VarType = 7
	//utf16-le string
	Vutf16   VarType = 8
	Vbool    VarType = 11
	Vvariant VarType = 12
	Vdecimal VarType = 14
	Vint8    VarType = 16
	Vbyte    VarType = 17
	Vuint16  VarType = 18
	//always 4-byte uint32
	Vuint   VarType = 19
	Vint64  VarType = 20
	Vuint64 VarType = 21
	Vint32  VarType = 22
	Vuint32 VarType = 23
	Vxml    VarType = 35
	Vuuid   VarType = 72
	Vastr   VarType = (Varray | Vint8)
	Vbytes  VarType = (Varray | Vbyte)
)

func (vt VarType) String() string {
	switch vt {
	case Vempty, Vnull:
		return "nil"
	case Vint, Vint32:
		return "int32"
	case Vuint, Vuint32:
		return "uint32"
	case Vint16:
		return "int16"
	case Vuint16:
		return "uint16"
	case Vint64:
		return "int64"
	case Vuint64:
		return "uint64"
	case Vfloat32:
		return "float32"
	case Vfloat64:
		return "float64"
	case Vcy:
		return "currency"
	case Vdate:
		return "time.Time"
	case Vutf16:
		return "utf16"
	case Vbool:
		return "bool"
	case Vdecimal:
		return "DECIMAL"
	case Vint8:
		return "int8"
	case Vbyte:
		return "byte"
	case Vxml:
		return "xml"
	case Vuuid:
		return "UUID"
	case Vastr:
		return "ascii"
	case Vbytes:
		return "[]byte"
	default:
		if (vt & Varray) == Varray {
			vt -= Varray
			return "[]" + vt.String()
		}
	}
	return ""
}

type CUQueue struct {
	buffer    []byte
	len       uint32
	headPos   uint32
	blockSize uint32
	os        OperationSystem
}

type IUSerializer interface {
	SaveTo(buffer *CUQueue) *CUQueue
}

func (b *CUQueue) setDefaultOS(os string) {
	switch os {
	case "android":
		b.os = Android
	case "windows":
		b.os = Win
	case "darwin":
		b.os = Apple
	default:
		b.os = Unix
	}
}

func MakeBuffer() *CUQueue {
	csBuffer.Lock()
	if qBufferPool.Len() > 0 {
		r := qBufferPool.PopBack()
		csBuffer.Unlock()
		return r
	}
	csBuffer.Unlock()
	var b CUQueue
	b.buffer = make([]byte, INITIAL_BUFFER_SIZE)
	b.blockSize = INITIAL_BLOCK_SIZE
	b.setDefaultOS(runtime.GOOS)
	return &b
}

func (b *CUQueue) Recycle() {
	if b == nil {
		return
	}
	b.SetSize(0)
	b.setDefaultOS(runtime.GOOS)
	csBuffer.Lock()
	qBufferPool.PushBack(b)
	csBuffer.Unlock()
}

func (b *CUQueue) GetOS() OperationSystem {
	return b.os
}

func (b *CUQueue) SetOS(os OperationSystem) OperationSystem {
	b.os = os
	return os
}

func (b *CUQueue) GetBlockSize() uint32 {
	return b.blockSize
}

func (b *CUQueue) SetBlockSize(bs uint32) uint32 {
	if bs == 0 {
		bs = INITIAL_BLOCK_SIZE
	}
	b.blockSize = bs
	return bs
}

func (b *CUQueue) GetSize() uint32 {
	if b.buffer == nil {
		return 0
	}
	return b.len
}

func (b *CUQueue) SetSize(size uint32) *CUQueue {
	if b.buffer == nil {
		b.len = 0
		b.headPos = 0
		return b
	}
	if size > uint32(len(b.buffer))-b.headPos {
		size = uint32(len(b.buffer)) - b.headPos
	}
	b.len = size
	if b.len == 0 {
		b.headPos = 0
	}
	return b
}

func (b *CUQueue) GetHeadPosition() uint32 {
	if b.buffer == nil {
		return 0
	}
	return b.headPos
}

func (b *CUQueue) SetHeadPosition(hp uint32) uint32 {
	if hp >= b.headPos {
		return b.headPos
	}
	b.len += (b.headPos - hp)
	b.headPos = hp
	return hp
}

func (b *CUQueue) GetMaxSize() uint32 {
	if b.buffer == nil {
		return 0
	}
	return uint32(len(b.buffer))
}

func (b *CUQueue) GetTailSize() uint32 {
	if b.buffer == nil {
		return 0
	}
	return uint32(len(b.buffer)) - b.headPos - b.len
}

func (b *CUQueue) Empty() *CUQueue {
	b.buffer = nil
	b.len = 0
	b.headPos = 0
	return b
}

func (b *CUQueue) Realloc(size uint32) *CUQueue {
	if size == 0 {
		size = b.blockSize
	}
	if b.len > size {
		b.len = size
	}
	temp := make([]byte, size)
	copy(temp[0:b.len], b.buffer[b.headPos:b.len+b.headPos])
	b.buffer = temp
	b.headPos = 0
	return b
}

func (b *CUQueue) CleanTrack() *CUQueue {
	if b.buffer == nil {
		return b
	}
	if b.headPos > 0 {
		copy(b.buffer[0:], make([]byte, b.headPos))
	}
	size := uint32(len(b.buffer)) - b.headPos - b.len
	if size > 0 {
		copy(b.buffer[(b.len+b.headPos):], make([]byte, uint32(len(b.buffer))-b.headPos-b.len))
	}
	return b
}

func (b *CUQueue) makeAvailable(add uint32) {
	if b.buffer == nil {
		b.buffer = make([]byte, b.blockSize)
	}
	idle := uint32(len(b.buffer)) - b.headPos - b.len
	if idle < add {
		add -= idle
		r := add % b.blockSize
		m := add / b.blockSize
		size := m * b.blockSize
		if r > 0 {
			size += b.blockSize
		}
		size += uint32(len(b.buffer))
		arr := make([]byte, size)
		copy(arr[0:], b.buffer)
		b.buffer = arr
	}
}

func (b *CUQueue) SaveFloat(f float32) *CUQueue {
	b.makeAvailable(4)
	t := (*float32)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = f
	b.len += 4
	return b
}

func (b *CUQueue) SaveDouble(f float64) *CUQueue {
	b.makeAvailable(8)
	t := (*float64)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = f
	b.len += 8
	return b
}

func (b *CUQueue) SaveUInt(n uint32) *CUQueue {
	b.makeAvailable(4)
	t := (*uint32)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = n
	b.len += 4
	return b
}

func (b *CUQueue) SaveInt(n int32) *CUQueue {
	return b.SaveUInt(uint32(n))
}

func (b *CUQueue) SaveULong(n uint64) *CUQueue {
	b.makeAvailable(8)
	t := (*uint64)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = n
	b.len += 8
	return b
}

func toUTime(t *time.Time) uint64 {
	year, month, day := t.Date()
	hour, min, sec := t.Clock()
	ns := t.Nanosecond()
	var mt uint64
	if year >= 1900 {
		mt = uint64((year - 1900))
	} else {
		mt = 1 //negative, before 1900-01-01
		//mt <<= 13; //year 13bits
		mt <<= 13
		mt += uint64(1900 - year)
	}
	mt <<= 4 //month 4bits
	mt += uint64(int(month) - 1)
	mt <<= 5 //day 5bits
	mt += uint64(day)
	mt <<= 5 //hour 5bits
	mt += uint64(hour)
	mt <<= 6 //minute 6bits
	mt += uint64(min)
	mt <<= 6 //second 6bits
	mt += uint64(sec)
	mt <<= 24 //ticks 24bits
	mt += uint64(ns / 100)
	return mt
}

func (b *CUQueue) SaveDate(t time.Time) *CUQueue {
	var lt uint64 = toUTime(&t)
	return b.SaveULong(lt)
}

func (b *CUQueue) SaveLong(n int64) *CUQueue {
	return b.SaveULong(uint64(n))
}

func (b *CUQueue) SaveUShort(n uint16) *CUQueue {
	b.makeAvailable(2)
	t := (*uint16)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = n
	b.len += 2
	return b
}

func (b *CUQueue) SaveShort(n int16) *CUQueue {
	return b.SaveUShort(uint16(n))
}

//Assuming u16 is utf16-le, 2 bytes only
func (b *CUQueue) SaveChar(u16 rune) *CUQueue {
	b.makeAvailable(2)
	//assuming u16 is utf16-le, 2 bytes only
	t := (*uint16)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = uint16(u16)
	b.len += 2
	return b
}

func (b *CUQueue) SaveByte(n byte) *CUQueue {
	b.makeAvailable(1)
	b.buffer[b.len+b.headPos] = n
	b.len += 1
	return b
}

func (b *CUQueue) SaveAChar(n int8) *CUQueue {
	return b.SaveByte(byte(n))
}

func (b *CUQueue) SaveBool(n bool) *CUQueue {
	b.makeAvailable(1)
	if n {
		b.buffer[b.len+b.headPos] = 1
	} else {
		b.buffer[b.len+b.headPos] = 0
	}
	b.len += 1
	return b
}

func (b *CUQueue) SaveUUID(id uuid.UUID) *CUQueue {
	b.makeAvailable(16)
	copy(b.buffer[(b.len+b.headPos):], id[:])
	b.len += 16
	return b
}

func (b *CUQueue) SaveDecimal(dec DECIMAL) *CUQueue {
	if dec.Sign > 0 {
		dec.Sign = 0x80
	}
	b.makeAvailable(16)
	t := (*DECIMAL)(unsafe.Pointer(&b.buffer[(b.len + b.headPos)]))
	*t = dec
	b.len += 16
	return b
}

type AStr string
type AStrArray []*AStr

func (s *AStr) String() string {
	if s == nil {
		return "<nil>"
	}
	return string(*s)
}

//Support saving a nil string
func (b *CUQueue) SaveAString(s *AStr) *CUQueue {
	if s == nil {
		b.SaveUInt(STRING_NULL_END)
		return b
	}
	len := uint32(len(*s))
	b.SaveUInt(len)
	if len == 0 {
		return b
	}
	b.makeAvailable(len)
	copy(b.buffer[(b.len+b.headPos):], *s)
	b.len += len
	return b
}

//Support saving a nil byte array
func (b *CUQueue) SaveBytes(s *[]byte) *CUQueue {
	if s == nil {
		b.SaveUInt(STRING_NULL_END)
		return b
	}
	len := uint32(len(*s))
	b.SaveUInt(len)
	if len == 0 {
		return b
	}
	b.makeAvailable(len)
	copy(b.buffer[(b.len+b.headPos):], *s)
	b.len += len
	return b
}

func (b *CUQueue) PushBytes(s []byte) *CUQueue {
	len := uint32(len(s))
	b.makeAvailable(len)
	copy(b.buffer[(b.len+b.headPos):], s)
	b.len += len
	return b
}

//Support saving a nil byte array
func (b *CUQueue) SaveBytesOffset(s *[]byte, offset uint32) *CUQueue {
	if s == nil {
		b.SaveUInt(STRING_NULL_END)
		return b
	}
	size := uint32(len(*s))
	if offset > size {
		offset = size
	}
	size -= offset
	b.SaveUInt(size)
	if size == 0 {
		return b
	}
	b.makeAvailable(size)
	copy(b.buffer[(b.len+b.headPos):], (*s)[offset:offset+size])
	b.len += size
	return b
}

//Support saving a nil byte array
func (b *CUQueue) SaveBytesOffsetLen(s *[]byte, offset uint32, count uint32) *CUQueue {
	if s == nil {
		b.SaveUInt(STRING_NULL_END)
		return b
	}
	size := uint32(len(*s))
	if offset > size {
		offset = size
	}
	size -= offset
	if count > size {
		count = size
	}
	b.SaveUInt(count)
	if count == 0 {
		return b
	}
	b.makeAvailable(count)
	copy(b.buffer[(b.len+b.headPos):], (*s)[offset:offset+count])
	b.len += count
	return b
}

//Support saving a nil string
func (b *CUQueue) SaveString(s *string) *CUQueue {
	if s == nil {
		b.SaveUInt(STRING_NULL_END)
		return b
	}
	codes := utf16.Encode([]rune(*s))
	bytes := uint32(len(codes)) << 1
	b.SaveUInt(bytes)
	if bytes == 0 {
		return b
	}
	b.makeAvailable(bytes)
	pos := b.len + b.headPos
	for i, val := range codes {
		*(*uint16)(unsafe.Pointer(&(b.buffer[(pos + uint32(i<<1))]))) = val
	}
	b.len += bytes
	return b
}

func (b *CUQueue) checkAvailable(size uint32) {
	if b.len < size {
		panic("Remaining data in size smaller than expected size")
	}
}

func (b *CUQueue) LoadUInt() uint32 {
	b.checkAvailable(4)
	n := *(*uint32)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 4
	b.len -= 4
	if b.len == 0 {
		b.headPos = 0
	}
	return n
}

func (b *CUQueue) LoadInt() int32 {
	n := b.LoadUInt()
	return int32(n)
}

func (b *CUQueue) LoadFloat() float32 {
	b.checkAvailable(4)
	f := *(*float32)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 4
	b.len -= 4
	if b.len == 0 {
		b.headPos = 0
	}
	return f
}

func (b *CUQueue) LoadDecimal() DECIMAL {
	var dec DECIMAL
	b.checkAvailable(16)
	dec = *(*DECIMAL)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 16
	b.len -= 16
	if b.len == 0 {
		b.headPos = 0
	}
	return dec
}

func (b *CUQueue) LoadDouble() float64 {
	b.checkAvailable(8)
	f := *(*float64)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 8
	b.len -= 8
	if b.len == 0 {
		b.headPos = 0
	}
	return f
}

func (b *CUQueue) LoadUShort() uint16 {
	b.checkAvailable(2)
	n := *(*uint16)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 2
	b.len -= 2
	if b.len == 0 {
		b.headPos = 0
	}
	return n
}

func (b *CUQueue) LoadChar() rune {
	n := b.LoadUShort()
	return rune(n)
}

func (b *CUQueue) LoadShort() int16 {
	n := b.LoadUShort()
	return int16(n)
}

func (b *CUQueue) LoadByte() byte {
	b.checkAvailable(1)
	n := b.buffer[b.headPos]
	b.headPos += 1
	b.len -= 1
	if b.len == 0 {
		b.headPos = 0
	}
	return n
}

func (b *CUQueue) LoadAChar() int8 {
	n := b.LoadByte()
	return int8(n)
}

func (b *CUQueue) LoadBool() bool {
	n := b.LoadByte()
	var ok bool
	if n == 0 {
		ok = false
	} else {
		ok = true
	}
	return ok
}

func (b *CUQueue) LoadULong() uint64 {
	b.checkAvailable(8)
	n := *(*uint64)(unsafe.Pointer(&(b.buffer[b.headPos])))
	b.headPos += 8
	b.len -= 8
	if b.len == 0 {
		b.headPos = 0
	}
	return n
}

func (b *CUQueue) LoadDate() time.Time {
	mt := b.LoadULong()
	ns100 := int(mt & 0xffffff) //24bits
	mt >>= 24
	second := int(mt & 0x3f) //6bits
	mt >>= 6
	minute := int(mt & 0x3f) //6bits
	mt >>= 6
	hour := int(mt & 0x1f) //5bits
	mt >>= 5
	day := int(mt & 0x1f) //5bits
	mt >>= 5

	//0 - 11 instead of 1 - 12
	month := int(mt & 0xf) //4bits
	mt >>= 4

	//8191 == 0x1fff, From BC 6291 (8191 - 1900) to AD 9991 (1900 + 8191)
	year := int(mt & 0x1fff) //13bits
	mt >>= 13

	//It will be 1 if date time is earlier than 1900-01-01
	neg := int(mt)
	if neg > 0 {
		year = 1900 - year
	} else {
		year += 1900
	}
	month++
	dt := time.Date(year, time.Month(month), day, hour, minute, second, ns100*100, time.UTC)
	return dt
}

func (b *CUQueue) LoadUUID() uuid.UUID {
	b.checkAvailable(16)
	id, _ := uuid.FromBytes(make([]byte, 16))
	copy(id[:], b.buffer[b.headPos:b.headPos+16])
	b.headPos += 16
	b.len -= 16
	if b.len == 0 {
		b.headPos = 0
	}
	return id
}

func (b *CUQueue) LoadLong() int64 {
	un := b.LoadULong()
	return int64(un)
}

//Support returning a nil byte array
func (b *CUQueue) LoadBytes() *[]byte {
	size := b.LoadUInt()
	if size == STRING_NULL_END {
		return nil
	}
	b.checkAvailable(size)
	s := b.buffer[b.headPos : b.headPos+size]
	b.headPos += size
	b.len -= size
	if b.len == 0 {
		b.headPos = 0
	}
	return &s
}

//Support returning a nil string.
func (b *CUQueue) LoadAString() *AStr {
	size := b.LoadUInt()
	if size == STRING_NULL_END {
		return nil
	}
	b.checkAvailable(size)
	s := AStr(b.buffer[b.headPos : b.headPos+size])
	b.headPos += size
	b.len -= size
	if b.len == 0 {
		b.headPos = 0
	}
	return &s
}

//Support returning a nil string.
func (b *CUQueue) LoadString() *string {
	bytes := b.LoadUInt()
	if bytes == STRING_NULL_END {
		return nil
	}
	b.checkAvailable(bytes)
	arr := make([]uint16, bytes>>1)
	pos := int(b.headPos)
	for i, _ := range arr {
		arr[i] = *(*uint16)(unsafe.Pointer(&(b.buffer[pos])))
		pos += 2
	}
	b.headPos += bytes
	b.len -= bytes
	if b.len == 0 {
		b.headPos = 0
	}
	s := string(utf16.Decode(arr))
	return &s
}

func (b *CUQueue) SaveUShortArray(arr []uint16) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 1
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadUShortArray() []uint16 {
	size := b.LoadUInt()
	arr := make([]uint16, size)
	if size > 0 {
		size <<= 1
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveShortArray(arr []int16) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 1
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadShortArray() []int16 {
	size := b.LoadUInt()
	arr := make([]int16, size)
	if size > 0 {
		size <<= 1
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveUIntArray(arr []uint32) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 2
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadUIntArray() []uint32 {
	size := b.LoadUInt()
	arr := make([]uint32, size)
	if size > 0 {
		size <<= 2
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) saveToBuffer(p unsafe.Pointer, size uint32) {
	b.makeAvailable(size)
	q := (*[STRING_NULL_END]byte)(p)
	copy(b.buffer[b.headPos+b.len:], (*q)[0:size])
	b.len += size
}

func (b *CUQueue) SaveIntArray(arr []int32) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 2
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) loadFromBuffer(p unsafe.Pointer, size uint32) {
	if size > b.len {
		panic("Remaining data in size smaller than expected size")
	}
	q := (*[STRING_NULL_END]byte)(p)
	copy((*q)[0:size], b.buffer[b.headPos:b.headPos+size])
	b.len -= size
	b.headPos += size
	if b.len == 0 {
		b.headPos = 0
	}
}

func (b *CUQueue) LoadIntArray() []int32 {
	size := b.LoadUInt()
	arr := make([]int32, size)
	if size > 0 {
		size <<= 2
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveUIntArray2(arr []uint) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveUInt(uint32(arr[n]))
	}
	return b
}

func (b *CUQueue) SaveIntArray2(arr []int) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveInt(int32(arr[n]))
	}
	return b
}

func (b *CUQueue) SaveULongArray(arr []uint64) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 3
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadULongArray() []uint64 {
	size := b.LoadUInt()
	arr := make([]uint64, size)
	if size > 0 {
		size <<= 3
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveLongArray(arr []int64) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 3
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadLongArray() []int64 {
	size := b.LoadUInt()
	arr := make([]int64, size)
	if size > 0 {
		size <<= 3
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveDecimalArray(arr []DECIMAL) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveDecimal(arr[n])
	}
	return b
}

func (b *CUQueue) LoadDecimalArray() []DECIMAL {
	size := b.LoadUInt()
	arr := make([]DECIMAL, size)
	for n := uint32(0); n < size; n++ {
		arr[n] = b.LoadDecimal()
	}
	return arr
}

func (b *CUQueue) SaveFloatArray(arr []float32) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 2
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadFloatArray() []float32 {
	size := b.LoadUInt()
	arr := make([]float32, size)
	if size > 0 {
		size <<= 2
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

func (b *CUQueue) SaveDoubleArray(arr []float64) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		size <<= 3
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadDoubleArray() []float64 {
	size := b.LoadUInt()
	arr := make([]float64, size)
	if size > 0 {
		size <<= 3
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

//A string array may contain a nil string
func (b *CUQueue) SaveStringArray(arr []*string) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveString(arr[n])
	}
	return b
}

//A string array may contain a nil string
func (b *CUQueue) SaveAStringArray(arr AStrArray) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveAString(arr[n])
	}
	return b
}

//A string array may contain a nil string
func (b *CUQueue) LoadStringArray() []*string {
	size := b.LoadUInt()
	arr := make([]*string, size)
	for n := uint32(0); n < size; n++ {
		arr[n] = b.LoadString()
	}
	return arr
}

//A string array may contain a nil string
func (b *CUQueue) LoadAStringArray() AStrArray {
	size := b.LoadUInt()
	arr := make(AStrArray, size)
	for n := uint32(0); n < size; n++ {
		arr[n] = b.LoadAString()
	}
	return arr
}

func (b *CUQueue) SaveUUIDArray(arr []uuid.UUID) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveUUID(arr[n])
	}
	return b
}

func (b *CUQueue) LoadUUIDArray() []uuid.UUID {
	size := b.LoadUInt()
	arr := make([]uuid.UUID, size)
	for n := uint32(0); n < size; n++ {
		arr[n] = b.LoadUUID()
	}
	return arr
}

func (b *CUQueue) SaveDateArray(arr []time.Time) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	for n := uint32(0); n < size; n++ {
		b.SaveDate(arr[n])
	}
	return b
}

func (b *CUQueue) LoadDateArray() []time.Time {
	size := b.LoadUInt()
	arr := make([]time.Time, size)
	for n := uint32(0); n < size; n++ {
		arr[n] = b.LoadDate()
	}
	return arr
}

func (b *CUQueue) SaveBoolArray(arr []bool) *CUQueue {
	size := uint32(len(arr))
	b.SaveUInt(size)
	if size > 0 {
		b.saveToBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return b
}

func (b *CUQueue) LoadBoolArray() []bool {
	size := b.LoadUInt()
	arr := make([]bool, size)
	if size > 0 {
		b.loadFromBuffer(unsafe.Pointer(&arr[0]), size)
	}
	return arr
}

//Pay attention to unicode char (rune)! cast unicode char (rune) into uint16 or int16.
//Passing a nil represents saving a nill string or byte array into the buffer.
func (b *CUQueue) Save(a ...interface{}) *CUQueue {
	for _, vt := range a {
		if vt == nil {
			b.SaveString(nil)
		} else {
			switch vt.(type) {
			case string:
				s := vt.(string)
				b.SaveString(&s)
				break
			case int:
				b.SaveInt(int32(vt.(int)))
				break
			case int32:
				b.SaveInt(vt.(int32))
				break
			case float64:
				b.SaveDouble(vt.(float64))
				break
			case float32:
				b.SaveFloat(vt.(float32))
				break
			case time.Time:
				b.SaveDate(vt.(time.Time))
				break
			case int64:
				b.SaveLong(vt.(int64))
				break
			case bool:
				b.SaveBool(vt.(bool))
				break
			case []byte:
				s := vt.([]byte)
				b.SaveBytes(&s)
				break
			case int16:
				b.SaveShort(vt.(int16))
				break
			case DECIMAL:
				b.SaveDecimal(vt.(DECIMAL))
				break
			case uuid.UUID:
				b.SaveUUID(vt.(uuid.UUID))
				break
			case uint16:
				b.SaveUShort(vt.(uint16))
				break
			case uint32:
				b.SaveUInt(vt.(uint32))
				break
			case uint:
				b.SaveUInt(uint32(vt.(uint)))
				break
			case uint64:
				b.SaveULong(vt.(uint64))
				break
			case int8:
				b.SaveAChar(vt.(int8))
				break
			case []int8:
				s := vt.([]int8)
				size := uint32(len(s))
				if size > 0 {
					p := (*[]byte)(unsafe.Pointer(&s))
					str := AStr(*p)
					b.SaveAString(&str)
				}
				break
			case AStr:
				s := vt.(AStr)
				size := uint32(len(s))
				if size > 0 {
					p := (*[]byte)(unsafe.Pointer(&s))
					str := AStr(*p)
					b.SaveAString(&str)
				}
				break
			case byte: //uint8
				b.SaveByte(vt.(byte))
				break
			case []*string:
				b.SaveStringArray(vt.([]*string))
				break
			case []string:
				aa := vt.([]string)
				b.SaveUInt(uint32(len(aa)))
				for _, a := range aa {
					b.SaveString(&a)
				}
				break
			case AStrArray:
				b.SaveAStringArray(vt.(AStrArray))
				break
			case []AStr:
				aa := vt.([]AStr)
				b.SaveUInt(uint32(len(aa)))
				for _, a := range aa {
					b.SaveAString(&a)
				}
				break
			case []int:
				b.SaveIntArray2(vt.([]int))
				break
			case []int32:
				b.SaveIntArray(vt.([]int32))
				break
			case []float64:
				b.SaveDoubleArray(vt.([]float64))
				break
			case []float32:
				b.SaveFloatArray(vt.([]float32))
				break
			case []DECIMAL:
				b.SaveDecimalArray(vt.([]DECIMAL))
				break
			case []uint64:
				b.SaveULongArray(vt.([]uint64))
				break
			case []uint:
				b.SaveUIntArray2(vt.([]uint))
				break
			case []uint32:
				b.SaveUIntArray(vt.([]uint32))
				break
			case []uint16:
				b.SaveUShortArray(vt.([]uint16))
				break
			case []int16:
				b.SaveShortArray(vt.([]int16))
				break
			case []uuid.UUID:
				b.SaveUUIDArray(vt.([]uuid.UUID))
				break
			case []bool:
				b.SaveBoolArray(vt.([]bool))
				break
			case []time.Time:
				b.SaveDateArray(vt.([]time.Time))
				break
			default:
				if s, ok := vt.(IUSerializer); ok {
					s.SaveTo(b)
				} else {
					panic("Unsupported data type for serialization")
				}
			}
		}
	}
	return b
}

//Pay attention to unicode char (rune)! cast unicode char (rune) into uint16.
func (b *CUQueue) SaveObject(vt interface{}) *CUQueue {
	if vt == nil {
		b.SaveUShort(uint16(Vnull))
		return b
	}
	switch vt.(type) {
	case string:
		s := vt.(string)
		return b.SaveUShort(uint16(Vutf16)).SaveString(&s)
	case AStr:
		s := vt.(AStr)
		return b.SaveUShort(uint16(Vastr)).SaveAString(&s)
	case int:
		return b.SaveUShort(uint16(Vint)).SaveInt(int32(vt.(int)))
	case int32:
		return b.SaveUShort(uint16(Vint)).SaveInt(vt.(int32))
	case float64:
		return b.SaveUShort(uint16(Vfloat64)).SaveDouble(vt.(float64))
	case float32:
		return b.SaveUShort(uint16(Vfloat32)).SaveFloat(vt.(float32))
	case time.Time:
		return b.SaveUShort(uint16(Vdate)).SaveDate(vt.(time.Time))
	case int64:
		return b.SaveUShort(uint16(Vint64)).SaveLong(vt.(int64))
	case bool:
		if vt.(bool) {
			return b.SaveUShort(uint16(Vbool)).SaveShort(-1)
		} else {
			return b.SaveUShort(uint16(Vbool)).SaveShort(0)
		}
	case []byte:
		s := vt.([]byte)
		return b.SaveUShort(uint16(Vbytes)).SaveBytes(&s)
	case int16:
		return b.SaveUShort(uint16(Vint16)).SaveShort(vt.(int16))
	case DECIMAL:
		return b.SaveUShort(uint16(Vdecimal)).SaveDecimal(vt.(DECIMAL))
	case uuid.UUID:
		return b.SaveUShort(uint16(Vuuid)).SaveUUID(vt.(uuid.UUID))
	case []interface{}:
		s := vt.([]interface{})
		b.SaveUShort(uint16(Vvariant | Varray))
		size := uint32(len(s))
		b.SaveUInt(size)
		for _, vt := range s {
			b.SaveObject(vt)
		}
		break
	case uint16:
		return b.SaveUShort(uint16(Vuint16)).SaveUShort(vt.(uint16))
	case uint:
		return b.SaveUShort(uint16(Vuint)).SaveUInt(uint32(vt.(uint)))
	case uint32:
		return b.SaveUShort(uint16(Vuint)).SaveUInt(vt.(uint32))
	case uint64:
		return b.SaveUShort(uint16(Vuint64)).SaveULong(vt.(uint64))
	case int8:
		return b.SaveUShort(uint16(Vint8)).SaveAChar(vt.(int8))
	case []int8:
		s := vt.([]int8)
		b.SaveUShort(uint16(Vastr))
		p := (*[]byte)(unsafe.Pointer(&s))
		str := AStr(*p)
		b.SaveAString(&str)
		break
	case byte:
		return b.SaveUShort(uint16(Vbyte)).SaveByte(vt.(byte))
	case []*string:
		return b.SaveUShort(uint16(Vutf16 | Varray)).SaveStringArray(vt.([]*string))
	case []int32:
		return b.SaveUShort(uint16(Vint | Varray)).SaveIntArray(vt.([]int32))
	case []int:
		return b.SaveUShort(uint16(Vint | Varray)).SaveIntArray2(vt.([]int))
	case []float64:
		return b.SaveUShort(uint16(Vfloat64 | Varray)).SaveDoubleArray(vt.([]float64))
	case []float32:
		return b.SaveUShort(uint16(Vfloat32 | Varray)).SaveFloatArray(vt.([]float32))
	case []DECIMAL:
		return b.SaveUShort(uint16(Vdecimal | Varray)).SaveDecimalArray(vt.([]DECIMAL))
	case []uint64:
		return b.SaveUShort(uint16(Vuint64 | Varray)).SaveULongArray(vt.([]uint64))
	case []uint32:
		return b.SaveUShort(uint16(Vuint | Varray)).SaveUIntArray(vt.([]uint32))
	case []uint:
		return b.SaveUShort(uint16(Vuint | Varray)).SaveUIntArray2(vt.([]uint))
	case []uint16:
		return b.SaveUShort(uint16(Vuint16 | Varray)).SaveUShortArray(vt.([]uint16))
	case []int16:
		return b.SaveUShort(uint16(Vint16 | Varray)).SaveShortArray(vt.([]int16))
	case []uuid.UUID:
		return b.SaveUShort(uint16(Vuuid | Varray)).SaveUUIDArray(vt.([]uuid.UUID))
	case []string:
		arr := vt.([]string)
		b.SaveUShort(uint16(Vutf16 | Varray))
		size := uint32(len(arr))
		b.SaveUInt(size)
		for _, s := range arr {
			b.SaveString(&s)
		}
		break
	case []bool:
		s := vt.([]bool)
		b.SaveUShort(uint16(Vbool | Varray))
		size := uint32(len(s))
		b.SaveUInt(size)
		for _, n := range s {
			if n {
				b.SaveShort(-1)
			} else {
				b.SaveShort(0)
			}
		}
		break
	case []time.Time:
		return b.SaveUShort(uint16(Vdate | Varray)).SaveDateArray(vt.([]time.Time))
	default:
		//pay attention to unicode char (rune)! cast unicode char (rune) into uint16
		panic("Unsupported data type for serialization")
	}
	return b
}

func (b *CUQueue) SaveObjects(a ...interface{}) *CUQueue {
	for _, vt := range a {
		b.SaveObject(vt)
	}
	return b
}

func (b *CUQueue) LoadObject() interface{} {
	dt := VarType(b.LoadUShort())
	if dt == Vempty || dt == Vnull {
		return nil
	}
	var vt interface{}
	switch dt {
	case Vint, Vint32:
		vt = b.LoadInt()
		break
	case (Vint | Varray), (Vint32 | Varray):
		vt = b.LoadIntArray()
		break
	case Vutf16:
		s := b.LoadString()
		if s != nil {
			vt = *s
		}
		break
	case (Vutf16 | Varray):
		vt = b.LoadStringArray()
		break
	case Vdate:
		vt = b.LoadDate()
		break
	case (Vdate | Varray):
		vt = b.LoadDateArray()
		break
	case Vastr:
		s := b.LoadAString()
		if s != nil {
			vt = *s
		}
		break
	case Vfloat64:
		vt = b.LoadDouble()
		break
	case (Vfloat64 | Varray):
		vt = b.LoadDoubleArray()
		break
	case Vint64:
		vt = b.LoadLong()
		break
	case (Vint64 | Varray):
		vt = b.LoadLongArray()
		break
	case Vfloat32:
		vt = b.LoadFloat()
		break
	case (Vfloat32 | Varray):
		vt = b.LoadFloatArray()
		break
	case Vdecimal:
		vt = b.LoadDecimal()
		break
	case (Vdecimal | Varray):
		vt = b.LoadDecimalArray()
		break
	case Vbool:
		ok := b.LoadUShort()
		if ok > 0 {
			vt = true
		} else {
			vt = false
		}
		break
	case (Vbool | Varray):
		size := b.LoadUInt()
		arr := make([]bool, size)
		for n := uint32(0); n < size; n++ {
			s := b.LoadUShort()
			if s > 0 {
				arr[n] = true
			}
		}
		vt = arr
		break
	case Vbytes:
		vt = b.LoadBytes()
		break
	case Vuuid:
		vt = b.LoadUUID()
		break
	case (Vuuid | Varray):
		vt = b.LoadUUIDArray()
		break
	case Vcy:
		var dec DECIMAL
		cy := b.LoadLong()
		if cy < 0 {
			dec.Sign = 0x80
		}
		dec.Lo64 = uint64(cy)
		dec.Scale = 4
		vt = dec
		break
	case (Vcy | Varray):
		size := b.LoadUInt()
		arr := make([]DECIMAL, size)
		for n := uint32(0); n < size; n++ {
			cy := b.LoadLong()
			if cy != 0 {
				var dec DECIMAL
				if cy < 0 {
					dec.Sign = 0x80
				}
				dec.Lo64 = uint64(cy)
				dec.Scale = 4
				arr[n] = dec
			}
		}
		vt = arr
		break
	case Vint16:
		vt = b.LoadShort()
		break
	case (Vint16 | Varray):
		vt = b.LoadShortArray()
		break
	case Vuint, Vuint32:
		vt = b.LoadUInt()
		break
	case (Vuint | Varray), (Vuint32 | Varray):
		vt = b.LoadUIntArray()
		break
	case Vuint64:
		vt = b.LoadULong()
		break
	case (Vuint64 | Varray):
		vt = b.LoadULongArray()
		break
	case Vuint16:
		vt = b.LoadUShort()
		break
	case (Vuint16 | Varray):
		vt = b.LoadUShortArray()
		break
	case Vint8:
		vt = b.LoadAChar()
		break
	case Vbyte:
		vt = b.LoadByte()
		break
	default:
		panic("Unsupported data type for deserialization")
	}
	return vt
}

func (b *CUQueue) Discard(size uint32) *CUQueue {
	if nil == b.buffer || 0 == len(b.buffer) {
		return b
	} else if size > b.len {
		size = b.len
	}
	b.headPos += size
	b.len -= size
	if 0 == b.len {
		b.headPos = 0
	}
	return b
}

func (b *CUQueue) GetBuffer(pos ...uint32) unsafe.Pointer {
	if nil == b.buffer || 0 == len(b.buffer) {
		return unsafe.Pointer(nil)
	} else if b.len == 0 {
		return unsafe.Pointer(&b.buffer[0])
	}
	var n uint32
	if len(pos) > 0 {
		n = pos[0]
	}
	if n >= b.len {
		return unsafe.Pointer(nil)
	}
	return unsafe.Pointer(&b.buffer[b.headPos+n])
}

func (b *CUQueue) SetBuffer(arr []byte, size uint32) *CUQueue {
	b.headPos = 0
	b.buffer = arr
	b.blockSize = INITIAL_BLOCK_SIZE
	if arr == nil {
		b.len = 0
	} else if size > uint32(len(arr)) {
		size = uint32(len(arr))
		b.len = size
	} else {
		b.len = size
	}
	b.setDefaultOS(runtime.GOOS)
	return b
}

func (b *CUQueue) PopBytes() []byte {
	if b.buffer == nil || 0 == b.len {
		return make([]byte, 0)
	}
	bytes := b.buffer[b.headPos : b.headPos+b.len]
	b.headPos = 0
	b.len = 0
	return bytes
}

func (b *CUQueue) GetInternalBuffer() []byte {
	return b.buffer
}

//args == offset, bytes
func (b *CUQueue) GetBytes(args ...uint32) []byte {
	var os uint32
	if b.buffer == nil || 0 == b.len {
		return make([]byte, 0)
	}
	if len(args) > 0 {
		os = args[0]
		if os > b.len {
			os = b.len
		}
	}
	max := b.len - os
	if max == 0 {
		return make([]byte, 0)
	}
	if len(args) > 1 {
		if args[1] < max {
			max = args[1]
		}
	}
	return b.buffer[b.headPos+os : b.headPos+os+max]
}

func (b *CUQueue) Swap(b0 *CUQueue) *CUQueue {
	if b0 == nil {
		return b
	}
	tb := b.buffer
	b.buffer = b0.buffer
	b0.buffer = tb

	d := b.len
	b.len = b0.len
	b0.len = d

	d = b.headPos
	b.headPos = b0.headPos
	b0.headPos = d

	d = b.blockSize
	b.blockSize = b0.blockSize
	b0.blockSize = d

	o := b.os
	b.os = b0.os
	b0.os = o

	return b
}
