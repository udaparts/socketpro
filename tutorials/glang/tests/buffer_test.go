package tests

import (
	"bytes"
	"fmt"
	"gspa"
	"reflect"
	"testing"
	"time"

	"github.com/google/uuid"
)

func Test_primitves(t *testing.T) {
	var as *gspa.AStr
	as0 := gspa.AStr("Mytest")
	aa := gspa.AStrArray{as, &as0}
	fmt.Println(aa)

	q := gspa.MakeBuffer()
	var c int8 = 't'
	var b byte = 0xD4
	var ok bool = true
	var s int16 = 12345
	var us uint16 = 42345
	var n int32 = 10
	var un uint32 = 4023456784
	var n64 int64 = 9999999988888
	var un64 uint64 = 9999999988888777
	var dt time.Time = time.Now()
	var f float32 = 29.876
	var d float64 = 87654.2756
	q.Save(c, b, ok, s, us, n, un, n64, un64, dt, f, d)
	c0 := q.LoadAChar()
	b0 := q.LoadByte()
	ok0 := q.LoadBool()
	s0 := q.LoadShort()
	us0 := q.LoadUShort()
	n0 := q.LoadInt()
	un0 := q.LoadUInt()
	n640 := q.LoadLong()
	un640 := q.LoadULong()
	dt0 := q.LoadDate()
	f0 := q.LoadFloat()
	d0 := q.LoadDouble()
	if c != c0 {
		t.Errorf("Test_primitves c0: %d, want: %d", c0, c)
	}
	if b != b0 {
		t.Errorf("Test_primitves b0: %d, want: %d", b0, b)
	}
	if ok != ok0 {
		t.Errorf("Test_primitves b0: %t, want: %t", ok0, ok)
	}
	if s != s0 {
		t.Errorf("Test_primitves s0: %d, want: %d", s0, s)
	}
	if us != us0 {
		t.Errorf("Test_primitves us0: %d, want: %d", us0, us)
	}
	if n != n0 {
		t.Errorf("Test_primitves n0: %d, want: %d", n0, n)
	}
	if un != un0 {
		t.Errorf("Test_primitves un0: %d, want: %d", un0, un)
	}
	if n64 != n640 {
		t.Errorf("Test_primitves n640: %d, want: %d", n640, n64)
	}
	if un64 != un640 {
		t.Errorf("Test_primitves un640: %d, want: %d", un640, un64)
	}
	ndt := dt.UnixNano() / 100
	ndt0 := dt0.UnixNano() / 100
	if ndt != ndt0 {
		t.Errorf("Test_primitves ndt0: %d, want: %d", ndt0, ndt)
	}
	if f != f0 {
		t.Errorf("Test_primitves f0: %f, want: %f", f0, f)
	}
	if d != d0 {
		t.Errorf("Test_primitves d0: %f, want: %f", d0, d)
	}
	if q.GetSize() != 0 {
		t.Errorf("Test_primitves buffer size: %d, want: 0", q.GetSize())
	}
}

func Test_string(t *testing.T) {
	var s string = "普丁滅國神器"
	var as gspa.AStr = "I founded Masterworks"
	q := gspa.MakeBuffer()
	q.SaveString(&s).Save(s).SaveAString(&as)
	s0 := *q.LoadString()
	s1 := *q.LoadString()
	as0 := *q.LoadAString()
	if s0 != s || s1 != s {
		t.Errorf("Test_string: %s, %s, want: %s", s0, s1, s)
	}
	if as0 != as {
		t.Errorf("Test_string: %s, want: %s", as0, as)
	}
	if q.GetSize() != 0 {
		t.Errorf("Test_string buffer size: %d, want: 0", q.GetSize())
	}
}

func Test_string_arr(t *testing.T) {
	var as gspa.AStr = "Test me"
	aa := gspa.AStrArray{&as}
	aaa := []gspa.AStr{as}
	q := gspa.MakeBuffer().Save(as).SaveAString(&as).Save(aa).Save(aaa)
	s := *q.LoadAString()
	if as != s {
		t.Errorf("Test_string: %s, want: %s", s, as)
	}
	s = *q.LoadAString()
	if as != s {
		t.Errorf("Test_string: %s, want: %s", s, as)
	}
	ar := q.LoadAStringArray()
	if as != *ar[0] {
		t.Errorf("Test_string: %s, want: %s", *ar[0], as)
	}
	ar = q.LoadAStringArray()
	if as != *ar[0] {
		t.Errorf("Test_string: %s, want: %s", *ar[0], as)
	}
	var was string = "李进进先生于3月14日在纽约不幸遇害"
	waa := []*string{&was}
	waaa := []string{was}
	q = gspa.MakeBuffer().Save(was).SaveString(&was).Save(waa).Save(waaa)
	w := *q.LoadString()
	if was != w {
		t.Errorf("Test_string: %s, want: %s", w, was)
	}
	w = *q.LoadString()
	if was != w {
		t.Errorf("Test_string: %s, want: %s", w, was)
	}
	wa := q.LoadStringArray()
	if was != *wa[0] {
		t.Errorf("Test_string: %s, want: %s", *wa[0], was)
	}
	wa = q.LoadStringArray()
	if was != *wa[0] {
		t.Errorf("Test_string: %s, want: %s", *wa[0], was)
	}
}

