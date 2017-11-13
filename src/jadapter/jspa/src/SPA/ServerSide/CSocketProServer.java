package SPA.ServerSide;

public class CSocketProServer {

    @Override
    protected void finalize() throws Throwable {
        ServerCoreLoader.StopSocketProServer();
        ServerCoreLoader.UninitSocketProServer();
        m_sps = null;
        super.finalize();
    }

    @SuppressWarnings("LeakingThisInConstructor")
    protected CSocketProServer(int param) {
        if (m_sps != null) {
            throw new UnsupportedOperationException("SocketPro doesn't allow multiple instances at the same time");
        }
        m_sps = this;
        ServerCoreLoader.InitSocketProServer(param, this);
    }

    @SuppressWarnings("LeakingThisInConstructor")
    protected CSocketProServer() {
        if (m_sps != null) {
            throw new UnsupportedOperationException("SocketPro doesn't allow multiple instances at the same time");
        }
        m_sps = this;
        ServerCoreLoader.InitSocketProServer(0, this);
    }

    /**
     * Use the method for debugging crash within cross development environments.
     *
     * @param str A string will be sent to server core library to be output into
     * a crash text file
     */
    public static void SetLastCallInfo(String str) {
        byte[] bytes;
        try {
            bytes = str.getBytes("UTF-8");
        } catch (java.io.UnsupportedEncodingException err) {
            bytes = new byte[0];
        }
        ServerCoreLoader.SetLastCallInfo(bytes, bytes.length);
    }

    protected boolean OnSettingServer() {
        return true;
    }

    public static boolean getRunning() {
        return ServerCoreLoader.IsRunning();
    }

    public static CSocketProServer getServer() {
        return m_sps;
    }

    public static boolean getIsMainThread() {
        return ServerCoreLoader.IsMainThread();
    }

    public static boolean getSSLEnabled() {
        return ServerCoreLoader.IsServerSSLEnabled();
    }

    public static int getLastSocketError() {
        return ServerCoreLoader.GetServerErrorCode();
    }

    public static String getErrorMessage() {
        return ServerCoreLoader.GetServerErrorMessage();
    }

    public static long getRequestCount() {
        return ServerCoreLoader.GetRequestCount();
    }

    public static int getCountOfClients() {
        return ServerCoreLoader.GetCountOfClients();
    }

    public static int[] getServices() {
        return ServerCoreLoader.GetServices();
    }

    public static String getVersion() {
        return ServerCoreLoader.GetUServerSocketVersion();
    }

    public static long GetClient(int index) {
        return ServerCoreLoader.GetClient(index);
    }

    public static String getLocalName() {
        return ServerCoreLoader.GetLocalName();
    }

    protected void OnAccept(long hSocket, int nError) {

    }

    private static void OnAcceptInternal(long hSocket, int nError) {
        if (m_sps != null) {
            m_sps.OnAccept(hSocket, nError);
        }
    }

    protected void OnClose(long hSocket, int nError) {

    }

    private static void OnCloseInternal(long hSocket, int nError) {
        if (m_sps != null) {
            m_sps.OnClose(hSocket, nError);
        }
    }

