#ifndef __UMB_COMM_CLIENT_H__
#define __UMB_COMM_CLIENT_H__

#include "ucomm.h"

namespace SPA {
    namespace ClientSide {
        static const unsigned int DEFAULT_RECV_TIMEOUT = 30000;
        static const unsigned int DEFAULT_CONN_TIMEOUT = 30000;

        enum tagConnectionState {
            csClosed = 0,
            csConnecting,
            csSslShaking,
            csClosing,
            csConnected,
            csSwitched
        };

        struct CMessageSender {
            const wchar_t* UserId;
            const char* IpAddress;
            unsigned short Port;
            unsigned int ServiceId;
            bool SelfMessage;
        };

        enum tagSocketPoolEvent {
            speUnknown = -1,
            speStarted = 0,
            speCreatingThread,
            speThreadCreated,
            speConnecting,
            speConnected,
            speKillingThread,
            speShutdown,
            speUSocketCreated,
            speHandShakeCompleted,
            speLocked,
            speUnlocked,
            speThreadKilled,
            speClosingSocket,
            speSocketClosed,
            speUSocketKilled,
            speTimer,
            speQueueMergedFrom,
            speQueueMergedTo,
        };

        struct UClientSocketBase {
        public:

            virtual ~UClientSocketBase() {
            }
        };
    }; //namespace ClientSide
}; //namespace SPA


