
#ifdef OLD_IMPL
#include "stdafx.h"
#include "clientsession.h"
#elif defined(_WIN32_WCE) || defined(WIN32_64)
#include "../ClientCoreWin/stdafx.h"
#include "../ClientCoreWin/clientsession.h"
#else
#include "../ClientCoreUnix/stdafx.h"
#include "../ClientCoreUnix/clientsession.h"
#endif

#include "../../include/commutil.h"
#include "../../include/uclient.h"
#include "socketpool.h"
#include "../../pinc/uzip.h"

extern boost::mutex g_mutex;
boost::mutex g_Mutex;

CClientSession *MapHandleToClientSession(USocket_Client_Handle h) {
    CClientSession *p = (CClientSession*) (h);
    return p;
}

std::string g_strVersion("6.3.0.1");

const char* WINAPI GetUClientSocketVersion() {
    return g_strVersion.c_str();
}

const char* WINAPI GetUClientAppName() {
    return MQ_FILE::CMqFile::m_strAppName.c_str();
}

void WINAPI SetMessageQueuePassword(const char *pwd) {
    MQ_FILE::CMyContainer::Container.Set(0, pwd);
}

void WINAPI Close(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        p->Close();
}

bool WINAPI Connect(USocket_Client_Handle h, const char* host, unsigned int portNumber, bool sync, bool v6) {
    bool ok = false;
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        ok = p->Connect(host, portNumber, sync, v6);
    }
    return ok;
}

unsigned int WINAPI GetCountOfRequestsQueued(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetCountOfRequestsInQueue();
    return 0;
}

unsigned short WINAPI GetCurrentRequestID(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetCurrentRequestID();
    return 0;
}

unsigned int WINAPI GetCurrentResultSize(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetCurrentResultSize();
    return 0;
}

SPA::tagEncryptionMethod WINAPI GetEncryptionMethod(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetEncryptionMethod();
    return SPA::NoEncryption;
}

int WINAPI GetErrorCode(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetErrorCode();
    return 0;
}

SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Client_Handle h, bool *endian) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetPeerOs(endian);
    if (endian)
        *endian = SPA::IsBigEndian();
    return SPA::GetOS();
}

bool WINAPI GetPeerName(USocket_Client_Handle h, unsigned int *peerPort, char *strIpAddr, unsigned short bufferLen) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetPeerName(peerPort, strIpAddr, bufferLen);
    return false;
}

bool WINAPI GetSockAddr(USocket_Client_Handle h, unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetSockAddr(sockPort, strIPAddrBuffer, chars);
    return false;
}

unsigned int WINAPI GetErrorMessage(USocket_Client_Handle h, char *str, unsigned int bufferLen) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (!p)
        return 0;
    std::string err = p->GetErrorMessage();
    if (str == nullptr || bufferLen == 0)
        return 0;
    --bufferLen; //for null-terminated
    if (bufferLen > 0) {
        if (bufferLen > (unsigned int) (err.size()))
            bufferLen = (unsigned int) (err.size());
        if (bufferLen > 0)
            ::memcpy(str, err.c_str(), bufferLen);
    }
    str[bufferLen] = 0;
    return bufferLen;
}

unsigned int WINAPI GetSocketPoolId(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        CSocketPool *pool = p->GetSocketPool();
        if (pool)
            return pool->GetPoolId();
    }
    return 0;
}

bool WINAPI IsOpened(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->IsOpened();
    return false;
}

unsigned int WINAPI RetrieveResult(USocket_Client_Handle h, unsigned char *pBuffer, unsigned int size) {
    //g_Mutex.lock();
    CClientSession *p = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (p) {
        return p->RetrieveResult(pBuffer, size);
    }
    return 0;
}

bool WINAPI SendRequest(USocket_Client_Handle h, unsigned short reqId, const unsigned char *pBuffer, unsigned int len) {
    bool ok = false;
    //g_Mutex.lock();
    CClientSession *p = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (p) {
        ok = p->SendRequest(reqId, pBuffer, len);
    }
    return ok;
}

