#ifndef _UMB_COMM_SERVER_H__
#define _UMB_COMM_SERVER_H__

#include "session.h"
#include <boost/filesystem.hpp>
#include "../core_shared/shared/certificateimpl.h"

struct PlugImpl {
    PInitServerLibrary InitServerLibrary;
    PUninitServerLibrary UninitServerLibrary;
    PGetNumOfServices GetNumOfServices;
    PGetAServiceID GetAServiceID;
    PGetOneSvsContext GetOneSvsContext;
    PGetNumOfSlowRequests GetNumOfSlowRequests;
    PGetOneSlowRequestID GetOneSlowRequestID;

    bool IsOk() {
        return (InitServerLibrary &&
                UninitServerLibrary &&
                GetNumOfServices &&
                GetAServiceID &&
                GetOneSvsContext &&
                GetNumOfSlowRequests &&
                GetOneSlowRequestID);
    }
};

class CServer : private boost::noncopyable {
    friend class CServerRegistration;

public:
    CServer(int nParam);
    ~CServer();

    enum {
        DefaultMaxThreadIdleTimeBeforeSuicide = 60000,
        DefaultMaxConnectionsPerClient = 32,
        DefaultTimerElapse = 1000,
        DefaultSMInterval = 60000,
        DefaultPingInterval = 60000,
        DefaultRecycleGlobalMemoryInterval = 1200000,
        DefaultMinSwitchTime = 60000,
    };

public:
    void PostQuitPump();
    bool StartIOPump();
    bool StartServerInternal(unsigned int uiPort, unsigned int uiMaxBacklog, bool v6);
    void PostSproMessage(SPA::CUThreadMessage message);
    bool PostSproMessage(CServerSession *pSession, unsigned int nMsgId, int nEvent, int nData);
    bool PostSproMessage(CServerSession *pSession, unsigned int nMsgId, const void *pBuffer, unsigned int nSize);
    void SetPrivateKeyFile(const char *keyFile);
    void SetCertFile(const char *certFile);
    void SetDhFile(const char* file);
    void SetPKFPassword(const char *certFile);
    void SetMessageQueuePassword(const char *pwd);
    void SetEncryptionMethod(SPA::tagEncryptionMethod em);
    SPA::tagEncryptionMethod GetEncryptionMethod();
    void SetPfxFile(const char *pfxFile);
    void SetAuthenticationMethod(SPA::ServerSide::tagAuthenticationMethod am);
    SPA::ServerSide::tagAuthenticationMethod GetAuthenticationMethod();
    void SetSharedAM(bool b);
    bool GetSharedAM();
    int GetErrorCode();
    std::string GetErrorMessage();
    bool IsRunning();
    bool IsSSLEnabled();
    void SetOnAccept(POnAccept p);
    void StopSocketProServer();
    CServiceContext* SeekServiceContext(unsigned int nServiceId);
    void RemoveASvsContext(unsigned int nServiceId);
    bool AddSlowRequest(unsigned int nServiceId, unsigned short sReqId);
    void RemoveSlowRequest(unsigned int nServiceId, unsigned short sReqId);
    size_t GetCountOfServices();
    unsigned int GetServices(unsigned int *pServiceId, unsigned int count);
    unsigned int GetCountOfSlowRequests(unsigned int nServiceId);
    unsigned int GetAllSlowRequestIds(unsigned int nServiceId, unsigned short *pReqId, unsigned int count);
    bool AddSvsContext(unsigned int nServiceId, CSvsContext SvsContext);
    void RemoveAllSlowRequests(unsigned int serviceId);

    //not thread-safe
    bool IsSsl();
    bool IsMainThread();

