package SPA.ClientSide;

import SPA.tagOperationSystem;

public final class CClientSocket {

    CClientSocket(long hSocket) {
        m_h = hSocket;
        synchronized (m_cs) {
            m_mapSocket.put(hSocket, this);
        }
        ClientCoreLoader.SetCs(this, m_h);
    }

    public interface DOnSocketClosed {

        void invoke(CClientSocket sender, int errorCode);
    }
    public DOnSocketClosed SocketClosed = null;

    public interface DOnHandShakeCompleted {

        void invoke(CClientSocket sender, int errorCode);
    }
    public DOnHandShakeCompleted HandShakeCompleted = null;

    public interface DOnSocketConnected {

        void invoke(CClientSocket sender, int errorCode);
    }
    public DOnSocketConnected SocketConnected = null;

    public interface DOnRequestProcessed {

        void invoke(CClientSocket sender, short reqId, int len);
    }
    public DOnRequestProcessed RequestProcessed = null;

    public interface DOnBaseRequestProcessed {

        void invoke(CClientSocket sender, SPA.tagBaseRequestID reqId);
    }
    public DOnBaseRequestProcessed BaseRequestProcessed = null;

    public interface DOnServerException {

        void invoke(CClientSocket sender, short reqId, String errMessage, String errWhere, int errCode);
    }
    public DOnServerException ServerException = null;

    public interface DOnAllRequestsProcessed {

        void invoke(CClientSocket sender, short lastReqId);
    }
    public DOnAllRequestsProcessed AllRequestsProcessed = null;

    public final static int DEFAULT_RECV_TIMEOUT = 30000;
    public final static int DEFAULT_CONN_TIMEOUT = 30000;

    private final IClientQueue m_qm = new CClientQueueImpl(this);

    public final IClientQueue getClientQueue() {
        return m_qm;
    }

    public static class QueueConfigure {

        public static boolean getIsClientQueueIndexPossiblyCrashed() {
            return ClientCoreLoader.IsClientQueueIndexPossiblyCrashed();
        }

        public static String getWorkDirectory() {
            return ClientCoreLoader.GetClientWorkDirectory();
        }

        public static void setWorkDirectory(String dir) {
            byte[] bytes = {};
            if (dir != null) {
                //java.nio.charset.Charset cs = java.nio.charset.Charset.forName("US-ASCII");
                bytes = dir.getBytes();
            }
            ClientCoreLoader.SetClientWorkDirectory(bytes, bytes.length);
        }

        public static void setMessageQueuePassword(String pwd) {
            byte[] bytes = {};
            if (pwd != null) {
                bytes = pwd.getBytes();
            }
            ClientCoreLoader.SetMessageQueuePassword(bytes, bytes.length);
        }
    }

    public static class SSL {

        static {
            ClientCoreLoader.SetCertificateVerifyCallback(true);
        }

        public interface DOnCertificateVerify {

            boolean verify(boolean preverified, int depth, int errorCode, String errMessage, CertInfo ci);
        }
        public static DOnCertificateVerify CertificateVerify;

        static boolean Verify(boolean preverified, int depth, int errorCode, String errMessage, Object ci) {
            if (CertificateVerify != null) {
                return CertificateVerify.verify(preverified, depth, errorCode, errMessage, (CertInfo) ci);
            }
            return true;
        }

        public static boolean SetVerifyLocation(String certFile) {
            byte[] cf = {};
            if (certFile != null) {
                //java.nio.charset.Charset cs = java.nio.charset.Charset.forName("US-ASCII");
                cf = certFile.getBytes();
            }
            return ClientCoreLoader.SetVerifyLocation(cf, cf.length);
        }
    }

    private final CPushImpl m_PushImpl = new CPushImpl(this);
    private volatile IUcert m_cert = null;

    public final IUcert getUCert() {
        return m_cert;
    }

    public final CPushImpl getPush() {
        return m_PushImpl;
    }

    void Detach(CAsyncServiceHandler ash) {
        if (ash == null) {
            return;
        }
        m_lstAsh.remove(ash);
        ash.SetNull();
    }

