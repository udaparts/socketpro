
#include "../include/definebase.h"

#if defined(OLD_IMPL)
#include "server.h"
#elif defined(WIN32_64)
#include "../uservercore_win/server.h"
#else
#include "../uservercore/server.h"
#endif

#include "../include/userver.h"
#include "../core_shared/pinc/getsysid.h"
#include "../core_shared/pinc/uzip.h"
#include <boost/filesystem.hpp>
#include "../core_shared/shared/includes.h"

std::mutex g_mutex;
CServer *g_pServer = nullptr;
extern bool g_bRegistered;

CServerSession *GetSvrSession(USocket_Server_Handle h, unsigned int &index);

std::string g_strVersion("6.3.0.1");

const char* WINAPI GetUServerSocketVersion() {
    return g_strVersion.c_str();
}

bool WINAPI InitSocketProServer(int nParam) {
    CAutoLock al(g_mutex);
    if (g_pServer != nullptr)
        return true;

    if (CServer::m_WorkingPath.size() == 0) {
        boost::filesystem::path cp = boost::filesystem::current_path();
#ifdef WIN32_64
        std::string path = cp.generic_string();
        std::replace(path.begin(), path.end(), '/', '\\');
        SetServerWorkDirectory(path.c_str());
#else
        SetServerWorkDirectory(cp.c_str());
#endif
    }

    g_pServer = new CServer(nParam);
    return true;
}

void WINAPI UninitSocketProServer() {
    try{
        CAutoLock al(g_mutex);
        if (!g_pServer) return;
        if (!g_pServer->m_bStopped) {
            g_pServer->m_bStopped = true;
            CServerSession::m_cv.notify_all();
        }
        StopSocketProServer();
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
        delete g_pServer;
        g_pServer = nullptr;
    }

    catch(...) {

    }
}

bool WINAPI StartSocketProServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    if (IsRunning())
        return true;
    if (g_pServer->StartServerInternal(listeningPort, maxBacklog, v6)) {
        g_pServer->StartIOPump();
        boost::this_thread::sleep(boost::posix_time::milliseconds(250));
        return true;
    }
    return false;
}

bool WINAPI IsMainThread() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsMainThread();
}

unsigned int WINAPI GetMainThreads() {
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetMainThreads();
}

void WINAPI PostQuitPump() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || !g_pServer->IsRunning())
        return;
    if (!g_pServer->m_bStopped) {
        g_pServer->m_bStopped = true;
        CServerSession::m_cv.notify_all();
    }
    g_pServer->PostQuitPump();
}

void WINAPI RemoveASvsContext(unsigned int nServiceId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->RemoveASvsContext(nServiceId);
}

bool WINAPI AddSvsContext(unsigned int nServiceId, CSvsContext SvsContext) {
#ifndef WIN32_64
    SvsContext.m_ta = SPA::taNone;
#endif
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->AddSvsContext(nServiceId, SvsContext);
}

CSvsContext WINAPI GetSvsContext(unsigned int serviceId) {
    CSvsContext svsContext;
    ::memset(&svsContext, 0, sizeof (svsContext));
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr) {
        return svsContext;
    }
    CServiceContext *svs = g_pServer->SeekServiceContext(serviceId);
    if (svs == nullptr) {
        return svsContext;
    }
    return svs->GetSvsContext();
}

void WINAPI RemoveAllSlowRequests(unsigned int serviceId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    CServiceContext *svs = g_pServer->SeekServiceContext(serviceId);
    if (svs == nullptr) {
        return;
    }
    svs->RemoveAllSlowRequests();
}

bool WINAPI GetReturnRandom(unsigned int serviceId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    CServiceContext *svs = g_pServer->SeekServiceContext(serviceId);
    if (svs == nullptr) {
        return false;
    }
    return svs->GetRandom();
}

void WINAPI SetReturnRandom(unsigned int serviceId, bool random) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || serviceId <= SPA::sidReserved)
        return;
    CServiceContext *svs = g_pServer->SeekServiceContext(serviceId);
    if (svs == nullptr) {
        return;
    }
    return svs->SetRandom(random);
}