    void AddAChatGroup(unsigned int chatGroupId, const wchar_t *description);
    unsigned int GetCountOfChatGroups();
    unsigned int GetJoinedGroupIds(unsigned int *pChatGroupId, unsigned int count);
    unsigned int GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars);
    void RemoveChatGroup(unsigned int chatGroupId);
    void GetJoinedGroupIds(std::vector<unsigned int> &vChatGroup);
    void SetOnIdle(POnIdle p);
    void KillMainThread();
    unsigned int GetMainThreads();
    void SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p);
    void SetOnClose(POnClose p);
    void SetOnIsPermitted(POnIsPermitted p);
    unsigned int GetCountOfClients();
    USocket_Server_Handle GetClient(unsigned int index);
    void SetDefaultZip(bool bZip);
    bool GetDefaultZip();
    void SetMaxConnectionsPerClient(unsigned int ulMaxConnectionsPerClient);
    unsigned int GetMaxConnectionsPerClient();
    void SetMaxThreadIdleTimeBeforeSuicide(unsigned int ulMaxThreadIdleTimeBeforeSuicide);
    unsigned int GetMaxThreadIdleTimeBeforeSuicide();
    void SetTimerElapse(unsigned int ulTimerElapse);
    unsigned int GetTimerElapse();
    unsigned int GetSMInterval();
    void SetSMInterval(unsigned int ulSMInterval);
    void SetPingInterval(unsigned int pingInterval);
    unsigned int GetPingInterval();
    void SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval);
    unsigned int GetRecycleGlobalMemoryInterval();
    SPA::UINT64 GetRequestCount();
    unsigned int GetSwitchTime();
    void SetSwitchTime(unsigned int switchTime);
    unsigned int StartQueue(const char *qName, bool dequeueShared, unsigned int ttl);
    const char* GetQueueFileName(unsigned int qHandle);
    unsigned int GetMessagesInDequeuing(unsigned int qHandle);
    SPA::UINT64 Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size);
    SPA::UINT64 BatchEnqueue(unsigned int qHandle, unsigned int count, const unsigned char *messages);
    SPA::UINT64 GetMessageCount(unsigned int qHandle);
    bool StopQueueByHandle(unsigned int qHandle, bool permanent);
    bool StopQueueByName(const char *qName, bool permanent);
    SPA::UINT64 GetQueueSize(unsigned int qHandle);
    SPA::UINT64 Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime);
    SPA::UINT64 Dequeue2(unsigned int qHandle, USocket_Server_Handle h, unsigned int bytes, bool beNotifiedWhenAvailable, unsigned int waitTime);
    void ConfirmQueue(unsigned int qHandle, SPA::UINT64 mqPos, SPA::UINT64 qIndex, bool successful);
    void ConfirmQueue(unsigned int qHandle, const MQ_FILE::QAttr *qa, size_t count);
    void ConfirmQueueJob(unsigned int qHandle, const MQ_FILE::QAttr *qa, size_t count, bool successful);
    bool IsQueueStarted(const char *qName);
    bool IsQueueStarted(unsigned int qHandle);
    bool IsQueueSecured(const char *qName);
    bool IsQueueSecured(unsigned int qHandle);
    bool IsDequeueShared(unsigned int qHandle);
    unsigned int GetTTL(unsigned int qHandle);
    void ResetQueue(unsigned int qHandle);
    const char* GetQueueName(unsigned int qHandle);
    unsigned int CancelQueuedRequests(unsigned int qHandle, const unsigned short *ids, unsigned int count);
    SPA::UINT64 CancelQueuedRequests(unsigned int qHandle, SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    HINSTANCE AddADll(const char *libFile, int nParam);
    bool RemoveALibrary(HINSTANCE hLib);
    bool HasUserId(const wchar_t *userId);
    unsigned int PeekQueuedRequests(unsigned int qHandle, SPA::CQueuedRequestInfo *qri, unsigned int count);
    bool AbortJob(unsigned int qHandle);
    bool StartJob(unsigned int qHandle);
    bool EndJob(unsigned int qHandle);
    SPA::UINT64 GetJobSize(unsigned int qHandle);
    bool SetRouting(unsigned int serviceId0, SPA::ServerSide::tagRoutingAlgorithm ra0, unsigned int serviceId1, SPA::ServerSide::tagRoutingAlgorithm ra1);
    unsigned int CheckRouting(unsigned int serviceId);
    bool AddAlphaRequest(unsigned int serviceId, unsigned short reqId);
    unsigned int GetAlphaRequestIds(unsigned int serviceId, unsigned short *reqIds, unsigned int count);
    SPA::UINT64 AppendQueue(unsigned int qHandle, unsigned int qqSrc);
    SPA::UINT64 GetQueueLastIndex(unsigned int qHandle);
    SPA::tagQueueStatus GetServerQueueStatus(unsigned int qHandle);
    std::shared_ptr<MQ_FILE::CMqFile> GetQueue(unsigned int qHandle);
    SPA::UINT64 RemoveQueuedRequestsByTTL(unsigned int qHandle);
    SPA::UINT64 GetLastQueueMessageTime(unsigned int qHandle);
    SPA::tagOptimistic GetOptimistic(unsigned int qHandle);
    void SetOptimistic(unsigned int qHandle, SPA::tagOptimistic bOptimistic);
    bool SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SendUserMessage(const wchar_t *userId, const SPA::UVariant &vtMessage);
    bool Speak(const unsigned int *pChatGroupId, unsigned int nCount, const SPA::UVariant &vtMsg);

private:
    void StartSubThread();
    void HandleServerPingInternal(SPA::UINT64 tNow, std::vector<CServerSession*> &aSession);
    void HandleThreadPoolInternal();
    void StartIOPumpInternal();
    void DestroyThreadPool();
    void PutThreadBackIntoPool(CServerThread *pThread);
    void RemoveThread(CServerThread *pThread);
    CServerThread *GetOneThread(SPA::tagThreadApartment ta);
    void CleanServiceContexts();
    bool IsTooMany(CServerSession *pSession);
    void OnMessage();
    void Recycle(CServerSession *pSession);
    std::string GetPassword() const;
    std::string GetMessageQueuePassword() const;
    void OnAccepted(CServerSession* pSession, const CErrorCode& Error);
    void OnTimer(const CErrorCode& Error);
    void OnTimerSM(const CErrorCode& Error);
    std::vector<CServerSession*>::iterator Seek(CServerSession *pSession);
    void OnQuit();
    void StartTimer();
    void Clean();
    std::vector<CServiceContext*>::iterator SeekSC(unsigned int nServiceId);
    bool Enter(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount);
    void Exit(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SpeakEx(CServerSession *pSession, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool Speak(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount, const SPA::UVariant &vtMsg);
    bool SendUserMessage(CServerSession *pSession, const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool SendUserMessage(CServerSession *pSession, const wchar_t *userId, const SPA::UVariant &vtMessage);
    void HandleSlowSwitchInternal(SPA::UINT64 tNow, std::vector<CServerSession*> &aSession);
    void DeleteSslContex();
    bool OpenPfx(const wchar_t *filePfx, const wchar_t *password);
    void FreeCredHandle();
    PCredHandle GetCredHandle();

private:
    CIoService m_IoService;

public:
    static std::string m_WorkingPath;

private:
    std::string m_strPrivateKeyFile;
    std::string m_strCertFile;
    std::string m_strDhFile;

    //max idle time (in ms) before a thread suicides, default to 60000 (60 seconds) and shouldn't be less than 1000 (1 second)
    unsigned int m_ulMaxThreadIdleTimeBeforeSuicide;

    //The max connections per client machine before rejecting socket connecting from a client machine, default to 32
    unsigned int m_ulMaxConnectionsPerClient;

    //Timer interval in ms, default to 1000 (1 second)
    unsigned int m_ulTimerElapse;

    //for shrink memory in ms, default to 60000 (60 seconds)
    unsigned int m_ulSMInterval;

    //check if a client is available after the client has no data transferred, default to 60000 (60 seconds)
    unsigned int m_ulPingInterval;

    //Recycle global memory, default to 1200000 (1200 seconds)
    unsigned int m_ulRecycleGlobalMemoryInterval;
    unsigned int m_ulMinSwitchTime;

    SPA::tagEncryptionMethod m_EncryptionMethod;
    SPA::ServerSide::tagAuthenticationMethod m_am;
    bool m_bSharedAM;
    std::string m_strPfxFile;
    bool m_bZip;
    std::vector<UTHREAD_ID> m_vMainThread;
    SPA::UINT64 m_tStart;
    atomic<SPA::UINT64> m_nRequestCount;
    SPA::UINT64 m_nRequestCountLast;

public:
    PCertificateVerifyCallback m_cvc;
    atomic<bool> m_bStopped;

private:
    SPA::CSwitchInfo m_ServerInfo;
    std::deque<CServerSession*> m_aSessionDead;
    std::vector<CServerSession*> m_aSession;
    CErrorCode m_ec;
    CAcceptor *m_pAcceptor;
    CServerSession *m_pSession;
    std::mutex m_mutex;
    unsigned int m_nConnIndex;
    POnAccept m_pOnAccept;
    std::vector<CServiceContext*> m_vSC;
    std::mutex m_mutexSC;
    std::mutex m_mTP;
    std::vector<CServerThread*> m_vThreadPool;
    std::map<unsigned int, std::wstring> m_mapChatGroup;
    boost::asio::deadline_timer m_Timer;
    boost::asio::deadline_timer m_TimerSM;
    POnIdle m_pOnIdle;
    std::vector<std::shared_ptr<boost::thread> > m_vThread;
    POnSSLHandShakeCompleted m_pOnSSLHandShakeCompleted;
    POnIsPermitted m_pOnIsPermitted;
    POnClose m_pOnClose;
    unsigned int m_nPort;

    SPA::CSpinLock m_mTH;
    std::queue<SPA::CUThreadMessage> m_qThreadMessage;

    //Queue
    typedef std::map<unsigned int, std::shared_ptr<MQ_FILE::CMqFile> > CMapQueue;
    std::mutex m_mQ;
    CMapQueue m_mapQueue;
    typedef std::pair<USocket_Server_Handle, bool> CSNotified;
    typedef std::map<unsigned int, std::vector<CSNotified> > CMapQNotified;
    CMapQNotified m_mapQNotified;

    std::map<HINSTANCE, PlugImpl> m_mapLib;
    int m_nParam;

    std::shared_ptr<SPA::CCertificateImpl> m_pSelfCert;
    bool m_clientCertAuth;
    SPA::tagCertStoreType m_cst;
    CredHandle m_hCreds;

    friend class CServerSession;
    friend class CConnectionContextBase;

public:
    static CServerRegistration m_reg;
};

extern CServer *g_pServer;

#endif