    boolean Attach(CAsyncServiceHandler ash) {
        if (ash == null) {
            return false;
        }
        for (CAsyncServiceHandler h : m_lstAsh) {
            if (ash.getSvsID() == h.getSvsID()) {
                return false;
            }
        }
        m_lstAsh.add(ash);
        return true;
    }

    public boolean getSendable() {
        return (ClientCoreLoader.IsOpened(m_h) || ClientCoreLoader.IsQueueStarted(m_h));
    }

    public final long getSslHandle() {
        return ClientCoreLoader.GetSSL(m_h);
    }

    public final CConnectionContext getConnectionContext() {
        return ConnectionContext;
    }

    public final SPA.tagZipLevel getZipLevel() {
        return SPA.tagZipLevel.forValue(ClientCoreLoader.GetZipLevel(m_h));
    }

    public final void setZipLevel(SPA.tagZipLevel zl) {
        if (zl == null) {
            zl = SPA.tagZipLevel.zlDefault;
        }
        ClientCoreLoader.SetZipLevel(m_h, zl.getValue());
    }

    public final boolean getAutoConn() {
        return ClientCoreLoader.GetAutoConn(m_h);
    }

    public final void setAutoConn(boolean autoConn) {
        ClientCoreLoader.SetAutoConn(m_h, autoConn);
    }

    public final boolean getBatching() {
        return ClientCoreLoader.IsBatching(m_h);
    }

    public final void setPassword(String pwd) {
        ClientCoreLoader.SetPassword(m_h, pwd);
    }

    public final boolean getConnected() {
        return ClientCoreLoader.IsOpened(m_h);
    }

    public boolean TurnOnZipAtSvr(boolean enableZip) {
        return ClientCoreLoader.TurnOnZipAtSvr(m_h, enableZip);
    }

    public boolean SetZipLevelAtSvr(SPA.tagZipLevel zl) {
        if (zl == null) {
            zl = SPA.tagZipLevel.zlDefault;
        }
        return ClientCoreLoader.SetZipLevelAtSvr(m_h, zl.getValue());
    }

    public final long getHandle() {
        return m_h;
    }

    public final long getSocketNativeHandle() {
        return ClientCoreLoader.GetSocketNativeHandle(m_h);
    }

    public final short getServerPingTime() {
        return ClientCoreLoader.GetServerPingTime(m_h);
    }

    public final boolean getDequeuedMessageAborted() {
        return ClientCoreLoader.IsDequeuedMessageAborted(m_h);
    }

    private volatile boolean m_bRouting = false;

    public final boolean getRouting() {
        return m_bRouting;
    }

    public static String getVersion() {
        return ClientCoreLoader.GetClientWorkDirectory();
    }

    private static void OnAllRequestsProcessed(long h, short reqId) {
        CClientSocket cs = Find(h);
        CAsyncServiceHandler ash = cs.Seek(cs.getCurrentServiceID());
        if (ash != null) {
            ash.OnAllProcessed();
        }
        if (cs.AllRequestsProcessed != null) {
            cs.AllRequestsProcessed.invoke(cs, reqId);
        }
    }

    /**
     * Use the method for debugging crash within cross development environments.
     *
     * @param str A string will be sent to client core library to be output into
     * a crash text file
     */
    public static void SetLastCallInfo(String str) {
        byte[] bytes;
        try {
            bytes = str.getBytes("UTF-8");
        } catch (java.io.UnsupportedEncodingException err) {
            bytes = new byte[0];
        }
        ClientCoreLoader.SetLastCallInfo(bytes, bytes.length);
    }

    private static void OnSocketClosed(long h, int errCode) {
        CClientSocket cs = Find(h);
        CAsyncServiceHandler ash = cs.Seek(cs.getCurrentServiceID());
        if (ash != null && !cs.getSendable()) {
            ash.CleanCallbacks();
        }
        if (cs.SocketClosed != null) {
            cs.SocketClosed.invoke(cs, errCode);
        }
    }

