package SPA.ServerSide;

final class ServerCoreLoader {

    static {
        System.loadLibrary("juserver");
        UseUTF16();
    }

    static java.util.GregorianCalendar m_gc = new java.util.GregorianCalendar();

    static native void UseUTF16();

    static native boolean InitSocketProServer(int param, Object server);

    static native void UninitSocketProServer();

    static native boolean StartSocketProServer(int listeningPort, int maxBacklog, boolean v6);

    static native void StopSocketProServer();

    static native boolean IsCanceled(long Handler);

    static native boolean IsRunning();

    static native void SetAuthenticationMethod(int am);

    static native int GetAuthenticationMethod();

    static native void SetSharedAM(boolean b);

    static native boolean GetSharedAM();

    static native void PostQuitPump();

    static native boolean IsMainThread();

    static native boolean AddSvsContext(int serviceId, int apartment);

    static native void RemoveASvsContext(int serviceId);

    static native boolean AddSlowRequest(int serviceId, short requestId);

    static native void RemoveSlowRequest(int serviceId, short requestId);

    static native int GetCountOfServices();

    static native int[] GetServices();

    static native int GetCountOfSlowRequests(int serviceId);

    static native void RemoveAllSlowRequests(int serviceId);

    static native int GetAllSlowRequestIds(int serviceId, short[] requestIds, int count);

    static native long AddADll(byte[] libFile, int nParam);

    static native boolean RemoveADllByHandle(long hInstance);

    static native void SetPrivateKeyFile(byte[] keyFile);

    static native void SetCertFile(byte[] certFile);

    static native void SetPKFPassword(byte[] pwd);

    static native void SetDHParmsFile(byte[] dhFile);

    static native void SetDefaultEncryptionMethod(int em);

    static native int GetDefaultEncryptionMethod();

    static native void SetPfxFile(byte[] pfxFile);

    static native int GetServerErrorCode();

    static native String GetServerErrorMessage();

    static native boolean IsServerRunning();

    static native boolean IsServerSSLEnabled();

    static native void Close(long h);

    static native short GetCurrentRequestID(long h);

    static native int GetCurrentRequestLen(long h);

    static native int GetRcvBytesInQueue(long h);

    static native int GetSndBytesInQueue(long h);

    static native void PostClose(long h, int errCode);

    static native int QueryRequestsInQueue(long h);

    static native byte[] RetrieveBuffer(long h, int len, boolean peek);

    static native boolean IsOpened(long h);

    static native long GetBytesReceived(long h);

    static native long GetBytesSent(long h);

    static native int SendReturnData(long h, short requestId, int bufferSize, byte[] buffer);

    static native int SendReturnDataIndex(long h, long index, short requestId, int bufferSize, byte[] buffer);

    static native int GetSvsID(long h);

    static native int GetServerSocketErrorCode(long h);

    static native long GetCurrentRequestIndex(long h);

    static native int GetServerSocketErrorMessage(long h, byte[] str, int bufferLen);

    static native boolean IsBatching(long h);

    static native int GetBytesBatched(long h);

    static native boolean StartBatching(long h);

    static native boolean CommitBatching(long h);

    static native boolean AbortBatching(long h);

    static native boolean SetUserID(long h, String userId);

    static native String GetUID(long h);

    static native boolean SetPassword(long h, String password);

    static native String GetPassword(long h);

    static native boolean Enter(long h, int[] chatGroupIds, int count);

    static native void Exit(long h);

    static native boolean Speak(long h, byte[] message, int size, int[] chatGroupIds, int count);

    static native boolean SpeakEx(long h, byte[] message, int size, int[] chatGroupIds, int count);

    static native boolean SendUserMessageEx(long h, String userId, byte[] message, int size);

    static native boolean SendUserMessage(long h, String userId, byte[] message, int size);

    static native boolean SpeakPush(byte[] message, int size, int[] chatGroupIds, int count);

    static native boolean SpeakExPush(byte[] message, int size, int[] chatGroupIds, int count);

    static native boolean SendUserMessageExPush(String userId, byte[] message, int size);

    static native boolean SendUserMessagePush(String userId, byte[] message, int size);

    static native int GetCountOfJoinedChatGroups(long h);

    static native int GetJoinedGroupIds(long h, int[] chatGroups, int count);

    static native String GetPeerName(long h, int[] peerPort, int n);

    static native String GetLocalName();

    static native boolean HasUserId(String userId);

    static native void DropCurrentSlowRequest(long h);

    static native void AddAChatGroup(int chatGroupId, String description);

    static native int GetCountOfChatGroups();

    static native int[] GetAllCreatedChatGroups();

    static native String GetAChatGroup(int chatGroupId);

    static native void RemoveChatGroup(int chatGroupId);

    static native long GetSocketNativeHandle(long h);

    static native byte GetPeerOs(long handler, boolean[] endian, int n);

    static native int SendExceptionResult(long handler, String errMessage, byte[] errWhere, short requestId, int errCode);

    static native int SendExceptionResultIndex(long handler, long index, String errMessage, byte[] errWhere, short requestId, int errCode);

    static native boolean MakeRequest(long handler, short requestId, byte[] request, int size);

    static native Object GetHTTPRequestHeaders(long h); //CHttpHV[]

    static native String GetHTTPPath(long h);

    static native long GetHTTPContentLength(long h);

    static native String GetHTTPQuery(long h);

    static native boolean DownloadFile(long handler, byte[] filePath, int len);

    static native int GetHTTPMethod(long h);