#ifdef __cplusplus
extern "C" {
#endif

    typedef SPA::ClientSide::UClientSocketBase *USocket_Client_Handle;

    typedef void (CALLBACK *POnSocketClosed) (USocket_Client_Handle handler, int nError);
    typedef void (CALLBACK *POnHandShakeCompleted) (USocket_Client_Handle handler, int nError);
    typedef void (CALLBACK *POnSocketConnected) (USocket_Client_Handle handler, int nError);
    typedef void (CALLBACK *POnRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
    typedef void (CALLBACK *POnBaseRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId);
    typedef void (CALLBACK *POnEnter) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count);
    typedef void (CALLBACK *POnExit) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count);
    typedef void (CALLBACK *POnSpeakEx) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
    typedef void (CALLBACK *POnSpeak) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
    typedef void (CALLBACK *POnSendUserMessage) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned char *message, unsigned int size);
    typedef void (CALLBACK *POnSendUserMessageEx) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned char *pMessage, unsigned int size);
    typedef void (CALLBACK *POnServerException) (USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
    typedef void (CALLBACK *POnAllRequestsProcessed) (USocket_Client_Handle handler, unsigned short lastRequestId);

    //<python>
    typedef void (CALLBACK *POnEnter2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
    typedef void (CALLBACK *POnExit2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
    typedef void (CALLBACK *POnSpeakEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
    typedef void (CALLBACK *POnSpeak2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
    typedef void (CALLBACK *POnSendUserMessage2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *message, unsigned int size);
    typedef void (CALLBACK *POnSendUserMessageEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *pMessage, unsigned int size);
    void WINAPI SetOnEnter2(USocket_Client_Handle h, POnEnter2 p);
    void WINAPI SetOnExit2(USocket_Client_Handle h, POnExit2 p);
    void WINAPI SetOnSpeakEx2(USocket_Client_Handle h, POnSpeakEx2 p);
    void WINAPI SetOnSendUserMessageEx2(USocket_Client_Handle h, POnSendUserMessageEx2 p);
    void WINAPI SetOnSendUserMessage2(USocket_Client_Handle h, POnSendUserMessage2 p);
    void WINAPI SetOnSpeak2(USocket_Client_Handle h, POnSpeak2 p);
    //</python>

    typedef void(CALLBACK *PSocketPoolCallback) (unsigned int, SPA::ClientSide::tagSocketPoolEvent, USocket_Client_Handle);

    unsigned int WINAPI CreateSocketPool(PSocketPoolCallback spc, unsigned int maxSocketsPerThread, unsigned int maxThreads = 0, bool bAvg = true, SPA::tagThreadApartment ta = SPA::taNone);

    //If successful, the method returns true. Note the method will return false if the pool has one or more sockets which are not released
    bool WINAPI DestroySocketPool(unsigned int poolId);

    unsigned int WINAPI GetNumberOfSocketPools();

    USocket_Client_Handle WINAPI FindAClosedSocket(unsigned int poolId);
    bool WINAPI AddOneThreadIntoPool(unsigned int poolId);
    unsigned int WINAPI GetLockedSockets(unsigned int poolId);
    unsigned int WINAPI GetIdleSockets(unsigned int poolId);
    unsigned int WINAPI GetConnectedSockets(unsigned int poolId);
    bool WINAPI DisconnectAll(unsigned int poolId);
    USocket_Client_Handle WINAPI LockASocket(unsigned int poolId, unsigned int timeout, USocket_Client_Handle hSameThread = NULL);
    bool WINAPI UnlockASocket(unsigned int poolId, USocket_Client_Handle h);

    unsigned int WINAPI GetSocketsPerThread(unsigned int poolId);
    bool WINAPI IsAvg(unsigned int poolId);

    //return the number of dead sockets which are in disconnected state
    unsigned int WINAPI GetDisconnectedSockets(unsigned int poolId);
    //return the number of threads in a pool with id = poolId
    unsigned int WINAPI GetThreadCount(unsigned int poolId);


    //client socket operations
    void WINAPI Close(USocket_Client_Handle h);
    bool WINAPI Connect(USocket_Client_Handle h, const char* host, unsigned int portNumber, bool sync = false, bool v6 = false);
    unsigned int WINAPI GetCountOfRequestsQueued(USocket_Client_Handle h);
    unsigned short WINAPI GetCurrentRequestID(USocket_Client_Handle h);
    unsigned int WINAPI GetCurrentResultSize(USocket_Client_Handle h);
    SPA::tagEncryptionMethod WINAPI GetEncryptionMethod(USocket_Client_Handle h);
    int WINAPI GetErrorCode(USocket_Client_Handle h);
    unsigned int WINAPI GetErrorMessage(USocket_Client_Handle h, char *str, unsigned int bufferLen);
    unsigned int WINAPI GetSocketPoolId(USocket_Client_Handle h);
    bool WINAPI IsOpened(USocket_Client_Handle h);
    unsigned int WINAPI RetrieveResult(USocket_Client_Handle h, unsigned char *pBuffer, unsigned int size);
    bool WINAPI SendRequest(USocket_Client_Handle h, unsigned short reqId, const unsigned char *pBuffer, unsigned int len);
    void WINAPI SetOnHandShakeCompleted(USocket_Client_Handle h, POnHandShakeCompleted p);
    void WINAPI SetOnRequestProcessed(USocket_Client_Handle h, POnRequestProcessed p);
    void WINAPI SetOnSocketClosed(USocket_Client_Handle h, POnSocketClosed p);
    void WINAPI SetOnSocketConnected(USocket_Client_Handle h, POnSocketConnected p);
    void WINAPI SetOnBaseRequestProcessed(USocket_Client_Handle h, POnBaseRequestProcessed p);
    void WINAPI SetOnAllRequestsProcessed(USocket_Client_Handle h, POnAllRequestsProcessed p);

    //If socket is closed, batching requests or timed out, it will return false
    bool WINAPI WaitAll(USocket_Client_Handle h, unsigned int nTimeout = (~0));
    bool WINAPI Cancel(USocket_Client_Handle h, unsigned int requestsQueued = (~0));
    bool WINAPI IsRandom(USocket_Client_Handle h);
    unsigned int WINAPI GetBytesInSendingBuffer(USocket_Client_Handle h);
    unsigned int WINAPI GetBytesInReceivingBuffer(USocket_Client_Handle h);
    bool WINAPI IsBatching(USocket_Client_Handle h);
    unsigned int WINAPI GetBytesBatched(USocket_Client_Handle h);
    bool WINAPI StartBatching(USocket_Client_Handle h);
    bool WINAPI CommitBatching(USocket_Client_Handle h, bool batchingAtServerSide);
    bool WINAPI AbortBatching(USocket_Client_Handle h);
    SPA::UINT64 WINAPI GetBytesReceived(USocket_Client_Handle h);
    SPA::UINT64 WINAPI GetBytesSent(USocket_Client_Handle h);
    void WINAPI SetUserID(USocket_Client_Handle h, const wchar_t *userId);
    unsigned int WINAPI GetUID(USocket_Client_Handle h, wchar_t *userId, unsigned int chars);
    void WINAPI SetPassword(USocket_Client_Handle h, const wchar_t *password);
    bool WINAPI SwitchTo(USocket_Client_Handle h, unsigned int serviceId);
    bool WINAPI Enter(USocket_Client_Handle h, const unsigned int *pChatGroupId, unsigned int count);
    void WINAPI Exit(USocket_Client_Handle h);
    bool WINAPI Speak(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
    bool WINAPI SpeakEx(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
    bool WINAPI SendUserMessage(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool WINAPI SendUserMessageEx(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Client_Handle h);
    void WINAPI SetOnEnter(USocket_Client_Handle h, POnEnter p);
    void WINAPI SetOnExit(USocket_Client_Handle h, POnExit p);
    void WINAPI SetOnSpeakEx(USocket_Client_Handle h, POnSpeakEx p);
    void WINAPI SetOnSendUserMessageEx(USocket_Client_Handle h, POnSendUserMessageEx p);
    SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Client_Handle h, bool *endian);
    void WINAPI SetOnServerException(USocket_Client_Handle h, POnServerException p);
    void WINAPI SetOnSendUserMessage(USocket_Client_Handle h, POnSendUserMessage p);
    void WINAPI SetOnSpeak(USocket_Client_Handle h, POnSpeak p);
    void WINAPI SetZip(USocket_Client_Handle h, bool zip);
    bool WINAPI GetZip(USocket_Client_Handle h);
    SPA::tagZipLevel WINAPI GetZipLevel(USocket_Client_Handle h);
    unsigned int WINAPI GetCurrentServiceId(USocket_Client_Handle h);

    bool WINAPI StartQueue(USocket_Client_Handle h, const char *qName, bool secure, bool dequeueShared, unsigned int ttl);
    void WINAPI StopQueue(USocket_Client_Handle h, bool permanent);
    bool WINAPI DequeuedResult(USocket_Client_Handle h);
    unsigned int WINAPI GetMessagesInDequeuing(USocket_Client_Handle h);
    SPA::UINT64 WINAPI GetMessageCount(USocket_Client_Handle h);
    SPA::UINT64 WINAPI GetQueueSize(USocket_Client_Handle h);
    bool WINAPI IsQueueSecured(USocket_Client_Handle h);
    bool WINAPI IsQueueStarted(USocket_Client_Handle h);
    const char* WINAPI GetQueueName(USocket_Client_Handle h);
    const char* WINAPI GetQueueFileName(USocket_Client_Handle h);
    bool WINAPI DoEcho(USocket_Client_Handle h);
    bool WINAPI TurnOnZipAtSvr(USocket_Client_Handle h, bool enableZip);
    bool WINAPI GetPeerName(USocket_Client_Handle h, unsigned int *peerPort, char *ipAddr, unsigned short chars);
    void WINAPI SetRecvTimeout(USocket_Client_Handle h, unsigned int timeout);
    unsigned int WINAPI GetRecvTimeout(USocket_Client_Handle h);
    void WINAPI SetConnTimeout(USocket_Client_Handle h, unsigned int timeout);
    unsigned int WINAPI GetConnTimeout(USocket_Client_Handle h);
    void WINAPI SetAutoConn(USocket_Client_Handle h, bool autoConnecting);
    bool WINAPI GetAutoConn(USocket_Client_Handle h);
    unsigned short WINAPI GetServerPingTime(USocket_Client_Handle h);
    SPA::CertInfo* WINAPI GetUCert(USocket_Client_Handle h);
    SPA::IUcert* WINAPI GetUCertEx(USocket_Client_Handle h);
    void* WINAPI GetSSL(USocket_Client_Handle h);
    bool WINAPI IgnoreLastRequest(USocket_Client_Handle h, unsigned short reqId);
    bool WINAPI SetVerifyLocation(const char *certFile);
    const char* WINAPI Verify(USocket_Client_Handle h, int *errCode);
    bool WINAPI IsDequeueEnabled(USocket_Client_Handle h);
    bool WINAPI AbortJob(USocket_Client_Handle h);
    bool WINAPI StartJob(USocket_Client_Handle h);
    bool WINAPI EndJob(USocket_Client_Handle h);
    SPA::UINT64 WINAPI GetJobSize(USocket_Client_Handle h);
    bool WINAPI IsRouteeRequest(USocket_Client_Handle h);
    bool WINAPI SendRouteeResult(USocket_Client_Handle h, unsigned short reqId, const unsigned char *buffer, unsigned int len);
    unsigned int WINAPI GetRouteeCount(USocket_Client_Handle h);
    void WINAPI UseUTF16();
    SPA::UINT64 WINAPI GetQueueLastIndex(USocket_Client_Handle h);
    SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(USocket_Client_Handle h, SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    bool WINAPI IsDequeueShared(USocket_Client_Handle h);
    SPA::tagQueueStatus WINAPI GetClientQueueStatus(USocket_Client_Handle h);
    bool WINAPI PushQueueTo(USocket_Client_Handle src, const USocket_Client_Handle *targets, unsigned int count);
    unsigned int WINAPI GetTTL(USocket_Client_Handle h);
    SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(USocket_Client_Handle h);
    void WINAPI ResetQueue(USocket_Client_Handle h);
    void WINAPI SetClientWorkDirectory(const char *dir);
    const char* WINAPI GetClientWorkDirectory();
    SPA::UINT64 WINAPI GetLastQueueMessageTime(USocket_Client_Handle h);
    bool WINAPI IsRouting(USocket_Client_Handle h);
    void WINAPI AbortDequeuedMessage(USocket_Client_Handle h);
    bool WINAPI IsDequeuedMessageAborted(USocket_Client_Handle h);
    bool WINAPI IsClientQueueIndexPossiblyCrashed();
    void WINAPI SetEncryptionMethod(USocket_Client_Handle h, SPA::tagEncryptionMethod em);
    void WINAPI SetZipLevel(USocket_Client_Handle h, SPA::tagZipLevel zl);
    bool WINAPI SetZipLevelAtSvr(USocket_Client_Handle h, SPA::tagZipLevel zipLevel);
    bool WINAPI SetSockOpt(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    bool WINAPI SetSockOptAtSvr(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    void WINAPI Shutdown(USocket_Client_Handle h, SPA::tagShutdownType how);
    const char* WINAPI GetUClientSocketVersion();
    void WINAPI SetMessageQueuePassword(const char *pwd);
    SPA::ClientSide::tagConnectionState WINAPI GetConnectionState(USocket_Client_Handle h);
    void WINAPI SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
    void WINAPI EnableRoutingQueueIndex(USocket_Client_Handle h, bool enable);
    bool WINAPI IsRoutingQueueIndexEnabled(USocket_Client_Handle h);
    const char* WINAPI GetUClientAppName();
    SPA::tagOptimistic WINAPI GetOptimistic(USocket_Client_Handle h);
    void WINAPI SetOptimistic(USocket_Client_Handle h, SPA::tagOptimistic optimistic);
    const unsigned char* WINAPI GetResultBuffer(USocket_Client_Handle h);
    bool WINAPI GetQueueAutoMergeByPool(unsigned int poolId);
    void WINAPI SetQueueAutoMergeByPool(unsigned int poolId, bool autoMerge);

#ifdef __cplusplus
}
#endif

#endif