bool WINAPI AddSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->AddSlowRequest(nServiceId, sReqId);
}

void WINAPI RemoveSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->RemoveSlowRequest(nServiceId, sReqId);
}

unsigned int WINAPI GetCountOfServices() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return (unsigned int) (g_pServer->GetCountOfServices());
}

unsigned int WINAPI GetServices(unsigned int *pServiceId, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetServices(pServiceId, count);
}

unsigned int WINAPI GetCountOfSlowRequests(unsigned int nServiceId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetCountOfSlowRequests(nServiceId);
}

unsigned int WINAPI GetAllSlowRequestIds(unsigned int nServiceId, unsigned short *pReqId, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetAllSlowRequestIds(nServiceId, pReqId, count);
}

void WINAPI AddAChatGroup(unsigned int chatGroupId, const wchar_t *description) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->GetCountOfClients() > 0)
        return;
    if (!description) {
        g_pServer->AddAChatGroup(chatGroupId, L"");
    } else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
        const SPA::UTF16 *str = (const SPA::UTF16 *)description;
        unsigned int len = SPA::Utilities::GetLen(str);
        std::wstring s = SPA::ToNativeString(str, len);
        g_pServer->AddAChatGroup(chatGroupId, s.c_str());
    } else
        g_pServer->AddAChatGroup(chatGroupId, description);
}

unsigned int WINAPI GetCountOfChatGroups() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetCountOfChatGroups();
}

unsigned int WINAPI GetAllCreatedChatGroups(unsigned int *pChatGroupId, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetJoinedGroupIds(pChatGroupId, count);
}

unsigned int WINAPI GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    if (!chars || !description)
        return 0;
#ifdef WCHAR32
    else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
        SPA::CScopeUQueue su;
        SPA::CUQueue &q = *su;
        unsigned int bufferSize = (chars + 1) * sizeof (wchar_t);
        if (q.GetMaxSize() < bufferSize)
            q.ReallocBuffer(bufferSize);
        wchar_t *des = (wchar_t *) q.GetBuffer();
        unsigned int len = g_pServer-> GetAChatGroup(chatGroupId, des, chars);
        SPA::CScopeUQueue suUTF16;
        SPA::CUQueue &qUTF16 = *suUTF16;
        SPA::Utilities::ToUTF16(des, len, qUTF16);
        ::memcpy(description, qUTF16.GetBuffer(), len * sizeof (SPA::UTF16));
        return len;
    }
#endif
    return g_pServer-> GetAChatGroup(chatGroupId, description, chars);
}

void WINAPI RemoveChatGroup(unsigned int chatGroupId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->GetCountOfClients() > 0)
        return;
    g_pServer->RemoveChatGroup(chatGroupId);
}

void WINAPI StopSocketProServer() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetOnAccept(nullptr);
    g_pServer->SetOnClose(nullptr);
    g_pServer->SetOnIdle(nullptr);
    g_pServer->SetOnIsPermitted(nullptr);
    g_pServer->SetOnSSLHandShakeCompleted(nullptr);
    if (!g_pServer->m_bStopped) {
        g_pServer->m_bStopped = true;
        CServerSession::m_cv.notify_all();
    }
    g_pServer->StopSocketProServer();
}

bool WINAPI IsRunning() {
    CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsRunning();
}

void WINAPI SetPrivateKeyFile(const char *keyFile) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetPrivateKeyFile(keyFile);
}

void WINAPI SetDHParmsFile(const char *dhFile) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetDhFile(dhFile);
}

void WINAPI SetCertFile(const char *certFile) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetCertFile(certFile);
}

void WINAPI SetPKFPassword(const char *pwd) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetPKFPassword(pwd);
}

void WINAPI SetMessageQueuePassword(const char *pwd) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetMessageQueuePassword(pwd);
}

