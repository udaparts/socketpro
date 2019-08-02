package SPA.ClientSide;

public final class CAsyncResult {

    CAsyncResult(CAsyncServiceHandler ash, short sReqId, SPA.CUQueue q, CAsyncServiceHandler.DAsyncResultHandler arh) {
        m_AsyncServiceHandler = ash;
        m_RequestId = sReqId;
        m_UQueue = q;
        m_CurrentAsyncResultHandler = arh;
    }

    void Reset(short sReqId, SPA.CUQueue q, CAsyncServiceHandler.DAsyncResultHandler arh) {
        m_RequestId = sReqId;
        m_UQueue = q;
        m_CurrentAsyncResultHandler = arh;
    }
    private final CAsyncServiceHandler m_AsyncServiceHandler;
    private short m_RequestId;
    private SPA.CUQueue m_UQueue;
    private CAsyncServiceHandler.DAsyncResultHandler m_CurrentAsyncResultHandler;

    public final CAsyncServiceHandler getAsyncServiceHandler() {
        return m_AsyncServiceHandler;
    }

    @Override
    protected void finalize() throws Throwable {
        Clean();
        super.finalize();
    }

    public final short getRequestId() {
        return m_RequestId;
    }

    public final SPA.CUQueue getUQueue() {
        return m_UQueue;
    }

    public final CAsyncServiceHandler.DAsyncResultHandler getCurrentAsyncResultHandler() {
        return m_CurrentAsyncResultHandler;
    }

    public final void Clean() {
        if (m_UQueue != null) {
            m_UQueue = null;
        }
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
}