    private static void OnHandShakeCompleted(long h, int errCode) {
        CClientSocket cs = Find(h);
        if (cs.HandShakeCompleted != null) {
            cs.HandShakeCompleted.invoke(cs, errCode);
        }
    }

    private static void OnSocketConnected(long h, int errCode) {
        CClientSocket cs = Find(h);
        if (errCode == 0 && ClientCoreLoader.GetSSL(h) != 0) {
            cs.m_cert = new CUCertImpl(cs);
        } else {
            cs.m_cert = null;
        }
        if (cs.SocketConnected != null) {
            cs.SocketConnected.invoke(cs, errCode);
        }
    }

    private static void OnRequestProcessed(long h, short reqId, int len, byte[] bytes, byte os, boolean endian) {
        CClientSocket cs = Find(h);
        CAsyncServiceHandler ash = cs.Seek(cs.getCurrentServiceID());
        if (ash != null) {
            if (len != 0 && bytes.length != len) {
                if (bytes.length == 0) {
                    return; //socket closed
                }
                //should never come here!
                String msg = String.format("Wrong number of bytes retrieved (expected = {0} and obtained = {1})", len, bytes.length);
                throw new java.lang.UnsupportedOperationException(msg);
            }
            //this does not cause re-allocting bytes memory
            SPA.CUQueue q = new SPA.CUQueue(bytes);

            q.setOS(tagOperationSystem.forValue(os));
            q.setEndian(endian);
            ash.onRR(reqId, q);
        }
        if (cs.RequestProcessed != null) {
            cs.RequestProcessed.invoke(cs, reqId, len);
        }
    }

    private static void OnBaseRequestProcessed(long h, short reqId) {
        CClientSocket cs = Find(h);
        if (reqId == SPA.tagBaseRequestID.idSwitchTo.getValue()) {
            cs.m_bRandom = ClientCoreLoader.IsRandom(h);
            cs.m_nCurrentServiceId = ClientCoreLoader.GetCurrentServiceId(h);
            cs.m_bRouting = ClientCoreLoader.IsRouting(h);
            boolean[] endian = {false};
            cs.m_os = SPA.tagOperationSystem.forValue(ClientCoreLoader.GetPeerOs(h, endian, 1));
            cs.m_bBigEndian = endian[0];
        }
        CAsyncServiceHandler ash = cs.Seek(cs.getCurrentServiceID());
        if (ash != null) {
            if (ash.BaseRequestProcessed != null) {
                ash.BaseRequestProcessed.invoke(ash, reqId);
            }
            ash.OnBaseRequestProcessed(reqId);
            if (reqId == SPA.tagBaseRequestID.idCancel.getValue()) {
                ash.CleanCallbacks();
            }
        }
        if (cs.BaseRequestProcessed != null) {
            cs.BaseRequestProcessed.invoke(cs, SPA.tagBaseRequestID.forValue(reqId));
        }
    }

    private static void OnServerException(long h, short reqId, String errMessage, String errWhere, int errCode) {
        CClientSocket cs = Find(h);
        CAsyncServiceHandler ash = cs.Seek(cs.getCurrentServiceID());
        if (ash != null) {
            ash.OnSE(reqId, errMessage, errWhere, errCode);
        }
        if (cs.ServerException != null) {
            cs.ServerException.invoke(cs, reqId, errMessage, errWhere, errCode);
        }
    }

