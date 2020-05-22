package SPA.ServerSide;

public class CHttpPeerBase extends CSocketPeer {

    @Override
    protected final void OnPublishEx(int[] groups, byte[] message) {
    }

    @Override
    protected final void OnSendUserMessageEx(String receiver, byte[] message) {
    }

    @Override
    protected final void OnFastRequestArrive(short reqId, int len) {
        Process(reqId, len);
    }

    @Override
    protected final int OnSlowRequestArrive(short reqId, int len) {
        Process(reqId, len);
        return 0;
    }

    class CHttpPushImpl implements SPA.IUPush {

        public CSocketPeer m_sp;

        public CHttpPushImpl(CSocketPeer sp) {
            m_sp = sp;
        }

        @Override
        public final boolean Publish(Object Message, int[] Groups) {
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            SPA.CUQueue q = SPA.CScopeUQueue.Lock();
            q.Save(Message);
            boolean ok = ServerCoreLoader.Speak(m_sp.getHandle(), q.GetBuffer(), q.GetSize(), Groups, len);
            SPA.CScopeUQueue.Unlock(q);
            return ok;
        }

        @Override
        public final boolean Subscribe(int[] Groups) {
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            return ServerCoreLoader.Enter(m_sp.getHandle(), Groups, len);
        }

        @Override
        public final boolean Unsubscribe() {
            ServerCoreLoader.Exit(m_sp.getHandle());
            return true;
        }

        @Override
        public final boolean SendUserMessage(Object Message, String UserId) {
            SPA.CUQueue q = SPA.CScopeUQueue.Lock();
            q.Save(Message);
            boolean ok = ServerCoreLoader.SendUserMessage(m_sp.getHandle(), UserId, q.GetBuffer(), q.GetSize());
            SPA.CScopeUQueue.Unlock(q);
            return ok;
        }
    }

    private final CHttpPushImpl m_push;

    protected CHttpPeerBase() {
        m_push = new CHttpPushImpl(this);
    }

    public final SPA.IUPush getPush() {
        return m_push;
    }

    protected boolean DoAuthentication(String userId, String password) {
        return true;
    }