void WINAPI SetEncryptionMethod(USocket_Client_Handle h, SPA::tagEncryptionMethod em) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        p->SetEncryptionMethod(em);
}

void WINAPI SetOnHandShakeCompleted(USocket_Client_Handle h, POnHandShakeCompleted p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnHandShakeCompleted(p);
}

void WINAPI SetOnRequestProcessed(USocket_Client_Handle h, POnRequestProcessed p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnRequestProcessed(p);
}

void WINAPI SetOnServerException(USocket_Client_Handle h, POnServerException p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnServerException(p);
}

void WINAPI SetOnSocketClosed(USocket_Client_Handle h, POnSocketClosed p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    s->SetOnSocketClosed(p);
}

void WINAPI SetOnSocketConnected(USocket_Client_Handle h, POnSocketConnected p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnSocketConnected(p);
}

void WINAPI SetOnBaseRequestProcessed(USocket_Client_Handle h, POnBaseRequestProcessed p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnBaseRequestProcessed(p);
}

void WINAPI SetOnAllRequestsProcessed(USocket_Client_Handle h, POnAllRequestsProcessed p) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnAllRequestsProcessed(p);
}

void WINAPI Shutdown(USocket_Client_Handle h, SPA::tagShutdownType how) {
    //CAutoLock al(g_Mutex);
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->Shutdown((nsIP::tcp::socket::shutdown_type)how);
}

bool WINAPI WaitAll(USocket_Client_Handle h, unsigned int nTimeout) {
    bool ok = false;
    //g_Mutex.lock();
    CClientSession *p = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (p) {
        ok = p->WaitAll(nTimeout);
    }
    return ok;
}

bool WINAPI Cancel(USocket_Client_Handle h, unsigned int requestsQueued) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->Cancel(requestsQueued);
    }
    return false;
}

bool WINAPI IsRandom(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->IsRandom();
    return false;
}

const SPA::CSwitchInfo* WINAPI GetServerInfo(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetServerInfo();
    return nullptr;
}

const SPA::CSwitchInfo* WINAPI GetClientInfo(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetClientInfo();
    return nullptr;
}

unsigned int WINAPI GetCurrentServiceId(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetCurrentServiceId();
    return SPA::sidStartup;
}

void WINAPI SetClientInfo(USocket_Client_Handle h, SPA::CSwitchInfo si) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        p->SetClientInfo(si);
}

unsigned int WINAPI GetBytesInSendingBuffer(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetBytesInSendingBuffer();
    return 0;
}

unsigned int WINAPI GetBytesInReceivingBuffer(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetBytesInReceivingBuffer();
    return 0;
}

bool WINAPI IsBatching(USocket_Client_Handle h) {
    CClientSession *p;
    {
        //CAutoLock al(g_Mutex);
        p = MapHandleToClientSession(h);
        if (!p)
            return false;
    }
    return p->IsBatching();
}

unsigned int WINAPI GetBytesBatched(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetBytesBatched();
    return 0;
}

bool WINAPI StartBatching(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->StartBatching();
    return false;
}

bool WINAPI CommitBatching(USocket_Client_Handle h, bool bBatchingAtServerSide) {
    bool ok = false;
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        ok = p->CommitBatching(bBatchingAtServerSide);
    }
    return ok;
}

bool WINAPI AbortBatching(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->AbortBatching();
    return false;
}

SPA::UINT64 WINAPI GetBytesReceived(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetBytesReceived();
    return 0;
}

SPA::UINT64 WINAPI GetBytesSent(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetBytesSent();
    return 0;
}

void WINAPI SetUserID(USocket_Client_Handle h, const wchar_t *strUserId) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        if (!strUserId) {
            p->SetUserID(L"");
        } else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)strUserId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            p->SetUserID(s.c_str());
        } else
            p->SetUserID(strUserId);
    }
}

