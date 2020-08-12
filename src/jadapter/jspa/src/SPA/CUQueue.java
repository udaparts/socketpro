package SPA;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public final class CUQueue {

    public static final int DEFAULT_BUFFER_SIZE = 4 * 1024;
    public static final int DEFAULT_BLOCK_SIZE = 4 * 1024;
    private int m_blockSize = DEFAULT_BLOCK_SIZE;
    private ByteBuffer m_bytes;
    private int m_len = 0;
    private boolean m_bReading = false;
    private int m_position = 0;

    public static final tagOperationSystem DEFAULT_OS;

    static {
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

    public final tagOperationSystem getOS() {
        return m_os;
    }

    public final void setOS(tagOperationSystem os) {
        m_os = os;
    }

    public CUQueue() {
        m_bytes = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public CUQueue(int maxSize) {
        if (maxSize <= 0) {
            maxSize = DEFAULT_BUFFER_SIZE;
        }
        m_bytes = ByteBuffer.allocateDirect(maxSize);
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public CUQueue(int maxSize, int blockSize) {
        if (maxSize <= 0) {
            maxSize = DEFAULT_BUFFER_SIZE;
        }
        if (blockSize <= 0) {
            blockSize = DEFAULT_BLOCK_SIZE;
        }
        m_blockSize = blockSize;
        m_bytes = ByteBuffer.allocateDirect(maxSize);
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public CUQueue(byte[] bytes) {
        if (bytes == null) {
            m_bytes = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);
        } else {
            m_len = bytes.length;
            m_bytes = ByteBuffer.wrap(bytes);
        }
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public CUQueue(byte[] bytes, int len) {
        if (bytes == null) {
            m_bytes = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);
        } else if (len > 0 && bytes.length > len) {
            m_len = len;
            m_bytes = ByteBuffer.wrap(bytes);
        } else {
            m_bytes = ByteBuffer.wrap(bytes);
            m_len = bytes.length;
        }
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public final boolean isDirect() {
        return m_bytes.isDirect();
    }

    public void UseBuffer(byte[] bytes) {
        if (bytes == null) {
            m_bytes = ByteBuffer.wrap(new byte[4]);
            m_len = 0;
        } else {
            m_bytes = ByteBuffer.wrap(bytes);
            m_len = bytes.length;
        }
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
        m_position = 0;
    }

    public void UseBuffer(byte[] bytes, int len) {
        if (bytes == null) {
            m_bytes = ByteBuffer.wrap(new byte[4]);
            m_len = 0;
        } else if (len > 0 && bytes.length > len) {
            m_bytes = ByteBuffer.wrap(bytes);
            m_len = len;
        } else {
            m_bytes = ByteBuffer.wrap(bytes);
            m_len = bytes.length;
        }
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
        m_position = 0;
    }

    public void UseBuffer(java.nio.ByteBuffer bytes, int len) {
        m_position = 0;
        if (bytes == null) {
            m_bytes = ByteBuffer.wrap(new byte[4]);
            m_len = 0;
        } else {
            m_bytes = bytes;
            m_bytes.position(0);
            m_bytes.limit(len);
            m_len = len;
        }
        m_bytes.order(ByteOrder.LITTLE_ENDIAN);
    }

    public final void setSize(int newSize) {
        SetSize(newSize);
    }

    public final void SetSize(int size) {
        if (size < 0) {
            throw new java.lang.IllegalArgumentException("New size can not be less than 0");
        } else if (size > (m_bytes.capacity() - m_position)) {
            throw new java.lang.IllegalArgumentException("Bad new size");
        } else if (size == 0) {
            m_len = 0;
            m_position = 0;
            m_bytes.clear();
        } else {
            m_len = size;
            m_bytes.position(m_position);
            m_bytes.limit(m_position + m_len);
        }
    }

    public final void Empty() {
        m_len = 0;
        m_bytes.clear();
        m_position = 0;
    }

    public final boolean getEndian() {
        return (m_bytes.order() == ByteOrder.BIG_ENDIAN);
    }

    public final void setEndian(boolean e) {
        m_bytes.order(e ? ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN);
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
        return (m_bytes.capacity() - m_position - m_len);
    }

    public final int getMaxBufferSize() {
        return m_bytes.capacity();
    }

    public final int Discard(int len) {
        if (len > m_len) {
            len = m_len;
        }
        m_len -= len;
        if (m_len == 0) {
            m_position = 0;
            m_bytes.clear();
        } else {
            m_position += len;
            m_bytes.position(m_position);
        }
        return len;
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

    public final <T extends IUSerializer> CUQueue Save(T s) {
        s.SaveTo(this);
        return this;
    }

    public final <T extends IUSerializer> T Load(Class<T> cls) {
        T t = null;
        try {
            t = cls.newInstance();
            t.LoadFrom(this);
        } catch (InstantiationException | IllegalAccessException err) {
            throw new UnsupportedOperationException("InstantiationException or IllegalAccessException");
        }
        return t;
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
                byte[] bytes = PopBytes(len);
                obj = new String(bytes, 0, len, m_UTF8);
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

    public final void Realloc(int newMaxSize) {
        if (newMaxSize <= 0) {
            newMaxSize = DEFAULT_BUFFER_SIZE;
        }
        newMaxSize = (newMaxSize / m_blockSize + 1) * m_blockSize;
        ByteBuffer bytes = ByteBuffer.allocateDirect(newMaxSize);
        bytes.order(m_bytes.order());
        if (m_len > 0) {
            m_len = (m_len > newMaxSize) ? newMaxSize : m_len;
            m_bytes.position(m_position);
            m_bytes.limit(m_position + m_len);
            bytes.put(m_bytes);
        }
        m_bytes = bytes;
        m_position = 0;
        if (!m_bReading) {
            bytes.position(m_len);
        }
    }

    private void Ensure(int addedBytes) {
        if (m_bReading) {
            m_bytes.position(m_position + m_len);
            m_bytes.limit(m_bytes.capacity());
            m_bReading = false;
        }
        if (getTailSize() <= addedBytes) {
            if (m_bytes.capacity() - m_len > addedBytes) {
                m_bytes.position(m_position);
                m_bytes.compact();
                m_position = 0;
                m_bytes.position(m_len);
            } else {
                Realloc(m_bytes.capacity() + addedBytes);
            }
        }
    }

    public final CUQueue Push(java.nio.ByteBuffer bb) {
        if (bb != null) {
            int pos = bb.position();
            int size = bb.remaining();
            Ensure(size);
            if (size > 0) {
                m_bytes.put(bb);
                bb.position(pos);
                m_len += size;
            }
        }
        return this;
    }

    public final CUQueue Push(byte[] bytes, int offset, int len) {
        if (offset < 0) {
            offset = 0;
        }
        if (bytes == null || len <= 0) {
            return this;
        }
        if (offset + len > bytes.length) {
            throw new IllegalArgumentException("Bad offset or length");
        }
        Ensure(len);
        m_bytes.put(bytes, offset, len);
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

    public final CUQueue Save(byte b) {
        Ensure(1);
        m_bytes.put(b);
        m_len += 1;
        return this;
    }

    public final CUQueue Save(byte[] bytes) {
        int len = -1;
        if (bytes != null) {
            len = bytes.length;
            Save(len);
            Ensure(len);
            m_bytes.put(bytes);
            m_len += len;
        } else {
            Save(len);
        }
        return this;
    }
    private static java.nio.charset.Charset m_UTF16 = java.nio.charset.Charset.forName("UTF-16LE");
    private static java.nio.charset.Charset m_UTF8 = java.nio.charset.Charset.forName("UTF-8");

    public final String LoadString() {
        byte[] bytes = LoadBytes();
        if (bytes == null) {
            return null;
        }
        return new String(bytes, 0, bytes.length, m_UTF16);
    }

    public final CUQueue Save(String s) {
        int len = -1;
        if (s != null) {
            len = s.length() * 2;
            Save(len);
            if (len > 0) {
                Ensure(len);
                java.nio.CharBuffer cb = java.nio.CharBuffer.wrap(s);
                m_UTF16.newEncoder().encode(cb, m_bytes, true);
                m_len += len;
            }
        } else {
            Save(len);
        }
        return this;
    }

    public final CUQueue Save(boolean b) {
        byte ok = b ? (byte) 1 : (byte) 0;
        return Save(ok);
    }

    public final CUQueue Save(short s) {
        Ensure(2);
        m_bytes.putShort(s);
        m_len += 2;
        return this;
    }

    public final CUQueue Save(int n) {
        Ensure(4);
        m_bytes.putInt(n);
        m_len += 4;
        return this;
    }

    public final CUQueue Save(float f) {
        Ensure(4);
        m_bytes.putFloat(f);
        m_len += 4;
        return this;
    }

    public final CUQueue Save(double d) {
        Ensure(8);
        m_bytes.putDouble(d);
        m_len += 8;
        return this;
    }

    public final CUQueue Save(long n) {
        Ensure(8);
        m_bytes.putLong(n);
        m_len += 8;
        return this;
    }

    public final CUQueue Save(java.util.UUID id) throws IllegalArgumentException {
        if (id == null) {
            throw new IllegalArgumentException("UUID id can not be null");
        }
        Ensure(16);
        m_bytes.putLong(id.getMostSignificantBits()).putLong(id.getLeastSignificantBits());
        m_len += 16;
        return this;
    }

    private static java.math.BigInteger UINT64_MOD = new java.math.BigInteger("10000000000000000", 16);

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

    public final CUQueue ForReading() {
        SetForReading(0);
        return this;
    }

    private void SetForReading(int bytes) {
        if (!m_bReading) {
            m_bytes.position(m_position);
            m_bytes.limit(m_position + m_len);
            m_bReading = true;
        }
        if (bytes > m_len) {
            throw new RuntimeException("Invalid data found");
        }
    }

    private void AdjustPosition(int bytes) {
        assert m_len >= bytes;
        m_len -= bytes;
        if (m_len == 0) {
            m_position = 0;
            m_bytes.position(0);
            m_bytes.limit(m_bytes.capacity());
        } else {
            m_position += bytes;
            assert m_position <= m_bytes.limit();
            assert m_position <= m_bytes.capacity();
        }
    }

    public final byte LoadByte() {
        SetForReading(1);
        byte b = m_bytes.get();
        AdjustPosition(1);
        return b;
    }

    public final short LoadShort() {
        SetForReading(2);
        short s = m_bytes.getShort();
        AdjustPosition(2);
        return s;
    }

    public final int LoadInt() {
        SetForReading(4);
        int n = m_bytes.getInt();
        AdjustPosition(4);
        return n;
    }

    public final float LoadFloat() {
        SetForReading(4);
        float f = m_bytes.getFloat();
        AdjustPosition(4);
        return f;
    }

    public final double LoadDouble() {
        SetForReading(8);
        double d = m_bytes.getDouble();
        AdjustPosition(8);
        return d;
    }

    public final long LoadLong() {
        SetForReading(8);
        long n = m_bytes.getLong();
        AdjustPosition(8);
        return n;
    }

    public final java.util.UUID LoadUUID() {
        SetForReading(16);
        long high = m_bytes.getLong();
        long low = m_bytes.getLong();
        AdjustPosition(16);
        return new java.util.UUID(high, low);
    }

    public final ByteBuffer getIntenalBuffer() {
        SetForReading(0);
        return m_bytes;
    }

    private static java.math.BigInteger UINT32_MOD = new java.math.BigInteger("100000000", 16);

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

    public final boolean LoadBoolean() {
        byte data = LoadByte();
        return data != 0;
    }

    public final char LoadChar() {
        return (char) LoadShort();
    }

    public final int PopBytes(byte[] bytes, int size) {
        if (size <= 0 || bytes == null) {
            return 0;
        }
        if (size > bytes.length) {
            size = bytes.length;
        }
        if (size > m_len) {
            size = m_len;
        }
        SetForReading(size);
        m_bytes.get(bytes, 0, size);
        AdjustPosition(size);
        return size;
    }

    public static String ToString(byte[] bytes) {
        String s = null;
        if (bytes != null) {
            s = new String(bytes, 0, bytes.length, m_UTF8);
        }
        return s;
    }

    public final byte[] PopBytes(int size) {
        if (size < 0) {
            return null;
        }
        if (size == 0) {
            return new byte[0];
        } else if (size > m_len) {
            size = m_len;
        }
        byte[] bytes = new byte[size];
        SetForReading(size);
        m_bytes.get(bytes, 0, size);
        AdjustPosition(size);
        return bytes;
    }

    public final byte[] GetBuffer(int offset) {
        byte[] bytes;
        if (offset < 0) {
            offset = 0;
        } else if (offset > m_len) {
            offset = m_len;
        }
        int len = m_len - offset;
        SetForReading(0);
        if (offset != 0 || m_bytes.isDirect()) {
            bytes = new byte[len];
            if (len > 0) {
                m_bytes.get(bytes);
                m_bytes.position(m_position);
            }
        } else {
            bytes = m_bytes.array();
        }
        return bytes;
    }

    public final byte[] GetBuffer() {
        return GetBuffer(0);
    }

    public final byte[] LoadBytes() {
        int size = LoadInt();
        if (size == -1) {
            return null;
        } else if (size > m_len) {
            throw new RuntimeException("Invalid data found");
        }
        byte[] bytes = new byte[size];
        m_bytes.get(bytes);
        AdjustPosition(size);
        return bytes;
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
        if (len > 0) {
            byte[] bytes = PopBytes(len);
            q.Push(bytes);
        }
        return q;
    }

    public final CUQueue Push(CUQueue q) {
        if (q != null) {
            int nSize = q.GetSize();
            if (nSize > 0) {
                q.SetForReading(0);
                q.m_bytes.limit(q.m_position + nSize);
                Ensure(nSize);
                m_bytes.put(q.m_bytes);
                m_len += nSize;
                q.m_bytes.position(q.m_position);
            }
        }
        return this;
    }

    public final CUQueue Save(CUQueue q) {
        int nSize = -1;
        if (q != null) {
            nSize = q.GetSize();
            Save(nSize);
            if (nSize > 0) {
                q.SetForReading(0);
                q.m_bytes.limit(q.m_position + nSize);
                Ensure(nSize);
                m_bytes.put(q.m_bytes);
                m_len += nSize;
                q.m_bytes.position(q.m_position);
            }
        } else {
            Save(nSize);
        }
        return this;
    }

    public final int PeekInt(int pos) {
        if (pos < 0 || pos + 4 > m_len) {
            throw new RuntimeException("Invalid operation for bad position");
        }
        return m_bytes.getInt(m_position + pos);
    }

    public final CUQueue ResetInt(int n, int pos) {
        if (pos < 0) {
            pos = 0;
        }
        if ((pos + 4) > m_len) {
            throw new RuntimeException("Invalid operation for bad positin");
        }
        m_bytes.putInt(m_position + pos, n);
        return this;
    }
}