    private static boolean DoAuthentication(long hSocket, String userId, String password) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return true;
        }
        return ((CHttpPeerBase) sp).DoAuthentication(userId, password);
    }

    protected void OnPost() {

    }

    protected void OnGet() {

    }

    protected void OnUserRequest() {

    }

    protected void OnDelete() {

    }

    protected void OnPut() {

    }

    protected void OnHead() {

    }

    protected void OnOptions() {

    }

    protected void OnTrace() {

    }

    protected void OnMultiPart() {

    }

    protected void OnConnect() {

    }

    private void Process(short reqId, int len) {
        SPA.ServerSide.tagHttpRequestID hid = SPA.ServerSide.tagHttpRequestID.forValue(reqId);
        switch (hid) {
            case idGet:
                OnGet();
                break;
            case idPost:
                OnPost();
                break;
            case idUserRequest:
                OnUserRequest();
                break;
            case idDelete:
                OnDelete();
                break;
            case idHead:
                OnHead();
                break;
            case idMultiPart:
                OnMultiPart();
                break;
            case idOptions:
                OnOptions();
                break;
            case idPut:
                OnPut();
                break;
            case idTrace:
                OnTrace();
                break;
            case idConnect:
                OnConnect();
                break;
            default:
                SetResponseCode(501);
                SendResult("Method not implemented");
                break;
        }
    }

    public final boolean DoHttpAuth(String userId, String password) {
        return DoAuthentication(userId, password);
    }

    public final int SendResult(String res) {
        Clear();
        if (res == null) {
            res = "";
        }
        byte[] bytes = res.getBytes(java.nio.charset.Charset.forName("UTF-8"));
        return ServerCoreLoader.SendHTTPReturnDataA(getHandle(), bytes, bytes.length);
    }

    private void Clear() {
        if (m_ReqHeaders != null && !ServerCoreLoader.IsWebSocket(getHandle())) {
            m_ReqHeaders = null;
        }
    }

    public final boolean DownloadFile(String filePath) {
        Clear();
        byte[] path = {};
        if (filePath != null) {
            path = filePath.getBytes();
        }
        return ServerCoreLoader.DownloadFile(getHandle(), path, path.length);
    }

    /**
     * Begin to send HTTP result in chunk
     *
     * @return The data size in bytes The method EndChunkResponse should be
     * called at the end after this method is called
     */
    public final int StartChunkResponse() {
        return ServerCoreLoader.StartHTTPChunkResponse(getHandle());
    }

    /**
     * Send a chunk of data after calling the method StartChunkResponse
     *
     * @param buffer A buffer data
     * @return The data size in byte You must call the method StartChunkResponse
     * before calling this method
     */
    public final int SendChunk(byte[] buffer) {
        if (buffer == null || buffer.length == 0) {
            return 0;
        }
        return ServerCoreLoader.SendHTTPChunk(getHandle(), buffer, (int) buffer.length);
    }

    /**
     * Send the last chunk of data and indicate the HTTP response is ended
     *
     * @param buffer The last chunk of data
     * @return The data size in byte You must call the method StartChunkResponse
     * before calling this method
     */
    public final int EndChunkResponse(byte[] buffer) {
        int len;
        if (buffer == null) {
            len = 0;
        } else {
            len = (int) buffer.length;
        }
        Clear();
        return ServerCoreLoader.EndHTTPChunkResponse(getHandle(), buffer, len);
    }

    public final String getRequestName() {
        return m_WebRequestName;
    }

    public final Object[] getArgs() {
        return m_vArg;
    }

    public final boolean getAuthenticated() {
        return m_bHttpOk;
    }

    public final String getPath() {
        return ServerCoreLoader.GetHTTPPath(getHandle());
    }

    public final long getContentLength() {
        return ServerCoreLoader.GetHTTPContentLength(getHandle());
    }

    public final String getQuery() {
        return ServerCoreLoader.GetHTTPQuery(getHandle());
    }

    public final tagHttpMethod getHttpMethod() {
        return tagHttpMethod.forValue(ServerCoreLoader.GetHTTPMethod(getHandle()));
    }

    public final boolean getIsWebSocket() {
        return ServerCoreLoader.IsWebSocket(getHandle());
    }

    public final boolean getIsCrossDomain() {
        return ServerCoreLoader.IsCrossDomain(getHandle());
    }

    public final double getVersion() {
        return ServerCoreLoader.GetHTTPVersion(getHandle());
    }

    public final String getHost() {
        return ServerCoreLoader.GetHTTPHost(getHandle());
    }

    public final tagTransport getTransport() {
        return tagTransport.forValue(ServerCoreLoader.GetHTTPTransport(getHandle()));
    }

    public final tagTransferEncoding getTransferEncoding() {
        return tagTransferEncoding.forValue(ServerCoreLoader.GetHTTPTransferEncoding(getHandle()));
    }

    public final tagContentMultiplax getContentMultiplax() {
        return tagContentMultiplax.forValue(ServerCoreLoader.GetHTTPContentMultiplax(getHandle()));
    }

    public final String getId() {
        return ServerCoreLoader.GetHTTPId(getHandle());
    }

    public final java.util.Map<String, String> getCurrentMultiplaxHeaders() {
        java.util.TreeMap<String, String> map = new java.util.TreeMap<>(String.CASE_INSENSITIVE_ORDER);
        CHttpHeaderValue[] hvs = (CHttpHeaderValue[]) ServerCoreLoader.GetHTTPCurrentMultiplaxHeaders(getHandle());
        for (CHttpHeaderValue hv : hvs) {
            map.put(hv.Header, hv.Value);
        }
        return map;
    }

    public final boolean SetResponseCode(int HttpCode) {
        return ServerCoreLoader.SetHTTPResponseCode(getHandle(), HttpCode);
    }

    public final boolean SetResponseHeader(String uft8Header, String utf8Value) {
        if (uft8Header == null) {
            uft8Header = "";
        }
        if (utf8Value == null) {
            utf8Value = "";
        }
        return ServerCoreLoader.SetHTTPResponseHeader(getHandle(), uft8Header.getBytes(), utf8Value.getBytes());
    }

    private volatile java.util.TreeMap<String, String> m_ReqHeaders = null;

    public final java.util.Map<String, String> getRequestHeaders() {
        if (m_ReqHeaders != null) {
            return m_ReqHeaders;
        }
        CHttpHeaderValue[] hvs = (CHttpHeaderValue[]) ServerCoreLoader.GetHTTPRequestHeaders(getHandle());
        for (CHttpHeaderValue hv : hvs) {
            m_ReqHeaders.put(hv.Header, hv.Value);
        }
        return m_ReqHeaders;
    }

    volatile String m_WebRequestName = "";
    volatile boolean m_bHttpOk = false;
    Object[] m_vArg = {};
}