void WINAPI SetDefaultEncryptionMethod(SPA::tagEncryptionMethod em) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->IsRunning())
        return;
    g_pServer->SetEncryptionMethod(em);
}

SPA::tagEncryptionMethod WINAPI GetDefaultEncryptionMethod() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || !g_pServer->IsRunning())
        return SPA::NoEncryption;
    return g_pServer->GetEncryptionMethod();
}

void WINAPI SetPfxFile(const char *pfxFile) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetPfxFile(pfxFile);
}

void WINAPI SetAuthenticationMethod(SPA::ServerSide::tagAuthenticationMethod am) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->IsRunning())
        return;
    g_pServer->SetAuthenticationMethod(am);
}

SPA::ServerSide::tagAuthenticationMethod WINAPI GetAuthenticationMethod() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return SPA::ServerSide::amOwn;
    return g_pServer->GetAuthenticationMethod();
}

void WINAPI SetSharedAM(bool b) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->IsRunning())
        return;
    g_pServer->SetSharedAM(b);
}

bool WINAPI GetSharedAM() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->GetSharedAM();
}

int WINAPI GetServerErrorCode() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetErrorCode();
}

unsigned int WINAPI GetServerErrorMessage(char *str, unsigned int bufferLen) {
    if (str == nullptr || bufferLen == 0 || g_pServer == nullptr)
        return 0;
    --bufferLen; //for null-terminated
    if (bufferLen > 0) {
        std::string err = g_pServer->GetErrorMessage();
        if (bufferLen > (unsigned int) (err.size()))
            bufferLen = (unsigned int) (err.size());
        if (bufferLen > 0)
            ::memcpy(str, err.c_str(), bufferLen);
    }
    str[bufferLen] = 0;
    return bufferLen;
}

bool WINAPI IsServerRunning() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsRunning();
}

bool WINAPI IsServerSSLEnabled() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsSSLEnabled();
}

void WINAPI SetOnAccept(POnAccept p) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetOnAccept(p);
}

void WINAPI SetOnIdle(POnIdle p) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetOnIdle(p);
}

unsigned int WINAPI GetSwitchTime() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultMinSwitchTime;
    return g_pServer->GetSwitchTime();
}

void WINAPI SetSwitchTime(unsigned int ulSwitchTime) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    if (ulSwitchTime < 1000)
        ulSwitchTime = 1000;
    g_pServer->SetSwitchTime(ulSwitchTime);
}

unsigned int WINAPI GetCountOfClients() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetCountOfClients();
}

USocket_Server_Handle WINAPI GetClient(unsigned int uiIndex) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetClient(uiIndex);
}

void WINAPI SetDefaultZip(bool bZip) {
    //CAutoLock al(g_mutex);
    if (g_pServer)
        g_pServer->SetDefaultZip(bZip);
}

bool WINAPI GetDefaultZip() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->GetDefaultZip();
}

bool WINAPI SpeakPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    if (!chatGroupIds || !count)
        return false;
    if (!message || size < sizeof (unsigned short))
        return false;
    SPA::CScopeUQueue sb;
    sb->Push(message, size);
    SPA::UVariant vtMsg;
    sb >> vtMsg;
    return g_pServer->Speak(chatGroupIds, count, vtMsg);
}

bool WINAPI SpeakExPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->SpeakEx(message, size, chatGroupIds, count);
}

bool WINAPI SendUserMessageExPush(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    if (userId && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
        const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
        unsigned int len = SPA::Utilities::GetLen(str);
        std::wstring s = SPA::ToNativeString(str, len);
        return g_pServer->SendUserMessage(s.c_str(), message, size);
    }
    return g_pServer->SendUserMessage(userId, message, size);
}

bool WINAPI SendUserMessagePush(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    SPA::CScopeUQueue sb;
    sb->Push(message, size);
    SPA::UVariant vtMsg;
    sb >> vtMsg;
    if (userId && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
        const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
        unsigned int len = SPA::Utilities::GetLen(str);
        std::wstring s = SPA::ToNativeString(str, len);
        return g_pServer->SendUserMessage(s.c_str(), vtMsg);
    }
    return g_pServer->SendUserMessage(userId, vtMsg);
}

