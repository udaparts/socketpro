import sys, struct, uuid, types
from decimal import Decimal
from decimal import localcontext
from spa import tagOperationSystem, tagVariantDataType, IUSerializer
import datetime
import threading
from collections import deque

class UDateTime(object):
    def __init__(self, dt):
        if isinstance(dt, (datetime.datetime, datetime.date, datetime.time)):
            self.Set(dt)
        elif isinstance(dt, int):
            self.time = dt
        else:
            self.time = dt

    def Set(self, dt):
        time = 0
        year = 0
        month = 0
        day = 0
        hour = 0
        minute = 0
        second = 0
        ns100 = 0
        if isinstance(dt, datetime.datetime):
            year = dt.year
            month = dt.month
            day = dt.day
            hour = dt.hour
            minute = dt.minute
            second = dt.second
            ns100 = dt.microsecond * 10
        elif isinstance(dt, datetime.date):
            year = dt.year
            month = dt.month
            day = dt.day
        elif isinstance(dt, datetime.time):
            hour = dt.hour
            minute = dt.minute
            second = dt.second
            ns100 = dt.microsecond * 10
        if day:
            if year >= 1900:
                time = year - 1900
            else:
                time = 1 # negative, before 1900-01-01
                time <<= 13 # year 13bits
                time += 1900 - year
            time <<= 4 # month 4bits
            time += month - 1
            time <<= 5 # day 5bits
            time += day
        time <<= 5 # hour 5bits
        time += hour
        time <<= 6 # minute 6bits
        time += minute
        time <<= 6  # second 6bits
        time += second
        time <<= 24 # ns100 24bits
        time += ns100
        self.time = time

    def Get(self):
        time = self.time
        ns100 = (time & 0xffffff) # 24bits

        time >>= 24
        second = time & 0x3f # 6bits
        time >>= 6
        minute = time & 0x3f # 6bits
        time >>= 6
        hour = time & 0x1f # 5bits
        time >>= 5
        day = time & 0x1f # 5bits
        time >>= 5

        # 0 - 11 instead of 1 - 12
        month = time & 0xf # 4bits
        time >>= 4

        # 8191 == 0x1fff, From BC 6291 (8191 - 1900) to AD 9991 (1900 + 8191)
        year = time & 0x1fff # 13bits
        time >>= 13

        # It will be 1 if date time is earlier than 1900-01-01
        neg = time
        if neg:
            year = 1900 - year
        else:
            year += 1900
        month += 1
        us = int(ns100 / 10)
        if day:
            if hour or minute or second or us:
                return datetime.datetime(year, month, day, hour, minute, second, us)
            else:
                return datetime.date(year, month, day)
        return datetime.time(hour, minute, second, us)