void WINAPI SetZip(USocket_Client_Handle h, bool zip) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        p->SetZip(zip);
}

bool WINAPI GetZip(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetZip();
    return false;
}

void WINAPI SetZipLevel(USocket_Client_Handle h, SPA::tagZipLevel zl) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        p->SetZipLevel(zl);
}

SPA::tagZipLevel WINAPI GetZipLevel(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetZipLevel();
    return SPA::zlDefault;
}

bool WINAPI StartQueue(USocket_Client_Handle h, const char *qName, bool secure, bool dequeueShared, unsigned int ttl) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->StartQueue(qName, secure, dequeueShared, ttl);
    }
    return false;
}

void WINAPI StopQueue(USocket_Client_Handle h, bool permanent) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        p->StopQueue(permanent);
    }
}

bool WINAPI DequeuedResult(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->DequeuedResult();
    return false;
}

void WINAPI UseUTF16() {
    SPA::g_bAdapterUTF16 = true;
}

unsigned int WINAPI GetUID(USocket_Client_Handle h, wchar_t *strUserId, unsigned int chars) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        if (!chars || !strUserId)
            return 0;
#ifdef WCHAR32
        else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            SPA::CScopeUQueue su;
            SPA::CUQueue &q = *su;
            unsigned int bufferSize = (chars + 1) * sizeof (wchar_t);
            if (q.GetMaxSize() < bufferSize)
                q.ReallocBuffer(bufferSize);
            wchar_t *userId = (wchar_t *)q.GetBuffer();
            unsigned int len = p->GetUID(userId, chars);
#if defined(__ANDROID__) || defined(ANDROID)
            auto uid = SPA::Utilities::ToUTF16(userId, len);
            ::memcpy(strUserId, uid.c_str(), len * sizeof (SPA::UTF16));
#else
            SPA::CScopeUQueue suUTF16;
            SPA::CUQueue &qUTF16 = *suUTF16;
            SPA::Utilities::ToUTF16(userId, len, qUTF16);
            ::memcpy(strUserId, qUTF16.GetBuffer(), len * sizeof (SPA::UTF16));
#endif
            return len;
        }
#endif
        return p->GetUID(strUserId, chars);
    }
    return 0;
}

void WINAPI SetPassword(USocket_Client_Handle h, const wchar_t *strPassword) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        if (!strPassword) {
            p->SetPassword(L"");
        } else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)strPassword;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            p->SetPassword(s.c_str());
        } else
            p->SetPassword(strPassword);
    }
}

bool WINAPI SwitchTo(USocket_Client_Handle h, unsigned int serviceId) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->SwitchTo(serviceId);
    }
    return false;
}

bool WINAPI Enter(USocket_Client_Handle h, const unsigned int *pChatGroupId, unsigned int nCount) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->Enter(pChatGroupId, nCount);
    }
    return false;
}

void WINAPI Exit(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        p->Exit();
    }
}

bool WINAPI SpeakEx(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->SpeakEx(message, size, pChatGroupId, nCount);
    }
    return false;
}

bool WINAPI Speak(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        return p->Speak(message, size, pChatGroupId, nCount);
    }
    return false;
}

bool WINAPI SendUserMessageEx(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            return p->SendUserMessageEx(s.c_str(), message, size);
        }
        return p->SendUserMessageEx(userId, message, size);
    }
    return false;
}

bool WINAPI SendUserMessage(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size) {
    bool ok;
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p) {
        //CRAutoLock ral(g_Mutex);
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            ok = p->SendUserMessage(s.c_str(), message, size);
        } else
            ok = p->SendUserMessage(userId, message, size);
        return ok;
    }
    return false;
}

SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *p = MapHandleToClientSession(h);
    if (p)
        return p->GetSocketNativeHandle();
    return INVALID_NUMBER;
}

