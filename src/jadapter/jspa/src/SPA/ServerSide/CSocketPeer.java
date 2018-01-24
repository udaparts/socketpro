package SPA.ServerSide;

public abstract class CSocketPeer {

    private volatile SPA.CUQueue m_qBuffer = new SPA.CUQueue();
    volatile long m_sh = 0;
    private boolean m_bRandom = false;
    CBaseService m_Service;
    public static final int SOCKET_NOT_FOUND = -1;
    public static final int REQUEST_CANCELED = -2;
    public static final int BAD_OPERATION = -3;
    public static final int RESULT_SENDING_FAILED = -4;

    protected boolean getRandom() {
        return m_bRandom;
    }

    static String getStack(StackTraceElement[] stes) {
        String s = "";
        for (StackTraceElement e : stes) {
            if (s.length() > 0) {
                s += System.lineSeparator();
            }
            s += e.getClassName();
            s += "|";
            s += e.getMethodName();
            s += "|";
            s += e.getLineNumber();
        }
        return s;
    }

    CSocketPeer() {
    }

    protected abstract void OnPublishEx(int[] groups, byte[] message);

    protected abstract void OnSendUserMessageEx(String receiver, byte[] message);

    protected abstract void OnFastRequestArrive(short requestId, int len);

    protected abstract int OnSlowRequestArrive(short requestId, int len);

    private void OnClosed(int errCode) {
        try {
            m_Service.ReleasePeer(getHandle(), true, errCode);
        } finally {
        }
    }

    protected volatile SPA.tagOperationSystem m_os = SPA.CUQueue.DEFAULT_OS;
    protected volatile boolean m_endian = false;

    static CSocketPeer findPeer(long hSocket) {
        CBaseService bs = CBaseService.SeekService(hSocket);
        if (bs == null) {
            return null;
        }
        return bs.Seek(hSocket);
    }

    private static void OnSwitch(long h, int oldServiceId, int newServiceId) {
        CBaseService bsOld;
        try {
            if (oldServiceId != SPA.BaseServiceID.sidStartup && (bsOld = CBaseService.SeekService(oldServiceId)) != null) {
                bsOld.ReleasePeer(h, false, newServiceId);
            }
            CBaseService bsNew = CBaseService.SeekService(newServiceId);
            if (bsNew != null) {
                CSocketPeer sp = bsNew.CreatePeer(h);
                sp.m_nSvsID = newServiceId;
                if (newServiceId == SPA.BaseServiceID.sidHTTP) {
                    CHttpPeerBase hp = (CHttpPeerBase) sp;
                    hp.m_bHttpOk = false;
                } else {
                    boolean endian[] = {false};
                    sp.m_os = SPA.tagOperationSystem.forValue(ServerCoreLoader.GetPeerOs(h, endian, 1));
                    sp.m_qBuffer.setOS(sp.m_os);
                    sp.m_endian = endian[0];
                    sp.m_qBuffer.setEndian(endian[0]);
                    sp.m_bRandom = bsNew.getReturnRandom();
                }
                sp.OnSwitchFrom(oldServiceId);
            }
        } catch (InstantiationException | IllegalAccessException err) {
            ServerCoreLoader.SendExceptionResult(h, err.getMessage(), getStack(err.getStackTrace()).getBytes(), SPA.tagBaseRequestID.idSwitchTo.getValue(), 0);
        }
    }

