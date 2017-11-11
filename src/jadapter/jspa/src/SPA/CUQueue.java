package SPA;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

public final class CUQueue {

    public static final tagOperationSystem DEFAULT_OS;

    private static int DEFAULT_CAPACITY = 4 * 1024;
    private static int DEFAULT_BLOCK_SIZE = 4 * 1024;

    private static java.math.BigInteger UINT64_MOD;
    private static java.math.BigInteger UINT32_MOD;
    //private static java.math.BigInteger MSDEC_MAX = new java.math.BigInteger("39614081257132168796771975167");
    //private static java.math.BigInteger MSDEC_MIN = new java.math.BigInteger("-39614081257132168796771975168");

    public final void setSize(int newSize) {
        SetSize(newSize);
    }

    public final void SetSize(int size) {
        if (size == 0) {
            m_position = 0;
            m_len = 0;
            return;
        }
        if (size < 0) {
            throw new java.lang.IllegalArgumentException("New size can not be less than 0");
        }

        if (size > (m_bytes.length - m_position)) {
            throw new java.lang.UnsupportedOperationException("Bad new size");
        }
        m_len = size;
    }

    public final void Empty() {
        m_len = 0;
        m_position = 0;
    }

    public CUQueue() {
        m_blockSize = DEFAULT_BLOCK_SIZE;
        m_bytes = new byte[DEFAULT_CAPACITY];
    }

    static {
        UINT64_MOD = new java.math.BigInteger("10000000000000000", 16);
        UINT32_MOD = new java.math.BigInteger("100000000", 16);
        String os = System.getProperty("os.name");
        if (os.startsWith("WindowsCE")) {
            DEFAULT_OS = tagOperationSystem.osWinCE;
        } else if (os.startsWith("Windows")) {
            DEFAULT_OS = tagOperationSystem.osWin;
        } else if (os.startsWith("MacOS")) {
            DEFAULT_OS = tagOperationSystem.osApple;
        } else {
            DEFAULT_OS = tagOperationSystem.osUnix;
        }
    }

    private tagOperationSystem m_os = DEFAULT_OS;
    private boolean m_bEndian = false;

    public final tagOperationSystem getOS() {
        return m_os;
    }

    public final void setOS(tagOperationSystem os) {
        m_os = os;
    }

    public final boolean getEndian() {
        return m_bEndian;
    }

    public final void setEndian(boolean e) {
        m_bEndian = e;
    }

    public CUQueue(int maxSize) {
        if (maxSize <= 0) {
            maxSize = DEFAULT_CAPACITY;
        }
        m_blockSize = DEFAULT_BLOCK_SIZE;
        m_bytes = new byte[maxSize];
    }

    public CUQueue(int maxSize, int blockSize) {
        if (maxSize <= 0) {
            maxSize = DEFAULT_CAPACITY;
        }
        if (blockSize < 0) {
            blockSize = DEFAULT_BLOCK_SIZE;
        }
        m_blockSize = blockSize;
        m_bytes = new byte[maxSize];
    }

    public CUQueue(byte[] bytes) {
        m_blockSize = DEFAULT_BLOCK_SIZE;
        if (bytes == null) {
            m_bytes = new byte[0];
        } else {
            m_bytes = bytes;
            m_len = bytes.length;
        }
    }

    public final int getHeadPosition() {
        return m_position;
    }

    public final int getSize() {
        return m_len;
    }

    public final int GetSize() {
        return m_len;
    }

    public final int getTailSize() {
        return (m_bytes.length - m_position - m_len);
    }

    public final int getMaxBufferSize() {
        return m_bytes.length;
    }

    public final int Discard(int len) {
        if (len > m_len) {
            len = m_len;
        }
        m_len -= len;
        if (m_len == 0) {
            m_position = 0;
        } else {
            m_position += len;
        }
        return len;
    }

    public final CUQueue Save(Byte b) {
        return Save((byte) b);
    }

