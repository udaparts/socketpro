package SPA.ClientSide;

public class CAsyncServiceHandler {

    public interface DAsyncResultHandler {

        void invoke(CAsyncResult AsyncResult);
    }

    public interface DOnResultReturned {

        boolean invoke(CAsyncServiceHandler sender, short reqId, SPA.CUQueue qData);
    }
    public volatile DOnResultReturned ResultReturned = null;

    public interface DOnExceptionFromServer {

        void invoke(CAsyncServiceHandler sender, short reqId, String errMessage, String errWhere, int errCode);
    }
    public volatile DOnExceptionFromServer ServerException = null;

    public final CClientSocket getAttachedClientSocket() {
        return m_ClientSocket;
    }

    protected CAsyncServiceHandler(int nServiceId) {
        m_nServiceId = nServiceId;
        m_ClientSocket = null;
    }

    void SetNull() {
        m_ClientSocket = null;
    }

    void Detach() {
        CClientSocket cs;
        if (m_ClientSocket == null) {
            return;
        }
        cs = m_ClientSocket;
        m_ClientSocket = null;
        cs.Detach(this);
    }

    boolean Attach(CClientSocket cs) {
        boolean ok = true;
        Detach();
        if (cs != null) {
            ok = cs.Attach(this);
            m_ClientSocket = cs;
        }
        return ok;
    }

    public final int getSvsID() {
        return m_nServiceId;
    }

    CClientSocket m_ClientSocket;
    int m_nServiceId;
    private final Object m_cs = new Object();
    private final Object m_csSend = new Object();
    private volatile java.util.ArrayDeque<java.util.Map.Entry<Short, DAsyncResultHandler>> m_kvCallback = new java.util.ArrayDeque<>();
    private volatile java.util.ArrayDeque<java.util.Map.Entry<Short, DAsyncResultHandler>> m_kvBatching = new java.util.ArrayDeque<>();

    public boolean SendRequest(short reqId, byte[] data, int len, DAsyncResultHandler ash) {
        if (data == null) {
            data = new byte[0];
            len = 0;
        } else if (len > data.length) {
            len = data.length;
        }
        long h = m_ClientSocket.getHandle();
        if (h == 0) {
            return false;
        }
        synchronized (m_csSend) {
            if (ash != null) {
                java.util.Map.Entry<Short, DAsyncResultHandler> kv = new java.util.AbstractMap.SimpleEntry<>(reqId, ash);
                if (ClientCoreLoader.IsBatching(h)) {
                    synchronized (m_cs) {
                        m_kvBatching.add(kv);
                    }
                } else {
                    synchronized (m_cs) {
                        m_kvCallback.add(kv);
                    }
                }
            }
            return ClientCoreLoader.SendRequest(h, reqId, data, len);
        }
    }

    public final boolean SendRequest(short reqId, SPA.CUQueue q, DAsyncResultHandler ash) {
        if (q == null) {
            return SendRequest(reqId, (byte[]) null, (int) 0, ash);
        } else if (q.getHeadPosition() > 0) {
            return SendRequest(reqId, q.GetBuffer(), q.GetSize(), ash);
        }
        return SendRequest(reqId, q.getIntenalBuffer(), q.GetSize(), ash);
    }

    public final boolean SendRequest(short reqId, SPA.CScopeUQueue q, DAsyncResultHandler ash) {
        if (q == null) {
            return SendRequest(reqId, (byte[]) null, (int) 0, ash);
        }
        return SendRequest(reqId, q.getUQueue(), ash);
    }

    public final boolean SendRequest(short reqId, DAsyncResultHandler ash) {
        return SendRequest(reqId, (byte[]) null, (int) 0, ash);
    }

    protected boolean SendRouteeResult(byte[] data, int len, short reqId) {
        long h;
        if (data == null) {
            data = new byte[0];
            len = 0;
        } else if (len > data.length) {
            len = data.length;
        }
        h = m_ClientSocket.getHandle();
        if (reqId == 0) {
            reqId = m_ClientSocket.getCurrentRequestID();
        }
        return ClientCoreLoader.SendRouteeResult(h, reqId, data, len);
    }

    protected final boolean SendRouteeResult(byte[] data, int len) {
        return SendRouteeResult(data, len, (short) 0);
    }

    protected final boolean SendRouteeResult(short reqId) {
        return SendRouteeResult((byte[]) null, (int) 0, reqId);
    }

    protected final boolean SendRouteeResult() {
        return SendRouteeResult((short) 0);
    }

    protected boolean SendRouteeResult(SPA.CUQueue q, short reqId) {
        if (q == null) {
            return SendRouteeResult(reqId);
        } else if (q.getHeadPosition() > 0) {
            return SendRouteeResult(q.GetBuffer(), q.GetSize(), reqId);
        }
        return SendRouteeResult(q.getIntenalBuffer(), q.GetSize(), reqId);
    }

