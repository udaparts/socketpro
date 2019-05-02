#pragma once


#include "serverthread_win.h"
#include "../../pinc/bf.h"
#include "../../pinc/mqfile.h"
#include "../../include/userver.h"
#include "../../apps/hparser/httpcontext.h"
#include "../../apps/hparser/connectioncontext.h"
#include "../../pinc/getsysid.h"
#ifndef NOT_USE_OPENSSL
#include "servicecontext.h"
#else
#include "../../comm/servercore/servicecontext.h"
#endif
#include "../shared/includes.h"

extern bool g_bRegistered;

class CServerRegistration : public SPA::ServerSide::URegistration {
public:
    CServerRegistration();

public:
    void Initialize();
    void AddCall(unsigned int svsId, SPA::tagOperationSystem os, bool queue, bool bigEndian);
    bool ShouldShutdown();
    void SetOSs();

private:
    SPA::UINT64 m_nCheapCall;
    bool m_bWin;
    bool m_bApple;
    bool m_bWinCe;
    bool m_bUnix;
    bool m_bAndroid;
    bool m_bTimeok;

private:
    static const unsigned int MAX_ALLOWED_CHEAP_CALLS = 1000;
};

//
#ifdef WIN32_64
#define MAX_SESSION_INDEX (0xFFFFF)
#define INDEX_SHIFT_BITS 20
#else
#define MAX_SESSION_INDEX (0xFFFF)
#define INDEX_SHIFT_BITS 16
#endif

#define SOCKET_CLOSE_EVENT              (WM_CONTINUE_PROCESSING + 0x10)
#define MEMOREY_QUEUE_HEADER_REST_SIZE	(8*1460)

/*
#define DEFAULT_MAX_REQUEST_SIZE        ((unsigned int)(100*1460))
#define MAX_RECV_HEADER_SIZE            (5*1460)
#define BINARY_REQUEST_SIZE             (10*1460)
 */

typedef std::vector<std::pair<SPA::UINT64, SPA::UINT64> > CQueueProperty;

class CServerSession : private boost::noncopyable {

    struct RouteMap {
        SPA::UINT64 Sender;
        MQ_FILE::QAttr Qa;
        SPA::UINT64 Receiver;

        inline bool operator ==(const RouteMap & rm) const {
            return (Sender == rm.Sender && Qa == rm.Qa && Receiver == rm.Receiver);
        }
        unsigned short RequestId;
    };

public:
    CServerSession();
    virtual ~CServerSession();