    public final CUQueue Save(Short s) {
        return Save((short) s);
    }

    public final CUQueue Save(Integer i) {
        return Save((int) i);
    }

    public final CUQueue Save(Long i) {
        return Save((long) i);
    }

    public final CUQueue Save(Float f) {
        return Save((float) f);
    }

    public final CUQueue Save(Double d) {
        return Save((double) d);
    }

    public final CUQueue Save(Boolean b) {
        return Save((boolean) b);
    }

    public final CUQueue Save(IUSerializer s) {
        s.SaveTo(this);
        return this;
    }

    public final <T extends IUSerializer> T Load(Class<T> cls) {
        T t = null;
        try {
            t = cls.newInstance();
            t.LoadFrom(this);
        } catch (InstantiationException | IllegalAccessException err) {
        }
        return t;
    }

    public final CUQueue Save(java.sql.Timestamp dt) throws IllegalArgumentException {
        if (dt == null) {
            throw new IllegalArgumentException("Timestamp dt can not be null");
        }
        short us = (short) ((dt.getNanos() / 1000) % 1000);
        UDateTime udt = new UDateTime(dt, us);
        return Save(udt.time);
    }

    public final CUQueue Save(java.util.Date dt) throws IllegalArgumentException {
        if (dt == null) {
            throw new IllegalArgumentException("Date dt can not be null");
        }
        return Save(new UDateTime(dt).time);
    }

    public final java.util.Date LoadDate() {
        long d = LoadLong();
        return new UDateTime(d).get();
    }

    public final java.sql.Timestamp LoadTimestamp() {
        long d = LoadLong();
        long us = (d & 0xfffff);
        UDateTime udt = new UDateTime(d);
        java.sql.Timestamp dt = new java.sql.Timestamp(udt.get().getTime());
        dt.setNanos((int) (us * 1000));
        return dt;
    }

    public final CUQueue Save(CUQueue q) {
        int nSize = -1;
        if (q != null) {
            nSize = q.GetSize();
            Save(nSize);
            Push(q.getIntenalBuffer(), q.getHeadPosition(), nSize);
        } else {
            Save(nSize);
        }
        return this;
    }

    public final CUQueue LoadUQueue() {
        int len = LoadInt();
        if (len == -1) {
            return null;
        }
        if (len < 0 || len > m_len) {
            throw new RuntimeException("Invalid data found");
        }
        CUQueue q = new CUQueue(len);
        q.Push(m_bytes, m_position, len);
        Discard(len);
        return q;
    }

    public final void Realloc(int newMaxSize) {
        if (newMaxSize <= 0) {
            newMaxSize = 4 * 1024;
        }
        newMaxSize = (newMaxSize / m_blockSize + 1) * m_blockSize;
        byte[] buffer = new byte[newMaxSize];
        if (buffer == null) {
            throw new java.lang.OutOfMemoryError("Can not allocate memory with " + newMaxSize + " bytes");
        }
        if (m_len > 0) {
            m_len = (m_len > newMaxSize) ? newMaxSize : m_len;
            System.arraycopy(m_bytes, m_position, buffer, 0, m_len);
        }
        m_bytes = buffer;
        m_position = 0;
    }

    public final byte[] GetBuffer(int offset) {
        if (offset < 0) {
            offset = 0;
        }
        if (offset > m_len) {
            offset = m_len;
        }
        byte[] bytes = new byte[m_len - offset];
        if (m_len > offset) {
            System.arraycopy(m_bytes, (int) (m_position + offset), bytes, 0, (int) (m_len - offset));
        }
        return bytes;
    }

    public final byte[] GetBuffer() {
        return GetBuffer(0);
    }

    public final byte[] getIntenalBuffer() {
        return m_bytes;
    }