void WINAPI SetMaxConnectionsPerClient(unsigned int ulMaxConnectionsPerClient) {
    if (!ulMaxConnectionsPerClient)
        ulMaxConnectionsPerClient = 1;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetMaxConnectionsPerClient(ulMaxConnectionsPerClient);
}

unsigned int WINAPI GetMaxConnectionsPerClient() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultMaxConnectionsPerClient;
    return g_pServer->GetMaxConnectionsPerClient();
}

void WINAPI SetMaxThreadIdleTimeBeforeSuicide(unsigned int ulMaxThreadIdleTimeBeforeSuicide) {
    if (ulMaxThreadIdleTimeBeforeSuicide < 5000)
        ulMaxThreadIdleTimeBeforeSuicide = 5000;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetMaxThreadIdleTimeBeforeSuicide(ulMaxThreadIdleTimeBeforeSuicide);
}

unsigned int WINAPI GetMaxThreadIdleTimeBeforeSuicide() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultMaxThreadIdleTimeBeforeSuicide;
    return g_pServer->GetMaxThreadIdleTimeBeforeSuicide();
}

void WINAPI SetTimerElapse(unsigned int timerElapse) {
    if (timerElapse < 50)
        timerElapse = 50;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetTimerElapse(timerElapse);
}

unsigned int WINAPI GetTimerElapse() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultTimerElapse;
    return g_pServer->GetTimerElapse();
}

unsigned int WINAPI GetSMInterval() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultSMInterval;
    return g_pServer->GetSMInterval();
}

void WINAPI SetSMInterval(unsigned int ulSMInterval) {
    if (ulSMInterval < 6000)
        ulSMInterval = 6000;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetSMInterval(ulSMInterval);
}

void WINAPI SetPingInterval(unsigned int pingInterval) {
    if (pingInterval < 2000)
        pingInterval = 2000;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetPingInterval(pingInterval);
}

unsigned int WINAPI GetPingInterval() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultPingInterval;
    return g_pServer->GetPingInterval();
}

void WINAPI SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval) {
    if (recycleGlobalMemoryInterval < 6000)
        recycleGlobalMemoryInterval = 6000;
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetRecycleGlobalMemoryInterval(recycleGlobalMemoryInterval);
}

unsigned int WINAPI GetRecycleGlobalMemoryInterval() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return CServer::DefaultRecycleGlobalMemoryInterval;
    return g_pServer->GetRecycleGlobalMemoryInterval();
}

SPA::UINT64 WINAPI GetRequestCount() {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetRequestCount();
}

unsigned int WINAPI StartQueue(const char *qName, bool dequeueShared, unsigned int ttl) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return INVALID_QUEUE_HANDLE;
    return g_pServer->StartQueue(qName, dequeueShared, ttl);
}

const char* WINAPI GetQueueFileName(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return nullptr;
    return g_pServer->GetQueueFileName(qHandle);
}

unsigned int WINAPI GetMessagesInDequeuing(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetMessagesInDequeuing(qHandle);
}

SPA::UINT64 WINAPI Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->Enqueue(qHandle, reqId, buffer, size);
}

SPA::UINT64 WINAPI BatchEnqueue(unsigned int qHandle, unsigned int count, const unsigned char *msgStruct) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->BatchEnqueue(qHandle, count, msgStruct);
}

SPA::UINT64 WINAPI GetMessageCount(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetMessageCount(qHandle);
}

unsigned int WINAPI GetTTL(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetTTL(qHandle);
}

void WINAPI ResetQueue(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->ResetQueue(qHandle);
}

SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->RemoveQueuedRequestsByTTL(qHandle);
}

SPA::UINT64 WINAPI GetLastQueueMessageTime(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetLastQueueMessageTime(qHandle);
}