    //no copy and assign operators
    CServerSession(const CServerSession &ss);
    CServerSession& operator=(const CServerSession &ss);

public:
    SPA::tagOperationSystem WINAPI GetPeerOs(bool *endian);
    CSocket& GetSocket();
    void Start();
    SPA::UINT64 GetSocketNativeHandle();
    void Initialize();
    void Close();
    void PostClose(int errCode = 0);
    bool IsOpened();
    int GetErrorCode();
    std::string GetErrorMessage();
    unsigned int GetSvsID();
    unsigned int GetCurrentRequestLen();
    unsigned short GetCurrentRequestID();
    unsigned int GetSndBytesInQueue();
    unsigned int GetRcvBytesInQueue();
    unsigned int QueryRequestsQueued();
    unsigned int GetConnIndex();
    unsigned int RetrieveRequestBuffer(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek);
    unsigned int SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize);

    SPA::UINT64 GetBytesReceived();
    SPA::UINT64 GetBytesSent();
    bool IsBatching();
    unsigned int GetBytesBatched();
    bool StartBatching();
    bool CommitBatching();
    bool AbortBatching();
    bool Wait(unsigned int nTimeout = 100);
    int ExecuteSlowRequestFromThreadPool(unsigned short sReqId);
    void SetUserID(const wchar_t *strUserId);
    unsigned int GetUID(wchar_t *strUserId, unsigned int chars);
    void SetPassword(const wchar_t *strPassword);
    unsigned int GetPassword(wchar_t *strPassword, unsigned int chars);
    bool Enter(const unsigned int *pChatGroupId, unsigned int nCount);
    void Exit();
    bool Speak(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size);
    unsigned int GetCountOfJoinedChatGroups();
    unsigned int GetJoinedGroupIds(unsigned int *pChatGroup, unsigned int count);
    void GetPeerName(std::string &addr, unsigned short *port);
    unsigned int SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
    unsigned int SendExceptionResult(const char* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
    bool FakeAClientRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int nBufferSize);
    bool IsCanceled();

    //HTTP
    unsigned int GetHTTPRequestHeaders(SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);
    const char* GetHTTPPath();
    SPA::UINT64 GetHTTPContentLength();
    const char* GetHTTPQuery();
    bool DownloadFile(const char *filePath);
    SPA::ServerSide::tagHttpMethod GetHTTPMethod();
    bool HTTPKeepAlive();
    bool IsWebSocket();
    bool IsCrossDomain();
    double GetHTTPVersion();
    bool HTTPGZipAccepted();
    const char* GetHTTPUrl();
    const char* GetHTTPHost();
    SPA::ServerSide::tagTransport GetHTTPTransport();
    SPA::ServerSide::tagTransferEncoding GetHTTPTransferEncoding();
    SPA::ServerSide::tagContentMultiplax GetHTTPContentMultiplax();
    bool SetHTTPResponseCode(unsigned int errCode);
    bool SetHTTPResponseHeader(const char *uft8Header, const char *utf8Value);
    unsigned int SendHTTPReturnDataA(const char *str, unsigned int chars);
    USocket_Server_Handle MakeHandler();
    const char* GetHTTPId();
    unsigned int GetHTTPCurrentMultiplaxHeaders(SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);
    unsigned int HTTPCallbackA(const char *name, const char *str);
    void ShrinkMemory();
    unsigned int StartChunkResponse();
    unsigned int SendChunk(const unsigned char *buffer, unsigned int len);
    unsigned int EndChunkResponse(const unsigned char *buffer, unsigned int len);
    bool IsFakeRequest();
    bool IsOld();
    void SetZip(bool bZip);
    bool GetZip();
    void SetZipLevel(SPA::tagZipLevel zl);
    SPA::tagZipLevel GetZipLevel();
    void* GetSSL();
    bool GetClientInfo(SPA::CSwitchInfo *pClientInfo);
    bool GetServerInfo(SPA::CSwitchInfo *pServerInfo);
    bool SetServerInfo(SPA::CSwitchInfo *pServerInfo);
    bool GetSockAddr(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars);
    void DropCurrentSlowRequest();
    void EnableClientDequeue(bool enable);
    void SetPeerDequeueFailed(bool fail);
    bool GetPeerDequeueFailed();
    bool IsDequeueRequest();
    unsigned int GetWritingBufferSizeAndSendTime(boost::posix_time::ptime &sendTime);
    unsigned int GetWritingBufferSize();

