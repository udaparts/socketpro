
#ifndef _SOCKETPRO_SERVER_FUNCTIONS_HEADER_H_
#define _SOCKETPRO_SERVER_FUNCTIONS_HEADER_H_

#include "userver.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef void (CALLBACK *POnSSLHandShakeCompleted)(USocket_Server_Handle Handler, int errCode);
    typedef void (CALLBACK *POnAccept)(USocket_Server_Handle Handler, int errCode);
    typedef void (CALLBACK *POnIdle)(SPA::INT64 milliseconds);
    typedef bool (CALLBACK *POnIsPermitted)(USocket_Server_Handle Handler, unsigned int serviceId);
    typedef void (CALLBACK *PThreadEvent) (SPA::ServerSide::tagThreadEvent te);

    //creating plugable and reusable standard window DLL
    typedef bool (WINAPI *PInitServerLibrary) (int param);
    typedef void (WINAPI *PUninitServerLibrary) ();
    typedef unsigned short (WINAPI *PGetNumOfServices) ();
    typedef unsigned int (WINAPI *PGetAServiceID) (unsigned short index);
    typedef CSvsContext(WINAPI *PGetOneSvsContext) (unsigned int serviceId);
    typedef unsigned short (WINAPI *PGetNumOfSlowRequests) (unsigned int serviceId);
    typedef unsigned short (WINAPI *PGetOneSlowRequestID) (unsigned int serviceId, unsigned short index);

    /**
     * Initialize the SocketPro server core library. The function should be called first before calling other methods.
     * @param nParam is ignored at this time.
     * @return true if succesfull, and false otherwise.
     */
    bool WINAPI InitSocketProServer(int param);

    /**
     * Uninitialize the SocketPro server core library
     * The function should be called at very end
     */
    void WINAPI UninitSocketProServer();

    /**
     * Start a running instance of SocketPro server. Note that an application is able to run one instance of SocketPro server only.
     * @param listeningPort a port number of listening socket
     * @param maxBacklog max queue size of coming connecting client sockets
     * @param v6 true if IPv6 is supported, and false if IPv6 is not supported
     * @return true if successful, and false otherwise
     */
    bool WINAPI StartSocketProServer(unsigned int listeningPort, unsigned int maxBacklog = 32, bool v6 = true);

    /**
     * Stop a running instance of SocketPro server 
     */
    void WINAPI StopSocketProServer();

    /**
     * Check if the current request is already canceled from a client by calling the method Cancel.
     * You can call the method within a worker thread but not main thread.
     * @return true if the current request is canceled, and false otherwise.
     */
    bool WINAPI IsCanceled(USocket_Server_Handle Handler);
    bool WINAPI IsRunning();

    void WINAPI SetAuthenticationMethod(SPA::ServerSide::tagAuthenticationMethod am);
    SPA::ServerSide::tagAuthenticationMethod WINAPI GetAuthenticationMethod();

    void WINAPI SetSharedAM(bool b);
    bool WINAPI GetSharedAM();

    void WINAPI PostQuitPump();
    bool WINAPI IsMainThread();

    /**
     * 
     * @param serviceId
     * @param svsContext
     * @return 
     */
    bool WINAPI AddSvsContext(unsigned int serviceId, CSvsContext svsContext); //ta ignored on non-window platforms
    void WINAPI RemoveASvsContext(unsigned int serviceId);
    CSvsContext WINAPI GetSvsContext(unsigned int serviceId);

    /**
     * 
     * @param serviceId
     * @param requestId
     * @return 
     */
    bool WINAPI AddSlowRequest(unsigned int serviceId, unsigned short requestId);

    /**
     * 
     * @param serviceIds
     * @param count
     */
    void WINAPI RemoveSlowRequest(unsigned int serviceId, unsigned short requestId);
    unsigned int WINAPI GetCountOfServices();
    unsigned int WINAPI GetServices(unsigned int *serviceIds, unsigned int count);
    unsigned int WINAPI GetCountOfSlowRequests(unsigned int serviceId);
    void WINAPI RemoveAllSlowRequests(unsigned int serviceId);

    /**
     * @param serviceId
     * @requestIds
     * @count
     * @return 
     */
    unsigned int WINAPI GetAllSlowRequestIds(unsigned int serviceId, unsigned short *requestIds, unsigned int count);

    //plug and unplug a dll that may contain one or more services
    HINSTANCE WINAPI AddADll(const char *libFile, int nParam);
    bool WINAPI RemoveADllByHandle(HINSTANCE hInstance);

    void WINAPI SetPrivateKeyFile(const char *keyFile);
    void WINAPI SetCertFile(const char *certFile);
    void WINAPI SetDHParmsFile(const char *dhFile);
    void WINAPI SetPKFPassword(const char *pwd);
    void WINAPI SetDefaultEncryptionMethod(SPA::tagEncryptionMethod em);
    SPA::tagEncryptionMethod WINAPI GetDefaultEncryptionMethod();
    void WINAPI SetPfxFile(const char *pfxFile);

    int WINAPI GetServerErrorCode();
    unsigned int WINAPI GetServerErrorMessage(char *str, unsigned int bufferLen);
    bool WINAPI IsServerRunning();
    bool WINAPI IsServerSSLEnabled();
    void WINAPI SetOnAccept(POnAccept p);

    void WINAPI Close(USocket_Server_Handle h);
    unsigned short WINAPI GetCurrentRequestID(USocket_Server_Handle h);
    unsigned int WINAPI GetCurrentRequestLen(USocket_Server_Handle h);
    unsigned int WINAPI GetRcvBytesInQueue(USocket_Server_Handle h);
    unsigned int WINAPI GetSndBytesInQueue(USocket_Server_Handle h);
    void WINAPI PostClose(USocket_Server_Handle h, int errCode = 0);
    unsigned int WINAPI QueryRequestsInQueue(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @param bufferSize
     * @param buffer
     * @param peek
     * @return 
     */
    unsigned int WINAPI RetrieveBuffer(USocket_Server_Handle h, unsigned int bufferSize, unsigned char *buffer, bool peek = false);

    /**
     * 
     * @param p
     */
    void WINAPI SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p);

    /**
     * 
     * @param p
     */
    void WINAPI SetOnClose(POnClose p);

    /**
     * 
     * @param p
     */
    void WINAPI SetOnIdle(POnIdle p);

    bool WINAPI IsOpened(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    SPA::UINT64 WINAPI GetBytesReceived(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    SPA::UINT64 WINAPI GetBytesSent(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @requestId
     * @bufferSize
     * @buffer
     * @return 
     */
    unsigned int WINAPI SendReturnData(USocket_Server_Handle h, unsigned short requestId, unsigned int bufferSize, const unsigned char *buffer);

    /**
     * 
     * @param h
     * @return 
     */
    unsigned int WINAPI GetSvsID(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    int WINAPI GetServerSocketErrorCode(USocket_Server_Handle h);

    /**
     * @param h
     * @str
     * @bufferLen
     * @return 
     */
    unsigned int WINAPI GetServerSocketErrorMessage(USocket_Server_Handle h, char *str, unsigned int bufferLen);

    /**
     * 
     * @param h
     * @return 
     */
    bool WINAPI IsBatching(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    unsigned int WINAPI GetBytesBatched(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    bool WINAPI StartBatching(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    bool WINAPI CommitBatching(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @return 
     */
    bool WINAPI AbortBatching(USocket_Server_Handle h);

    /**
     * 
     * @param h
     * @param userId
     * @return 
     */
    bool WINAPI SetUserID(USocket_Server_Handle h, const wchar_t *userId);
    unsigned int WINAPI GetUID(USocket_Server_Handle h, wchar_t *userId, unsigned int chars);
    bool WINAPI SetPassword(USocket_Server_Handle h, const wchar_t *password);
    unsigned int WINAPI GetPassword(USocket_Server_Handle h, wchar_t *password, unsigned int chars);
    void WINAPI SetOnIsPermitted(POnIsPermitted p);
    bool WINAPI Enter(USocket_Server_Handle h, const unsigned int *chatGroupIds, unsigned int count);
    void WINAPI Exit(USocket_Server_Handle h);
    bool WINAPI Speak(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    bool WINAPI SpeakEx(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    bool WINAPI SendUserMessageEx(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool WINAPI SendUserMessage(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    unsigned int WINAPI GetCountOfJoinedChatGroups(USocket_Server_Handle h);
    unsigned int WINAPI GetJoinedGroupIds(USocket_Server_Handle h, unsigned int *chatGroups, unsigned int count);

    bool WINAPI GetPeerName(USocket_Server_Handle h, unsigned int *peerPort, char *strPeerAddr, unsigned short chars);
    unsigned int WINAPI GetLocalName(char *localName, unsigned short chars);
    bool WINAPI HasUserId(const wchar_t *userId);
    void WINAPI DropCurrentSlowRequest(USocket_Server_Handle h);

    void WINAPI AddAChatGroup(unsigned int chatGroupId, const wchar_t *description = nullptr);
    unsigned int WINAPI GetCountOfChatGroups();
    unsigned int WINAPI GetAllCreatedChatGroups(unsigned int *chatGroupIds, unsigned int count);
    unsigned int WINAPI GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars);
    void WINAPI RemoveChatGroup(unsigned int chatGroupId);
    SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Server_Handle h);
    SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Server_Handle handler, bool *endian);
    unsigned int WINAPI SendExceptionResult(USocket_Server_Handle handler, const wchar_t* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
    bool WINAPI MakeRequest(USocket_Server_Handle handler, unsigned short requestId, const unsigned char *request, unsigned int size);

    unsigned int WINAPI GetHTTPRequestHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);
    const char* WINAPI GetHTTPPath(USocket_Server_Handle h);
    SPA::UINT64 WINAPI GetHTTPContentLength(USocket_Server_Handle h);
    const char* WINAPI GetHTTPQuery(USocket_Server_Handle h);
    bool WINAPI DownloadFile(USocket_Server_Handle handler, const char *filePath);
    SPA::ServerSide::tagHttpMethod WINAPI GetHTTPMethod(USocket_Server_Handle h);
    bool WINAPI HTTPKeepAlive(USocket_Server_Handle h);
    bool WINAPI IsWebSocket(USocket_Server_Handle h);
    bool WINAPI IsCrossDomain(USocket_Server_Handle h);
    double WINAPI GetHTTPVersion(USocket_Server_Handle h);
    bool WINAPI HTTPGZipAccepted(USocket_Server_Handle h);
    const char* WINAPI GetHTTPUrl(USocket_Server_Handle h);
    const char* WINAPI GetHTTPHost(USocket_Server_Handle h);
    SPA::ServerSide::tagTransport WINAPI GetHTTPTransport(USocket_Server_Handle h);
    SPA::ServerSide::tagTransferEncoding WINAPI GetHTTPTransferEncoding(USocket_Server_Handle h);
    SPA::ServerSide::tagContentMultiplax WINAPI GetHTTPContentMultiplax(USocket_Server_Handle h);
    bool WINAPI SetHTTPResponseCode(USocket_Server_Handle h, unsigned int errCode);
    bool WINAPI SetHTTPResponseHeader(USocket_Server_Handle h, const char *uft8Header, const char *utf8Value);
    unsigned int WINAPI SendHTTPReturnDataA(USocket_Server_Handle h, const char *str, unsigned int chars = (~0));
    unsigned int WINAPI SendHTTPReturnDataW(USocket_Server_Handle h, const wchar_t *str, unsigned int chars = (~0));
    const char* WINAPI GetHTTPId(USocket_Server_Handle h);
    unsigned int WINAPI GetHTTPCurrentMultiplaxHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);

    void* WINAPI GetSSL(USocket_Server_Handle h);
    bool WINAPI GetReturnRandom(unsigned int serviceId);
    void WINAPI SetReturnRandom(unsigned int serviceId, bool random);

    unsigned int WINAPI GetSwitchTime();
    void WINAPI SetSwitchTime(unsigned int switchTime);

    //Query the number of socket connections
    unsigned int WINAPI GetCountOfClients();

    //Get a socket handle by a zero-based index
    USocket_Server_Handle WINAPI GetClient(unsigned int index);

    //bool WINAPI IsMainThread();

    /*
    Turn on/off default online compressiong
    Note that you can overwrite it at any time at either client or server side.
     */
    void WINAPI SetDefaultZip(bool zip);
    bool WINAPI GetDefaultZip();

    /*
    The maximun of socket connections can be made from a machine
    By default, it is 32.
    It is designed for preventing Denial of Sevice attack
     */
    void WINAPI SetMaxConnectionsPerClient(unsigned int maxConnectionsPerClient);
    unsigned int WINAPI GetMaxConnectionsPerClient();

    /*
    All of worker threads except STA threads can suicide after they are idle over a limited time.
    By default, the time limit is 60,000 ms.
     */
    void WINAPI SetMaxThreadIdleTimeBeforeSuicide(unsigned int maxThreadIdleTimeBeforeSuicide);
    unsigned int WINAPI GetMaxThreadIdleTimeBeforeSuicide();

    void WINAPI SetTimerElapse(unsigned int timerElapse);
    unsigned int WINAPI GetTimerElapse();

    /*
    A property indicating a time interval for shrinking memory 
    if a socket session doesn't detect any data movement.
     */
    unsigned int WINAPI GetSMInterval();
    void WINAPI SetSMInterval(unsigned int SMInterval);

    /*
    Ping interval time.
    It is 60,000 ms by default.
    Note that if a socket connection has data movement, it will no be pinged
     */
    void WINAPI SetPingInterval(unsigned int pingInterval);
    unsigned int WINAPI GetPingInterval();

    /*
    Interval time for recycling global memory
    It is 1,200,000 ms by default
     */
    void WINAPI SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval);
    unsigned int WINAPI GetRecycleGlobalMemoryInterval();

    SPA::UINT64 WINAPI GetRequestCount();

    unsigned int WINAPI StartHTTPChunkResponse(USocket_Server_Handle h);
    unsigned int WINAPI SendHTTPChunk(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len);
    unsigned int WINAPI EndHTTPChunkResponse(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len);

    bool WINAPI IsFakeRequest(USocket_Server_Handle h);

    bool WINAPI SetZip(USocket_Server_Handle h, bool bZip);
    bool WINAPI GetZip(USocket_Server_Handle h);
    void WINAPI SetZipLevel(USocket_Server_Handle h, SPA::tagZipLevel zl);
    SPA::tagZipLevel WINAPI GetZipLevel(USocket_Server_Handle h);

    unsigned int WINAPI StartQueue(const char *qName, bool dequeueShared, unsigned int ttl);
    unsigned int WINAPI GetMessagesInDequeuing(unsigned int qHandle);
    SPA::UINT64 WINAPI Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size);
    SPA::UINT64 WINAPI GetMessageCount(unsigned int qHandle);
    bool WINAPI StopQueueByHandle(unsigned int qHandle, bool permanent);
    bool WINAPI StopQueueByName(const char *qName, bool permanent);
    SPA::UINT64 WINAPI GetQueueSize(unsigned int qHandle);
    SPA::UINT64 WINAPI Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime);
    bool WINAPI IsQueueStartedByName(const char *qName);
    bool WINAPI IsQueueStartedByHandle(unsigned int qHandle);
    bool WINAPI IsQueueSecuredByName(const char *qName);
    bool WINAPI IsQueueSecuredByHandle(unsigned int qHandle);
    const char* WINAPI GetQueueName(unsigned int qHandle);
    const char* WINAPI GetQueueFileName(unsigned int qHandle);
    SPA::UINT64 WINAPI Dequeue2(unsigned int qHandle, USocket_Server_Handle h, unsigned int maxBytes, bool beNotifiedWhenAvailable, unsigned int waitTime);
    void WINAPI EnableClientDequeue(USocket_Server_Handle h, bool enable);
    bool WINAPI IsDequeueRequest(USocket_Server_Handle h);

    bool WINAPI AbortJob(unsigned int qHandle);
    bool WINAPI StartJob(unsigned int qHandle);
    bool WINAPI EndJob(unsigned int qHandle);
    SPA::UINT64 WINAPI GetJobSize(unsigned int qHandle);
    bool WINAPI SetRouting(unsigned int serviceId0, SPA::ServerSide::tagRoutingAlgorithm ra0, unsigned int serviceId1, SPA::ServerSide::tagRoutingAlgorithm ra1);
    unsigned int WINAPI CheckRouting(unsigned int serviceId);
    bool WINAPI AddAlphaRequest(unsigned int serviceId, unsigned short reqId);
    unsigned int WINAPI GetAlphaRequestIds(unsigned int serviceId, unsigned short *reqIds, unsigned int count);
    SPA::UINT64 WINAPI GetQueueLastIndex(unsigned int qHandle);
    void WINAPI UseUTF16();
    SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(unsigned int qHandle, SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    bool WINAPI IsDequeueShared(unsigned int qHandle);
    SPA::tagQueueStatus WINAPI GetServerQueueStatus(unsigned int qHandle);
    bool WINAPI PushQueueTo(unsigned int srcHandle, const unsigned int *targetHandles, unsigned int count);
    unsigned int WINAPI GetTTL(unsigned int qHandle);
    SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(unsigned int qHandle);
    void WINAPI ResetQueue(unsigned int qHandle);
    bool WINAPI IsServerQueueIndexPossiblyCrashed();
    void WINAPI SetServerWorkDirectory(const char *dir);
    const char* WINAPI GetServerWorkDirectory();
    SPA::UINT64 WINAPI GetLastQueueMessageTime(unsigned int qHandle);
    void WINAPI AbortDequeuedMessage(USocket_Server_Handle h);
    bool WINAPI IsDequeuedMessageAborted(USocket_Server_Handle h);
    const char* WINAPI GetUServerSocketVersion();
    void WINAPI SetMessageQueuePassword(const char *pwd);
    SPA::tagOptimistic WINAPI GetOptimistic(unsigned int qHandle);
    void WINAPI SetOptimistic(unsigned int qHandle, SPA::tagOptimistic bOptimistic);
    const unsigned char* WINAPI GetRequestBuffer(USocket_Server_Handle h);
    void WINAPI SetThreadEvent(PThreadEvent func);
    unsigned int WINAPI GetMainThreads();
    SPA::CertInfo* WINAPI GetUCert(USocket_Server_Handle h);
    SPA::IUcert* WINAPI GetUCertEx(USocket_Server_Handle h);
    void WINAPI SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
    bool WINAPI SetVerifyLocation(const char *certFile);
    bool WINAPI SpeakPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    bool WINAPI SpeakExPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    bool WINAPI SendUserMessageExPush(const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool WINAPI SendUserMessagePush(const wchar_t *userId, const unsigned char *message, unsigned int size);
    void WINAPI RegisterMe(unsigned int svsId, SPA::UINT64 secretNumber);
    SPA::UINT64 WINAPI BatchEnqueue(unsigned int qHandle, unsigned int count, const unsigned char *msgStruct);
    unsigned int WINAPI SendReturnDataIndex(USocket_Server_Handle h, SPA::UINT64 index, unsigned short usReqId, unsigned int ulBufferSize, const unsigned char *pBuffer);
    unsigned int WINAPI SendExceptionResultIndex(USocket_Server_Handle h, SPA::UINT64 index, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode);
    SPA::UINT64 WINAPI GetCurrentRequestIndex(USocket_Server_Handle h);

#ifdef __cplusplus
}
#endif


#endif