    private void OnFast(short reqId, int len) {
        try {
            if (m_Service.m_dicMethod.containsKey(reqId)) {
                java.lang.reflect.Method m = m_Service.m_dicMethod.get(reqId);
                Object[] args = getArgs(m);
                Object res = m.invoke(this, args); //<== linux may crash here with /lib/x86_64-linux-gnu/libpthread.so.0(+0x10340) right before processing a user request
                Class<?> rt = m.getReturnType();
                SendReturnResult(reqId, rt, res);
            } else {
                OnFastRequestArrive(reqId, len);
            }
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException | java.lang.reflect.InvocationTargetException err) {
            ServerCoreLoader.SendExceptionResult(m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), reqId, 0);
        }
    }

    private Object[] getArgs(java.lang.reflect.Method m) throws InstantiationException, IllegalAccessException {
        int n = 0;
        Class<?>[] ptypes = m.getParameterTypes();
        Object[] args = new Object[ptypes.length];
        for (Class<?> pt : ptypes) {
            String name = pt.getName();
            switch (name) {
                case "byte":
                case "java.lang.Byte":
                    args[n] = m_qBuffer.LoadByte();
                    break;
                case "[B":
                    args[n] = m_qBuffer.LoadBytes();
                    break;
                case "boolean":
                case "java.lang.Boolean":
                    args[n] = m_qBuffer.LoadBoolean();
                    break;
                case "java.lang.short":
                case "java.lang.Short":
                    args[n] = m_qBuffer.LoadShort();
                    break;
                case "int":
                case "java.lang.Integer":
                    args[n] = m_qBuffer.LoadInt();
                    break;
                case "float":
                case "java.lang.Float":
                    args[n] = m_qBuffer.LoadFloat();
                    break;
                case "long":
                case "java.lang.Long":
                    args[n] = m_qBuffer.LoadLong();
                    break;
                case "double":
                case "java.lang.Double":
                    args[n] = m_qBuffer.LoadDouble();
                    break;
                case "java.util.Date":
                    args[n] = m_qBuffer.LoadDate();
                    break;
                case "java.util.java.util.UUID":
                    args[n] = m_qBuffer.LoadUUID();
                    break;
                case "java.math.BigDecimal":
                    args[n] = m_qBuffer.LoadDecimal();
                    break;
                case "java.lang.String":
                    args[n] = m_qBuffer.LoadString();
                    break;
                case "java.lang.Object":
                    args[n] = m_qBuffer.LoadObject();
                    break;
                default: {
                    SPA.IUSerializer s = (SPA.IUSerializer) pt.newInstance();
                    s.LoadFrom(m_qBuffer);
                    args[n] = s;
                }
                break;
            }
            ++n;
        }
        return args;
    }

    private void SendReturnResult(short reqId, Class<?> pt, Object res) {
        SPA.CUQueue q = SPA.CScopeUQueue.Lock();
        String name = pt.getName();
        switch (name) {
            case "byte":
            case "java.lang.Byte":
                q.Save((byte) res);
                break;
            case "[B":
                q.Save((byte[]) res);
                break;
            case "boolean":
            case "java.lang.Boolean":
                q.Save((boolean) res);
                break;
            case "char":
            case "java.lang.Character":
                q.Save((char) res);
                break;
            case "short":
            case "java.lang.Short":
                q.Save((short) res);
                break;
            case "int":
            case "java.lang.Integer":
                q.Save((int) res);
                break;
            case "float":
            case "java.lang.Float":
                q.Save((float) res);
                break;
            case "long":
            case "java.lang.Long":
                q.Save((long) res);
                break;
            case "double":
            case "java.lang.Double":
                q.Save((double) res);
                break;
            case "java.util.Date":
                q.Save((java.util.Date) res);
                break;
            case "java.sql.Timestamp":
                q.Save((java.sql.Timestamp) res);
                break;
            case "java.util.UUID":
                q.Save((java.util.UUID) res);
                break;
            case "java.math.BigDecimal":
                q.Save((java.math.BigDecimal) res);
                break;
            case "java.lang.String":
                q.Save((String) res);
                break;
            case "java.lang.Object":
                q.Save((Object) res);
                break;
            case "void":
                break;
            case "SPA.CUQueue":
                SPA.CUQueue r = (SPA.CUQueue) res;
                if (r != null) {
                    SPA.CScopeUQueue.Unlock(q);
                    ServerCoreLoader.SendReturnData(m_sh, reqId, r.GetSize(), r.getIntenalBuffer());
                    return;
                }
                break;
            case "SPA.CScopeUQueue": {
                SPA.CScopeUQueue scope = (SPA.CScopeUQueue) res;
                if (scope != null) {
                    SPA.CScopeUQueue.Unlock(q);
                    ServerCoreLoader.SendReturnData(m_sh, reqId, scope.getUQueue().GetSize(), scope.getUQueue().getIntenalBuffer());
                    scope.Clean();
                    return;
                }
            }
            break;
            default:
                q.Save((SPA.IUSerializer) res);
                break;
        }
        ServerCoreLoader.SendReturnData(m_sh, reqId, q.GetSize(), q.getIntenalBuffer());
        SPA.CScopeUQueue.Unlock(q);
    }

    private int OnSlow(short reqId, int len) {
        try {
            if (m_Service.m_dicMethod.containsKey(reqId)) {
                java.lang.reflect.Method m = m_Service.m_dicMethod.get(reqId);
                Object[] args = getArgs(m);
                Object res = m.invoke(this, args); //<== linux may crash here with /lib/x86_64-linux-gnu/libpthread.so.0(+0x10340) right before processing a user request
                Class<?> rt = m.getReturnType();
                SendReturnResult(reqId, rt, res);
                return 0;
            }
            return OnSlowRequestArrive(reqId, len);
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException | java.lang.reflect.InvocationTargetException err) {
            ServerCoreLoader.SendExceptionResult(m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), reqId, 0);
        }
        return -1;
    }

    private volatile short m_sCurrentRequestId = 0;

    public final short getCurrentRequestID() {
        return m_sCurrentRequestId;
    }

    public final int getCurrentRequestLen() {
        return ServerCoreLoader.GetCurrentRequestLen(m_sh);
    }

    public final long getBytesReceived() {
        return ServerCoreLoader.GetBytesReceived(m_sh);
    }

    public final long getBytesSent() {
        return ServerCoreLoader.GetBytesSent(m_sh);
    }

    public final long getCurrentRequestIndex() {
        return ServerCoreLoader.GetCurrentRequestIndex(m_sh);
    }

    public final int getCountOfJoinedChatGroups() {
        return ServerCoreLoader.GetCountOfJoinedChatGroups(m_sh);
    }

    public final int getErrorCode() {
        return ServerCoreLoader.GetServerSocketErrorCode(m_sh);
    }

    public final String getErrorMessage() {
        byte[] errMsg = new byte[4097];
        int res = ServerCoreLoader.GetServerSocketErrorMessage(m_sh, errMsg, 4097);
        return new String(errMsg, 0, res);
    }

    public final String GetPeerName(SPA.RefObject<Integer> port) {
        int[] myPort = {0};
        String addr = ServerCoreLoader.GetPeerName(m_sh, myPort, 1);
        if (port != null) {
            port.Value = myPort[0];
        }
        return addr;
    }

    public final String GetPeerName() {
        return GetPeerName(null);
    }

    public final int getBytesInReceivingBuffer() {
        return ServerCoreLoader.GetRcvBytesInQueue(m_sh);
    }

    public final int getBytesInSendingBuffer() {
        return ServerCoreLoader.GetSndBytesInQueue(m_sh);
    }

    public static long getRequestCount() {
        return ServerCoreLoader.GetRequestCount();
    }

    public final long getSocketNativeHandle() {
        return ServerCoreLoader.GetSocketNativeHandle(m_sh);
    }

    public final CBaseService getBaseService() {
        return m_Service;
    }

    protected final SPA.CUQueue getUQueue() {
        return m_qBuffer;
    }

    public final long getHandle() {
        return m_sh;
    }

    public final long getSSL() {
        return ServerCoreLoader.GetSSL(m_sh);
    }

    public final String getUID() {
        return CSocketProServer.CredentialManager.GetUserID(m_sh);
    }

    public final void setUID(String value) {
        ServerCoreLoader.SetUserID(m_sh, value);
    }

    private int m_nSvsID = SPA.BaseServiceID.sidStartup;

    public final int getSvsID() {
        return m_nSvsID;
    }

    public final boolean getBatching() {
        return ServerCoreLoader.IsBatching(m_sh);
    }

    public final boolean getIsCanceled() {
        return ServerCoreLoader.IsCanceled(m_sh);
    }

    public final boolean getIsFakeRequest() {
        return ServerCoreLoader.IsFakeRequest(m_sh);
    }

    public static boolean getIsMainThread() {
        return ServerCoreLoader.IsMainThread();
    }

    public final boolean getConnected() {
        return ServerCoreLoader.IsOpened(m_sh);
    }

    public final void DropCurrentSlowRequest() {
        ServerCoreLoader.DropCurrentSlowRequest(m_sh);
    }

    public final void Close() {
        ServerCoreLoader.Close(m_sh);
    }

    public final void PostClose() {
        ServerCoreLoader.PostClose(m_sh, 0);
    }

    public final void PostClose(int errCode) {
        ServerCoreLoader.PostClose(m_sh, errCode);
    }

    public final int getRequestsInQueue() {
        return ServerCoreLoader.QueryRequestsInQueue(m_sh);
    }

    public final int SendExceptionResult(String errMessage, String errWhere) {
        return SendExceptionResult(errMessage, errWhere, 0, (short) 0);
    }

    public final int SendExceptionResult(String errMessage, String errWhere, int errCode) {
        return SendExceptionResult(errMessage, errWhere, errCode, (short) 0);
    }

    public final int SendExceptionResult(String errMessage, String errWhere, int errCode, short requestId) {
        byte[] where = {};
        if (errWhere != null) {
            where = errWhere.getBytes();
        }
        long reqIndex = ServerCoreLoader.GetCurrentRequestIndex(m_sh);
        if (reqIndex == -1) {
            return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, where, requestId, errCode);
        }
        return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, where, requestId, errCode);
    }

    public final int SendExceptionResult(String errMessage, String errWhere, int errCode, short requestId, long reqIndex) {
        byte[] where = {};
        if (errWhere != null) {
            where = errWhere.getBytes();
        }
        if (reqIndex == -1) {
            return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, where, requestId, errCode);
        }
        return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, where, requestId, errCode);
    }

    protected void OnSwitchFrom(int oldServiceId) {

    }

    protected void OnSlowRequestProcessed(short reqId) {

    }

    private static void OnSlowRequestProcessed(long hSocket, short reqId) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnSlowRequestProcessed(reqId);
    }

    protected void OnReleaseResource(boolean bClosing, int info) {

    }

    final void OnRelease(boolean bClosing, int info) {
        OnReleaseResource(bClosing, info);
    }

    protected void OnBaseRequestCame(SPA.tagBaseRequestID reqId) {

    }

    private static void OnBRCame(long hSocket, short reqId) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        try {
            sp.OnBaseRequestCame(SPA.tagBaseRequestID.forValue(reqId));
        } catch (java.lang.Exception err) {
            ServerCoreLoader.SendExceptionResult(sp.m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), reqId, 0);
        }
    }

    protected void OnChatRequestCame(SPA.tagChatRequestID chatRequestId) {

    }

    private static void OnCRCame(long hSocket, short chatRequestId) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        try {
            sp.OnChatRequestCame(SPA.tagChatRequestID.forValue(chatRequestId));
        } catch (java.lang.Exception err) {
            ServerCoreLoader.SendExceptionResult(sp.m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), chatRequestId, 0);
        }
    }

    protected void OnRequestArrive(short requestId, int len) {

    }

    private static void OnRArrive(long hSocket, short requestId, int len) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnRArrive(requestId, len);
    }

    private void OnRArrive(short requestId, int len) {
        m_sCurrentRequestId = requestId;
        m_qBuffer = new SPA.CUQueue(ServerCoreLoader.RetrieveBuffer(m_sh, len, false));
        SPA.CUQueue q = m_qBuffer;
        if (getSvsID() != SPA.BaseServiceID.sidHTTP) {
            q.setOS(m_os);
            q.setEndian(m_endian);
        } else {
            CHttpPeerBase hp = (CHttpPeerBase) this;
            hp.m_WebRequestName = null;
            java.util.ArrayList<Object> args = new java.util.ArrayList<>();
            if (requestId == tagHttpRequestID.idUserRequest.getValue()) {
                byte[] reqName = q.LoadBytes();
                hp.m_WebRequestName = new String(reqName);
                int count = q.LoadInt();
                for (int n = 0; n < count; ++n) {
                    Object arg = q.LoadObject();
                    args.add(arg);
                }
            }
            hp.m_vArg = args.toArray();
        }
        try {
            OnRequestArrive(requestId, len);
        } catch (java.lang.Exception err) {
            ServerCoreLoader.SendExceptionResult(m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), requestId, 0);
        }
    }

    private static void OnClosed(long hSocket, int errCode) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnClosed(errCode);
    }

    private static void OnFast(long hSocket, short reqId, int len) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnFast(reqId, len);
    }

    private static int OnSlow(long hSocket, short reqId, int len) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return 0;
        }
        return sp.OnSlow(reqId, len);
    }

    protected void OnSubscribe(int[] groups) {

    }

    protected void OnUnsubscribe(int[] groups) {

    }

    protected void OnPublish(Object message, int[] groups) {

    }

    protected void OnSendUserMessage(String receiver, Object message) {

    }

    protected void OnResultsSent() {

    }

    private static void OnResultsSent(long hSocket) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnResultsSent();
    }

    public final int[] getChatGroups() {
        int res = ServerCoreLoader.GetCountOfJoinedChatGroups(m_sh);
        int[] groups = new int[res];
        ServerCoreLoader.GetJoinedGroupIds(m_sh, groups, res);
        return groups;
    }

    private static void OnChatComing(long hSocket, short chatId, int len) {
        CSocketPeer sp = findPeer(hSocket);
        if (sp == null) {
            return;
        }
        sp.OnChatComing(chatId, len);
    }

    private void OnChatComing(short chatId, int len) {
        try {
            SPA.tagChatRequestID chatRequestID = SPA.tagChatRequestID.forValue(chatId);
            int svsId = ServerCoreLoader.GetSvsID(m_sh);
            m_qBuffer = new SPA.CUQueue(ServerCoreLoader.RetrieveBuffer(m_sh, len, true));
            SPA.CUQueue q = m_qBuffer;
            if (svsId != SPA.BaseServiceID.sidHTTP) {
                q.setEndian(m_endian);
                q.setOS(m_os);
            }
            switch (chatRequestID) {
                case idEnter:
                    OnSubscribe((int[]) q.LoadObject());
                    break;
                case idExit:
                    OnUnsubscribe(getChatGroups());
                    break;
                case idSendUserMessage:
                    OnSendUserMessage(q.LoadString(), q.LoadObject());
                    break;
                case idSpeak: {
                    int[] groups = (int[]) q.LoadObject();
                    Object msg = q.LoadObject();
                    OnPublish(msg, groups);
                }
                break;
                case idSpeakEx: {
                    byte[] bytes = q.LoadBytes();
                    int size = q.GetSize() / 4;
                    int[] groups = new int[size];
                    for (int n = 0; n < size; ++n) {
                        groups[n] = q.LoadInt();
                    }
                    OnPublishEx(groups, bytes);
                }
                break;
                case idSendUserMessageEx: {
                    String userId = q.LoadString();
                    byte[] bytes = q.GetBuffer();
                    q.SetSize(0);
                    OnSendUserMessageEx(userId, bytes);
                }
                break;
                default:
                    ServerCoreLoader.SendExceptionResult(m_sh, "Unexpected chat request", "SPA.ServerSide.CSocketPeer.OnChatComing".getBytes(), chatRequestID.getValue(), 0);
                    break;
            }
        } catch (java.lang.Exception err) {
            ServerCoreLoader.SendExceptionResult(m_sh, err.getMessage(), getStack(err.getStackTrace()).getBytes(), chatId, 0);
        }
    }
}