    private static boolean IsPermittedInternal(long hSocket, int nSvsID) {
        try {
            if (m_sps != null) {
                return m_sps.OnIsPermitted(hSocket, CredentialManager.GetUserID(hSocket), CredentialManager.GetPassword(hSocket), nSvsID);
            }
        } catch (java.lang.Exception err) {
        }
        return false;
    }

    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        return true;
    }

    protected void OnIdle(long milliseconds) {

    }

    private static void OnIdleInternal(long milliseconds) {
        if (m_sps != null) {
            m_sps.OnIdle(milliseconds);
        }
    }

    protected void OnSSLShakeCompleted(long hSocket, int errCode) {

    }

    private static void OnSSLShakeCompletedInternal(long hSocket, int errCode) {
        if (m_sps != null) {
            m_sps.OnSSLShakeCompleted(hSocket, errCode);
        }
    }

    public void PostQuit() {
        ServerCoreLoader.PostQuitPump();
    }

    public final boolean Run(int port) {
        return Run(port, 32, false);
    }

    public final boolean Run(int port, int maxBacklog) {
        return Run(port, maxBacklog, false);
    }

    public boolean Run(int port, int maxBacklog, boolean v6Supported) {
        if (!OnSettingServer()) {
            return false;
        }
        java.lang.Class type = getClass();
        java.lang.reflect.Field[] fis = type.getDeclaredFields();
        for (java.lang.reflect.Field fi : fis) {
            if (fi.getType() != CSocketProService.class) {
                continue;
            }
            ServiceAttr sa = (ServiceAttr) fi.getAnnotation(ServiceAttr.class);
            if (sa != null) {
                Object obj;
                fi.setAccessible(true);
                try {
                    java.lang.reflect.Method meth = CBaseService.class.getMethod("AddMe", new Class[]{int.class, SPA.tagThreadApartment.class});
                    obj = fi.get(this);
                    obj = meth.invoke(obj, sa.ServiceID(), sa.ThreadApartment());
                } catch (NoSuchMethodException | IllegalArgumentException | IllegalAccessException | java.lang.reflect.InvocationTargetException err) {
                    return false;
                }
                if (!(boolean) obj) {
                    throw new UnsupportedOperationException("Failed in registering service = " + sa.ServiceID() + ", and check if the service ID is duplicated with the previous one or if the service ID is less or equal to SocketProAdapter.BaseServiceID.sidReserved");
                }
            }
        }
        return ServerCoreLoader.StartSocketProServer(port, maxBacklog, v6Supported);
    }

    public void StopSocketProServer() {
        ServerCoreLoader.PostQuitPump();
        ServerCoreLoader.StopSocketProServer();
    }

    public final void UseSSL(String certFile, String keyFile, String pwdForPrivateKeyFile) {
        UseSSL(certFile, keyFile, pwdForPrivateKeyFile, "");
    }

    public final void UseSSL(String certFile, String keyFile, String pwdForPrivateKeyFile, String dhFile) {
        if (certFile != null) {
            ServerCoreLoader.SetCertFile(certFile.getBytes());
        }
        if (keyFile != null) {
            ServerCoreLoader.SetPrivateKeyFile(keyFile.getBytes());
        }
        if (pwdForPrivateKeyFile != null) {
            ServerCoreLoader.SetPKFPassword(pwdForPrivateKeyFile.getBytes());
        }
        if (dhFile != null) {
            ServerCoreLoader.SetDHParmsFile(dhFile.getBytes());
        }
        ServerCoreLoader.SetDefaultEncryptionMethod(SPA.tagEncryptionMethod.TLSv1.getValue());
    }

    public final static class SwitchError {

        public static final int seERROR_WRONG_SWITCH = 0x7FFFF100;
        public static final int seERROR_AUTHENTICATION_FAILED = 0x7FFFF101;
        public static final int seERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE = 0x7FFFF102;
        public static final int seERROR_NOT_SWITCHED_YET = 0x7FFFF103;
        public static final int seERROR_BAD_REQUEST = 0x7FFFF104;
    }

    public final static class Config {

        public static int getMaxThreadIdleTimeBeforeSuicide() {
            return ServerCoreLoader.GetMaxThreadIdleTimeBeforeSuicide();
        }

        public static void setMaxThreadIdleTimeBeforeSuicide(int value) {
            ServerCoreLoader.SetMaxThreadIdleTimeBeforeSuicide(value);
        }

        public static int getMaxConnectionsPerClient() {
            return ServerCoreLoader.GetMaxConnectionsPerClient();
        }

        public static void setMaxConnectionsPerClient(int value) {
            ServerCoreLoader.SetMaxConnectionsPerClient(value);
        }

        public static int getTimerElapse() {
            return ServerCoreLoader.GetTimerElapse();
        }

        public static void setTimerElapse(int value) {
            ServerCoreLoader.SetTimerElapse(value);
        }

        public static int getSMInterval() {
            return ServerCoreLoader.GetSMInterval();
        }

        public static void setSMInterval(int value) {
            ServerCoreLoader.SetSMInterval(value);
        }

        public static int getPingInterval() {
            return ServerCoreLoader.GetPingInterval();
        }

        public static void setPingInterval(int value) {
            ServerCoreLoader.SetPingInterval(value);
        }

        public static boolean getDefaultZip() {
            return ServerCoreLoader.GetDefaultZip();
        }

        public static void setDefaultZip(boolean value) {
            ServerCoreLoader.SetDefaultZip(value);
        }

        public static SPA.tagEncryptionMethod getDefaultEncryptionMethod() {
            return SPA.tagEncryptionMethod.forValue(ServerCoreLoader.GetDefaultEncryptionMethod());
        }

        public static void setDefaultEncryptionMethod(SPA.tagEncryptionMethod value) {
            ServerCoreLoader.SetDefaultEncryptionMethod(value.getValue());
        }

        public static int getSwitchTime() {
            return ServerCoreLoader.GetSwitchTime();
        }

        public static void setSwitchTime(int value) {
            ServerCoreLoader.SetSwitchTime(value);
        }

        public static tagAuthenticationMethod getAuthenticationMethod() {
            return tagAuthenticationMethod.forValue(ServerCoreLoader.GetAuthenticationMethod());
        }

        public static void setAuthenticationMethod(tagAuthenticationMethod value) {
            ServerCoreLoader.SetAuthenticationMethod(value.getValue());
        }

        public static boolean getSharedAM() {
            return ServerCoreLoader.GetSharedAM();
        }

        public static void setSharedAM(boolean value) {
            ServerCoreLoader.SetSharedAM(value);
        }

        public static void setPassword(String value) {
            ServerCoreLoader.SetPKFPassword(value.getBytes());
        }

        public static int getMainThreads() {
            return ServerCoreLoader.GetMainThreads();
        }
    }

    public final static class DllManager {

        public static long AddALibrary(String libFile) {
            if (libFile == null) {
                libFile = "";
            }
            return ServerCoreLoader.AddADll(libFile.getBytes(), 0);
        }

        public static long AddALibrary(String libFile, int param) {
            if (libFile == null) {
                libFile = "";
            }
            return ServerCoreLoader.AddADll(libFile.getBytes(), param);
        }

        public static boolean RemoveALibrary(long hLib) {
            return ServerCoreLoader.RemoveADllByHandle(hLib);
        }
    }

    public final static class PushManager {

        public static void AddAChatGroup(int groupId) {
            ServerCoreLoader.AddAChatGroup(groupId, "");
        }

        public static void AddAChatGroup(int groupId, String description) {
            if (description == null) {
                description = "";
            }
            ServerCoreLoader.AddAChatGroup(groupId, description);
        }

        public static void RemoveChatGroup(int chatGroupId) {
            ServerCoreLoader.RemoveChatGroup(chatGroupId);
        }

        public static String GetAChatGroup(int groupId) {
            return ServerCoreLoader.GetAChatGroup(groupId);
        }

        public static int getCountOfChatGroups() {
            return ServerCoreLoader.GetCountOfChatGroups();
        }

        public static int[] getAllCreatedChatGroups() {
            return ServerCoreLoader.GetAllCreatedChatGroups();
        }

        public static boolean Publish(byte[] Message, int... Groups) {
            int size;
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            if (Message == null) {
                size = 0;
            } else {
                size = (int) Message.length;
            }
            return ServerCoreLoader.SpeakExPush(Message, size, Groups, len);
        }

        public static boolean SendUserMessage(String UserId, byte[] Message) {
            int size;
            if (Message == null) {
                size = 0;
            } else {
                size = (int) Message.length;
            }
            return ServerCoreLoader.SendUserMessageExPush(UserId, Message, size);
        }

        public static boolean Publish(Object Message, int... Groups) {
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            SPA.CScopeUQueue su = new SPA.CScopeUQueue();
            SPA.CUQueue q = su.getUQueue();
            q.Save(Message);
            return ServerCoreLoader.SpeakPush(q.getIntenalBuffer(), q.GetSize(), Groups, len);

        }

        public static boolean SendUserMessage(Object Message, String UserId) {
            SPA.CScopeUQueue su = new SPA.CScopeUQueue();
            SPA.CUQueue q = su.getUQueue();
            q.Save(Message);
            return ServerCoreLoader.SendUserMessagePush(UserId, q.getIntenalBuffer(), q.GetSize());
        }
    }

    public final static class QueueManager {

        public static CServerQueue StartQueue(String qName, int ttl) {
            if (qName == null) {
                qName = "";
            }
            return new CServerQueue(ServerCoreLoader.StartQueue(qName.getBytes(), true, ttl));
        }

        public static CServerQueue StartQueue(String qName, int ttl, boolean dequeueShared) {
            if (qName == null) {
                qName = "";
            }
            return new CServerQueue(ServerCoreLoader.StartQueue(qName.getBytes(), dequeueShared, ttl));
        }

        public static boolean StopQueue(String qName) {
            if (qName == null) {
                qName = "";
            }
            return ServerCoreLoader.StopQueueByName(qName.getBytes(), false);
        }

        public static boolean StopQueue(String qName, boolean permanent) {
            if (qName == null) {
                qName = "";
            }
            return ServerCoreLoader.StopQueueByName(qName.getBytes(), permanent);
        }

        public static boolean IsQueueStarted(String qName) {
            if (qName == null) {
                qName = "";
            }
            return ServerCoreLoader.IsQueueStartedByName(qName.getBytes());
        }

        public static boolean IsQueueSecured(String qName) {
            if (qName == null) {
                qName = "";
            }
            return ServerCoreLoader.IsQueueSecuredByName(qName.getBytes());
        }

        public static boolean getIsServerQueueIndexPossiblyCrashed() {
            return ServerCoreLoader.IsServerQueueIndexPossiblyCrashed();
        }

        public static void setMessageQueuePassword(String value) {
            if (value == null) {
                value = "";
            }
            ServerCoreLoader.SetMessageQueuePassword(value.getBytes());
        }

        public static String getWorkDirectory() {
            return ServerCoreLoader.GetServerWorkDirectory();
        }

        public static void setWorkDirectory(String value) {
            if (value == null) {
                value = "";
            }
            ServerCoreLoader.SetServerWorkDirectory(value.getBytes());
        }
    }

    public final static class Router {

        /**
         * Set a route with two given service ids
         *
         * @param serviceId0 The first service Id
         * @param serviceId1 The second service id
         * @return True if successful; and false if failed If any one of the two
         * given service ids does not exist, the route is broken
         */
        public static boolean SetRouting(int serviceId0, int serviceId1) {
            return ServerCoreLoader.SetRouting(serviceId0, tagRoutingAlgorithm.raDefault.getValue(), serviceId1, tagRoutingAlgorithm.raDefault.getValue());
        }

        /**
         * Set a route with two given service ids
         *
         * @param serviceId0 The first service Id
         * @param ra0 Routing algorithm for serviceId0. It is default to
         * raDefault
         * @param serviceId1 The second service id
         * @param ra1 Routing algorithm for serviceId1. It is default to
         * raDefault
         * @return True if successful; and false if failed If any one of the two
         * given service ids does not exist, the route is broken
         */
        public static boolean SetRouting(int serviceId0, tagRoutingAlgorithm ra0, int serviceId1, tagRoutingAlgorithm ra1) {
            if (ra0 == null) {
                ra0 = tagRoutingAlgorithm.raDefault;
            }
            if (ra1 == null) {
                ra1 = tagRoutingAlgorithm.raDefault;
            }
            return ServerCoreLoader.SetRouting(serviceId0, ra0.getValue(), serviceId1, ra1.getValue());
        }

        /**
         * Query a routee service id from a given service id
         *
         * @param serviceId A given service id
         * @return A valid routee service id if this service id is valid and set
         * to be routed
         */
        public static int CheckRouting(int serviceId) {
            return ServerCoreLoader.CheckRouting(serviceId);
        }
    }

    public final static class CredentialManager {

        public static boolean HasUserId(String userId) {
            return ServerCoreLoader.HasUserId(userId);
        }

        public static String GetUserID(long hSocket) {
            return ServerCoreLoader.GetUID(hSocket);
        }

        public static boolean SetUserID(long hSocket, String userId) {
            if (userId == null) {
                userId = "";
            }
            return ServerCoreLoader.SetUserID(hSocket, userId);
        }

        public static String GetPassword(long hSocket) {
            return ServerCoreLoader.GetPassword(hSocket);
        }

        public static boolean SetPassword(long hSocket, String password) {
            if (password == null) {
                password = "";
            }
            return ServerCoreLoader.SetPassword(hSocket, password);
        }
    }

    private static CSocketProServer m_sps = null;
}