    public final CUQueue Push(byte[] bytes, int offset, int len) {
        if (offset < 0 || len < 0) {
            throw new IllegalArgumentException();
        }
        if (bytes == null || len == 0) {
            return this;
        }
        if (offset + len > bytes.length) {
            throw new IllegalArgumentException("Bad offset and length");
        }
        int tailSize = getTailSize();
        if (tailSize < len) {
            int addedSize = ((len - tailSize) / m_blockSize + 1) * m_blockSize;
            Realloc(getMaxBufferSize() + addedSize);
        }
        System.arraycopy(bytes, offset, m_bytes, m_position + m_len, len);
        m_len += len;
        return this;
    }

    public final CUQueue Push(byte[] bytes, int len) {
        return Push(bytes, 0, len);
    }

    public final CUQueue Push(byte[] bytes) {
        if (bytes != null) {
            Push(bytes, bytes.length);
        }
        return this;
    }

    public final CUQueue Save(int n) {
        m_b[3] = (byte) (n >>> 24);
        m_b[2] = (byte) (n >>> 16);
        m_b[1] = (byte) (n >>> 8);
        m_b[0] = (byte) n;
        return Push(m_b, 4);
    }

    public final CUQueue Save(short n) {
        m_b[1] = (byte) (n >>> 8);
        m_b[0] = (byte) n;
        return Push(m_b, 2);
    }

    public final CUQueue Save(char c) {
        return Save((short) c);
    }

    public final CUQueue Save(byte n) {
        m_b[0] = n;
        return Push(m_b, 1);
    }

    public final CUQueue Save(boolean b) {
        m_b[0] = b ? (byte) 1 : (byte) 0;
        return Push(m_b, 1);
    }

    public final CUQueue Save(long n) {
        m_b[7] = (byte) (n >> 56);
        m_b[6] = (byte) (n >> 48);
        m_b[5] = (byte) (n >> 40);
        m_b[4] = (byte) (n >> 32);
        m_b[3] = (byte) (n >> 24);
        m_b[2] = (byte) (n >> 16);
        m_b[1] = (byte) (n >> 8);
        m_b[0] = (byte) n;
        return Push(m_b, 8);
    }

    public final CUQueue Save(float f) {
        return Save(Float.floatToIntBits(f));
    }

    public final CUQueue Save(double d) {
        return Save(Double.doubleToLongBits(d));
    }

    private void Ensure(int len) {
        if (getTailSize() < len) {
            Realloc(getMaxBufferSize() + (len - getTailSize()));
        }
    }

    public final CUQueue Save(String s) {
        int len = -1;
        if (s != null) {
            len = s.length() * 2;
            Save(len);
            if (len > 0) {
                Ensure(len);
                CharBuffer cb = CharBuffer.wrap(s);
                ByteBuffer bb = ByteBuffer.wrap(m_bytes, m_position + m_len, len);
                m_UTF16.newEncoder().encode(cb, bb, true);
                m_len += len;
            }
        } else {
            Save(len);
        }
        return this;
    }

    public final CUQueue Save(byte[] bytes) {
        int len = -1;
        if (bytes != null) {
            len = bytes.length;
            Save(len);
            Push(bytes, len);
        } else {
            Save(len);
        }
        return this;
    }

    private void ChangeHeadLong() {
        byte temp = m_b[7];
        m_b[7] = m_b[6];
        m_b[6] = temp;
        temp = m_b[5];
        m_b[5] = m_b[4];
        m_b[4] = temp;
        temp = m_b[3];
        m_b[3] = m_b[0];
        m_b[0] = temp;
        temp = m_b[2];
        m_b[2] = m_b[1];
        m_b[1] = temp;
    }

    public final CUQueue Save(java.util.UUID id) throws IllegalArgumentException {
        if (id == null) {
            throw new IllegalArgumentException("UUID id can not be null");
        }
        java.nio.ByteBuffer bb = java.nio.ByteBuffer.wrap(m_b);
        bb.putLong(id.getMostSignificantBits());
        bb.putLong(id.getLeastSignificantBits());
        ChangeHeadLong();
        return Push(m_b, 16);
    }

