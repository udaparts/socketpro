package SPA.ClientSide;

class ClientCoreLoader {

    static {
        System.loadLibrary("juclient");
        UseUTF16();
    }

    static java.util.GregorianCalendar m_gc = new java.util.GregorianCalendar();

    static native void SetCertificateVerifyCallback(boolean enabled);

    static native int CreateSocketPool(Object spc, int maxSocketsPerThread, int maxThreads, boolean bAvg, int ta);

    static native void SetCs(Object cs, long h);

    static native void UseUTF16();

    static native boolean DestroySocketPool(int poolId);

    static native long FindAClosedSocket(int poolId);

    static native boolean AddOneThreadIntoPool(int poolId);

    static native int GetLockedSockets(int poolId);

    static native int GetIdleSockets(int poolId);

    static native int GetConnectedSockets(int poolId);

    static native boolean DisconnectAll(int poolId);

    static native long LockASocket(int poolId, int timeout, long hSameThread);

    static native boolean UnlockASocket(int poolId, long h);

    static native int GetSocketsPerThread(int poolId);

    static native void SetBufferForCurrentThread(java.nio.ByteBuffer bytes, int len);

    static native boolean IsAvg(int poolId);

    static native int GetDisconnectedSockets(int poolId);

    static native int GetThreadCount(int poolId);

    static native void Close(long h);

    static native int GetCountOfRequestsQueued(long h);

    static native short GetCurrentRequestID(long h);

    static native int GetCurrentResultSize(long h);

    static native boolean IsDequeuedMessageAborted(long h);

    static native void AbortDequeuedMessage(long h);

    static native int GetEncryptionMethod(long h);

    static native int GetConnectionState(long h);

    static native int GetErrorCode(long h);

    static native String GetErrorMessage(long h);

    static native int GetSocketPoolId(long h);

    static native boolean IsOpened(long h);

    static native boolean SendRequest(long h, short reqId, java.nio.ByteBuffer buffer, int len, int offset);

    static native boolean WaitAll(long h, int nTimeout);

    static native boolean Cancel(long h, int requestsQueued);

    static native boolean IsRandom(long h);

    static native int GetBytesInSendingBuffer(long h);

    static native int GetBytesInReceivingBuffer(long h);

    static native boolean IsBatching(long h);

    static native int GetBytesBatched(long h);

    static native boolean StartBatching(long h);

    static native boolean CommitBatching(long h, boolean batchingAtServerSide);

    static native boolean AbortBatching(long h);

    static native void SetUserID(long h, String userId);

    static native int GetUID(long h, char[] userId, int chars);

    static native void SetPassword(long h, String password);

    static native boolean SwitchTo(long h, int serviceId);

    static native boolean Connect(long h, byte[] host, int len, int portNumber, boolean sync, boolean v6);

    static native boolean Enter(long h, int[] groups, int count);

    static native boolean Speak(long h, byte[] message, int size, int[] groups, int count);

    static native boolean SpeakEx(long h, byte[] message, int size, int[] groups, int count);

    static native boolean SendUserMessage(long h, String userId, byte[] message, int size);

    static native boolean SendUserMessageEx(long h, String userId, byte[] message, int size);

    static native boolean StartQueue(long h, byte[] qName, int len, boolean secure, boolean dequeueShared, int ttl);

    static native boolean SetVerifyLocation(byte[] certFile, int len);

    static native void SetClientWorkDirectory(byte[] dir, int len);

    static native void Exit(long h);

    static native long GetBytesReceived(long h);

    static native long GetBytesSent(long h);

    static native long GetSocketNativeHandle(long h);

    static native long GetMessageCount(long h);

    static native long GetQueueSize(long h);

    static native long GetQueueLastIndex(long h);

    static native long CancelQueuedRequestsByIndex(long h, long startIndex, long endIndex);

    static native long RemoveQueuedRequestsByTTL(long h);

    static native long GetLastQueueMessageTime(long h);

    static native long GetJobSize(long h);

    static native byte GetPeerOs(long h, boolean[] endian, int len);

    static native void SetZip(long h, boolean zip);

    static native boolean GetZip(long h);

    static native void SetZipLevel(long h, int zl);

    static native int GetZipLevel(long h);

    static native int GetCurrentServiceId(long h);

    static native void StopQueue(long h, boolean permanent);

    static native boolean DequeuedResult(long h);

    static native int GetMessagesInDequeuing(long h);

    static native boolean IsQueueSecured(long h);

    static native boolean IsQueueStarted(long h);

    static native boolean DoEcho(long h);

    static native boolean SetSockOpt(long h, int optName, int optValue, int level);

    static native boolean SetSockOptAtSvr(long h, int optName, int optValue, int level);

    static native boolean TurnOnZipAtSvr(long h, boolean enableZip);

    static native boolean SetZipLevelAtSvr(long h, int zipLevel);

    static native void SetRecvTimeout(long h, int timeout);

    static native int GetRecvTimeout(long h);

    static native void SetConnTimeout(long h, int timeout);

    static native int GetConnTimeout(long h);

    static native void SetAutoConn(long h, boolean autoConnecting);

    static native boolean GetAutoConn(long h);

    static native short GetServerPingTime(long h);

    static native long GetSSL(long h);

    static native boolean IgnoreLastRequest(long h, short reqId);

    static native boolean IsDequeueEnabled(long h);

    static native String GetQueueName(long h);

    static native String GetQueueFileName(long h);

    static native String GetPeerName(long h, int[] peerPort, int count);

    static native Object GetUCert(long h);

    static native String Verify(long h, int[] errCode, int count);

    static native boolean AbortJob(long h);

    static native boolean StartJob(long h);

    static native boolean EndJob(long h);

    static native boolean IsRouteeRequest(long h);

    static native int GetRouteeCount(long h);

    static native boolean SendRouteeResult(long h, short reqId, java.nio.ByteBuffer buffer, int len, int offset);

    static native boolean IsDequeueShared(long h);

    static native int GetClientQueueStatus(long h);

    static native boolean PushQueueTo(long srcHandle, long[] targetHandles, int count);

    static native int GetTTL(long h);

    static native void ResetQueue(long h);

    static native boolean IsClientQueueIndexPossiblyCrashed();

    static native String GetClientWorkDirectory();

    static native int GetNumberOfSocketPools();

    static native boolean IsRouting(long h);

    static native void SetEncryptionMethod(long h, int em);

    static native void Shutdown(long h, int how);

    static native String GetUClientSocketVersion();

    static native void SetMessageQueuePassword(byte[] pwd, int chars);

    static native void EnableRoutingQueueIndex(long h, boolean enable);

    static native boolean IsRoutingQueueIndexEnabled(long h);

    static native String GetUClientAppName();

    static native int GetOptimistic(long h);

    static native void SetOptimistic(long h, int optimistic);

    static native void SetLastCallInfo(byte[] str, int len);

    static native boolean GetQueueAutoMergeByPool(int poolId);

    static native void SetQueueAutoMergeByPool(int poolId, boolean autoMerge);

    static native void PostProcessing(long h, int hint, long data);

    static native boolean SendInterruptRequest(long h, long options);
}