    protected final boolean SendRouteeResult(SPA.CUQueue q) {
        return SendRouteeResult(q, (short) 0);
    }

    protected final boolean SendRouteeResult(SPA.CScopeUQueue q) {
        return SendRouteeResult(q, (short) 0);
    }

    protected final boolean SendRouteeResult(SPA.CScopeUQueue q, short reqId) {
        if (q == null) {
            return SendRouteeResult(reqId);
        }
        return SendRouteeResult(q.getUQueue(), reqId);
    }

    protected void OnExceptionFromServer(short reqId, String errMessage, String errWhere, int errCode) {
    }

    public boolean WaitAll(int timeOut) {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        if (ClientCoreLoader.IsBatching(h)) {
            throw new UnsupportedOperationException("Can't call the method WaitAll during batching requests");
        }
        if (ClientCoreLoader.IsQueueStarted(h) && ClientCoreLoader.GetJobSize(h) > 0) {
            throw new UnsupportedOperationException("Can't call the method WaitAll during enqueuing transactional requests");
        }
        return ClientCoreLoader.WaitAll(h, timeOut);
    }

    public final boolean WaitAll() {
        return WaitAll(-1);
    }

    public final boolean CommitBatching() {
        return CommitBatching(false);
    }

    public boolean CommitBatching(boolean bBatchingAtServerSide) {
        long h;
        synchronized (m_cs) {
            m_kvCallback.addAll(m_kvBatching);
            m_kvBatching.clear();
            h = m_ClientSocket.getHandle();
        }
        return ClientCoreLoader.CommitBatching(h, bBatchingAtServerSide);
    }

    void AppendTo(CAsyncServiceHandler to) {
        synchronized (to.m_cs) {
            synchronized (m_cs) {
                to.m_kvCallback.addAll(m_kvCallback);
                m_kvCallback.clear();
            }
        }
    }

    public final boolean StartBatching() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.StartBatching(h);
    }

    private java.util.Map.Entry<Short, DAsyncResultHandler> GetAsyncResultHandler(short reqId) {
        if (m_ClientSocket.getRandom()) {
            synchronized (m_cs) {
                for (java.util.Map.Entry<Short, DAsyncResultHandler> kv : m_kvCallback) {
                    if (kv.getKey() == reqId) {
                        m_kvCallback.remove(kv);
                        return kv;
                    }
                }
            }
        } else {
            synchronized (m_cs) {
                if (m_kvCallback.size() > 0 && m_kvCallback.getFirst().getKey() == reqId) {
                    return m_kvCallback.removeFirst();
                }
            }
        }
        return new java.util.AbstractMap.SimpleEntry<>((short) 0, null);
    }

    final void OnSE(short reqId, String errMessage, String errWhere, int errCode) {
        java.util.Map.Entry<Short, DAsyncResultHandler> p = GetAsyncResultHandler(reqId);
        OnExceptionFromServer(reqId, errMessage, errWhere, errCode);
        if (ServerException != null) {
            ServerException.invoke(this, reqId, errMessage, errWhere, errCode);
        }
    }

    final void onRR(short reqId, SPA.CUQueue mc) {
        java.util.Map.Entry<Short, DAsyncResultHandler> p = GetAsyncResultHandler(reqId);
        if (p.getValue() != null) {
            CAsyncResult ar = new CAsyncResult(this, reqId, mc, p.getValue());
            p.getValue().invoke(ar);
        } else {
            if (ResultReturned != null && ResultReturned.invoke(this, reqId, mc)) {
            } else {
                OnResultReturned(reqId, mc);
            }
        }
    }

    protected void OnResultReturned(short sRequestId, SPA.CUQueue UQueue) {
    }

    protected void OnBaseRequestProcessed(short reqId) {

    }

    public final boolean getRouteeRequest() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.IsRouteeRequest(h);
    }

    public final boolean AbortBatching() {
        synchronized (m_cs) {
            m_kvBatching.clear();
        }
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.AbortBatching(h);
    }

    public final void AbortDequeuedMessage() {
        long h = getCSHandle();
        if (h == 0) {
            return;
        }
        ClientCoreLoader.AbortDequeuedMessage(h);
    }

    public final boolean getDequeuedResult() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.DequeuedResult(h);
    }

    private long getCSHandle() {
        return m_ClientSocket.getHandle();
    }

    public final boolean getDequeuedMessageAborted() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.IsDequeuedMessageAborted(h);
    }

    public final boolean getBatching() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.IsBatching(h);
    }

    public int CleanCallbacks() {
        synchronized (m_cs) {
            int size = m_kvBatching.size() + m_kvCallback.size();
            m_kvBatching.clear();
            m_kvCallback.clear();
            return size;
        }
    }
}