    public final java.util.UUID LoadUUID() {
        java.nio.ByteBuffer bb = java.nio.ByteBuffer.wrap(m_b);
        ChangeHeadLong();
        long high = bb.getLong();
        long low = bb.getLong();
        return new java.util.UUID(high, low);
    }

    private byte[] Peek(int size, int pos) {
        if (size < 0 || pos < 0 || (pos + size) > m_len) {
            throw new RuntimeException("Invalid operation");
        }
        byte[] bytes = new byte[size];
        System.arraycopy(m_bytes, m_position + pos, bytes, 0, size);
        return bytes;
    }

    private void Reset(byte[] bytes, int pos) {
        if (bytes == null || bytes.length == 0) {
            return;
        }
        if (pos < 0) {
            throw new RuntimeException("Invalid operation");
        }
        if ((pos + bytes.length) > m_len) {
            throw new RuntimeException("Invalid operation");
        }
        System.arraycopy(bytes, 0, m_bytes, m_position + pos, bytes.length);
    }

    public final CUQueue ResetInt(int n, int pos) {
        byte bytes[] = {(byte) n, (byte) (n >>> 8), (byte) (n >>> 16), (byte) (n >>> 24)};
        Reset(bytes, pos);
        return this;
    }

    public final int PeekInt(int pos) {
        byte[] data = Peek(4, pos);
        return (int) ((0xff & data[3]) << 24
                | (0xff & data[2]) << 16
                | (0xff & data[1]) << 8
                | (0xff & data[0]));
    }

    private byte[] Load(int size) throws RuntimeException {
        byte[] bytes;
        if (size < 0 || size > m_len) {
            throw new RuntimeException("Invalid data found");
        }
        if (size > 16) {
            bytes = new byte[size];
        } else {
            bytes = m_b;
        }
        System.arraycopy(m_bytes, m_position, bytes, 0, size);
        m_len -= size;
        if (m_len != 0) {
            m_position += size;
        } else {
            m_position = 0;
        }
        return bytes;
    }

    public final byte[] LoadBytes() {
        //!!!!
        int size = LoadInt();
        if (size == -1) {
            return null;
        }
        byte[] bytes = new byte[size];
        System.arraycopy(m_bytes, m_position, bytes, 0, size);
        m_len -= size;
        if (m_len != 0) {
            m_position += size;
        } else {
            m_position = 0;
        }
        return bytes;
    }

    public final int PopBytes(byte[] bytes, int size) {
        //!!!!
        if (size < 0 || bytes == null) {
            return 0;
        }
        if (size > bytes.length) {
            size = bytes.length;
        }
        if (size > m_len) {
            size = m_len;
        }
        System.arraycopy(m_bytes, m_position, bytes, 0, size);
        m_len -= size;
        if (m_len != 0) {
            m_position += size;
        } else {
            m_position = 0;
        }
        return size;
    }

    public final byte[] PopBytes(int size) {
        //!!!!
        if (size < 0) {
            return null;
        }
        if (size > m_len) {
            size = m_len;
        }
        byte[] bytes = new byte[size];
        System.arraycopy(m_bytes, m_position, bytes, 0, size);
        m_len -= size;
        if (m_len != 0) {
            m_position += size;
        } else {
            m_position = 0;
        }
        return bytes;
    }

    public final int LoadInt() {
        byte[] data = Load(4);
        return (int) ((0xff & data[3]) << 24
                | (0xff & data[2]) << 16
                | (0xff & data[1]) << 8
                | (0xff & data[0]));
    }

    public final short LoadShort() {
        byte[] data = Load(2);
        short s = data[1];
        s <<= 8;
        s += data[0];
        return s;
    }

    public final byte LoadByte() {
        byte[] data = Load(1);
        return data[0];
    }