func Test_others(t *testing.T) {
	var d gspa.DECIMAL
	d.SetString("-65789345123456789867432.45")
	clsid := uuid.New()
	bs := make([]byte, 4)
	bs[0] = 0x64
	bs[1] = 0x89
	bs[2] = 0x34
	bs[3] = 0xa9
	q := gspa.MakeBuffer()
	d0 := q.SaveDecimal(d).SaveUUID(clsid).SaveBytes(&bs).LoadDecimal()
	clsid0 := q.LoadUUID()
	bs0 := q.LoadBytes()
	if d0 != d {
		t.Errorf("Test_others: %s, want: %s", d0.String(), d.String())
	}
	if clsid0 != clsid {
		t.Errorf("Test_others: %s, want: %s", clsid0.String(), clsid.String())
	}
	if !bytes.Equal(bs, *bs0) {
		t.Errorf("Test_others: %v, want: %v", bs0, bs)
	}
	if q.GetSize() != 0 {
		t.Errorf("Test_others buffer size: %d, want: 0", q.GetSize())
	}
}

func Test_objects_1(t *testing.T) {
	var d gspa.DECIMAL
	d.SetString("-65789345123456789867432.45")
	clsid := uuid.New()
	bs := make([]byte, 4)
	bs[0] = 0x64
	bs[1] = 0x89
	bs[2] = 0x34
	bs[3] = 0xa9
	q := gspa.MakeBuffer()
	d0 := q.SaveObject(d).SaveObject(clsid).SaveObjects(bs).LoadObject()
	clsid0 := q.LoadObject()
	bs0 := *q.LoadObject().(*[]byte)
	if d0 != d {
		t.Errorf("Test_objects_1: Decimal failed, want: %s", d.String())
	}
	if clsid0 != clsid {
		t.Errorf("Test_objects_1: UUID failed, want: %s", clsid.String())
	}
	if !bytes.Equal(bs, bs0) {
		t.Errorf("Test_objects_1: %v, want: %v", bs0, bs)
	}
	if q.GetSize() != 0 {
		t.Errorf("Test_objects_1 buffer size: %d, want: 0", q.GetSize())
	}
}

func Test_objects(t *testing.T) {
	q := gspa.MakeBuffer()
	var c int8 = 't'
	var b byte = 0xD4
	var ok bool = true
	var s int16 = 12345
	var us uint16 = 42345
	var n int32 = 10
	var un uint32 = 4023456784
	var n64 int64 = 9999999988888
	var un64 uint64 = 9999999988888777
	var dt time.Time = time.Now()
	var str string = "乌克兰公民是真正的受害者！"
	var f float32 = 29.876
	var d float64 = 87654.2756
	q.SaveObjects(c, b, str, ok, s, us, n, un, n64, un64, dt, f, d)
	c0 := q.LoadObject()
	b0 := q.LoadObject()
	str0 := q.LoadObject()
	ok0 := q.LoadObject()
	s0 := q.LoadObject()
	us0 := q.LoadObject()
	n0 := q.LoadObject()
	un0 := q.LoadObject()
	n640 := q.LoadObject()
	un640 := q.LoadObject()
	dt0 := q.LoadObject()
	f0 := q.LoadObject()
	d0 := q.LoadObject()
	if c != c0 {
		t.Errorf("Test_objects c0: %d, want: %d", c0, c)
	}
	if b != b0 {
		t.Errorf("Test_objects b0: %d, want: %d", b0, b)
	}
	if str != str0 {
		t.Errorf("Test_objects b0: %s, want: %s", str0, str)
	}
	if ok != ok0 {
		t.Errorf("Test_objects b0: %t, want: %t", ok0, ok)
	}
	if s != s0 {
		t.Errorf("Test_objects s0: %d, want: %d", s0, s)
	}
	if us != us0 {
		t.Errorf("Test_objects us0: %d, want: %d", us0, us)
	}
	if n != n0 {
		t.Errorf("Test_objects n0: %d, want: %d", n0, n)
	}
	if un != un0 {
		t.Errorf("Test_objects un0: %d, want: %d", un0, un)
	}
	if n64 != n640 {
		t.Errorf("Test_objects n640: %d, want: %d", n640, n64)
	}
	if un64 != un640 {
		t.Errorf("Test_objects un640: %d, want: %d", un640, un64)
	}
	ndt := dt.UnixNano()
	ndt0 := dt0.(time.Time).UnixNano()
	if ndt != ndt0 {
		t.Errorf("Test_objects ndt0: %d, want: %d", ndt0, ndt)
	}
	if f != f0 {
		t.Errorf("Test_objects f0: %f, want: %f", f0, f)
	}
	if d != d0 {
		t.Errorf("Test_objects d0: %f, want: %f", d0, d)
	}
	if q.GetSize() != 0 {
		t.Errorf("Test_objects buffer size: %d, want: 0", q.GetSize())
	}
}

func Test_queue_getbytes(t *testing.T) {
	var offset uint32
	var was string = "李进进先生于3月14日在纽约不幸遇害CN helps you with your domain transfer! If you are conducting your transaction through the 4.CN Marketplace, we offer our escrow services as part of our service"
	q := gspa.MakeBuffer().Save(was)
	count := q.GetSize()
	for offset < count {
		bytes := q.GetBytes(offset, 100)
		bs := uint32(len(bytes))
		offset += bs
	}
}

func Test_bytes(t *testing.T) {
	arr := []bool{true, false}
	q := gspa.MakeBuffer().Save("MyTest")
	q.SaveBoolArray(arr)
	s := q.LoadString()
	aout := q.LoadBoolArray()
	if *s != "MyTest" {
		t.Errorf("Test string: %s, want: %s", *s, "MyTest")
	}
	if !reflect.DeepEqual(arr, aout) {
		t.Errorf("Test array: %t, want: %t", aout, arr)
	}
}