void WINAPI SetOnEnter(USocket_Client_Handle h, POnEnter p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnEnter(p);
}

void WINAPI SetOnExit(USocket_Client_Handle h, POnExit p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnExit(p);
}

void WINAPI SetOnSpeakEx(USocket_Client_Handle h, POnSpeakEx p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnSpeakEx(p);
}

void WINAPI SetOnSpeak(USocket_Client_Handle h, POnSpeak p) {
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnSpeak(p);
}

void WINAPI SetOnSendUserMessageEx(USocket_Client_Handle h, POnSendUserMessageEx p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnSendUserMessageEx(p);
}

void WINAPI SetOnSendUserMessage(USocket_Client_Handle h, POnSendUserMessage p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOnSendUserMessage(p);
}

void WINAPI SetOnEnter2(USocket_Client_Handle h, POnEnter2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnSubscribe2 = p;
}

void WINAPI SetOnExit2(USocket_Client_Handle h, POnExit2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnUnsubscribe2 = p;
}

void WINAPI SetOnSpeakEx2(USocket_Client_Handle h, POnSpeakEx2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnBroadcastEx2 = p;
}

void WINAPI SetOnSendUserMessageEx2(USocket_Client_Handle h, POnSendUserMessageEx2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnPostUserMessageEx2 = p;
}

void WINAPI SetOnSendUserMessage2(USocket_Client_Handle h, POnSendUserMessage2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnPostUserMessage2 = p;
}

void WINAPI SetOnSpeak2(USocket_Client_Handle h, POnSpeak2 p) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->m_OnBroadcast2 = p;
}

unsigned int WINAPI GetMessagesInDequeuing(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetMessagesInDequeuing();
    return 0;
}

SPA::UINT64 WINAPI GetQueueLastIndex(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetQueueLastIndex();
    return 0;
}

SPA::UINT64 WINAPI GetMessageCount(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetMessageCount();
    return 0;
}

SPA::UINT64 WINAPI GetQueueSize(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->GetQueueSize();
    }
    return 0;
}

bool WINAPI IsDequeueShared(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        return cs->IsDequeueShared();
    }
    return false;
}

SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(USocket_Client_Handle h, SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->CancelQueuedRequests(startIndex, endIndex);
    }
    return 0;
}

bool WINAPI IsQueueSecured(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsQueueSecured();
    return false;
}

const char* WINAPI GetQueueName(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetQueueName();
    return nullptr;
}

unsigned int WINAPI CancelQueuedRequests(USocket_Client_Handle h, const unsigned short *ids, unsigned int count) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->CancelQueuedRequests(ids, count);
    }
    return 0;
}

SPA::tagOptimistic WINAPI GetOptimistic(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetOptimistic();
    return SPA::oSystemMemoryCached;
}

const unsigned char* WINAPI GetResultBuffer(USocket_Client_Handle h) {
    //g_Mutex.lock();
    CClientSession *cs = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (cs)
        return cs->GetResultBuffer();
    return nullptr;
}

void WINAPI SetOptimistic(USocket_Client_Handle h, SPA::tagOptimistic bOptimistic) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetOptimistic(bOptimistic);
}

const char* WINAPI GetQueueFileName(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetQueueFileName();
    return nullptr;
}

bool WINAPI IsQueueStarted(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsQueueStarted();
    return false;
}

bool WINAPI DoEcho(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->DoEcho();
    }
    return false;
}

bool WINAPI SetSockOpt(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->SetSockOpt(optName, optValue, level);
    return false;
}

bool WINAPI SetSockOptAtSvr(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->SetSockOptAtSvr(optName, optValue, level);
    }
    return false;
}

bool WINAPI TurnOnZipAtSvr(USocket_Client_Handle h, bool enableZip) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //CRAutoLock ral(g_Mutex);
        return cs->TurnOnZipAtSvr(enableZip);
    }
    return false;
}