class CUQueue(object):
    DEFAULT_OS = 0
    TIME_ZONE = int((datetime.datetime.now() - datetime.datetime.utcnow()).total_seconds())
    _name_ = sys.platform.lower()
    if _name_ == 'win32':
        pass
    elif _name_.index('linux') != -1:
        DEFAULT_OS = tagOperationSystem.osUnix
    elif _name_.index('darwin') != -1:
        DEFAULT_OS = tagOperationSystem.osApple
    del _name_

    def __init__(self, bytes=None, capacity=1024*4, blockSize=1024*4):
        if capacity <= 0:
            capacity = 1024
        if bytes is None:
            self._m_len_ = 0
            self._m_bytes_ = bytearray(capacity)
        else:
            self._m_bytes_ = bytes
            self._m_len_ = len(bytes)
        if blockSize <= 0:
            blockSize = 1024
        self._m_blockSize_ = blockSize
        self._m_position_ = 0
        self._m_os_ = CUQueue.DEFAULT_OS

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.Empty()

    def __del__(self):
        self.Empty()

    def Empty(self):
        self._m_position_ = 0
        self._m_len_ = 0
        self._m_bytes_ = bytearray(0)

    def Swap(self, q):
        my_temp = self._m_len_
        self._m_len_ = q._m_len_
        q._m_len_ = my_temp

        my_temp = self._m_bytes_
        self._m_bytes_ = q._m_bytes_
        q._m_bytes_ = my_temp

        my_temp = self._m_blockSize_
        self._m_blockSize_ = q._m_blockSize_
        q._m_blockSize_ = my_temp

        my_temp = self._m_position_
        self._m_position_ = q._m_position_
        q._m_position_ = my_temp

        my_temp = self._m_os_
        self._m_os_ = q._m_os_
        q._m_os_ = my_temp

    @property
    def OS(self):
        return self._m_os_

    @OS.setter
    def OS(self, value):
        self._m_os_ = value

    def SetSize(self, size):
        if size == 0:
            self._m_position_ = 0
            self._m_len_ = 0
            return
        if size < 0 or size > (len(self._m_bytes_) - self._m_position_):
            raise ValueError("Bad new size")
        self._m_len_ = size

    def GetBuffer(self, offset=0):
        if offset > self._m_len_:
            offset = self._m_len_
        return self._m_bytes_[offset + self._m_position_:self._m_position_+self._m_len_]

    @staticmethod
    def _getSSInt96_(dec):
        dec_tuple = dec.as_tuple()
        digits = dec_tuple[1]
        allLen = len(digits)
        if allLen > 28:
            with localcontext() as ctx:
                ctx.prec = 28
                dec = dec/1
                dec_tuple = dec.as_tuple()
                digits = dec_tuple[1]
                allLen = len(digits)
        sign = 0
        if dec.is_signed():
            sign = 0x80
        scale = dec_tuple[2]
        if scale < 0:
            scale = -scale
        else:
            scale = 0
        s = str(dec)
        if sign:
            s = s[1:]
        if scale:
            pot = s.index('.')
            s = s[:pot] + s[pot+1:]
        from spa import isVersion3
        if isVersion3:
            return (sign, scale, int(s))
        else:
            return (sign, scale, long(s))

    def SaveDecimal(self, dec):
        ssi = CUQueue._getSSInt96_(dec)
        self.SaveUShort(14) #wReserved
        self.SaveByte(ssi[1]) #scale
        self.SaveByte(ssi[0]) #sign
        high = ssi[2] >> 64
        low = ssi[2] % 0x10000000000000000
        self.SaveUInt(high) #Hi32
        self.SaveULong(low) #lo64
        return self

    def LoadDecimal(self):
        wReserved = self.LoadUShort()
        scale = self.LoadByte()
        sign = self.LoadByte()
        hi32 = self.LoadUInt()
        lo64 = self.LoadULong()
        largeInt = 0
        from spa import isVersion3
        if isVersion3:
            largeInt = int(hi32)
        else:
            largeInt = long(hi32)
        largeInt <<= 64
        largeInt += lo64
        if sign != 0:
            largeInt = -largeInt
        largeInt = str(largeInt)
        if scale != 0:
            pos = len(largeInt) - scale
            largeInt = largeInt[:pos] + '.' + largeInt[pos:]
        return Decimal(largeInt)

    def Load(self, obj):
        if not isinstance(obj, IUSerializer):
            raise ValueError('Must pass in a valid object implemented with the interface IUSerializer')
        obj.LoadFrom(self)
        return obj

    def Save(self, obj):
        if not isinstance(obj, IUSerializer):
            raise ValueError('Must pass in a valid object implemented with the interface IUSerializer')
        obj.SaveTo(self)
        return self

    @property
    def Size(self):
        return self._m_len_

    @Size.setter
    def Size(self, value):
        self.SetSize(value)

    def GetSize(self):
        return self._m_len_

    @property
    def HeadPosition(self):
        return self._m_position_

    @property
    def MaxBufferSize(self):
        return len(self._m_bytes_)

    @property
    def IntenalBuffer(self):
        return self._m_bytes_

    @property
    def TailSize(self):
        return len(self._m_bytes_) - self._m_len_ - self._m_position_

    def Realloc(self, newSize):
        if newSize <= 0:
            newSize = 1024
        bytes = bytearray(newSize)
        if self._m_len_ > 0:
            if self._m_len_ > newSize:
                self._m_len_ = newSize
            bytes[0:self._m_len_] = self._m_bytes_[self._m_position_:self._m_position_+self._m_len_]
        self._m_position_ = 0
        self._m_bytes_ = bytes

    def _ensure_(self, len):
        ts = self.TailSize
        if ts < len:
            addedSize = int(((len - ts) / self._m_blockSize_ + 1) * self._m_blockSize_)
            self.Realloc(self.MaxBufferSize + addedSize)

    def _available_(self, size):
        if size > self._m_len_:
            self._m_len_ = 0
            self._m_position_ = 0
            raise ValueError('Invalid data found')

    def _set_(self, size):
        self._m_len_ -= size
        if self._m_len_ == 0:
            self._m_position_ = 0
        else:
            self._m_position_ += size

    def SaveInt(self, n):
        self._ensure_(4)
        struct.pack_into('i', self._m_bytes_, self._m_position_ + self._m_len_, n)
        self._m_len_ += 4
        return self

    def LoadInt(self):
        self._available_(4)
        n = struct.unpack_from('i', self._m_bytes_, self._m_position_)
        self._set_(4)
        return n[0]

    def PeakUInt(self, pos):
        self._available_(pos + 4)
        n = struct.unpack_from('I', self._m_bytes_, self._m_position_ + pos)
        return n[0]

    def ResetUInt(self, n, pos):
        self._available_(pos + 4)
        struct.pack_into('I', self._m_bytes_, self._m_position_ + pos, n)
        return self

    def SaveUInt(self, n):
        self._ensure_(4)
        struct.pack_into('I', self._m_bytes_, self._m_position_ + self._m_len_, n)
        self._m_len_ += 4
        return self

    def LoadUInt(self):
        self._available_(4)
        n = struct.unpack_from('I', self._m_bytes_, self._m_position_)
        self._set_(4)
        return n[0]

    def Discard(self, len):
        if len > self._m_len_:
            len = self._m_len_
        self._m_len_ -= len
        if self._m_len_ == 0:
            self._m_position_ = 0
        else:
            self._m_position_ += len
        return len

    def SaveShort(self, s):
        self._ensure_(2)
        struct.pack_into('h', self._m_bytes_, self._m_position_ + self._m_len_, s)
        self._m_len_ += 2
        return self

    def LoadShort(self):
        self._available_(2)
        n = struct.unpack_from('h', self._m_bytes_, self._m_position_)
        self._set_(2)
        return n[0]

    def SaveUShort(self, s):
        self._ensure_(2)
        struct.pack_into('H', self._m_bytes_, self._m_position_ + self._m_len_, s)
        self._m_len_ += 2
        return self

    def LoadUShort(self):
        self._available_(2)
        n = struct.unpack_from('H', self._m_bytes_, self._m_position_)
        self._set_(2)
        return n[0]

    def SaveByte(self, b):
        self._ensure_(1)
        struct.pack_into('B', self._m_bytes_, self._m_position_ + self._m_len_, b)
        self._m_len_ += 1
        return self

    def LoadByte(self):
        self._available_(1)
        n = struct.unpack_from('B', self._m_bytes_, self._m_position_)
        self._set_(1)
        return n[0]

    def SaveAChar(self, c):
        self._ensure_(1)
        struct.pack_into('b', self._m_bytes_, self._m_position_ + self._m_len_, c)
        self._m_len_ += 1
        return self

    def LoadAChar(self):
        self._available_(1)
        n = struct.unpack_from('b', self._m_bytes_, self._m_position_)
        self._set_(1)
        return n[0]

    def SaveChar(self, c):
        return self.SaveUShort(ord(c))

    def LoadChar(self):
        return unichr(self.LoadUShort())

    def SaveLong(self, n):
        self._ensure_(8)
        struct.pack_into('q', self._m_bytes_, self._m_position_ + self._m_len_, n)
        self._m_len_ += 8
        return self

    def LoadLong(self):
        self._available_(8)
        n = struct.unpack_from('q', self._m_bytes_, self._m_position_)
        self._set_(8)
        return n[0]

    def SaveULong(self, n):
        self._ensure_(8)
        struct.pack_into('Q', self._m_bytes_, self._m_position_ + self._m_len_, n)
        self._m_len_ += 8
        return self

    def LoadULong(self):
        self._available_(8)
        n = struct.unpack_from('Q', self._m_bytes_, self._m_position_)
        self._set_(8)
        return n[0]

    def SaveFloat(self, f):
        self._ensure_(4)
        struct.pack_into('f', self._m_bytes_, self._m_position_ + self._m_len_, f)
        self._m_len_ += 4
        return self

    def LoadFloat(self):
        self._available_(4)
        n = struct.unpack_from('f', self._m_bytes_, self._m_position_)
        self._set_(4)
        return n[0]

    def SaveDouble(self, d):
        self._ensure_(8)
        struct.pack_into('d', self._m_bytes_, self._m_position_ + self._m_len_, d)
        self._m_len_ += 8
        return self

    def LoadDouble(self):
        self._available_(8)
        n = struct.unpack_from('d', self._m_bytes_, self._m_position_)
        self._set_(8)
        return n[0]

    def SaveBool(self, b):
        self._ensure_(1)
        struct.pack_into('?', self._m_bytes_, self._m_position_ + self._m_len_, b)
        self._m_len_ += 1
        return self

    def LoadBool(self):
        self._available_(1)
        n = struct.unpack_from('?', self._m_bytes_, self._m_position_)
        self._set_(1)
        return n[0]

    def SaveAString(self, s):
        if s is None:
            self.SaveInt(-1)
        else:
            s = s.encode('utf-8')
            length = len(s)
            fmt = str(length).join(('I', 's'))
            self._ensure_(length + 4)
            pos = self._m_position_ + self._m_len_
            struct.pack_into(fmt, self._m_bytes_, pos, length, s)
            self._m_len_ += (length + 4)
        return self

    def LoadAString(self):
        length = self.LoadUInt()
        if length == -1 or length == 0xffffffff:
            return None
        elif length == 0:
            return ''
        self._available_(length)
        s = self._m_bytes_[self._m_position_:self._m_position_+length]
        self._set_(length)
        return s.decode('utf-8')

    def SaveString(self, s):
        if s is None:
            self.SaveInt(-1)
        else:
            s = s.encode('utf-16-le')
            length = len(s)
            fmt = str(length).join(('I', 's'))
            self._ensure_(length + 4)
            struct.pack_into(fmt, self._m_bytes_, self._m_position_ + self._m_len_, length, s)
            self._m_len_ += (length + 4)
        return self

    def LoadString(self):
        length = self.LoadUInt()
        if length == -1 or length == 0xffffffff:
            return None
        elif length == 0:
            return u''
        self._available_(length)
        bytes = self._m_bytes_[self._m_position_:self._m_position_+length]
        self._set_(length)
        return bytes.decode('utf-16-le')

    def SaveDate(self, dt):
        udt = UDateTime(dt)
        ut = udt.time
        return self.SaveULong(ut)

    def LoadDate(self):
        return UDateTime(self.LoadULong()).Get()

    def Push(self, bytes, length):
        if bytes is None or length is None or length == 0:
            return
        if  length > len(bytes):
            length = len(bytes)
        self._ensure_(length)
        #fmt = str(length) + 's'
        #struct.pack_into(fmt, self._m_bytes_, self._m_position_ + self._m_len_, bytes)
        self._m_bytes_[self._m_position_ + self._m_len_:] = bytes
        self._m_len_ += length
        return self

    def SaveBytes(self, bytes, length):
        if bytes is None:
            length = -1
            self.SaveInt(length)
            return self
        if  length > len(bytes):
            length = len(bytes)
        self.SaveInt(length)
        return self.Push(bytes, length)

    def LoadBytes(self):
        length = self.LoadUInt()
        if length == -1 or length == 0xffffffff:
            return None
        elif length == 0:
            return bytearray(0)
        self._available_(length)
        start = self._m_position_
        bytes = self._m_bytes_[start:start+length]
        self._set_(length)
        return bytes

    def PopBytes(self, length):
        if length == -1 or length == 0xffffffff:
            return None
        elif length == 0:
            return bytearray(0)
        self._available_(length)
        start = self._m_position_
        bytes = self._m_bytes_[start:start+length]
        self._set_(length)
        return bytes

    def SaveUUID(self, uuid):
        return self.Push(uuid.bytes_le, 16)

    def LoadByClass(self, cls):
        obj = cls()
        obj.LoadFrom(self)
        return obj

    def LoadUUID(self):
        self._available_(16)
        bytes = struct.unpack_from('16s', self._m_bytes_, self._m_position_)
        self._set_(16)
        return uuid.UUID(bytes_le=bytes[0])

    def _saveList_(self, lst, hint):
        from spa import isVersion3
        length = len(lst)
        if all(isinstance(x,bool) for x in lst):
            s = None
            self.SaveUShort(tagVariantDataType.sdVT_BOOL|tagVariantDataType.sdVT_ARRAY)
            self.SaveUInt(length)
            for data in lst:
                if data:
                    s = -1
                else:
                    s = 0
                self.SaveShort(s)
        elif all(isinstance(x,int) for x in lst) and not any(isinstance(x,bool) for x in lst):
            if hint == 'h': #short
                self.SaveUShort(tagVariantDataType.sdVT_I2|tagVariantDataType.sdVT_ARRAY) #VT_I2
                self.SaveUInt(length)
                for data in lst:
                    self.SaveShort(data)
            elif hint == 'H': #ushort
                self.SaveUShort(tagVariantDataType.sdVT_UI2|tagVariantDataType.sdVT_ARRAY) #VT_UI2
                self.SaveUInt(length)
                for data in lst:
                    self.SaveUShort(data)
            elif hint == 'I' or hint == 'L': #uint
                self.SaveUShort(tagVariantDataType.sdVT_UINT|tagVariantDataType.sdVT_ARRAY) #VT_UINT
                self.SaveUInt(length)
                for data in lst:
                    self.SaveUInt(data)
            elif hint == 'b': #int8
                self.SaveUShort(tagVariantDataType.sdVT_I1|tagVariantDataType.sdVT_ARRAY) #VT_I1
                self.SaveAString(lst)
            elif hint == 'B': #uint8
                self.SaveUShort(tagVariantDataType.sdVT_UI1|tagVariantDataType.sdVT_ARRAY) #VT_UI1
                self.SaveBytes(lst)
            elif hint == 'Q': #uint64
                self.SaveUShort(tagVariantDataType.sdVT_UI8|tagVariantDataType.sdVT_ARRAY) #VT_UI8
                self.SaveUInt(length)
                for data in lst:
                    self.SaveULong(data)
            elif hint == 'q': #int64
                self.SaveUShort(tagVariantDataType.sdVT_I8|tagVariantDataType.sdVT_ARRAY) #VT_I8
                self.SaveUInt(length)
                for data in lst:
                    self.SaveLong(data)
            else: #int or long
                self.SaveUShort(tagVariantDataType.sdVT_INT|tagVariantDataType.sdVT_ARRAY) #VT_INT
                self.SaveUInt(length)
                for data in lst:
                    self.SaveInt(data)
        elif not isVersion3 and all(isinstance(x,long) for x in lst):
            if hint == 'Q': #uint64
                self.SaveUShort(tagVariantDataType.sdVT_UI8|tagVariantDataType.sdVT_ARRAY) #VT_UI8
                self.SaveUInt(length)
                for data in lst:
                    self.SaveULong(data)
            else: #int64
                self.SaveUShort(tagVariantDataType.sdVT_I8|tagVariantDataType.sdVT_ARRAY) #VT_I8
                self.SaveUInt(length)
                for data in lst:
                    self.SaveLong(data)
        elif all(isinstance(x,float) for x in lst):
            if hint == 'time':
                self.SaveUShort(tagVariantDataType.sdVT_DATE|tagVariantDataType.sdVT_ARRAY) #VT_DATE
                self.SaveUInt(length)
                for data in lst:
                    self.SaveDate(data)
            elif hint == 'f':
                self.SaveUShort(tagVariantDataType.sdVT_R4|tagVariantDataType.sdVT_ARRAY) #VT_R4
                self.SaveUInt(length)
                for data in lst:
                    self.SaveFloat(data)
            else:
                self.SaveUShort(tagVariantDataType.sdVT_R8|tagVariantDataType.sdVT_ARRAY) #VT_R8
                self.SaveUInt(length)
                for data in lst:
                    self.SaveDouble(data)
        elif all(isinstance(x,uuid.UUID) for x in lst):
            self.SaveUShort(tagVariantDataType.sdVT_CLSID|tagVariantDataType.sdVT_ARRAY) #VT_CLSID
            self.SaveUInt(length)
            for data in lst:
                self.SaveUUID(data)
        elif all(isinstance(x,Decimal) for x in lst):
            self.SaveUShort(tagVariantDataType.sdVT_DECIMAL|tagVariantDataType.sdVT_ARRAY) #VT_DECIMAL
            self.SaveUInt(length)
            for data in lst:
                self.SaveDecimal(data)
        elif all(isinstance(x,IUSerializer) for x in lst):
            self.SaveUShort(tagVariantDataType.sdVT_USERIALIZER_OBJECT|tagVariantDataType.sdVT_ARRAY) #VT_USERIALIZER_OBJECT
            self.SaveUInt(length)
            for data in lst:
                data.SaveTo(self)
        else:
            self.SaveUShort(tagVariantDataType.sdVT_VARIANT|tagVariantDataType.sdVT_ARRAY) #VT_VARIANT
            self.SaveUInt(length)
            for data in lst:
                self.SaveObject(data)
        return self

    def SaveObject(self, obj, hint=''):
        from spa import isVersion3
        if obj is None:
            return self.SaveUShort(tagVariantDataType.sdVT_NULL)
        if isinstance(obj, str):
            if hint == 'c':
                self.SaveUShort(tagVariantDataType.sdVT_I1|tagVariantDataType.sdVT_ARRAY)
                return self.SaveAString(obj)
            else:
                self.SaveUShort(tagVariantDataType.sdVT_BSTR)
                return self.SaveString(obj)
        elif not isVersion3 and isinstance(obj, unicode):
            self.SaveUShort(tagVariantDataType.sdVT_BSTR)
            return self.SaveString(obj)
        elif isinstance(obj, datetime.datetime):
            self.SaveUShort(tagVariantDataType.sdVT_DATE) #VT_DATE
            return self.SaveDate(obj)
        elif isinstance(obj, bytearray):
            self.SaveUShort(tagVariantDataType.sdVT_UI1|tagVariantDataType.sdVT_ARRAY) #byte array
            self.SaveUInt(len(obj))
            return self.Push(obj, len(obj))
        elif isinstance(obj, list):
            return self._saveList_(obj, hint)
        elif isinstance(obj, tuple):
            return self._saveList_(list(obj), hint)
        elif isinstance(obj, bool):
            self.SaveUShort(tagVariantDataType.sdVT_BOOL) #VT_BOOL
            if obj:
                return self.SaveUShort(0xffff)
            else:
                return self.SaveUShort(0)
        elif isinstance(obj, int):
            if hint == 'h': #short
                self.SaveUShort(tagVariantDataType.sdVT_I2) #VT_I2
                self.SaveShort(obj)
            elif hint == 'H': #ushort
                self.SaveUShort(tagVariantDataType.sdVT_UI2) #VT_UI2
                self.SaveUShort(obj)
            elif hint == 'I' or hint == 'L': #uint
                self.SaveUShort(tagVariantDataType.sdVT_UINT) #VT_UINT
                self.SaveUInt(obj)
            elif hint == 'b': #int8
                self.SaveUShort(tagVariantDataType.sdVT_I1) #VT_I1
                self.SaveAChar(obj)
            elif hint == 'B': #uint8
                self.SaveUShort(tagVariantDataType.sdVT_UI1) #VT_UI1
                self.SaveByte(obj)
            elif hint == 'Q': #uint64
                self.SaveUShort(tagVariantDataType.sdVT_UI8) #VT_UI8
                self.SaveULong(obj)
            elif hint == 'q': #int64
                self.SaveUShort(tagVariantDataType.sdVT_I8) #VT_I8
                self.SaveLong(obj)
            else: #int or long
                self.SaveUShort(tagVariantDataType.sdVT_INT) #VT_INT
                self.SaveInt(obj)
        elif isinstance(obj, float):
            if hint == 'f':
                self.SaveUShort(tagVariantDataType.sdVT_R4) #VT_R4
                self.SaveFloat(obj)
            else:
                self.SaveUShort(tagVariantDataType.sdVT_R8) #VT_R8
                self.SaveDouble(obj)
        else:
            if isinstance(obj, uuid.UUID):
                self.SaveUShort(tagVariantDataType.sdVT_CLSID) #VT_CLSID
                self.SaveUUID(obj)
            elif isinstance(obj, Decimal):
                self.SaveUShort(tagVariantDataType.sdVT_DECIMAL) #VT_DECIMAL
                self.SaveDecimal(obj)
            elif isinstance(obj, IUSerializer):
                self.SaveUShort(tagVariantDataType.sdVT_USERIALIZER_OBJECT) #VT_USERIALIZER_OBJECT
                obj.SaveTo(self)
            elif type(obj) is object:
                return self.SaveShort(tagVariantDataType.sdVT_EMPTY)
            else:
                raise NotImplementedError('Unsupported data type for saving')
        return self

    def _loadList_(self, vt, func):
        arr = []
        if vt == tagVariantDataType.sdVT_I1: #signed char
            return self.LoadAString()
        elif vt == tagVariantDataType.sdVT_UI1:
            return self.LoadBytes()
        else:
            length = self.LoadUInt()
            while length > 0:
                if vt == tagVariantDataType.sdVT_I4 or vt == tagVariantDataType.sdVT_INT: #long or int
                    arr.append(self.LoadInt())
                elif vt == tagVariantDataType.sdVT_UI4 or vt == tagVariantDataType.sdVT_UINT: #unsigned long or uint
                    arr.append(self.LoadUInt())
                elif vt == tagVariantDataType.sdVT_I8: #int64
                    arr.append(self.LoadLong())
                elif vt == tagVariantDataType.sdVT_UI8: #uint64
                    arr.append(self.LoadULong())
                elif vt == tagVariantDataType.sdVT_R4: #float
                    arr.append(self.LoadFloat())
                elif vt == tagVariantDataType.sdVT_R8: #double
                    arr.append(self.LoadDouble())
                elif vt == tagVariantDataType.sdVT_CY: #CY
                    arr.append(float(self.LoadULong())/10000.0)
                elif vt == tagVariantDataType.sdVT_DATE: #Date
                    arr.append(self.LoadDate())
                elif vt == tagVariantDataType.sdVT_BSTR: #BSTR
                    arr.append(self.LoadString())
                elif vt == tagVariantDataType.sdVT_BOOL: #VARIANT_BOOL
                    arr.append(self.LoadUShort() != 0)
                elif vt == tagVariantDataType.sdVT_DECIMAL: #VT_DECIMAL
                    arr.append(self.LoadDecimal())
                elif vt == tagVariantDataType.sdVT_I2: #short
                    arr.append(self.LoadShort())
                elif vt == tagVariantDataType.sdVT_UI2: #ushort
                    arr.append(self.LoadUShort())
                elif vt == tagVariantDataType.sdVT_CLSID:
                    arr.append(self.LoadUUID())
                elif vt == tagVariantDataType.sdVT_USERIALIZER_OBJECT: #USERIALIZER_OBJECT
                    obj = func()
                    obj.LoadFrom(self)
                    arr.append(obj)
                elif vt == tagVariantDataType.sdVT_VARIANT: #VARIANT
                    arr.append(self.LoadObject(func))
                length -= 1
        return arr

    def LoadObject(self, func=None):
        vt = self.LoadUShort()
        if vt == tagVariantDataType.sdVT_EMPTY:
            return object() #VT_EMPTY
        elif vt == tagVariantDataType.sdVT_NULL:
            return None #VT_NULL
        isList = False
        if vt > tagVariantDataType.sdVT_ARRAY:
            isList = True
            vt -= tagVariantDataType.sdVT_ARRAY
        if isList:
            return self._loadList_(vt, func)
        else:
            if vt == tagVariantDataType.sdVT_I4 or vt == tagVariantDataType.sdVT_INT: #long or int
                return self.LoadInt()
            elif vt == tagVariantDataType.sdVT_UI4 or vt == tagVariantDataType.sdVT_UINT: #unsigned long or uint
                return self.LoadUInt()
            elif vt == tagVariantDataType.sdVT_I8: #int64
                return self.LoadLong()
            elif vt == tagVariantDataType.sdVT_UI8: #uint64
                return self.LoadULong()
            elif vt == tagVariantDataType.sdVT_R4: #float
                return self.LoadFloat()
            elif vt == tagVariantDataType.sdVT_R8: #double
                return self.LoadDouble()
            elif vt == tagVariantDataType.sdVT_CY: #CY
                return float(self.LoadULong())/10000.0
            elif vt == tagVariantDataType.sdVT_DATE: #Date
                return self.LoadDate()
            elif vt == tagVariantDataType.sdVT_BSTR: #BSTR
                return self.LoadString()
            elif vt == tagVariantDataType.sdVT_BOOL: #VARIANT_BOOL
                b = self.LoadUShort()
                return b != 0
            elif vt == tagVariantDataType.sdVT_DECIMAL: #Decimal
                return self.LoadDecimal()
            elif vt == tagVariantDataType.sdVT_I1: #signed char
                return self.LoadAChar()
            elif vt == tagVariantDataType.sdVT_UI1: #unsigned char
                return self.LoadByte()
            elif vt == tagVariantDataType.sdVT_DATE: #Date
                return self.LoadDate()
            elif vt == tagVariantDataType.sdVT_CLSID: #VT_CLSID
                return self.LoadUUID()
            elif vt == tagVariantDataType.sdVT_I2: #short
                return self.LoadShort()
            elif vt == tagVariantDataType.sdVT_UI2: #ushort
                return self.LoadUShort()
            elif vt == tagVariantDataType.sdVT_USERIALIZER_OBJECT: #USERIALIZER_OBJECT
                obj = func()
                obj.LoadFrom(self)
                return obj
        raise NotImplementedError('Unsupported data type for loading')