    private static void OnEnter(long h, Object sender, int[] groups) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnSubscribe != null) {
            cs.m_PushImpl.OnSubscribe.invoke(cs, (CMessageSender) sender, groups);
        }
    }

    private static void OnExit(long h, Object sender, int[] groups) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnUnsubscribe != null) {
            cs.m_PushImpl.OnUnsubscribe.invoke(cs, (CMessageSender) sender, groups);
        }
    }

    private static void OnSendUserMessage(long h, Object sender, byte[] message) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnSendUserMessage != null) {
            SPA.CUQueue q = SPA.CScopeUQueue.Lock();
            q.Push(message);
            Object obj = q.LoadObject();
            cs.m_PushImpl.OnSendUserMessage.invoke(cs, (CMessageSender) sender, obj);
            SPA.CScopeUQueue.Unlock(q);
        }
    }

    private static void OnSendUserMessageEx(long h, Object sender, byte[] message) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnSendUserMessageEx != null) {
            cs.m_PushImpl.OnSendUserMessageEx.invoke(cs, (CMessageSender) sender, message);
        }
    }

    private static void OnSpeakEx(long h, Object sender, int[] groups, byte[] message) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnPublishEx != null) {
            cs.m_PushImpl.OnPublishEx.invoke(cs, (CMessageSender) sender, groups, message);
        }
    }

    private static void OnSpeak(long h, Object sender, int[] groups, byte[] message) {
        CClientSocket cs = Find(h);
        if (cs.m_PushImpl.OnPublish != null) {
            SPA.CUQueue q = SPA.CScopeUQueue.Lock();
            q.Push(message);
            Object obj = q.LoadObject();
            CMessageSender ms = (CMessageSender) sender;
            cs.m_PushImpl.OnPublish.invoke(cs, ms, groups, obj);
            SPA.CScopeUQueue.Unlock(q);
        }
    }

    public final int getRouteeCount() {
        return ClientCoreLoader.GetRouteeCount(m_h);
    }

    public final void Shutdown() {
        ClientCoreLoader.Shutdown(m_h, SPA.tagShutdownType.stBoth.getValue());
    }

    public void Shutdown(SPA.tagShutdownType st) {
        if (st == null) {
            st = SPA.tagShutdownType.stBoth;
        }
        ClientCoreLoader.Shutdown(m_h, st.getValue());
    }

    private volatile boolean m_bRandom = false;

    public final boolean getRandom() {
        return m_bRandom;
    }

    private volatile SPA.tagOperationSystem m_os = SPA.CUQueue.DEFAULT_OS;
    private volatile boolean m_bBigEndian = false;

    public final SPA.tagOperationSystem GetPeerOs() {
        return m_os;
    }

    public final SPA.tagOperationSystem GetPeerOs(SPA.RefObject<Boolean> bigEndian) {
        if (bigEndian != null) {
            bigEndian.Value = m_bBigEndian;
        }
        return m_os;
    }

    public boolean DoEcho() {
        return ClientCoreLoader.DoEcho(m_h);
    }

    public final void Close() {
        ClientCoreLoader.Close(m_h);
    }

    public final boolean WaitAll() {
        return WaitAll(-1);
    }

    public final boolean WaitAll(int timeOut) {
        if (ClientCoreLoader.IsBatching(m_h)) {
            throw new java.lang.UnsupportedOperationException("Can't call the method WaitAll during batching requests");
        }
        if (ClientCoreLoader.IsQueueStarted(m_h) && ClientCoreLoader.GetJobSize(m_h) > 0) {
            throw new UnsupportedOperationException("Can't call the method WaitAll during enqueuing transactional requests");
        }
        return ClientCoreLoader.WaitAll(m_h, timeOut);
    }

    public final boolean Cancel() {
        if (ClientCoreLoader.IsBatching(m_h)) {
            throw new UnsupportedOperationException("Can't call the method Cancel during batching requests");
        }
        return ClientCoreLoader.Cancel(m_h, -1);
    }

    public final int getBytesBatched() {
        return ClientCoreLoader.GetBytesBatched(m_h);
    }

    public final int getBytesInReceivingBuffer() {
        return ClientCoreLoader.GetBytesInReceivingBuffer(m_h);
    }

    public final int getBytesInSendingBuffer() {
        return ClientCoreLoader.GetBytesInSendingBuffer(m_h);
    }

    public final void AbortDequeuedMessage() {
        ClientCoreLoader.AbortDequeuedMessage(m_h);
    }

    public long getBytesReceived() {
        return ClientCoreLoader.GetBytesReceived(m_h);
    }

    public final int getConnectingTimeout() {
        return ClientCoreLoader.GetConnTimeout(m_h);
    }

    public final void setConnectingTimeout(int timeout) {
        ClientCoreLoader.SetConnTimeout(m_h, timeout);
    }

    public final String GetPeerName(SPA.RefObject<Integer> port) {
        int[] p = {0};
        String s = ClientCoreLoader.GetPeerName(m_h, p, 1);
        if (port != null) {
            port.Value = p[0];
        }
        return s;
    }

    public final SPA.tagEncryptionMethod getEncryptionMethod() {
        return SPA.tagEncryptionMethod.forValue(ClientCoreLoader.GetEncryptionMethod(m_h));
    }

    public final void setEncryptionMethod(SPA.tagEncryptionMethod em) {
        if (em == null) {
            em = SPA.tagEncryptionMethod.NoEncryption;
        }
        ClientCoreLoader.SetEncryptionMethod(m_h, em.getValue());
    }

    public final tagConnectionState getConnectionState() {
        return tagConnectionState.forValue(ClientCoreLoader.GetConnectionState(m_h));
    }

    public final int getCurrentResultSize() {
        return ClientCoreLoader.GetCurrentResultSize(m_h);
    }

    public int GetCurrentServiceID() {
        return ClientCoreLoader.GetCurrentServiceId(m_h);
    }

    public final String GetPeerName() {
        int[] p = {0};
        return ClientCoreLoader.GetPeerName(m_h, p, 1);
    }

    public final String getErrorMsg() {
        return ClientCoreLoader.GetErrorMessage(m_h);
    }

    public final int getErrorCode() {
        return ClientCoreLoader.GetErrorCode(m_h);
    }

    public final int getReceivingTimeout() {
        return ClientCoreLoader.GetRecvTimeout(m_h);
    }

    public final int getCountOfRequestsInQueue() {
        return ClientCoreLoader.GetCountOfRequestsQueued(m_h);
    }

    public final long getBytesSent() {
        return ClientCoreLoader.GetBytesSent(m_h);
    }

    public final void setReceivingTimeout(int timeout) {
        ClientCoreLoader.SetRecvTimeout(m_h, timeout);
    }

    public final String getUID() {
        char[] s = new char[256];
        int len = ClientCoreLoader.GetUID(m_h, s, 256);
        return new String(s, 0, len);
    }

    public final void setUID(String uid) {
        if (uid == null) {
            uid = "";
        }
        ClientCoreLoader.SetUserID(m_h, uid);
    }

    public final boolean getZip() {
        return ClientCoreLoader.GetZip(m_h);
    }

    public final void setZip(boolean zip) {
        ClientCoreLoader.SetZip(m_h, zip);
    }

    CAsyncServiceHandler Seek(int svsId) {
        for (CAsyncServiceHandler ash : m_lstAsh) {
            if (ash.getSvsID() == svsId) {
                return ash;
            }
        }
        return null;
    }

    public final CAsyncServiceHandler getCurrentHandler() {
        return Seek(getCurrentServiceID());
    }

    private volatile int m_nCurrentServiceId = SPA.BaseServiceID.sidStartup;

    public final int getCurrentServiceID() {
        return m_nCurrentServiceId;
    }

    public short getCurrentRequestID() {
        return ClientCoreLoader.GetCurrentRequestID(m_h);
    }
    private final long m_h;
    volatile CConnectionContext ConnectionContext;
    private final java.util.ArrayList<CAsyncServiceHandler> m_lstAsh = new java.util.ArrayList<>();

    //
    private static CClientSocket Find(long hSocket) {
        synchronized (m_cs) {
            if (m_mapSocket.containsKey(hSocket)) {
                return m_mapSocket.get(hSocket);
            }
        }
        return null;
    }

    static void Remove(long hSocket) {
        synchronized (m_cs) {
            m_mapSocket.remove(hSocket);
        }
    }
    private static final Object m_cs = new Object();
    private static final java.util.HashMap<Long, CClientSocket> m_mapSocket = new java.util.HashMap<>(); //locked by m_cs
}