bool WINAPI SetZipLevelAtSvr(USocket_Client_Handle h, SPA::tagZipLevel zipLevel) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->SetZipLevelAtSvr(zipLevel);
    return false;
}

void WINAPI SetRecvTimeout(USocket_Client_Handle h, unsigned int timeout) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetRecvTimeout(timeout);
}

unsigned int WINAPI GetRecvTimeout(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetRecvTimeout();
    return 0;
}

void WINAPI SetConnTimeout(USocket_Client_Handle h, unsigned int timeout) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetConnTimeout(timeout);
}

unsigned int WINAPI GetConnTimeout(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetConnTimeout();
    return 0;
}

void WINAPI EnableRoutingQueueIndex(USocket_Client_Handle h, bool enable) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->EnableRoutingQueueIndex(enable);
}

bool WINAPI IsRoutingQueueIndexEnabled(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsRoutingQueueIndexEnabled();
    return false;
}

void WINAPI SetAutoConn(USocket_Client_Handle h, bool autoConnecting) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetAutoConn(autoConnecting);
}

bool WINAPI GetAutoConn(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetAutoConn();
    return false;
}

bool WINAPI GetReturnRandom(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsRandom();
    return false;
}

unsigned short WINAPI GetServerPingTime(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetServerPingTime();
    return 0;
}

SPA::CertInfo* WINAPI GetUCert(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetUCert();
    return nullptr;
}

SPA::IUcert* WINAPI GetUCertEx(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetUCert();
    return nullptr;
}

void WINAPI SetPeerDequeueFailed(USocket_Client_Handle h, bool fail) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->SetPeerDequeueFailed(fail);
}

bool WINAPI GetPeerDequeueFailed(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetPeerDequeueFailed();
    return false;
}

bool WINAPI IsRouteeRequest(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsRouteeRequest();
    return false;
}

bool WINAPI AbortJob(USocket_Client_Handle h) {
    //g_Mutex.lock();
    CClientSession *cs = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (cs)
        return cs->AbortJob();
    return false;
}

SPA::UINT64 WINAPI GetJobSize(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetJobSize();
    return false;
}

bool WINAPI StartJob(USocket_Client_Handle h) {
    //g_Mutex.lock();
    CClientSession *cs = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (cs)
        return cs->StartJob();
    return false;
}

bool WINAPI EndJob(USocket_Client_Handle h) {
    //g_Mutex.lock();
    CClientSession *cs = MapHandleToClientSession(h);
    //g_Mutex.unlock();
    if (cs)
        return cs->EndJob();
    return false;
}

bool WINAPI IsDequeueEnabled(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IsDequeueEnabled();
    return false;
}

void* WINAPI GetSSL(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetSSL();
    return nullptr;
}

bool WINAPI IgnoreLastRequest(USocket_Client_Handle h, unsigned short reqId) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->IgnoreLastRequest(reqId);
    return false;
}

bool WINAPI SetVerifyLocation(const char *certFile) {
    //CAutoLock al(g_Mutex);
#if defined(OLD_IMPL)
    return CUCertImpl::SetVerifyLocation(certFile);
#elif defined(_WIN32_WCE) || defined(WIN32_64)
    return SPA::CCertificateImpl::SetVerifyLocation(certFile);
#else
    return CUCertImpl::SetVerifyLocation(certFile);
#endif
}

const char* WINAPI Verify(USocket_Client_Handle h, int *errCode) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        SPA::IUcert *p = cs->GetUCert();
        if (!p)
            return nullptr;
        return p->Verify(errCode);
    }
    return nullptr;
}

unsigned int WINAPI PeekQueuedRequests(USocket_Client_Handle h, SPA::CQueuedRequestInfo *qri, unsigned int count) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs) {
        //g_Mutex.unlock();
        unsigned int res = cs->PeekQueuedRequests(qri, count);
        //g_Mutex.lock();
        return res;
    }
    return 0;
}