    public final long LoadLong() {
        byte[] data = Load(8);
        return ((long) (0xff & data[7]) << 56
                | (long) (0xff & data[6]) << 48
                | (long) (0xff & data[5]) << 40
                | (long) (0xff & data[4]) << 32
                | (long) (0xff & data[3]) << 24
                | (long) (0xff & data[2]) << 16
                | (long) (0xff & data[1]) << 8
                | (long) (0xff & data[0]));
    }

    public final boolean LoadBoolean() {
        byte[] data = Load(1);
        return (data[0] != 0);
    }

    public final char LoadChar() {
        return (char) LoadShort();
    }

    public final float LoadFloat() {
        return Float.intBitsToFloat(LoadInt());
    }

    public final double LoadDouble() {
        return Double.longBitsToDouble(LoadLong());
    }

    private static java.nio.charset.Charset m_UTF16 = java.nio.charset.Charset.forName("UTF-16LE");
    private static java.nio.charset.Charset m_UTF8 = java.nio.charset.Charset.forName("UTF-8");

    public final String LoadString() {
        int len = LoadInt();
        if (len == -1) {
            return null;
        }
        if (len < 0 || len > m_len) {
            throw new RuntimeException("Invalid string found");
        }
        String s = new String(m_bytes, m_position, len, m_UTF16);
        Discard(len);
        return s;
    }

