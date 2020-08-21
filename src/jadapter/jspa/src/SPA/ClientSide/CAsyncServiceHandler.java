package SPA.ClientSide;

public class CAsyncServiceHandler implements AutoCloseable {

    public static final int SESSION_CLOSED_AFTER = -1000;
    public static final int SESSION_CLOSED_BEFORE = -1001;
    public static final int REQUEST_CANCELED = -1002;
    public static final long DEFAULT_INTERRUPT_OPTION = 1;

    @Override
    public void close() {
        CleanCallbacks();
    }

    @Override
    protected void finalize() throws Throwable {
        CleanCallbacks();
        super.finalize();
    }

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

    public interface DDiscarded {

        void invoke(CAsyncServiceHandler sender, boolean discarded);
    }

    public interface DOnBaseRequestProcessed {

        void invoke(CAsyncServiceHandler sender, short reqId);
    }

    public DOnBaseRequestProcessed BaseRequestProcessed = null;

    class CResultCb {

        public CResultCb(DAsyncResultHandler ash, DDiscarded dis, DOnExceptionFromServer ex) {
            AsyncResultHandler = ash;
            Discarded = dis;
            ExceptionFromServer = ex;
        }
        public DAsyncResultHandler AsyncResultHandler;
        public DDiscarded Discarded;
        public DOnExceptionFromServer ExceptionFromServer;
    }

    private static final Object m_csCallIndex = new Object();
    private static long m_CallIndex = 0;

    public static long GetCallIndex() {
        synchronized (m_csCallIndex) {
            return ++m_CallIndex;
        }
    }

    /**
     * Check the number of requests queued inside asynchronous handler
     *
     * @return the number of requests queued inside asynchronous handler
     */
    public int getRequestsQueued() {
        synchronized (m_cs) {
            return m_kvCallback.size();
        }
    }

    public final CClientSocket getAttachedClientSocket() {
        return m_ClientSocket;
    }