bool WINAPI SendRouteeResult(USocket_Client_Handle h, unsigned short reqId, const unsigned char *buffer, unsigned int len) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->SendRouteeResult(reqId, buffer, len);
    return false;
}

unsigned int WINAPI GetRouteeCount(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        return cs->GetRouteeCount();
    return 0;
}

SPA::UINT64 WINAPI AppendQueue(USocket_Client_Handle h, USocket_Client_Handle hQueue) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    CClientSession *csQueue = MapHandleToClientSession(hQueue);
    if (!cs || !csQueue || cs == csQueue)
        return 0;
    {
        //CRAutoLock ral(g_Mutex);
        return cs->AppendQueue(csQueue->GetQueue());
    }
}

unsigned int WINAPI GetTTL(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return 0;
    return cs->GetTTL();
}

SPA::UINT64 WINAPI GetLastQueueMessageTime(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return 0;
    return cs->GetLastQueueMessageTime();
}

SPA::ClientSide::tagConnectionState WINAPI GetConnectionState(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return SPA::ClientSide::csClosed;
    return cs->GetConnectionState();
}

bool WINAPI IsRouting(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return false;
    return cs->IsRouting();
}

void WINAPI AbortDequeuedMessage(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return;
    cs->SetPeerDequeueFailed(true);
}

bool WINAPI IsDequeuedMessageAborted(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return false;
    return cs->GetPeerDequeueFailed();
}

void WINAPI ResetQueue(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return;
    cs->ResetQueue();
}

SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(USocket_Client_Handle h) {
    CClientSession *cs;
    {
        //CAutoLock al(g_Mutex);
        cs = MapHandleToClientSession(h);
        if (!cs)
            return 0;
    }
    return cs->RemoveQueuedRequestsByTTL();
}

SPA::tagQueueStatus WINAPI GetClientQueueStatus(USocket_Client_Handle h) {
    //CAutoLock al(g_Mutex);
    CClientSession *cs = MapHandleToClientSession(h);
    if (!cs)
        return SPA::qsNormal;
    boost::shared_ptr<MQ_FILE::CMqFile> q = cs->GetQueue();
    if (!q)
        return SPA::qsNormal;
    return q->GetQueueOpenStatus();
}

bool WINAPI PushQueueTo(USocket_Client_Handle h, const USocket_Client_Handle *handles, unsigned int count) {
    unsigned int n;
    CClientSession *src;
    if (!handles || !count)
        return false;
    std::vector<CClientSession *> vClient;
    {
        //CAutoLock al(g_Mutex);
        src = MapHandleToClientSession(h);
        if (!src)
            return false;
        for (n = 0; n < count; ++n) {
            CClientSession *target = MapHandleToClientSession(handles[n]);
            if (!target)
                return false;
            vClient.push_back(target);
        }
    }
    return src->PushQueueTo(vClient);
}

bool WINAPI IsClientQueueIndexPossiblyCrashed() {
    //CAutoLock al(g_mutex);
    if (CClientSession::m_pQLastIndex.get() == nullptr)
        return false;
    return CClientSession::m_pQLastIndex->IsCrashed();
}

void WINAPI SetClientWorkDirectory(const char *dir) {
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
    CAutoLock al(g_mutex);
    CClientSession::m_WorkingPath = str;
}

const char* WINAPI GetClientWorkDirectory() {
    CAutoLock al(g_mutex);
    return CClientSession::m_WorkingPath.c_str();
}

void WINAPI SetOnPostProcessing(USocket_Client_Handle h, POnPostProcessing p) {
    CClientSession *s = MapHandleToClientSession(h);
    if (s)
        s->SetOnPostProcessing(p);
}

void WINAPI PostProcessing(USocket_Client_Handle h, unsigned int hint, SPA::UINT64 data) {
    CClientSession *cs = MapHandleToClientSession(h);
    if (cs)
        cs->PostProcessing(hint, data);
}

