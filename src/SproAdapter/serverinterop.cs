using System;
using System.Runtime.InteropServices;
using System.Security;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        delegate void DOnClose(ulong Handler, int errCode);
        delegate void DOnSSLHandShakeCompleted(ulong Handler, int errCode);
        delegate void DOnAccept(ulong Handler, int errCode);
        delegate void DOnIdle(ulong milliseconds);
        delegate bool DOnIsPermitted(ulong Handler, uint serviceId);
        delegate void DOnRequestArrive(ulong Handler, ushort requestId, uint len);
        delegate void DOnFastRequestArrive(ulong Handler, ushort requestId, uint len);
        delegate int DSLOW_PROCESS(ushort requestId, uint len, ulong Handler);
        delegate void DOnRequestProcessed(ulong Handler, ushort requestId);
        delegate void DOnBaseRequestCame(ulong Handler, ushort requestId);
        delegate void DOnSwitchTo(ulong Handler, uint oldServiceId, uint newServiceId);
        delegate void DOnChatRequestComing(ulong handler, tagChatRequestID chatRequestID, uint len);
        delegate void DOnChatRequestCame(ulong handler, tagChatRequestID chatRequestId);
        delegate bool DOnHttpAuthentication(ulong handler, IntPtr userId, IntPtr password);
        delegate void DOnResultsSent(ulong handler);
        public delegate void DOnThreadEvent(tagThreadEvent te);

        [StructLayout(LayoutKind.Sequential)]
        struct CSvsContext
        {
            public tagThreadApartment m_ta; //required with a worker thread

            //called within main thread
            public DOnSwitchTo m_OnSwitchTo; //called when a service is switched
            public DOnRequestArrive m_OnRequestArrive;
            public DOnFastRequestArrive m_OnFastRequestArrive; //request processed within main thread
            public DOnBaseRequestCame m_OnBaseRequestCame; //SocketPro defines a set of base requests
            public DOnRequestProcessed m_OnRequestProcessed; //called when a slow request processed

            public DOnClose m_OnClose; //native socket event

            //called within worker thread
            public DSLOW_PROCESS m_SlowProcess; //required with a worker thread	

            public DOnChatRequestComing m_OnChatRequestComing;
            public DOnChatRequestCame m_OnChatRequestCame;
            public DOnResultsSent m_OnResultsSent;
            public DOnHttpAuthentication m_OnHttpAuthentication; //HttpAuthentication
        };

        //make it works within SQL Server
        [SuppressUnmanagedCodeSecurity]
        static class ServerCoreLoader
        {
            const string SERVER_CORE_DLL = "uservercore";

            static ServerCoreLoader()
            {
                switch (System.Environment.OSVersion.Platform)
                {
                    case PlatformID.Win32S:
                    case PlatformID.Win32NT:
                    case PlatformID.Win32Windows:
                    case PlatformID.WinCE:
                    case PlatformID.Xbox:
                        break;
                    default:
                        ServerCoreLoader.UseUTF16();
                        break;
                }
            }

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void UseUTF16();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool InitSocketProServer(int param);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void UninitSocketProServer();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool StartSocketProServer(uint listeningPort, uint maxBacklog, bool v6);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void StopSocketProServer();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsCanceled(ulong Handler);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsRunning();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetAuthenticationMethod(tagAuthenticationMethod am);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagAuthenticationMethod GetAuthenticationMethod();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetSharedAM(bool b);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool GetSharedAM();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void PostQuitPump();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsMainThread();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool AddSvsContext(uint serviceId, CSvsContext svsContext); //ta ignored on non-window platforms

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void RemoveASvsContext(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern CSvsContext GetSvsContext(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool AddSlowRequest(uint serviceId, ushort requestId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void RemoveSlowRequest(uint serviceId, ushort requestId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCountOfServices();

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetServices(uint* serviceIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCountOfSlowRequests(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void RemoveAllSlowRequests(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetAllSlowRequestIds(uint serviceId, ushort* requestIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr AddADll([MarshalAs(UnmanagedType.LPStr)]string libFile, int nParam);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool RemoveADllByHandle(IntPtr hInstance);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetPrivateKeyFile([MarshalAs(UnmanagedType.LPStr)]string keyFile);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetCertFile([MarshalAs(UnmanagedType.LPStr)]string certFile);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetPKFPassword([MarshalAs(UnmanagedType.LPStr)]string pwd);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetDHParmsFile([MarshalAs(UnmanagedType.LPStr)]string dhFile);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetDefaultEncryptionMethod(tagEncryptionMethod em);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagEncryptionMethod GetDefaultEncryptionMethod();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetPfxFile([MarshalAs(UnmanagedType.LPStr)]string pfxFile);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern int GetServerErrorCode();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern unsafe uint GetServerErrorMessage(sbyte* str, uint bufferLen);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsServerRunning();

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsServerSSLEnabled();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOnAccept(DOnAccept p);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetThreadEvent(DOnThreadEvent p);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void Close(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ushort GetCurrentRequestID(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCurrentRequestLen(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetRcvBytesInQueue(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetSndBytesInQueue(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void PostClose(ulong h, int errCode);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint QueryRequestsInQueue(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint RetrieveBuffer(ulong h, uint bufferSize, byte* buffer, bool peek);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOnSSLHandShakeCompleted(DOnSSLHandShakeCompleted p);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOnClose(DOnClose p);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOnIdle(DOnIdle p);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsOpened(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetBytesReceived(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetBytesSent(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint SendReturnData(ulong h, ushort requestId, uint bufferSize, byte* buffer);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetSvsID(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern int GetServerSocketErrorCode(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetServerSocketErrorMessage(ulong h, sbyte* str, uint bufferLen);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsBatching(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetBytesBatched(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool StartBatching(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool CommitBatching(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool AbortBatching(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetUserID(ulong h, [MarshalAs(UnmanagedType.LPWStr)]string userId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern unsafe uint GetUID(ulong h, char* userId, uint chars);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetPassword(ulong h, [MarshalAs(UnmanagedType.LPWStr)]string password);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern unsafe uint GetPassword(ulong h, char* password, uint chars);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOnIsPermitted(DOnIsPermitted p);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool Enter(ulong h, uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void Exit(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool Speak(ulong h, byte* message, uint size, uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SpeakEx(ulong h, byte* message, uint size, uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SendUserMessageEx(ulong h, [MarshalAs(UnmanagedType.LPWStr)]string userId, byte* message, uint size);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SendUserMessage(ulong h, [MarshalAs(UnmanagedType.LPWStr)]string userId, byte* message, uint size);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SpeakPush(byte* message, uint size, uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SpeakExPush(byte* message, uint size, uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SendUserMessageExPush([MarshalAs(UnmanagedType.LPWStr)]string userId, byte* message, uint size);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool SendUserMessagePush([MarshalAs(UnmanagedType.LPWStr)]string userId, byte* message, uint size);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCountOfJoinedChatGroups(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetJoinedGroupIds(ulong h, uint* chatGroups, uint count);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool GetPeerName(ulong h, ref uint peerPort, sbyte* strPeerAddr, ushort chars);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern unsafe uint GetLocalName(sbyte* localName, ushort chars);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool HasUserId([MarshalAs(UnmanagedType.LPWStr)]string userId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void DropCurrentSlowRequest(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void AddAChatGroup(uint chatGroupId, [MarshalAs(UnmanagedType.LPWStr)]string description);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCountOfChatGroups();

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetAllCreatedChatGroups(uint* chatGroupIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetAChatGroup(uint chatGroupId, char* description, uint chars);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void RemoveChatGroup(uint chatGroupId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetSocketNativeHandle(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagOperationSystem GetPeerOs(ulong handler, ref bool endian);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint SendExceptionResult(ulong handler, [MarshalAs(UnmanagedType.LPWStr)]string errMessage, [MarshalAs(UnmanagedType.LPStr)] string errWhere, ushort requestId, uint errCode);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool MakeRequest(ulong handler, ushort requestId, byte* request, uint size);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetHTTPRequestHeaders(ulong h, CHttpHV* HeaderValue, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetHTTPPath(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetHTTPContentLength(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetHTTPQuery(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool DownloadFile(ulong handler, [MarshalAs(UnmanagedType.LPStr)]string filePath);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagHttpMethod GetHTTPMethod(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool HTTPKeepAlive(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsWebSocket(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsCrossDomain(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern double GetHTTPVersion(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool HTTPGZipAccepted(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetHTTPUrl(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetHTTPHost(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagTransport GetHTTPTransport(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagTransferEncoding GetHTTPTransferEncoding(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagContentMultiplax GetHTTPContentMultiplax(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetHTTPResponseCode(ulong h, uint errCode);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetHTTPResponseHeader(ulong h, [MarshalAs(UnmanagedType.LPStr)]string uft8Header, [MarshalAs(UnmanagedType.LPStr)]string utf8Value);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint SendHTTPReturnDataA(ulong h, byte* str, int chars);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint SendHTTPReturnDataW(ulong h, [MarshalAs(UnmanagedType.LPWStr)]string str, uint chars);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetHTTPId(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetHTTPCurrentMultiplaxHeaders(ulong h, CHttpHV* HeaderValue, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetSSL(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool GetReturnRandom(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetReturnRandom(uint serviceId, bool random);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetSwitchTime();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetSwitchTime(uint switchTime);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetCountOfClients();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetClient(uint index);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetDefaultZip(bool zip);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool GetDefaultZip();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetMaxConnectionsPerClient(uint maxConnectionsPerClient);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetMaxConnectionsPerClient();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetMaxThreadIdleTimeBeforeSuicide(uint maxThreadIdleTimeBeforeSuicide);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetMaxThreadIdleTimeBeforeSuicide();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetTimerElapse(uint timerElapse);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetTimerElapse();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetSMInterval();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetSMInterval(uint SMInterval);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetPingInterval(uint pingInterval);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetPingInterval();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetRecycleGlobalMemoryInterval(uint recycleGlobalMemoryInterval);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetRecycleGlobalMemoryInterval();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetRequestCount();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint StartHTTPChunkResponse(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsDequeuedMessageAborted(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void AbortDequeuedMessage(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint SendHTTPChunk(ulong h, byte* buffer, uint len);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint EndHTTPChunkResponse(ulong h, byte* buffer, uint len);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsFakeRequest(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetZip(ulong h, bool bZip);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool GetZip(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetZipLevel(ulong h, tagZipLevel zl);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagZipLevel GetZipLevel(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint StartQueue([MarshalAs(UnmanagedType.LPStr)]string qName, bool dequeueShared, uint ttl);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetMessagesInDequeuing(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern ulong Enqueue(uint qHandle, ushort reqId, byte* buffer, uint size);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetMessageCount(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool StopQueueByHandle(uint qHandle, bool permanent);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool StopQueueByName([MarshalAs(UnmanagedType.LPStr)]string qName, bool permanent);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetQueueSize(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong Dequeue(uint qHandle, ulong h, uint messageCount, bool beNotifiedWhenAvailable, uint waitTime);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsQueueStartedByName([MarshalAs(UnmanagedType.LPStr)]string qName);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsQueueStartedByHandle(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsQueueSecuredByName([MarshalAs(UnmanagedType.LPStr)]string qName);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsQueueSecuredByHandle(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetQueueName(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetQueueFileName(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong Dequeue2(uint qHandle, ulong h, uint maxBytes, bool beNotifiedWhenAvailable, uint waitTime);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void EnableClientDequeue(ulong h, bool enable);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsDequeueRequest(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool AbortJob(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool StartJob(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool EndJob(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetJobSize(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool SetRouting(uint serviceId0, tagRoutingAlgorithm ra0, uint serviceId1, tagRoutingAlgorithm ra1);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint CheckRouting(uint serviceId);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool AddAlphaRequest(uint serviceId, ushort reqId);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint GetAlphaRequestIds(uint serviceId, ushort* reqIds, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetQueueLastIndex(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong CancelQueuedRequestsByIndex(uint qHandle, ulong startIndex, ulong endIndex);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsDequeueShared(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagQueueStatus GetServerQueueStatus(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern bool PushQueueTo(uint srcHandle, uint* targetHandles, uint count);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetTTL(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong RemoveQueuedRequestsByTTL(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void ResetQueue(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern bool IsServerQueueIndexPossiblyCrashed();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetServerWorkDirectory([MarshalAs(UnmanagedType.LPStr)]string dir);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetServerWorkDirectory();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetLastQueueMessageTime(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetUServerSocketVersion();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetMessageQueuePassword([MarshalAs(UnmanagedType.LPStr)]string pwd);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern tagOptimistic GetOptimistic(uint qHandle);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetOptimistic(uint qHandle, tagOptimistic optimistic);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern void SetLastCallInfo(IntPtr str);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint GetMainThreads();

            [DllImport(SERVER_CORE_DLL)]
            internal static extern IntPtr GetRequestBuffer(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern ulong GetCurrentRequestIndex(ulong h);

            [DllImport(SERVER_CORE_DLL)]
            internal static extern uint SendExceptionResultIndex(ulong handler, ulong index, [MarshalAs(UnmanagedType.LPWStr)]string errMessage, [MarshalAs(UnmanagedType.LPStr)] string errWhere, ushort requestId, uint errCode);

            [DllImport(SERVER_CORE_DLL)]
            internal static unsafe extern uint SendReturnDataIndex(ulong h, ulong index, ushort requestId, uint bufferSize, byte* buffer);
        }
    }
}