    public final CClientSocket getSocket() {
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
    protected final Object m_cs = new Object();
    private final Object m_csSend = new Object();
    private volatile java.util.ArrayDeque<java.util.Map.Entry<Short, CResultCb>> m_kvCallback = new java.util.ArrayDeque<>();
    private volatile java.util.ArrayDeque<java.util.Map.Entry<Short, CResultCb>> m_kvBatching = new java.util.ArrayDeque<>();

    protected final java.util.ArrayDeque<java.util.Map.Entry<Short, CResultCb>> GetCallbacks() {
        return m_kvCallback;
    }

    protected final void EraseBack(int count) {
        int total = m_kvCallback.size();
        if (count > total) {
            count = total;
        }
        while (count > 0) {
            java.util.Map.Entry<Short, CResultCb> p = m_kvCallback.removeLast();
            if (p.getValue().Discarded != null) {
                p.getValue().Discarded.invoke(this, true);
            }
            --count;
        }
    }

    public final boolean SendRequest(short reqId, java.nio.ByteBuffer data, DAsyncResultHandler ash) {
        if (data != null) {
            return SendRequest(reqId, data, data.limit() - data.position(), ash, null, null);
        }
        return SendRequest(reqId, data, 0, ash, null, null);
    }

    public final boolean SendRequest(short reqId, java.nio.ByteBuffer data, DAsyncResultHandler ash, DDiscarded discarded) {
        if (data != null) {
            return SendRequest(reqId, data, data.limit() - data.position(), ash, discarded, null);
        }
        return SendRequest(reqId, data, 0, ash, discarded, null);
    }

    public final boolean SendRequest(short reqId, java.nio.ByteBuffer data, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        if (data != null) {
            return SendRequest(reqId, data, data.limit() - data.position(), ash, discarded, exception);
        }
        return SendRequest(reqId, data, 0, ash, discarded, exception);
    }

    public boolean SendRequest(short reqId, java.nio.ByteBuffer data, int len, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        int offset;
        if (data == null || len < 0) {
            len = 0;
            offset = 0;
        } else {
            offset = data.position();
        }
        long h = m_ClientSocket.getHandle();
        if (h == 0) {
            return false;
        }
        boolean sent, batching;
        java.util.Map.Entry<Short, CResultCb> kv;
        if (ash != null || discarded != null || exception != null) {
            kv = new java.util.AbstractMap.SimpleEntry<>(reqId, new CResultCb(ash, discarded, exception));
            synchronized (m_csSend) {
                synchronized (m_cs) {
                    batching = m_bBatching;
                    if (batching) {
                        m_kvBatching.add(kv);
                    } else {
                        m_kvCallback.add(kv);
                    }
                }
                sent = ClientCoreLoader.SendRequest(h, reqId, data, len, offset);
            }
        } else {
            batching = false;
            kv = null;
            sent = ClientCoreLoader.SendRequest(h, reqId, data, len, offset);
        }
        if (sent) {
            return true;
        }
        if (kv != null) {
            synchronized (m_cs) {
                if (batching) {
                    m_kvBatching.clear();
                } else {
                    m_kvCallback.clear();
                }
            }
        }
        return false;
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param data A memory buffer containing request parameters
     * @param len Actual data size in bytes
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(short reqId, java.nio.ByteBuffer data, int len) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, data, len, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param data A memory buffer containing request parameters
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(short reqId, java.nio.ByteBuffer data) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, data, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    public final boolean SendRequest(short reqId, byte[] data, int len, DAsyncResultHandler ash) {
        return SendRequest(reqId, data, len, ash, null, null);
    }

    public final boolean SendRequest(short reqId, byte[] data, int len, DAsyncResultHandler ash, DDiscarded discarded) {
        return SendRequest(reqId, data, len, ash, discarded, null);
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param data An array of bytes or null
     * @param len An integer for data size in bytes
     * @param ash A callback for tracking an instance of CAsyncResult containing
     * an expected result
     * @param discarded A callback for tracking communication channel events,
     * close and cancel
     * @param exception A callback for tracking an exception from server
     * @return True if communication channel is sendable, and False if
     * communication channel is not sendable
     */
    public boolean SendRequest(short reqId, byte[] data, int len, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        SPA.CUQueue q;
        if (data == null || len <= 0) {
            q = null;
        } else {
            if (len > data.length) {
                len = data.length;
            }
            q = SPA.CScopeUQueue.Lock();
            q.Push(data, len);
        }
        boolean ok = SendRequest(reqId, q, ash, discarded, exception);
        if (q != null) {
            SPA.CScopeUQueue.Unlock(q);
        }
        return ok;
    }

    /**
     * Generate a DDiscarded callback
     *
     * @param <V> A generic type
     * @param f a future instance
     * @return a DDiscarded callback
     */
    public static <V> DDiscarded get_aborted(final UFuture<V> f) {
        return new DDiscarded() {
            @Override
            public void invoke(CAsyncServiceHandler sender, boolean discarded) {
                if (discarded) {
                    f.cancel(false);
                } else {
                    CClientSocket cs = sender.getSocket();
                    int ec = cs.getErrorCode();
                    if (cs.getErrorCode() == 0) {
                        CSocketError ex = new CSocketError(SESSION_CLOSED_AFTER, "Session closed after sending the request " + f.getMethodName(), f.getReqId(), false);
                        f.setException(ex);
                    } else {
                        CSocketError ex = new CSocketError(ec, cs.getErrorMsg(), f.getReqId(), false);
                        f.setException(ex);
                    }
                }
            }
        };
    }

    /**
     * Generate a DOnExceptionFromServer callback
     *
     * @param <V> A generic type
     * @param f a future instance
     * @return a DOnExceptionFromServer callback
     */
    public static <V> DOnExceptionFromServer get_se(final UFuture<V> f) {
        return new DOnExceptionFromServer() {
            @Override
            public void invoke(CAsyncServiceHandler sender, short reqId, String errMessage, String errWhere, int errCode) {
                SPA.CServerError ex = new SPA.CServerError(errCode, errMessage, errWhere, reqId);
                f.setException(ex);
            }
        };
    }

    private DAsyncResultHandler get_ash(final UFuture<SPA.CScopeUQueue> f) {
        DAsyncResultHandler ash = new DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                SPA.CScopeUQueue sb = new SPA.CScopeUQueue();
                sb.getUQueue().Push(ar.getUQueue());
                ar.getUQueue().SetSize(0);
                f.set(sb);
            }
        };
        return ash;
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param data An array of bytes or null
     * @param len An integer for data size in bytes
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(final short reqId, byte[] data, int len) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, data, len, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    /**
     * Throw an exception CSocketError
     *
     * @param f a future
     * @throws CSocketError if communication channel is not sendable
     */
    public void raise(IUFExtra f) throws CSocketError {
        CClientSocket cs = getSocket();
        int ec = cs.getErrorCode();
        if (ec == 0) {
            throw new CSocketError(SESSION_CLOSED_BEFORE, "Session already closed before sending the request " + f.getMethodName(), f.getReqId(), true);
        } else {
            throw new CSocketError(ec, cs.getErrorMsg(), f.getReqId(), true);
        }
    }

    /**
     * Throw an exception CSocketError
     *
     * @param mName A non-empty request method name
     * @param reqId A non-zero unique request id within an async handler
     * @throws CSocketError if communication channel is not sendable
     */
    public void raise(String mName, short reqId) throws CSocketError {
        if (mName == null || mName.length() == 0) {
            throw new IllegalArgumentException("Method name cannot be empty");
        }
        if (reqId == 0) {
            throw new IllegalArgumentException("Request id cannot be zero");
        }
        CClientSocket cs = getSocket();
        int ec = cs.getErrorCode();
        if (ec == 0) {
            throw new CSocketError(SESSION_CLOSED_BEFORE, "Session already closed before sending the request " + mName, reqId, true);
        } else {
            throw new CSocketError(ec, cs.getErrorMsg(), reqId, true);
        }
    }

    public final boolean SendRequest(short reqId, SPA.CUQueue q, DAsyncResultHandler ash) {
        return SendRequest(reqId, q, ash, null, null);
    }

    public final boolean SendRequest(short reqId, SPA.CUQueue q, DAsyncResultHandler ash, DDiscarded discarded) {
        return SendRequest(reqId, q, ash, discarded, null);
    }

    public final boolean SendRequest(short reqId, SPA.CUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        if (q == null) {
            return SendRequest(reqId, (java.nio.ByteBuffer) null, (int) 0, ash, discarded, exception);
        }
        return SendRequest(reqId, q.getIntenalBuffer(), q.GetSize(), ash, discarded, exception);
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param q A memory buffer containing request parameters
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(short reqId, SPA.CUQueue q) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, q, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    public final boolean SendRequest(short reqId, SPA.CScopeUQueue q, DAsyncResultHandler ash) {
        return SendRequest(reqId, q, ash, null, null);
    }

    public final boolean SendRequest(short reqId, SPA.CScopeUQueue q, DAsyncResultHandler ash, DDiscarded discarded) {
        return SendRequest(reqId, q, ash, discarded, null);
    }

    public final boolean SendRequest(short reqId, SPA.CScopeUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        if (q == null) {
            return SendRequest(reqId, (java.nio.ByteBuffer) null, (int) 0, ash, discarded, exception);
        }
        return SendRequest(reqId, q.getUQueue(), ash, discarded, exception);
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @param q A memory buffer containing request parameters
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(short reqId, SPA.CScopeUQueue q) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, q, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    public final boolean SendRequest(short reqId, DAsyncResultHandler ash) {
        return SendRequest(reqId, (java.nio.ByteBuffer) null, (int) 0, ash, null, null);
    }

    public final boolean SendRequest(short reqId, DAsyncResultHandler ash, DDiscarded discarded) {
        return SendRequest(reqId, (java.nio.ByteBuffer) null, (int) 0, ash, discarded, null);
    }

    public final boolean SendRequest(short reqId, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
        return SendRequest(reqId, (java.nio.ByteBuffer) null, (int) 0, ash, discarded, exception);
    }

    /**
     * Send a request onto a remote server for processing, and return a future
     * immediately without blocking
     *
     * @param reqId An unique request id within a service handler
     * @return A future for an instance of CScopeUQueue containing an expected
     * result
     * @throws CSocketError if communication channel is not sendable
     */
    public UFuture<SPA.CScopeUQueue> sendRequest(final short reqId) throws CSocketError {
        UFuture<SPA.CScopeUQueue> f = new UFuture<>("SendRequest", reqId, this);
        if (!SendRequest(reqId, get_ash(f), get_aborted(f), get_se(f))) {
            raise(f);
        }
        return f;
    }

    protected boolean SendRouteeResult(java.nio.ByteBuffer data, int len, short reqId) {
        if (data == null || len < 0) {
            len = 0;
        } else if (len > data.limit() - data.position()) {
            len = data.limit() - data.position();
        }
        long h = m_ClientSocket.getHandle();
        if (reqId == 0) {
            reqId = m_ClientSocket.getCurrentRequestID();
        }
        return ClientCoreLoader.SendRouteeResult(h, reqId, data, len, data.position());
    }

    protected boolean SendRouteeResult(java.nio.ByteBuffer data, short reqId) {
        int len;
        if (data != null) {
            len = data.limit() - data.position();
        } else {
            len = 0;
        }
        return SendRouteeResult(data, len, reqId);
    }

    protected boolean SendRouteeResult(byte[] data, int len, short reqId) {
        if (data != null && len > 0) {
            if (len > data.length) {
                len = data.length;
            }
            SPA.CUQueue q = SPA.CScopeUQueue.Lock();
            q.Push(data, len);
            return SendRouteeResult(q.getIntenalBuffer(), len, reqId);
        } else {
            return SendRouteeResult((java.nio.ByteBuffer) null, len, reqId);
        }
    }

    protected final boolean SendRouteeResult(byte[] data, int len) {
        return SendRouteeResult(data, len, (short) 0);
    }

    protected final boolean SendRouteeResult(short reqId) {
        return SendRouteeResult((java.nio.ByteBuffer) null, (int) 0, reqId);
    }

    protected final boolean SendRouteeResult() {
        return SendRouteeResult((short) 0);
    }

    protected boolean SendRouteeResult(SPA.CUQueue q, short reqId) {
        if (q == null) {
            return SendRouteeResult(reqId);
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

    public boolean Interrupt(long options) {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.SendInterruptRequest(h, options);
    }

    private boolean m_bBatching = false;

    public boolean CommitBatching(boolean bBatchingAtServerSide) {
        long h;
        synchronized (m_cs) {
            m_kvCallback.addAll(m_kvBatching);
            m_kvBatching.clear();
            h = m_ClientSocket.getHandle();
            m_bBatching = false;
            return ClientCoreLoader.CommitBatching(h, bBatchingAtServerSide);
        }
    }

    protected void OnMergeTo(CAsyncServiceHandler to) {

    }

    void AppendTo(CAsyncServiceHandler to) {
        synchronized (to.m_cs) {
            synchronized (m_cs) {
                OnMergeTo(to);
                to.m_kvCallback.addAll(m_kvCallback);
                m_kvCallback.clear();
            }
        }
    }

    public boolean StartBatching() {
        synchronized (m_cs) {
            long h = getCSHandle();
            if (h == 0) {
                return false;
            }
            m_bBatching = false;
            return ClientCoreLoader.StartBatching(h);
        }
    }

    private java.util.Map.Entry<Short, CResultCb> GetAsyncResultHandler(short reqId) {
        if (m_ClientSocket.getRandom()) {
            synchronized (m_cs) {
                for (java.util.Map.Entry<Short, CResultCb> kv : m_kvCallback) {
                    if (kv.getKey() == reqId) {
                        m_kvCallback.remove(kv);
                        return kv;
                    }
                }
            }
        } else {
            synchronized (m_cs) {
                if (!m_kvCallback.isEmpty()) {
                    java.util.Map.Entry<Short, CResultCb> first = m_kvCallback.getFirst();
                    if (first.getKey() == reqId) {
                        return m_kvCallback.pollFirst();
                    }
                    return null;
                }
            }
        }
        return null;
    }

    final void OnSE(short reqId, String errMessage, String errWhere, int errCode) {
        java.util.Map.Entry<Short, CResultCb> p = GetAsyncResultHandler(reqId);
        OnExceptionFromServer(reqId, errMessage, errWhere, errCode);
        if (p != null) {
            CResultCb rcb = p.getValue();
            if (rcb != null && rcb.ExceptionFromServer != null) {
                rcb.ExceptionFromServer.invoke(this, reqId, errMessage, errWhere, errCode);
            }
        }
        if (ServerException != null) {
            ServerException.invoke(this, reqId, errMessage, errWhere, errCode);
        }
    }

    protected void OnInterrupted(long options) {

    }

    private final CAsyncResult m_ar = new CAsyncResult(this, (short) 0, null, null);

    final void onRR(short reqId, SPA.CUQueue mc) {
        if (reqId == SPA.tagBaseRequestID.idInterrupt.getValue()) {
            long options = mc.LoadLong();
            OnInterrupted(options);
            return;
        }
        java.util.Map.Entry<Short, CResultCb> p = GetAsyncResultHandler(reqId);
        if (p != null && p.getValue() != null && p.getValue().AsyncResultHandler != null) {
            m_ar.Reset(reqId, mc, p.getValue().AsyncResultHandler);
            p.getValue().AsyncResultHandler.invoke(m_ar);
        } else if (ResultReturned != null && ResultReturned.invoke(this, reqId, mc)) {
        } else {
            OnResultReturned(reqId, mc);
        }
    }

    protected void OnResultReturned(short sRequestId, SPA.CUQueue UQueue) {

    }

    protected void OnBaseRequestProcessed(short reqId) {

    }

    protected void OnAllProcessed() {

    }

    protected void OnPostProcessing(int hint, long data) {

    }

    public final boolean getRouteeRequest() {
        long h = getCSHandle();
        if (h == 0) {
            return false;
        }
        return ClientCoreLoader.IsRouteeRequest(h);
    }

    public boolean AbortBatching() {
        synchronized (m_cs) {
            for (java.util.Map.Entry<Short, CResultCb> p : m_kvBatching) {
                CResultCb rcb = p.getValue();
                if (rcb.Discarded != null) {
                    rcb.Discarded.invoke(this, true);
                }
            }
            m_kvBatching.clear();
            m_bBatching = false;
            long h = getCSHandle();
            if (h == 0) {
                return false;
            }
            return ClientCoreLoader.AbortBatching(h);
        }
    }

    public void AbortDequeuedMessage() {
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
            for (java.util.Map.Entry<Short, CResultCb> p : m_kvBatching) {
                CResultCb rcb = p.getValue();
                if (rcb.Discarded != null) {
                    rcb.Discarded.invoke(this, getSocket().getCurrentRequestID() == SPA.tagBaseRequestID.idCancel.getValue());
                }
            }
            m_kvBatching.clear();
            for (java.util.Map.Entry<Short, CResultCb> p : m_kvCallback) {
                CResultCb rcb = p.getValue();
                if (rcb.Discarded != null) {
                    rcb.Discarded.invoke(this, getSocket().getCurrentRequestID() == SPA.tagBaseRequestID.idCancel.getValue());
                }
            }
            m_kvCallback.clear();
            return size;
        }
    }
}
