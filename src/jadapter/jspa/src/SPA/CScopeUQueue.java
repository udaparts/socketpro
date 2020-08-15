package SPA;

import java.util.ArrayDeque;

public final class CScopeUQueue implements AutoCloseable {

    @Override
    public void close() {
        Clean();
    }

    public static long getMemoryConsumed() {
        long mem = 0;
        synchronized (m_cs) {
            for (CUQueue q : m_sQueue) {
                mem += q.getMaxBufferSize();
            }
        }
        return mem;
    }

    public static void DestroyUQueuePool() {
        synchronized (m_cs) {
            while (m_sQueue.size() > 0) {
                CUQueue q = m_sQueue.removeLast();
                q.Empty();
            }
        }
    }

    private static int m_mem_size = 32 * 1024;

    public static int getSHARED_BUFFER_CLEAN_SIZE() {
        synchronized (m_cs) {
            return m_mem_size;
        }
    }

    public static void setSHARED_BUFFER_CLEAN_SIZE(int size) {
        synchronized (m_cs) {
            if (size <= 512) {
                size = 512;
            }
            m_mem_size = size;
        }
    }

    public static CUQueue Lock(tagOperationSystem os) {
        CUQueue UQueue = null;
        synchronized (m_cs) {
            if (m_sQueue.size() > 0) {
                UQueue = m_sQueue.removeLast();
            }
        }
        if (UQueue == null) {
            UQueue = new CUQueue();
        }
        UQueue.setOS(os);
        return UQueue;
    }

    public static CUQueue Lock() {
        return Lock(CUQueue.DEFAULT_OS);
    }

    public static void Unlock(CUQueue UQueue) {
        if (UQueue != null) {
            if (!UQueue.isDirect()) {
                throw new UnsupportedOperationException("Cannot put non direct memory buffer into pool for resuse");
            }
            UQueue.SetSize(0);
            synchronized (m_cs) {
                m_sQueue.addLast(UQueue);
            }
        }
    }

    public final CUQueue Detach() {
        CUQueue q = m_UQueue;
        m_UQueue = null;
        return q;
    }

    public void Clean() {
        if (m_UQueue != null) {
            Unlock(m_UQueue);
            m_UQueue = null;
        }
    }

    public final void Attach(CUQueue q) {
        Unlock(m_UQueue);
        m_UQueue = q;
    }

    public final CUQueue getUQueue() {
        return m_UQueue;
    }

    public final CUQueue Save(Byte b) {
        m_UQueue.Save(b);
        return m_UQueue;
    }

    public final CUQueue Save(Short s) {
        m_UQueue.Save(s);
        return m_UQueue;
    }

    public final CUQueue Save(Integer i) {
        m_UQueue.Save(i);
        return m_UQueue;
    }

    public final CUQueue Save(Long i) {
        m_UQueue.Save(i);
        return m_UQueue;
    }

    public final CUQueue Save(Float f) {
        m_UQueue.Save(f);
        return m_UQueue;
    }

    public final CUQueue Save(Double d) {
        m_UQueue.Save(d);
        return m_UQueue;
    }

    public final CUQueue Save(Boolean b) {
        m_UQueue.Save(b);
        return m_UQueue;
    }

    public final <T extends IUSerializer> CUQueue Save(T s) {
        s.SaveTo(m_UQueue);
        return m_UQueue;
    }

    public final CUQueue Save(int n) {
        m_UQueue.Save(n);
        return m_UQueue;
    }

    public final CUQueue Save(short n) {
        m_UQueue.Save(n);
        return m_UQueue;
    }

    public final CUQueue Save(char c) {
        m_UQueue.Save(c);
        return m_UQueue;
    }

    public final CUQueue Save(byte n) {
        m_UQueue.Save(n);
        return m_UQueue;
    }

    public final CUQueue Save(boolean b) {
        m_UQueue.Save(b);
        return m_UQueue;
    }

    public final CUQueue Save(long n) {
        m_UQueue.Save(n);
        return m_UQueue;
    }

    public final CUQueue Save(float f) {
        m_UQueue.Save(f);
        return m_UQueue;
    }

    public final CUQueue Save(double d) {
        m_UQueue.Save(d);
        return m_UQueue;
    }

    public final CUQueue Save(String s) {
        m_UQueue.Save(s);
        return m_UQueue;
    }

    public final CUQueue Save(byte[] bytes) {
        m_UQueue.Save(bytes);
        return m_UQueue;
    }

    public final CUQueue Save(java.util.UUID id) {
        m_UQueue.Save(id);
        return m_UQueue;
    }

    public final CUQueue Save(Object obj) {
        m_UQueue.Save(obj);
        return m_UQueue;
    }

    public final CUQueue Save(CUQueue q) {
        m_UQueue.Save(q);
        return m_UQueue;
    }

    public final CUQueue Save(java.math.BigDecimal dec) {
        m_UQueue.Save(dec);
        return m_UQueue;
    }

    public final CUQueue Save(java.util.Date dt) {
        m_UQueue.Save(dt);
        return m_UQueue;
    }

    public final java.util.UUID LoadUUID() {
        return m_UQueue.LoadUUID();
    }

    public final byte[] LoadBytes() {
        return m_UQueue.LoadBytes();
    }

    public final int LoadInt() {
        return m_UQueue.LoadInt();
    }

    public final short LoadShort() {
        return m_UQueue.LoadShort();
    }

    public final byte LoadByte() {
        return m_UQueue.LoadByte();
    }

    public final long LoadLong() {
        return m_UQueue.LoadLong();
    }

    public final boolean LoadBoolean() {
        return m_UQueue.LoadBoolean();
    }

    public final char LoadChar() {
        return m_UQueue.LoadChar();
    }

    public final float LoadFloat() {
        return m_UQueue.LoadFloat();
    }

    public final double LoadDouble() {
        return m_UQueue.LoadDouble();
    }

    public final String LoadString() {
        return m_UQueue.LoadString();
    }

    public final Object LoadObject() throws UnsupportedOperationException {
        return m_UQueue.LoadObject();
    }

    public final java.math.BigDecimal LoadDecimal() {
        return m_UQueue.LoadDecimal();
    }

    public final <T extends SPA.IUSerializer> T Load(Class<T> cls) {
        return m_UQueue.Load(cls);
    }

    public final <T extends SPA.IUSerializer> Object LoadObject(Class<T> cls) {
        return m_UQueue.LoadObject(cls);
    }

    public final java.util.Date LoadDate() {
        return m_UQueue.LoadDate();
    }

    private CUQueue m_UQueue = Lock();
    private final static Object m_cs = new Object();
    private final static ArrayDeque<CUQueue> m_sQueue = new ArrayDeque<>();
}