bool WINAPI StopQueueByHandle(unsigned int qHandle, bool permanent) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->StopQueueByHandle(qHandle, permanent);
}

bool WINAPI StopQueueByName(const char *qName, bool permanent) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->StopQueueByName(qName, permanent);
}

SPA::UINT64 WINAPI GetQueueSize(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->GetQueueSize(qHandle);
}

SPA::UINT64 WINAPI Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (SPA::sidHTTP == pSession->GetSvsID() || SPA::sidStartup == pSession->GetSvsID())
        return BAD_OPERATION;
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return g_pServer->Dequeue(qHandle, h, messageCount, beNotifiedWhenAvailable, waitTime);
}

SPA::UINT64 WINAPI Dequeue2(unsigned int qHandle, USocket_Server_Handle h, unsigned int bytes, bool beNotifiedWhenAvailable, unsigned int waitTime) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (SPA::sidHTTP == pSession->GetSvsID() || SPA::sidStartup == pSession->GetSvsID())
        return BAD_OPERATION;
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return g_pServer->Dequeue2(qHandle, h, bytes, beNotifiedWhenAvailable, waitTime);
}

bool WINAPI IsQueueStartedByName(const char *qName) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsQueueStarted(qName);
}

bool WINAPI IsQueueStartedByHandle(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsQueueStarted(qHandle);
}

bool WINAPI AbortJob(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->AbortJob(qHandle);
}

bool WINAPI StartJob(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->StartJob(qHandle);
}

bool WINAPI EndJob(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->EndJob(qHandle);
}

SPA::UINT64 WINAPI GetJobSize(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->GetJobSize(qHandle);
}

bool WINAPI IsQueueSecuredByName(const char *qName) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsQueueSecured(qName);
}

bool WINAPI IsQueueSecuredByHandle(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsQueueSecured(qHandle);
}

const char* WINAPI GetQueueName(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return nullptr;
    return g_pServer->GetQueueName(qHandle);
}

SPA::UINT64 WINAPI GetQueueLastIndex(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetQueueLastIndex(qHandle);
}

SPA::tagOptimistic WINAPI GetOptimistic(unsigned int qHandle) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return SPA::oSystemMemoryCached;
    return g_pServer->GetOptimistic(qHandle);
}

void WINAPI SetOptimistic(unsigned int qHandle, SPA::tagOptimistic bOptimistic) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->SetOptimistic(qHandle, bOptimistic);
}

SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(unsigned int qHandle, SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->CancelQueuedRequests(qHandle, startIndex, endIndex);
}

bool WINAPI IsDequeueShared(unsigned int qHandle) {
    if (g_pServer == nullptr)
        return false;
    return g_pServer->IsDequeueShared(qHandle);
}

HINSTANCE WINAPI AddADll(const char *libFile, int nParam) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return nullptr;
    return g_pServer->AddADll(libFile, nParam);
}

bool WINAPI RemoveADllByHandle(HINSTANCE hInstance) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->RemoveALibrary(hInstance);
}

bool WINAPI HasUserId(const wchar_t *userId) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return false;
    return g_pServer->HasUserId(userId);
}

unsigned int WINAPI CancelQueuedRequests(unsigned int qHandle, const unsigned short *ids, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->CancelQueuedRequests(qHandle, ids, count);
}

unsigned int WINAPI PeekQueuedRequests(unsigned int qHandle, SPA::CQueuedRequestInfo *qri, unsigned int count) {
    //CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->PeekQueuedRequests(qHandle, qri, count);
}

bool WINAPI SetRouting(unsigned int serviceId0, SPA::ServerSide::tagRoutingAlgorithm ra0, unsigned int serviceId1, SPA::ServerSide::tagRoutingAlgorithm ra1) {
    if (g_pServer == nullptr)
        return false;
    return g_pServer->SetRouting(serviceId0, ra0, serviceId1, ra1);
}

unsigned int WINAPI CheckRouting(unsigned int serviceId) {
    if (g_pServer == nullptr)
        return false;
    return g_pServer->CheckRouting(serviceId);
}