private:
    static unsigned int CompressResultTo(bool old, unsigned short reqId, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q);
    static unsigned int DecompressRequestTo(unsigned short ratio, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q);
    USocket_Server_Handle MakeHandlerInternal();
    void ConfirmFailed();
    bool Process();
    bool ProcessWebSocketRequest();
    bool ProcessAjaxRequest();
    bool ProcessJavaScriptRequest();
    bool PreocessWebRequest(SPA::CUQueue &q);
    bool IsSecure();
    bool ProcessHttpRequest();
    inline bool IsSameEndian();
    bool ProcessWithLock();
    void PostCloseInternal(int errCode);
    unsigned int GetUserIdInternally(wchar_t *strUserId, unsigned int chars);
    void CloseInternal();
    unsigned int RetrieveRequestBufferInternally(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek);
    unsigned int QueryRequestsQueuedInternally();
    bool IsCanceledInternally();
    void Exit(const unsigned int *pChatGroupId, unsigned int nCount);
    void BounceBackMessage(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size);
    void SendChatResult(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, unsigned short usReqId, const unsigned char *pBuffer, unsigned int size);
    boost::posix_time::ptime& GetLatestTime();
    void OnRA();
    void SetContext();
    void OnSslHandShake(const CErrorCode& Error);
    void OnReadCompleted(const CErrorCode& Error, size_t bytes_transferred);
    void OnWriteCompleted(const CErrorCode& Error);
    void OnClose();
    void Read();
    unsigned int Write(const unsigned char *s, unsigned int nSize);
    void OnSlowRequestProcessed(unsigned int res, unsigned short usRequestId);
    void OnBaseRequestArrive();
    void OnNonBaseRequestArrive();
    void OnChatRequestArrive();
    void OnChatVariantRequestArrive();
    static unsigned char* GetIoBuffer();
    static void ReleaseIoBuffer(unsigned char *buffer);
    bool DoAuthentication(unsigned int ServiceId);
    void OnSwitchTo(unsigned int OldServiceId, unsigned int NewServiceId);
    bool Decompress();
    bool SetServerInfoInternal(SPA::CSwitchInfo *pServerInfo);
    void NotifyDequeued();
    void SetVQtrans(unsigned int qHandle);
    bool IsRoutable(unsigned short reqId);
    bool Route();
    static bool RemoveARouteMap(const RouteMap &rm);
    static void NotifyFailRoutes(SPA::UINT64 receiver);

private:
    CServerThread *m_pUThread;
    SPA::CSwitchInfo m_ClientInfo;
    SPA::CSwitchInfo m_ServerInfo;
    SPA::CStreamHeader m_ReqInfo;
    unsigned int ServiceId;
    CSocket *m_pSocket;
	std::shared_ptr<SPA::CMsSsl> m_pSsl;
    SPA::CUQueue m_qRead;
    SPA::CUQueue m_qWrite;
    unsigned char *m_ReadBuffer;
    bool m_bRBLocked;
    unsigned char *m_WriteBuffer;
    bool m_bWBLocked;
    boost::mutex m_mutex;
    unsigned int m_ulIndex;
    bool m_bZip;
    SPA::tagZipLevel m_zl;
    SPA::CUQueue *m_pQBatch;
    CErrorCode m_ec;
    CConditionVariable m_cv;
    bool m_bDropSlowRequest;
    UHTTP::CHttpContext *m_pHttpContext;
    Connection::CConnectionContextBase m_ccb;
    unsigned int m_nHttpCallCount;
    tagConnectionState m_cs;

    //persistent queue
    typedef std::map<unsigned int, CQueueProperty> CQueueMap;
    CQueueMap m_mapDequeue;
    MQ_FILE::CQueueInitialInfo m_ClientQFile;
    bool m_bFail;
    bool m_bDequeueTrans;
    MQ_FILE::QAttr m_qa;
    std::vector<MQ_FILE::QAttr> m_vQTrans;
    bool m_bConfirmTrans;
    bool m_bConfirmFail;

    //routing
    CServiceContext *m_pServiceContext;
    CServiceContext *m_pRoutingServiceContext;
    SPA::UINT64 m_routeeHandle;
    SPA::UINT64 m_routerHandle;
	bool m_bRouteBatching;
    static SPA::CUQueue m_qRouteRequestId;

    static boost::mutex m_qMutex;
    static std::vector<unsigned char*> m_aBuffer;
    static boost::mutex m_mutexBuffer;
    static const unsigned int MULTIPLE_CONTEXT_LENGTH = 30 * 1024;

public:
    static std::shared_ptr<MQ_FILE::CQLastIndex> m_pQLastIndex;

private:
    friend class CServer;
    friend class CServiceContext;
    friend class CServerRegistration;
};

typedef CServerSession* PSession;