class CScopeUQueue:
    _cs = threading.Lock()
    _vQueue = deque()
    SHARED_BUFFER_CLEAN_SIZE = 32 * 1024

    def __init__(self):
        self._Buffer_ = CScopeUQueue.Lock()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.Dispose()

    def __del__(self):
        self.Dispose()

    def Dispose(self):
        if self._Buffer_:
            CScopeUQueue.Unlock(self._Buffer_)
            self._Buffer_ = None

    @property
    def UQueue(self):
        return self._Buffer_

    @property
    def Size(self):
        if self._Buffer_:
            return self._Buffer_.GetSize()
        return 0

    def GetSize(self):
        if self._Buffer_:
            return self._Buffer_.GetSize()
        return 0

    @Size.setter
    def Size(self, value):
        if self._Buffer_:
            self._Buffer_.SetSize(value)

    def SaveDecimal(self, dec):
        self._Buffer_.SaveDecimal(dec)
        return self

    def LoadDecimal(self):
        return self._Buffer_.LoadDecimal()

    def Save(self, obj):
        self._Buffer_.Save(obj)
        return self

    def Load(self, obj):
        return self._Buffer_.Load(obj)

    def LoadByClass(self, cls):
        return self._Buffer_.LoadByClass(cls)

    def SaveInt(self, n):
        self._Buffer_.SaveInt(n)
        return self

    def LoadInt(self):
        return self._Buffer_.LoadInt()

    def SaveUInt(self, n):
        self._Buffer_.SaveUInt(n)
        return self

    def LoadUInt(self):
        return self._Buffer_.LoadUInt()

    def SaveShort(self, s):
        self._Buffer_.SaveShort(s)
        return self

    def LoadShort(self):
        return self._Buffer_.LoadShort()

    def SaveUShort(self, s):
        self._Buffer_.SaveUShort(s)
        return self

    def LoadUShort(self):
        return self._Buffer_.LoadUShort()

    def SaveChar(self, c):
        self._Buffer_.SaveChar(c)
        return self

    def LoadChar(self):
        return self._Buffer_.LoadChar()

    def SaveAChar(self, c):
        self._Buffer_.SaveAChar(c)
        return self

    def LoadAChar(self):
        return self._Buffer_.LoadAChar()

    def SaveLong(self, n):
        self._Buffer_.SaveLong(n)
        return self

    def LoadLong(self):
        return self._Buffer_.LoadLong()

    def SaveULong(self, n):
        self._Buffer_.SaveULong(n)
        return self

    def LoadULong(self):
        return self._Buffer_.LoadULong()

    def SaveFloat(self, f):
        self._Buffer_.SaveFloat(f)
        return self

    def LoadFloat(self):
        return self._Buffer_.LoadFloat()

    def SaveDouble(self, d):
        self._Buffer_.SaveDouble(d)
        return self

    def LoadDouble(self):
        return self._Buffer_.LoadDouble()

    def SaveBool(self, b):
        self._Buffer_.SaveBool(b)
        return self

    def LoadBool(self):
        return self._Buffer_.LoadBool()

    def SaveAString(self, s):
        self._Buffer_.SaveAString(s)
        return self

    def LoadAString(self):
        return self._Buffer_.LoadAString()

    def SaveString(self, s):
        self._Buffer_.SaveString(s)
        return self

    def LoadString(self):
        return self._Buffer_.LoadString()

    def SaveDate(self, dt):
        self._Buffer_.SaveDate(dt)
        return self

    def LoadDate(self):
        return self._Buffer_.LoadDate()

    def SaveBytes(self, bytes, length):
        self._Buffer_.SaveBytes(bytes, length)
        return self

    def LoadBytes(self):
        return self._Buffer_.LoadBytes()

    def SaveUUID(self, uuid):
        self._Buffer_.SaveUUID(uuid)
        return self

    def LoadUUID(self):
        return self._Buffer_.LoadUUID()

    def SaveObject(self, obj, hint=''):
        self._Buffer_.SaveObject(obj, hint)
        return self

    def LoadObject(self):
        return self._Buffer_.LoadObject()

    def SaveByte(self, b):
        self._Buffer_.SaveByte(b)
        return self

    def LoadByte(self):
        return self._Buffer_.LoadByte()

    @staticmethod
    def MemoryConsumed():
        mem = 0;
        with CScopeUQueue._cs:
            for q in CScopeUQueue._vQueue:
                mem += q.MaxBufferSize
        return mem

    @staticmethod
    def Lock(os = CUQueue.DEFAULT_OS):
        q = None
        with CScopeUQueue._cs:
            if len(CScopeUQueue._vQueue) > 0:
                q = CScopeUQueue._vQueue.pop()
        if q is None:
            q = CUQueue()
        q.OS = os
        return q

    @staticmethod
    def Unlock(q):
        if q is None:
            return
        q.SetSize(0)
        with CScopeUQueue._cs:
            CScopeUQueue._vQueue.append(q)

    @staticmethod
    def DestroyUQueuePool():
        with CScopeUQueue._cs:
            for q in CScopeUQueue._vQueue:
                q.Empty()
            CScopeUQueue._vQueue = deque()