bool WINAPI AddAlphaRequest(unsigned int serviceId, unsigned short reqId) {
    if (g_pServer == nullptr)
        return false;
    return g_pServer->AddAlphaRequest(serviceId, reqId);
}

unsigned int WINAPI GetAlphaRequestIds(unsigned int serviceId, unsigned short *reqIds, unsigned int count) {
    if (g_pServer == nullptr)
        return 0;
    return g_pServer->GetAlphaRequestIds(serviceId, reqIds, count);
}

SPA::tagQueueStatus WINAPI GetServerQueueStatus(unsigned int qHandle) {
    if (g_pServer == nullptr)
        return SPA::qsNormal;
    return g_pServer->GetServerQueueStatus(qHandle);
}

bool WINAPI PushQueueTo(unsigned int srcHandle, const unsigned int *targetHandles, unsigned int count) {
    unsigned int n;
    if (!targetHandles || !count)
        return false;
    if (g_pServer == nullptr)
        return false;
    std::shared_ptr<MQ_FILE::CMqFile> qSrc = g_pServer->GetQueue(srcHandle);
    if (!qSrc || !qSrc->IsAvailable())
        return false;
    std::vector<MQ_FILE::CMqFile*> vQ;
    for (n = 0; n < count; ++n) {
        vQ.push_back(g_pServer->GetQueue(targetHandles[n]).get());
    }
    MQ_FILE::CMqFile* &targets = vQ.front();
    return qSrc->AppendTo(&targets, (unsigned int) vQ.size());
}

void WINAPI SetServerWorkDirectory(const char *dir) {
    std::string str;
    if (dir)
        str = dir;
#ifdef WIN32_64
    char c = '\\';
#else
    char c = '/';
#endif
    std::string::size_type pos = str.rfind(c);
    if (str.size() && pos != str.size() - 1) {
        str += c;
    }
    CServer::m_WorkingPath = str;
}

const char* WINAPI GetServerWorkDirectory() {
    return CServer::m_WorkingPath.c_str();
}

SPA::CertInfo* WINAPI GetUCert(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    if (pSession)
        return pSession->GetUCert();
    return nullptr;
}

SPA::UINT64 WINAPI GetCurrentRequestIndex(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return (~0);
    if (pSession)
        return pSession->GetCallIndex();
    return (~0);
}

SPA::IUcert* WINAPI GetUCertEx(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    if (pSession)
        return pSession->GetUCert();
    return nullptr;
}

void WINAPI SetCertificateVerifyCallback(PCertificateVerifyCallback cvc) {
#if defined(OLD_IMPL)
    CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->m_pSslContext == nullptr)
        return;
    g_pServer->m_cvc = cvc;
    g_pServer->m_pSslContext->set_verify_mode(boost::asio::ssl::verify_peer);
    g_pServer->m_pSslContext->set_verify_callback(boost::bind(&CServer::verify_certificate_cb, _1, _2));
#elif defined(WIN32_64)
    CAutoLock al(g_mutex);
    if (g_pServer == nullptr)
        return;
    g_pServer->m_cvc = cvc;
#else
    CAutoLock al(g_mutex);
    if (g_pServer == nullptr || g_pServer->m_pSslContext == nullptr)
        return;
    g_pServer->m_cvc = cvc;
    g_pServer->m_pSslContext->set_verify_mode(boost::asio::ssl::verify_peer);
    g_pServer->m_pSslContext->set_verify_callback(boost::bind(&CServer::verify_certificate_cb, _1, _2));
#endif
}

bool WINAPI SetVerifyLocation(const char *certFile) {
#if defined(OLD_IMPL)
    return CUCertImpl::SetVerifyLocation(certFile);
#elif defined(WIN32_64)
    return SPA::CCertificateImpl::SetVerifyLocation(certFile);
#else
    return CUCertImpl::SetVerifyLocation(certFile);
#endif
}