    static native boolean HTTPKeepAlive(long h);

    static native boolean IsWebSocket(long h);

    static native boolean IsCrossDomain(long h);

    static native double GetHTTPVersion(long h);

    static native boolean HTTPGZipAccepted(long h);

    static native String GetHTTPUrl(long h);

    static native String GetHTTPHost(long h);

    static native int GetHTTPTransport(long h);

    static native int GetHTTPTransferEncoding(long h);

    static native int GetHTTPContentMultiplax(long h);

    static native boolean SetHTTPResponseCode(long h, int errCode);

    static native boolean SetHTTPResponseHeader(long h, byte[] uft8Header, byte[] utf8Value);

    static native int SendHTTPReturnDataA(long h, byte[] str, int chars);

    static native int SendHTTPReturnDataW(long h, String str, int chars);

    static native String GetHTTPId(long h);

    static native Object GetHTTPCurrentMultiplaxHeaders(long h); //CHttpHV[]

    static native long GetSSL(long h);

    static native boolean GetReturnRandom(int serviceId);

    static native void SetReturnRandom(int serviceId, boolean random);

    static native int GetSwitchTime();

    static native void SetSwitchTime(int switchTime);

    static native int GetCountOfClients();

    static native long GetClient(int index);

    static native void SetDefaultZip(boolean zip);

    static native boolean GetDefaultZip();

    static native void SetMaxConnectionsPerClient(int maxConnectionsPerClient);

    static native int GetMaxConnectionsPerClient();

    static native void SetMaxThreadIdleTimeBeforeSuicide(int maxThreadIdleTimeBeforeSuicide);

    static native int GetMaxThreadIdleTimeBeforeSuicide();

    static native void SetTimerElapse(int timerElapse);

    static native int GetTimerElapse();

    static native int GetSMInterval();

    static native void SetSMInterval(int SMInterval);

    static native void SetPingInterval(int pingInterval);

    static native int GetPingInterval();

    static native void SetRecycleGlobalMemoryInterval(int recycleGlobalMemoryInterval);

    static native int GetRecycleGlobalMemoryInterval();

    static native long GetRequestCount();

    static native int StartHTTPChunkResponse(long h);

    static native boolean IsDequeuedMessageAborted(long h);

    static native void AbortDequeuedMessage(long h);

    static native int SendHTTPChunk(long h, byte[] buffer, int len);

    static native int EndHTTPChunkResponse(long h, byte[] buffer, int len);

    static native boolean IsFakeRequest(long h);

    static native boolean SetZip(long h, boolean bZip);

    static native boolean GetZip(long h);

    static native void SetZipLevel(long h, int zl);

    static native int GetZipLevel(long h);

    static native int StartQueue(byte[] qName, boolean dequeueShared, int ttl);

    static native int GetMessagesInDequeuing(int qHandle);

    static native long Enqueue(int qHandle, short reqId, byte[] buffer, int size);

    static native long GetMessageCount(int qHandle);

    static native boolean StopQueueByHandle(int qHandle, boolean permanent);

    static native boolean StopQueueByName(byte[] qName, boolean permanent);

    static native long GetQueueSize(int qHandle);

    static native long Dequeue(int qHandle, long h, int messageCount, boolean beNotifiedWhenAvailable, int waitTime);

    static native boolean IsQueueStartedByName(byte[] qName);

    static native boolean IsQueueStartedByHandle(int qHandle);

    static native boolean IsQueueSecuredByName(byte[] qName);

    static native boolean IsQueueSecuredByHandle(int qHandle);

    static native String GetQueueName(int qHandle);

    static native String GetQueueFileName(int qHandle);

    static native long Dequeue2(int qHandle, long h, int maxBytes, boolean beNotifiedWhenAvailable, int waitTime);

    static native void EnableClientDequeue(long h, boolean enable);

    static native boolean IsDequeueRequest(long h);

    static native boolean AbortJob(int qHandle);

    static native boolean StartJob(int qHandle);

    static native boolean EndJob(int qHandle);

    static native long GetJobSize(int qHandle);

    static native boolean SetRouting(int serviceId0, int ra0, int serviceId1, int ra1);

    static native int CheckRouting(int serviceId);

    static native boolean AddAlphaRequest(int serviceId, short reqId);

    static native int GetAlphaRequestIds(int serviceId, short[] reqIds, int count);

    static native long GetQueueLastIndex(int qHandle);

    static native long CancelQueuedRequestsByIndex(int qHandle, long startIndex, long endIndex);

    static native boolean IsDequeueShared(int qHandle);

    static native int GetServerQueueStatus(int qHandle);

    static native boolean PushQueueTo(int srcHandle, int[] targetHandles, int count);

    static native int GetTTL(int qHandle);

    static native long RemoveQueuedRequestsByTTL(int qHandle);

    static native void ResetQueue(int qHandle);

    static native boolean IsServerQueueIndexPossiblyCrashed();

    static native void SetServerWorkDirectory(byte[] dir);

    static native String GetServerWorkDirectory();

    static native long GetLastQueueMessageTime(int qHandle);

    static native String GetUServerSocketVersion();

    static native void SetMessageQueuePassword(byte[] pwd);

    static native void SetPeer(long h, Object peer);

    static native void RemovePeer(long h);

    static native int GetOptimistic(int qHandle);

    static native void SetOptimistic(int qHandle, int optimistic);

    static native void SetLastCallInfo(byte[] str, int len);

    static native int GetMainThreads();
}