    public final <T extends IUSerializer> Object LoadObject(Class<T> cls) {
        Object obj = null;
        short vt = LoadShort();
        switch (vt) {
            case tagVariantDataType.sdVT_EMPTY:
                obj = new Object();
                break;
            case tagVariantDataType.sdVT_NULL:
                break;
            case tagVariantDataType.sdVT_NETObject: {
                try {
                    T t = cls.newInstance();
                    t.LoadFrom(this);
                    obj = t;
                } catch (InstantiationException | java.lang.IllegalAccessException err) {
                    throw new UnsupportedOperationException("InstantiationException or IllegalAccessException");
                }
            }
            break;
            case tagVariantDataType.sdVT_NETObject | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                Object[] a = new Object[len];
                for (int n = 0; n < len; ++n) {
                    try {
                        T t = cls.newInstance();
                        t.LoadFrom(this);
                        a[n] = t;
                    } catch (InstantiationException | java.lang.IllegalAccessException err) {
                        throw new UnsupportedOperationException("InstantiationException or IllegalAccessException");
                    }
                }
                obj = a;
            }
            break;
            default:
                throw new UnsupportedOperationException("unsupported data type from this method");
        }
        return obj;
    }

    public static String ToString(byte[] bytes) {
        String s = null;
        if (bytes != null) {
            s = new String(bytes, 0, bytes.length, m_UTF8);
        }
        return s;
    }

    public Object LoadObject(short[] datatype) {
        Object obj = null;
        datatype[0] = LoadShort();
        switch (datatype[0]) {
            case tagVariantDataType.sdVT_EMPTY:
                obj = new Object();
                break;
            case tagVariantDataType.sdVT_NULL:
                break;
            case tagVariantDataType.sdVT_I1:
            case tagVariantDataType.sdVT_UI1:
                obj = LoadByte();
                break;
            case tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                obj = new String(m_bytes, m_position, len, m_UTF8);
                Discard(len);
            }
            break;
            case tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY:
                obj = LoadBytes();
                break;
            case tagVariantDataType.sdVT_BOOL:
                if (LoadShort() != 0) {
                    obj = true;
                } else {
                    obj = false;
                }
                break;
            case tagVariantDataType.sdVT_BOOL | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                boolean[] a = new boolean[len];
                for (int n = 0; n < len; ++n) {
                    if (LoadShort() != 0) {
                        a[n] = true;
                    } else {
                        a[n] = false;
                    }
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_I2:
            case tagVariantDataType.sdVT_UI2:
                obj = LoadShort();
                break;
            case tagVariantDataType.sdVT_I2 | tagVariantDataType.sdVT_ARRAY:
            case tagVariantDataType.sdVT_UI2 | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                short[] a = new short[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadShort();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_I4:
            case tagVariantDataType.sdVT_UI4:
            case tagVariantDataType.sdVT_INT:
            case tagVariantDataType.sdVT_UINT:
                obj = LoadInt();
                break;
            case tagVariantDataType.sdVT_I4 | tagVariantDataType.sdVT_ARRAY:
            case tagVariantDataType.sdVT_UI4 | tagVariantDataType.sdVT_ARRAY:
            case tagVariantDataType.sdVT_INT | tagVariantDataType.sdVT_ARRAY:
            case tagVariantDataType.sdVT_UINT | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                int[] a = new int[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadInt();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_I8:
            case tagVariantDataType.sdVT_UI8:
                obj = LoadLong();
                break;
            case tagVariantDataType.sdVT_I8 | tagVariantDataType.sdVT_ARRAY:
            case tagVariantDataType.sdVT_UI8 | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                long[] a = new long[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadLong();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_R4:
                obj = LoadFloat();
                break;
            case tagVariantDataType.sdVT_R4 | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                float[] a = new float[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadFloat();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_R8:
                obj = LoadDouble();
                break;
            case tagVariantDataType.sdVT_R8 | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                double[] a = new double[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadDouble();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_BSTR:
                obj = LoadString();
                break;
            case tagVariantDataType.sdVT_BSTR | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                String[] a = new String[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadString();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_CLSID:
                obj = LoadUUID();
                break;
            case tagVariantDataType.sdVT_CLSID | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                java.util.UUID[] a = new java.util.UUID[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadUUID();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_DECIMAL:
                obj = LoadDecimal();
                break;
            case tagVariantDataType.sdVT_DECIMAL | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                java.math.BigDecimal[] a = new java.math.BigDecimal[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadDecimal();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_DATE:
                obj = LoadTimestamp();
                break;
            case tagVariantDataType.sdVT_DATE | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                java.sql.Timestamp[] a = new java.sql.Timestamp[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadTimestamp();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_VARIANT | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                Object[] a = new Object[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadObject();
                }
                obj = a;
            }
            break;
            case tagVariantDataType.sdVT_CY:
                obj = LoadLong() / 10000.0;
                break;
            case tagVariantDataType.sdVT_CY | tagVariantDataType.sdVT_ARRAY: {
                int len = LoadInt();
                double[] a = new double[len];
                for (int n = 0; n < len; ++n) {
                    a[n] = LoadLong() / 10000.0;
                }
                obj = a;
            }
            break;
            default:
                throw new UnsupportedOperationException("unsupported data type");
        }
        return obj;
    }

    public final Object LoadObject() {
        short[] datatype = new short[1];
        return LoadObject(datatype);
    }

    public final CUQueue Save(java.math.BigDecimal dec) throws IllegalArgumentException {
        if (dec == null) {
            throw new IllegalArgumentException("BigDecimal dec can not be null");
        }
        short wReserved = 14;
        byte sign = 0;
        if (dec.signum() < 0) {
            sign = (byte) 128;
        }
        Save(wReserved);
        int precision = dec.precision();
        int scale = dec.scale();
        int left = precision - scale;
        if (left > 28) {
            throw new IllegalArgumentException("BigDecimal too big for MS Decimal");
        }
        int maxScale = 28 - left;
        if (scale > maxScale) {
            dec = dec.setScale(maxScale, java.math.RoundingMode.UP);
        }
        scale = dec.scale();
        Save((byte) scale);
        Save(sign);
        if (sign != 0) {
            dec = dec.negate();
        }
        java.math.BigInteger ubi = dec.unscaledValue();
        int high = ubi.divide(UINT64_MOD).intValue();
        long low = ubi.mod(UINT64_MOD).longValue();
        Save(high);
        return Save(low);
    }

    public final java.math.BigDecimal LoadDecimal() {
        short wReserved = LoadShort();
        byte scale = LoadByte();
        byte sign = LoadByte();
        int high = LoadInt();
        long low = LoadLong();
        java.math.BigInteger biLow = new java.math.BigInteger("" + low);
        if (low < 0) {
            biLow = biLow.add(UINT64_MOD);
        }
        java.math.BigInteger biHigh = new java.math.BigInteger("" + high);
        if (high < 0) {
            biHigh = biHigh.add(UINT32_MOD);
        }
        java.math.BigInteger bi = biHigh.shiftLeft(64);
        bi = bi.add(biLow);
        java.math.BigDecimal dec = new java.math.BigDecimal(bi, scale);
        if (sign != 0) {
            dec = dec.negate();
        }
        return dec;
    }

    public final CUQueue Save(Object obj) throws UnsupportedOperationException {
        short vt = tagVariantDataType.sdVT_NULL;
        if (obj == null) {
            return Save(vt);
        } else {
            String type = obj.getClass().getName();
            switch (type) {
                case "java.lang.Byte":
                    vt = tagVariantDataType.sdVT_UI1;
                    Save(vt);
                    Save((byte) obj);
                    break;
                case "[B": {
                    vt = (tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    byte[] a = (byte[]) obj;
                    Save(a);
                }
                break;
                case "java.lang.Boolean":
                    vt = tagVariantDataType.sdVT_BOOL;
                    Save(vt);
                    Save((short) ((boolean) obj ? -1 : 0));
                    break;
                case "[Z": {
                    vt = (tagVariantDataType.sdVT_BOOL | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    boolean[] a = (boolean[]) obj;
                    Save(a.length);
                    for (boolean b : a) {
                        Save((short) (b ? -1 : 0));
                    }
                }
                break;
                case "java.lang.Character":
                    vt = tagVariantDataType.sdVT_UI2;
                    Save(vt);
                    Save((char) obj);
                    break;
                case "java.lang.Short":
                    vt = tagVariantDataType.sdVT_I2;
                    Save(vt);
                    Save((short) obj);
                    break;
                case "[S": {
                    vt = (tagVariantDataType.sdVT_I2 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    short[] a = (short[]) obj;
                    Save(a.length);
                    for (short n : a) {
                        Save(n);
                    }
                }
                break;
                case "java.lang.Integer":
                    vt = tagVariantDataType.sdVT_INT;
                    Save(vt);
                    Save((int) obj);
                    break;
                case "[I": {
                    vt = (tagVariantDataType.sdVT_INT | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    int[] a = (int[]) obj;
                    Save(a.length);
                    for (int n : a) {
                        Save(n);
                    }
                }
                break;
                case "java.lang.Float":
                    vt = tagVariantDataType.sdVT_R4;
                    Save(vt);
                    Save((float) obj);
                    break;
                case "[F": {
                    vt = (tagVariantDataType.sdVT_R4 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    float[] a = (float[]) obj;
                    Save(a.length);
                    for (float f : a) {
                        Save(f);
                    }
                }
                break;
                case "java.lang.Long":
                    vt = tagVariantDataType.sdVT_I8;
                    Save(vt);
                    Save((long) obj);
                    break;
                case "[J": {
                    vt = (tagVariantDataType.sdVT_I8 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    long[] a = (long[]) obj;
                    Save(a.length);
                    for (long n : a) {
                        Save(n);
                    }
                }
                break;
                case "java.lang.Double":
                    vt = tagVariantDataType.sdVT_R8;
                    Save(vt);
                    Save((double) obj);
                    break;
                case "[D": {
                    vt = (tagVariantDataType.sdVT_R8 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    double[] a = (double[]) obj;
                    Save(a.length);
                    for (double d : a) {
                        Save(d);
                    }
                }
                break;
                case "java.util.Date":
                    vt = tagVariantDataType.sdVT_DATE;
                    Save(vt);
                    Save((java.util.Date) obj);
                    break;
                case "[Ljava.util.Date;": {
                    vt = (tagVariantDataType.sdVT_DATE | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    java.util.Date[] a = (java.util.Date[]) obj;
                    Save(a.length);
                    for (java.util.Date d : a) {
                        Save(d);
                    }
                }
                break;
                case "java.sql.Timestamp":
                    vt = tagVariantDataType.sdVT_DATE;
                    Save(vt);
                    Save((java.sql.Timestamp) obj);
                    break;
                case "[Ljava.sql.Timestamp;": {
                    vt = (tagVariantDataType.sdVT_DATE | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    java.sql.Timestamp[] a = (java.sql.Timestamp[]) obj;
                    Save(a.length);
                    for (java.util.Date d : a) {
                        Save(d);
                    }
                }
                break;
                case "java.util.UUID":
                    vt = tagVariantDataType.sdVT_CLSID;
                    Save(vt);
                    Save((java.util.UUID) obj);
                    break;
                case "[Ljava.util.UUID;": {
                    vt = (tagVariantDataType.sdVT_CLSID | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    java.util.UUID[] a = (java.util.UUID[]) obj;
                    Save(a.length);
                    for (java.util.UUID d : a) {
                        Save(d);
                    }
                }
                break;
                case "java.math.BigDecimal":
                    vt = tagVariantDataType.sdVT_DECIMAL;
                    Save(vt);
                    Save((java.math.BigDecimal) obj);
                    break;
                case "[Ljava.math.BigDecimal;": {
                    vt = (tagVariantDataType.sdVT_DECIMAL | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    java.math.BigDecimal[] a = (java.math.BigDecimal[]) obj;
                    Save(a.length);
                    for (java.math.BigDecimal d : a) {
                        Save(d);
                    }
                }
                break;
                case "java.lang.String":
                    vt = tagVariantDataType.sdVT_BSTR;
                    Save(vt);
                    Save((String) obj);
                    break;
                case "[C": {
                    vt = (tagVariantDataType.sdVT_UI2 | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    char[] a = (char[]) obj;
                    Save(a.length);
                    for (char c : a) {
                        Save(c);
                    }
                }
                break;
                case "[Ljava.lang.String;": {
                    vt = (tagVariantDataType.sdVT_BSTR | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    String[] a = (String[]) obj;
                    Save(a.length);
                    for (String s : a) {
                        Save(s);
                    }
                }
                break;
                case "java.lang.Object":
                    vt = tagVariantDataType.sdVT_EMPTY;
                    Save(vt);
                    break;
                case "[Ljava.lang.Object;": {
                    vt = (tagVariantDataType.sdVT_VARIANT | tagVariantDataType.sdVT_ARRAY);
                    Save(vt);
                    Object[] a = (Object[]) obj;
                    Save(a.length);
                    for (Object o : a) {
                        Save(o);
                    }
                }
                break;
                default:
                    if (obj instanceof IUSerializer[]) {
                        vt = (tagVariantDataType.sdVT_NETObject | tagVariantDataType.sdVT_ARRAY);
                        Save(vt);
                        IUSerializer[] a = (IUSerializer[]) obj;
                        Save(a.length);
                        for (IUSerializer o : a) {
                            Save(o);
                        }
                    } else if (obj instanceof IUSerializer) {
                        vt = tagVariantDataType.sdVT_NETObject;
                        Save(vt);
                        Save((IUSerializer) obj);
                    } else {
                        throw new UnsupportedOperationException("Unsupported data type for saving");
                    }
                    break;
            }
        }
        return this;
    }

    private final int m_blockSize;
    private int m_position = 0;
    private int m_len = 0;
    private byte[] m_bytes;
    private final byte[] m_b = new byte[16]; //MS DECIMAL and UUID
}
