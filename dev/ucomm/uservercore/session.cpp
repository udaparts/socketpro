
#include "session.h"
#include "server.h"
#include "../servercore/serverthread.h"
#include <algorithm>
#include "../servercore/jsloader.h"
#include <assert.h>
#include "../servercore/webresponseProcessor.h"
#include "../core_shared/pinc/uzip.h"
#include <assert.h>
#include "../include/mysql/umysql.h"
#include "../include/sqlite/usqlite.h"

extern CServer *g_pServer;

CServerSession *GetSvrSession(USocket_Server_Handle h, unsigned int &index);

std::vector<unsigned char*> CServerSession::m_aBuffer;
CServerSession::mutex CServerSession::m_mutexBuffer;
CServerSession::mutex CServerSession::m_qMutex;

CServerRegistration::CServerRegistration()
: SPA::ServerSide::URegistration(),
m_nCheapCall(0),
m_bWin(false),
m_bApple(false),
m_bWinCe(false),
m_bUnix(false),
m_bAndroid(false),
m_bTimeok(true) {

}

void CServerRegistration::Initialize() {
    if (g_pServer->m_aSession.size() > ManyClients)
        ManyClients = 0;
    time_t now = ::time(nullptr);
    if (now > GetEndDate())
        m_bTimeok = false;
}

bool CServerRegistration::ShouldShutdown() {
    if (m_nCheapCall < MAX_ALLOWED_CHEAP_CALLS)
        return false;
    unsigned int badcount = 0;
    time_t endTime = GetEndDate() + 50 * 3600; //2.5 days
    for (auto it = g_pServer->m_aSession.begin(), end = g_pServer->m_aSession.end(); it != end; ++it) {
        CServerSession *session = *it;
        if (session->m_ClientInfo.SwitchTime > (SPA::UINT64)endTime && session->m_pServiceContext && !session->m_pServiceContext)
            ++badcount;
    }

    size_t all = g_pServer->m_aSession.size();
    switch (all) {
        case 0:
        case 1:
            break;
        case 2:
        case 3:
            if (badcount >= 1)
                return true;
            break;
        case 4:
        case 5:
        case 6:
            if (badcount >= 2)
                return true;
            break;
        case 7:
        case 8:
        case 9:
            if (badcount >= 3)
                return true;
            break;
        default:
            if (badcount * 3 > all)
                return true;
            break;
    }
    return false;
}

void CServerRegistration::SetOSs() {
    for (auto it = Platforms.begin(), end = Platforms.end(); it != end; ++it) {
        std::string &s = *it;
        if (UHTTP::iequals(s.c_str(), "win"))
            m_bWin = true;
        else if (UHTTP::iequals(s.c_str(), "wince") || UHTTP::iequals(s.c_str(), "winphone"))
            m_bWinCe = true;
        else if (UHTTP::iequals(s.c_str(), "apple") || UHTTP::iequals(s.c_str(), "mac") || UHTTP::iequals(s.c_str(), "ios") || UHTTP::iequals(s.c_str(), "iphone"))
            m_bApple = true;
        else if (UHTTP::iequals(s.c_str(), "unix") || UHTTP::iequals(s.c_str(), "linux") || boost::ifind_first(*it, "bsd"))
            m_bUnix = true;
        else if (UHTTP::iequals(s.c_str(), "android") || UHTTP::iequals(s.c_str(), "google"))
            m_bAndroid = true;
    }
}

void CServerRegistration::AddCall(unsigned int svsId, SPA::tagOperationSystem os, bool queue, bool endian) {
    do {
        if (!g_bRegistered || !m_bTimeok || !ManyClients) {
            ++m_nCheapCall;
            break;
        }

        if (!RequestQueue && queue) {
            ++m_nCheapCall;
            break;
        }

        if (svsId == SPA::sidHTTP) {
            if (!JavaScript) {
                ++m_nCheapCall;
                break;
            }
        } else {
            if (endian != SPA::IsBigEndian() && !Endian) {
                ++m_nCheapCall;
                break;
            }

            switch (os) {
                case SPA::osWin:
                    if (!m_bWin)
                        ++m_nCheapCall;
                    break;
                case SPA::osApple:
                    if (!m_bApple)
                        ++m_nCheapCall;
                    break;
                case SPA::osUnix:
                    if (!m_bUnix)
                        ++m_nCheapCall;
                    break;
                case SPA::osWinCE:
                    if (!m_bWinCe)
                        ++m_nCheapCall;
                    break;
                case SPA::osAndroid:
                    if (!m_bAndroid)
                        ++m_nCheapCall;
                    break;
                default:
                    break;
            }
        }
    } while (false);
}

CConditionVariable CServerSession::m_cv;

unsigned char* CServerSession::GetIoBuffer() {
    unsigned char *s = nullptr;
    m_mutexBuffer.lock();
    size_t size = m_aBuffer.size();
    if (size) {
        s = m_aBuffer[size - 1];
        m_aBuffer.pop_back();
    }
    m_mutexBuffer.unlock();
    if (s == nullptr) {
        s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + 16);
    }
    return s;
}

SPA::tagOperationSystem CServerSession::GetPeerOs(bool *endian) {
    //CAutoLock sl(m_mutex);
    if (endian)
        *endian = m_ReqInfo.IsBigEndian();
    return m_ReqInfo.GetOS();
}

void CServerSession::ReleaseIoBuffer(unsigned char *buffer) {
    if (buffer == nullptr)
        return;
    m_mutexBuffer.lock();
    m_aBuffer.push_back(buffer);
    m_mutexBuffer.unlock();
}

std::shared_ptr<MQ_FILE::CQLastIndex> CServerSession::m_pQLastIndex;

CServerSession::CServerSession()
:
#ifndef NDEBUG
m_nJobRequest(0),
m_nJobConfirm(0),
#endif
m_pSocket(nullptr),
ServiceId(SPA::sidStartup),
m_pUThread(nullptr),
m_ReadBuffer(GetIoBuffer()),
m_bRBLocked(false),
m_WriteBuffer(GetIoBuffer()),
m_bWBLocked(0),
m_bZip(g_pServer->m_bZip),
m_zl(SPA::zlDefault),
m_pQBatch(nullptr),
m_bCanceled(false),
m_bDropSlowRequest(false),
m_pHttpContext(nullptr),
m_nHttpCallCount(0),
m_cs(csClosed),
m_bFail(false),
m_bDequeueTrans(false),
m_bConfirmTrans(false),
m_bConfirmFail(false),
m_pServiceContext(nullptr),
m_pRoutingServiceContext(nullptr),
m_receiverHandle(0),
m_senderHandle(0),
m_bRouteBatching(false),
m_routingRequestCount(0),
m_bCloseInternal(false),
m_bChatting(false) {
    memset(&m_ReqInfo, 0, sizeof (m_ReqInfo));
    memset(&m_ClientInfo, 0, sizeof (m_ClientInfo));

}

CServerSession::~CServerSession() {
    try{
        CAutoLock sl(m_mutex);
        Initialize();
        m_bRBLocked = true;
        m_bWBLocked = 0;
        m_pSsl.reset();
        delete m_pSocket;
        ReleaseIoBuffer(m_ReadBuffer);
        ReleaseIoBuffer(m_WriteBuffer);
    }

    catch(...) {
    }
}

void CServerSession::ResetRoutingRequestCount() {
    m_routingRequestCount = 0;
}

SPA::UINT64 CServerSession::GetRoutingRequestCount() {
    return m_routingRequestCount;
}

void CServerSession::IncreaseRoutingRequestCount() {
    ++m_routingRequestCount;
}

void CServerSession::SetContext() {
    delete m_pSocket;
    m_pSocket = new CSocket(g_pServer->m_IoService);
}

CUCertImpl* CServerSession::GetUCert() {
    return m_pCert.get();
}

void CServerSession::Initialize() {
    ConfirmFailed();
    m_qRead.SetSize(0);
    if (m_qRead.GetMaxSize() > 5 * IO_BUFFER_SIZE)
        m_qRead.ReallocBuffer(5 * IO_BUFFER_SIZE);
    m_qWrite.SetSize(0);
    if (m_qWrite.GetMaxSize() > 5 * IO_BUFFER_SIZE)
        m_qWrite.ReallocBuffer(5 * IO_BUFFER_SIZE);
    memset(&m_ReqInfo, 0, sizeof (m_ReqInfo));
    if (m_ccb.Id.size()) {
        Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
        if (sp) {
            Connection::CConnectionContextBase *p = sp.get();
            *p = m_ccb;
        }
    }
    m_ccb.Initialize(boost::posix_time::microsec_clock::local_time());
    memset(&m_ClientInfo, 0, sizeof (m_ClientInfo));
    m_ServerInfo = g_pServer->m_ServerInfo;
    m_pUThread = nullptr;
    m_bZip = g_pServer->m_bZip;
    m_zl = SPA::zlDefault;
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
    m_bDropSlowRequest = false;
    m_nHttpCallCount = 0;
    m_cs = csClosed;
    m_bFail = false;
    m_bDequeueTrans = false;
    m_bConfirmTrans = false;
    m_bConfirmFail = false;
    m_ulIndex = 0;
    m_pServiceContext = nullptr;
    m_pRoutingServiceContext = nullptr;
    m_receiverHandle = 0;
    m_senderHandle = 0;
    m_bRBLocked = false;
    m_bWBLocked = 0;
    m_bRouteBatching = false;
    m_routingRequestCount = 0;
    m_bCloseInternal = false;
    if (g_pServer->IsSsl()) {
        m_pSsl.reset();
    }
}

void CServerSession::SetPeerDequeueFailed(bool fail) {
    m_mutex.lock();
    m_bFail = fail;
    m_mutex.unlock();
}

bool CServerSession::IsDequeueRequest() {
    m_mutex.lock();
    bool b = m_ReqInfo.GetQueued();
    m_mutex.unlock();
    return b;
}

unsigned int CServerSession::RemoveDequeueCache(unsigned int handle, SPA::UINT64 index) {
    CQueueMap::iterator it = m_mapDequeue.find(handle);
    if (it != m_mapDequeue.end()) {
        CVQAttr &v = it->second.second;
        CVQAttr::iterator vi, ve = v.end();
        for (vi = v.begin(); vi != ve; ++vi) {
            if (vi->MessageIndex == index) {
                unsigned int count = (unsigned int) (vi - v.begin()) + 1;
                v.erase(v.begin(), vi + 1);
                return count;
            }
        }
    }
    return 0;
}

void CServerSession::ConfirmFailed() {
    unsigned int removed = 0;
    CQueueMap::iterator it, end = m_mapDequeue.end();
    for (it = m_mapDequeue.begin(); it != end; ++it) {
        if (!m_bConfirmTrans) {
            CVQAttr &vQA = it->second.first;
            if (vQA.size() >= 2) {
                g_pServer->ConfirmQueue(it->first, vQA.data(), vQA.size());
                removed = RemoveDequeueCache(it->first, vQA.back().MessageIndex);
            } else if (vQA.size()) {
                MQ_FILE::QAttr &qa = vQA.front();
                g_pServer->ConfirmQueue(it->first, qa.MessagePos, qa.MessageIndex, true);
                removed = RemoveDequeueCache(it->first, qa.MessageIndex);
            }
        }
        CVQAttr &v = it->second.second;
        CVQAttr::iterator vi, ve = v.end();
        for (vi = v.begin(); vi != ve; ++vi) {
            g_pServer->ConfirmQueue(it->first, vi->MessagePos, vi->MessageIndex, false);
        }
    }
    m_mapDequeue.clear();
}

void CServerSession::SetUserID(const wchar_t *strUserId) {
    m_mutex.lock();
    m_ccb.UserId = strUserId;
    m_mutex.unlock();
}

unsigned int CServerSession::GetWritingBufferSizeAndSendTime(SPA::UINT64 &sendTime) {
    m_mutex.lock();
    unsigned int size = m_qWrite.GetSize();
    m_mutex.unlock();
    sendTime = m_ccb.SendTime;
    return size;
}

unsigned int CServerSession::GetWritingBufferSize() {
    m_mutex.lock();
    unsigned int size = m_qWrite.GetSize();
    m_mutex.unlock();
    return size;
}

unsigned int CServerSession::GetPassword(wchar_t *strPassword, unsigned int chars) {
    if (strPassword == nullptr || chars == 0)
        return 0;
    --chars;
    m_mutex.lock();
    if (chars > (unsigned int) m_ccb.Password.size())
        chars = (unsigned int) m_ccb.Password.size();
    if (chars > 0)
        ::memcpy(strPassword, m_ccb.Password.c_str(), sizeof (wchar_t) * chars);
    strPassword[chars] = 0;
    m_mutex.unlock();
    return chars;
}

unsigned int CServerSession::GetUID(wchar_t *strUserId, unsigned int chars) {
    m_mutex.lock();
    chars = GetUserIdInternally(strUserId, chars);
    m_mutex.unlock();
    return chars;
}

unsigned int CServerSession::GetUserIdInternally(wchar_t *strUserId, unsigned int chars) {
    if (strUserId == nullptr || chars == 0)
        return 0;
    --chars;
    if (chars > (unsigned int) m_ccb.UserId.size())
        chars = (unsigned int) m_ccb.UserId.size();
    if (chars > 0)
        ::memcpy(strUserId, m_ccb.UserId.c_str(), sizeof (wchar_t) * chars);
    strUserId[chars] = 0;
    return chars;
}

void CServerSession::SetPassword(const wchar_t *strPassword) {
    m_mutex.lock();
    m_ccb.Password = strPassword;
    m_mutex.unlock();
}

SPA::UINT64 CServerSession::GetBytesReceived() {
    CAutoLock sl(m_mutex);
    return m_ccb.m_ulRead;
}

void* CServerSession::GetSSL() {
    CAutoLock sl(m_mutex);
    if (m_pSsl)
        return m_pSsl->GetSSL();
    return nullptr;
}

bool CServerSession::GetClientInfo(SPA::CSwitchInfo *pClientInfo) {
    CAutoLock sl(m_mutex);
    if (pClientInfo)
        *pClientInfo = m_ClientInfo;
    return true;
}

bool CServerSession::GetServerInfo(SPA::CSwitchInfo *pServerInfo) {
    if (pServerInfo) {
        CAutoLock sl(m_mutex);
        *pServerInfo = m_ServerInfo;
        pServerInfo->ServiceId = ServiceId;
    }
    return true;
}

bool CServerSession::SetServerInfo(SPA::CSwitchInfo *pServerInfo) {
    CAutoLock sl(m_mutex);
    return SetServerInfoInternal(pServerInfo);
}

bool CServerSession::GetSockAddr(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars) {
    CErrorCode ec;
    unsigned short n;
    boost::asio::ip::tcp::endpoint ep;
    CAutoLock sl(m_mutex);
    ep = m_pSocket->local_endpoint(ec);
    if (ec)
        return false;
    if (sockPort)
        *sockPort = ep.port();
    if (!strIPAddrBuffer || !chars)
        return true;
    --chars;
    std::string str = ep.address().to_string();
    unsigned short max = (unsigned short) str.size();
    for (n = 0; n < max && n < chars; ++n) {
        strIPAddrBuffer[n] = str[n];
    }
    strIPAddrBuffer[n] = 0;
    return true;
}

bool CServerSession::SetServerInfoInternal(SPA::CSwitchInfo *pServerInfo) {
    if (pServerInfo && ServiceId != SPA::sidStartup) {
        m_ServerInfo.Param2 = g_pServer->m_ulPingInterval / 1000;
        m_ServerInfo.Param2 += g_pServer->m_am * 0x10000;
        m_ServerInfo.SockMinorVersion |= (m_pServiceContext->GetRandom() ? RETURN_RESULT_RANDOM : 0);
        m_ServerInfo.SockMinorVersion |= (m_pServiceContext->GetRoutingSvsId() ? (IS_ROUTING_PARTNER | RETURN_RESULT_RANDOM) : 0);
        {
            CRAutoLock ral(m_mutex, m_bChatting);
            m_ServerInfo.Param0 = (unsigned int) (m_pServiceContext->GetRouteeSize());
        }
        m_ServerInfo.SwitchTime = pServerInfo->SwitchTime;
        m_ServerInfo.ServiceId = ServiceId;
    }
    return true;
}

SPA::UINT64 CServerSession::GetBytesSent() {
    CAutoLock sl(m_mutex);
    return m_ccb.m_ulSent;
}

bool CServerSession::IsBatching() {
    CAutoLock sl(m_mutex);
    return (m_ccb.m_bBatching || m_pQBatch);
}

bool CServerSession::Enter(const unsigned int *pChatGroupId, unsigned int nCount) {
    if (pChatGroupId == nullptr)
        nCount = 0;
    unsigned short vt = (VT_UINT | VT_ARRAY);
    SPA::CScopeUQueue sb;
    sb << vt;
    sb << nCount;
    sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    return FakeAClientRequest(SPA::idEnter, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::ShrinkMemory() {
    CAutoLock sl(m_mutex);
    if (m_qRead.GetSize() == 0 && m_qRead.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE)
        m_qRead.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE);
    if (m_qWrite.GetSize() == 0 && m_qWrite.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE)
        m_qWrite.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE);
    if (ServiceId == SPA::sidHTTP && m_pHttpContext && m_pHttpContext->IsWebSocket()) {
        UHTTP::CWebSocketMsg *p = m_pHttpContext->GetWebSocketMsg();
        if (p && p->Content.GetSize() == 0 && p->Content.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE / 2)
            p->Content.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE / 2);
        UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
        pWebRequestProcessor->ShrinkMemory();
    }
}

void CServerSession::Exit() {
    FakeAClientRequest(SPA::idExit, nullptr, 0);
}

void CServerSession::Exit(const unsigned int *pChatGroupId, unsigned int nCount) {
    SPA::CScopeUQueue sb;
    if (pChatGroupId == nullptr)
        nCount = 0;
    if (nCount > 0) {
        sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    }
    FakeAClientRequest(SPA::idExit, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::Speak(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    if (!pChatGroupId || !nCount)
        return false;
    if (!message || size < sizeof (unsigned short))
        return false;
    SPA::CScopeUQueue sb;
    unsigned short vt = (VT_UINT | VT_ARRAY);
    sb << vt;
    sb << nCount;
    sb->Push((const unsigned char*) pChatGroupId, sizeof (unsigned int) * nCount);
    sb->Push(message, size);
    return FakeAClientRequest(SPA::idSpeak, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    if (!pChatGroupId)
        nCount = 0;
    if (!message)
        size = 0;
    SPA::CScopeUQueue sb;
    sb << size;
    sb->Push(message, size);
    sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    return FakeAClientRequest(SPA::idSpeakEx, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == nullptr)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    return FakeAClientRequest(SPA::idSendUserMessage, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == nullptr)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    return FakeAClientRequest(SPA::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
}

int CServerSession::ExecuteSlowRequestFromThreadPool(unsigned short sReqId) {
    int res = 0;
    PSLOW_PROCESS p = m_ccb.SvsContext.m_SlowProcess;
    if (p != nullptr) {
        try{
            res = p(sReqId, m_ReqInfo.Size, MakeHandlerInternal());
        }

        catch(SPA::CUException & err) {
            SendExceptionResult(err.what(), err.GetStack().c_str(), sReqId, err.GetErrCode());
            res = err.GetErrCode();
        }

        catch(std::exception & err) {
            SendExceptionResult(err.what(), "Inside worker thread for processing slow request", sReqId, MB_STL_EXCEPTION);
            res = MB_STL_EXCEPTION;
        }

        catch(...) {
            SendExceptionResult(L"Unknown exception caught", "Inside worker thread for processing slow request", sReqId, MB_UNKNOWN_EXCEPTION);
            res = MB_UNKNOWN_EXCEPTION;
        }
    }
    return res;
}

bool CServerSession::IsCanceled() {
    return m_bCanceled;
}

bool CServerSession::IsCanceledInternally() {
    unsigned int pos = 0;
    unsigned int lenAll = m_qRead.GetSize();
    unsigned int total = (m_ReqInfo.RequestId == SPA::idCancel) ? 1 : 0;
    if (!total) {
        SPA::CStreamHeader *p = &m_ReqInfo;
        while (lenAll > p->Size) {
            pos += p->Size;
            lenAll -= p->Size;
            if (lenAll < sizeof (SPA::CStreamHeader))
                break;
            p = (SPA::CStreamHeader*)m_qRead.GetBuffer(pos);
            if (p->RequestId == SPA::idCancel) {
                total = 1;
                break;
            }
            pos += sizeof (SPA::CStreamHeader);
            lenAll -= sizeof (SPA::CStreamHeader);
        }
    }
    if (total) {
        if (pos) {
            m_qRead.Pop(pos); //remove previous requests queued
#ifndef NDEBUG
            std::cout << "Canceled bytes = " << pos << std::endl;
#endif
        }
        m_qRead >> m_ReqInfo; //idCancel
        m_mapIndex.clear();
        OnBaseRequestArrive();
    }
    return (total > 0);
}

bool CServerSession::FakeAClientRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int nBufferSize) {
    SPA::CStreamHeader reqInfo;
    reqInfo.MakeFake();
    reqInfo.RequestId = reqId;
    if (pBuffer == nullptr)
        nBufferSize = 0;
    reqInfo.Size = nBufferSize;
    SPA::CScopeUQueue sb;
    sb << reqInfo;
    if (nBufferSize > 0)
        sb->Push(pBuffer, nBufferSize);
    CAutoLock sl(m_mutex);
    if (ServiceId == SPA::sidHTTP && m_ccb.Id.size() == 0) {
        if (m_pHttpContext == nullptr)
            return false;
        UHTTP::CWebRequestProcessor *p = m_pHttpContext->GetWebRequestProcessor();
        if (p == nullptr)
            return false;
        if (p->GetUHttpRequest().SpRequest != UHTTP::srSwitchTo)
            return false;
    }
    if (ServiceId == SPA::sidHTTP || m_pHttpContext)
        ++m_nHttpCallCount;
    m_qRead.Insert(sb->GetBuffer(), sb->GetSize(), m_ReqInfo.Size);
    return true;
}

unsigned int CServerSession::GetBytesBatched() {
    CAutoLock sl(m_mutex);
    if (m_pQBatch == nullptr)
        return 0;
    return m_pQBatch->GetSize();
}

bool CServerSession::StartBatching() {
    CAutoLock sl(m_mutex);
    if (m_pQBatch)
        return true;
    m_pQBatch = SPA::CScopeUQueue::Lock();
    return (m_pQBatch != nullptr);
}

bool CServerSession::CommitBatching() {
    CAutoLock sl(m_mutex);
    return CommitBatchingInternal();
}

void CServerSession::GetPeerName(std::string &addr, unsigned short *port) {
    CErrorCode ec;
    addr = GetSocket().remote_endpoint(ec).address().to_string();
    if (port != nullptr)
        *port = GetSocket().remote_endpoint(ec).port();
}

bool CServerSession::StartBatchingInternal() {
    if (m_pQBatch)
        return true;
    m_pQBatch = SPA::CScopeUQueue::Lock();
    return (m_pQBatch != nullptr);
}

bool CServerSession::CommitBatchingInternal() {
    if (!m_pQBatch)
        return false;
    if (m_pQBatch->GetSize() == 0) {
        SPA::CScopeUQueue::Unlock(m_pQBatch);
        m_pQBatch = nullptr;
        return true;
    }
    if (!g_pServer->m_bStopped) {
        if (m_bZip) {
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                if (m_pQBatch->GetSize() && CompressResultTo(IsOld(), SPA::idBatchZipped, m_zl, m_pQBatch->GetBuffer(), m_pQBatch->GetSize(), *sb)) {
                    Write(sb->GetBuffer(), sb->GetSize());
                }
            } else {
                if (m_pQBatch->GetSize() && CompressResultTo(IsOld(), SPA::idBatchZipped, m_zl, m_pQBatch->GetBuffer(), m_pQBatch->GetSize(), m_qWrite)) {
                    Write(nullptr, 0);
                }
            }
        } else if (m_pSsl) {
            Write(m_pQBatch->GetBuffer(), m_pQBatch->GetSize());
        } else if (m_qWrite.GetSize() == 0) {
            m_qWrite.Swap(*m_pQBatch);
            Write(nullptr, 0);
        } else {
            Write(m_pQBatch->GetBuffer(), m_pQBatch->GetSize());
        }
    }
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
    return true;
}

bool CServerSession::AbortBatchingInternal() {
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
    return true;
}

bool CServerSession::AbortBatching() {
    CAutoLock sl(m_mutex);
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
    return true;
}

SPA::UINT64 CServerSession::GetSocketNativeHandle() {
    return (SPA::UINT64) GetSocket().native_handle();
}

bool CServerSession::IsFakeRequest() {
    CAutoLock sl(m_mutex);
    return m_ReqInfo.IsFake();
}

void CServerSession::OnSlowRequestProcessed(unsigned int res, unsigned short usRequestId) {
    CAutoLock sl(m_mutex);
    g_pServer->PutThreadBackIntoPool(m_pUThread);
    m_pUThread = nullptr;
    if (m_cs < csConnected) {
        g_pServer->PostSproMessage(this, WM_SOCKET_SVR_NOTIFY, SOCKET_CLOSE_EVENT, m_ec.value());
        return;
    }
    POnSlowRequestProcessed p = m_ccb.SvsContext.m_OnRequestProcessed;
    m_ReqInfo.RequestId = 0;
    if (m_ReqInfo.Size > 0) {
        m_qRead.Pop(m_ReqInfo.Size);
        m_ReqInfo.Size = 0;
    }

    if (p != nullptr) {
        try{
            bool chatting = false;
            USocket_Server_Handle index = MakeHandlerInternal();
            CRAutoLock ral(m_mutex, chatting);
            p(index, usRequestId);
        }

        catch(SPA::CUException & err) {
            SendExceptionResultInternal(err.what(), err.GetStack().c_str(), usRequestId, err.GetErrCode());
        }

        catch(std::exception & err) {
            SendExceptionResultInternal(err.what(), "Inside slow request processed notification", usRequestId, MB_STL_EXCEPTION);
        }

        catch(...) {
            SendExceptionResultInternal(L"Unknown exception caught", "Inside slow request processed notification", usRequestId, MB_UNKNOWN_EXCEPTION);
        }
    }

    if (m_qRead.GetSize() >= sizeof (m_ReqInfo)) {
        Process();
    }
    Read();
    Write(nullptr, 0);
}

CSocket& CServerSession::GetSocket() {
    return *m_pSocket;
}

void CServerSession::Start() {
    boost::asio::ip::tcp::no_delay nodelay(true);
    boost::asio::socket_base::keep_alive option(false);
    CAutoLock rl(m_mutex);
#ifndef NDEBUG
    m_nJobRequest = 0;
    m_nJobConfirm = 0;
#endif
    m_bChatting = false;
    m_qBatchDequeueConfirm.SetSize(0);
    GetSocket().set_option(option, m_ec);
    GetSocket().set_option(nodelay, m_ec);
    m_qRead.SetSize(0);
    m_qWrite.SetSize(0);
    m_bRBLocked = false;
    m_bWBLocked = 0;
    m_ccb.RecvTime = (GetTimeTick() - g_pServer->m_tStart);
    m_ccb.SendTime = m_ccb.RecvTime.load();
    if (g_pServer->IsSsl()) {
        m_cs = csSslShaking;
        m_pSsl.reset(new CMyOpenSSL(g_pServer->m_pSslContext->native_handle(), false));
    } else {
        m_cs = csConnected;
    }
    m_bCanceled = false;
    ServiceId = SPA::sidStartup;
    Read();
}

void CServerSession::OnSslHandShake(const CErrorCode& Error) {
    POnSSLHandShakeCompleted p = g_pServer->m_pOnSSLHandShakeCompleted;
    //m_ccb.RecvTime = (boost::posix_time::microsec_clock::local_time() - g_pServer->m_tStart).total_milliseconds();
    //m_ccb.SendTime = m_ccb.RecvTime.load();
    //CAutoLock sl(m_mutex);
    m_ec = Error;
    if (p != nullptr) {
        try{
            USocket_Server_Handle index = MakeHandlerInternal();
            CRAutoLock rsl(m_mutex, m_bChatting);
            p(index, Error.value());
        }

        catch(...) {
        }
    }
    if (!Error) {
        m_cs = csConnected;
        Read();
    } else {
        CloseInternal();
    }
}

void CServerSession::OnClose() {
    POnClose p = m_ccb.SvsContext.m_OnClose;
    int errCode = m_ec.value();
    m_cv.notify_all();
    USocket_Server_Handle index = MakeHandlerInternal();
    CRAutoLock rsl(m_mutex, m_bChatting);
    if (p != nullptr) {
        try{
            p(index, errCode);
        }

        catch(...) {
        }
    }
    p = g_pServer->m_pOnClose;
    if (p != nullptr) {
        try{
            p(index, errCode);
        }

        catch(...) {
        }
    }
}

USocket_Server_Handle CServerSession::MakeHandlerInternal() {
    USocket_Server_Handle h = (USocket_Server_Handle) this;
    h <<= INDEX_SHIFT_BITS;
    h += m_ulIndex;
    return h;
}

USocket_Server_Handle CServerSession::MakeHandler() {
    CAutoLock sl(m_mutex);
    return MakeHandlerInternal();
}

void CServerSession::PostCloseInternal(int errCode) {
    if (errCode != 0) {
        m_ec.assign(errCode, boost::asio::error::get_system_category());
    }
    g_pServer->m_IoService.post(boost::bind(&CServerSession::Close, this));
}

void CServerSession::PostClose(int errCode) {
    CAutoLock sl(m_mutex);
    if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
        m_qRead.SetSize(0);
        if (m_pSsl) {
            SPA::CScopeUQueue sb;
            m_pHttpContext->PrepareWSResponseMessage(nullptr, 0, UHTTP::ocConnectionClose, *sb);
            Write(sb->GetBuffer(), sb->GetSize());
        } else {
            m_pHttpContext->PrepareWSResponseMessage(nullptr, 0, UHTTP::ocConnectionClose, m_qWrite);
            Write(nullptr, 0);
        }
    } else if (m_bChatting) {
        g_pServer->m_IoService.post(boost::bind(&CServerSession::PostClose, this, errCode));
    } else {
        PostCloseInternal(errCode);
    }
}

bool CServerSession::IsOpened() {
    CAutoLock sl(m_mutex);
    return (m_cs > csClosing && GetSocket().is_open());
}

bool CServerSession::IsSameEndian() {
    return (m_ReqInfo.IsBigEndian() == SPA::IsBigEndian());
}

void CServerSession::Close() {
    CAutoLock sl(m_mutex);
    CloseInternal();
}

bool CServerSession::IsRoutable(unsigned short reqId) {
    switch (reqId) {
        case SPA::idUnknown:
        case SPA::idSwitchTo:
        case SPA::idRouteeChanged:
        case SPA::idEncrypted:
        case SPA::idBatchZipped:
        case SPA::idCancel:
        case SPA::idGetSockOptAtSvr:
        case SPA::idSetSockOptAtSvr:
        case SPA::idDoEcho:
        case SPA::idPing:
        case SPA::idTurnOnZipAtSvr:
        case SPA::idSetZipLevelAtSvr:
        case SPA::idHttpClose:
        case SPA::idEnter:
        case SPA::idSpeak:
        case SPA::idSpeakEx:
        case SPA::idSendUserMessage:
        case SPA::idSendUserMessageEx:
        case SPA::idExit:
        case SPA::idStartQueue:
        case SPA::idStopQueue:
        case SPA::idDequeueBatchConfirmed:
            return false;
            break;
        default:
            break;
    }
    return true;
}

void CServerSession::EnableClientDequeue(bool enable) {
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idEnableClientDequeue;
    sh.Size = 1;
    SPA::CScopeUQueue sb;
    sb << sh << enable;
    CAutoLock sl(m_mutex);
    if (ServiceId == SPA::sidHTTP || ServiceId == SPA::sidStartup)
        return;
    Write(sb->GetBuffer(), sb->GetSize());
}

void CServerSession::SetZip(bool bZip) {
    CAutoLock sl(m_mutex);
    m_bZip = bZip;
}

bool CServerSession::GetZip() {
    CAutoLock sl(m_mutex);
    return m_bZip;
}

void CServerSession::SetZipLevel(SPA::tagZipLevel zl) {
    if (zl != SPA::zlBestSpeed && zl != SPA::zlDefault)
        return;
    CAutoLock sl(m_mutex);
    m_zl = zl;
}

SPA::tagZipLevel CServerSession::GetZipLevel() {
    CAutoLock sl(m_mutex);
    return m_zl;
}

bool CServerSession::IsOld() {
    return (m_ClientInfo.MajorVersion < 2);
}

unsigned int CServerSession::DecompressRequestTo(unsigned short ratio, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q) {
    unsigned int zSize = ratio * size;
    if (q.GetTailSize() < zSize) {
        unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
        q.ReallocBuffer(bufferSize);
    }
    zSize = q.GetTailSize();
    unsigned int start = q.GetSize();
    if (SPA::Decompress(zl, buffer, size, (void*) q.GetBuffer(start), zSize))
        q.SetSize(start + zSize);
    return (q.GetSize() - start);
}

unsigned int CServerSession::CompressResultTo(bool old, unsigned short reqId, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q) {
    unsigned int zSize;
    unsigned short ratio;
    bool ok = true;
    unsigned int start = q.GetSize();
    if (!buffer)
        size = 0;
    SPA::CStreamHeader sh;
    sh.RequestId = reqId;
    zSize = size;
    sh.Size = size;
    q << sh;
    switch (zl) {
        case SPA::zlDefault:
            if (size > ZLIB_COMPRESS_MIN_SIZE) {
                zSize = (unsigned int) (1.1 * size + 16);
                if (q.GetTailSize() < zSize) {
                    unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
                    q.ReallocBuffer(bufferSize);
                }
                zSize = q.GetTailSize();
                ok = SPA::Compress(zl, buffer, size, (void*) q.GetBuffer(q.GetSize()), zSize);
                if (ok) {
                    ratio = (unsigned short) (size / zSize) + 1;
                    sh.OrTreat((unsigned char) (ratio / 256), old, false);
                    sh.SetRatio((unsigned char) (ratio % 256));
                    assert(sh.GetZipRatio(old) == ratio || sh.GetZipRatio() > SPA::CStreamHeader::ALL_ZIPPED);
                    sh.Size = zSize;
                    q.SetSize(q.GetSize() + zSize);
                    q.Replace(start, sizeof (sh), (const unsigned char*) &sh, sizeof (sh));
                } else {
                    assert(false);
                }
            } else {
                q.Push(buffer, size);
            }
            break;
        case SPA::zlBestSpeed:
            if (size > FAST_COMPRESS_MIN_SIZE) {
                zSize = (unsigned int) (size + 420);
                if (q.GetTailSize() < zSize) {
                    unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
                    q.ReallocBuffer(bufferSize);
                }
                zSize = q.GetTailSize();
                ok = SPA::Compress(zl, buffer, size, (void*) q.GetBuffer(q.GetSize()), zSize);
                if (ok) {
                    ratio = (unsigned short) (size / zSize) + 1;
                    assert(ratio < 256);
                    sh.SetRatio((unsigned char) ratio);
                    assert(sh.GetZipRatio(old) == ratio);
                    sh.Size = zSize;
                    q.SetSize(q.GetSize() + zSize);
                    q.Replace(start, sizeof (sh), (const unsigned char*) &sh, sizeof (sh));
                } else {
                    assert(false);
                }
            } else {
                q.Push(buffer, size);
            }
            break;
        default:
            ok = false;
            assert(false); //not implemented
            break;
    }
    if (!ok)
        q.SetSize(start);
    return (q.GetSize() - start);
}

unsigned int CServerSession::SendExceptionResultInternal(const char* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    SPA::CScopeUQueue su;
    if (errMessage)
        SPA::Utilities::ToWide(errMessage, ::strlen(errMessage), *su);
    return SendExceptionResultInternal((const wchar_t*) su->GetBuffer(), errWhere, requestId, errCode);
}

unsigned int CServerSession::SendExceptionResult(const char* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    SPA::CScopeUQueue su;
    if (errMessage)
        SPA::Utilities::ToWide(errMessage, ::strlen(errMessage), *su);
    return SendExceptionResult((const wchar_t*) su->GetBuffer(), errWhere, requestId, errCode);
}

unsigned int CServerSession::SendExceptionResultInternal(const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    if (ServiceId == SPA::sidHTTP) {
        UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
        if (m_pHttpContext)
            pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
        if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake() && m_pHttpContext->GetResponseProgress().Status != UHTTP::hrsCompleted)) {
            unsigned int res;
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, *sb);
                Write(sb->GetBuffer(), sb->GetSize());
            } else {
                res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, m_qWrite);
                Write(nullptr, 0);
            }
            return res;
        } else {
            return Connection::CConnectionContext::SendExceptionResult(m_ccb.Id, errMessage, errWhere, requestId, errCode);
        }
    }
    if (requestId == 0)
        requestId = m_ReqInfo.RequestId;
    if (errCode == 0)
        errCode = MB_UNKNOWN_EXCEPTION;
    SPA::CScopeUQueue sb;
    sb->Push((const unsigned char*) &requestId, sizeof (requestId));
    sb << errMessage << errCode << errWhere;
    return SendReturnDataInternal(SPA::idServerException, sb->GetBuffer(), sb->GetSize());
}

unsigned int CServerSession::SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    {
        CAutoLock sl(m_mutex);
        if (ServiceId == SPA::sidHTTP) {
            UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
            if (m_pHttpContext)
                pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
            if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake() && m_pHttpContext->GetResponseProgress().Status != UHTTP::hrsCompleted)) {
                unsigned int res;
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, *sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, m_qWrite);
                    Write(nullptr, 0);
                }
                return res;
            } else {
                return Connection::CConnectionContext::SendExceptionResult(m_ccb.Id, errMessage, errWhere, requestId, errCode);
            }
        }
    }
    if (requestId == 0)
        requestId = GetCurrentRequestID();
    if (errCode == 0)
        errCode = MB_UNKNOWN_EXCEPTION;
    SPA::CScopeUQueue sb;
    sb->Push((const unsigned char*) &requestId, sizeof (requestId));
    sb << errMessage << errCode << errWhere;
    return SendReturnData(SPA::idServerException, sb->GetBuffer(), sb->GetSize());
}

unsigned int CServerSession::SendExceptionResultIndex(SPA::UINT64 indexCall, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    {
        CAutoLock sl(m_mutex);
        if (ServiceId == SPA::sidHTTP) {
            UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
            if (m_pHttpContext)
                pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
            if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake() && m_pHttpContext->GetResponseProgress().Status != UHTTP::hrsCompleted)) {
                unsigned int res;
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, *sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, m_qWrite);
                    Write(nullptr, 0);
                }
                return res;
            } else {
                return Connection::CConnectionContext::SendExceptionResult(m_ccb.Id, errMessage, errWhere, requestId, errCode);
            }
        }
    }
    if (requestId == 0)
        requestId = GetCurrentRequestID();
    if (errCode == 0)
        errCode = MB_UNKNOWN_EXCEPTION;
    SPA::CScopeUQueue sb;
    sb->Push((const unsigned char*) &requestId, sizeof (requestId));
    sb << errMessage << errCode << errWhere;
    return SendReturnDataIndex(indexCall, SPA::idServerException, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::IsBuiltinAllowed(unsigned int sid) {
    switch (sid) {
        case SPA::sidChat:
        case SPA::sidODBC:
        case SPA::sidFile:
        case SPA::Sqlite::sidSqlite:
        case SPA::Mysql::sidMysql:
            return true;
            break;
        default:
            break;
    }
    return false;
}

void CServerSession::CloseInternal() {
    if (m_bCloseInternal) {
        return;
    }
    m_bCloseInternal = true;
    if (m_cs >= csConnected && m_pRoutingServiceContext != nullptr && m_pServiceContext != nullptr) {
        USocket_Server_Handle index = MakeHandlerInternal();
        CRAutoLock ral(m_mutex, m_bChatting);
        NotifyFailRoutes(index, m_pServiceContext);
    }
    if (m_cs == csClosed) {
        m_bCloseInternal = false;
        return;
    }
    m_cs = csClosing;
    bool notify = m_ccb.ChatGroups.size() > 0;
    if (notify) {
        if (ServiceId != SPA::sidHTTP || (m_pHttpContext && m_pHttpContext->IsWebSocket())) {
            {
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->Exit(this, m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
            }
            m_ccb.ChatGroups.clear();
        } else if (ServiceId == SPA::sidHTTP) {
            Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
            if (sp && !sp->IsGet && sp->IsOpera) {
                //Opera AJAX has problem in keep-alive
            } else {
                {
                    CRAutoLock rsl(m_mutex, m_bChatting);
                    g_pServer->Exit(this, m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
                }
                m_ccb.ChatGroups.clear();
                Connection::CConnectionContext::RemoveConnectionContext(m_ccb.Id.c_str());
            }
        }
        g_pServer->m_IoService.post(boost::bind(&CServerSession::Close, this));
        m_bCloseInternal = false;
        return;
    } else if (m_pUThread) {
        m_bCloseInternal = false;
        return; //wait until worker thread completes the current processing
    }
    OnClose();
    UHTTP::CHttpContext::Unlock(m_pHttpContext);
    m_pHttpContext = nullptr;

#ifndef NDEBUG
    if (m_nJobRequest) {
        std::cout << "Bad dequeue job balance = " << __FUNCTION__ << ", balance = " << m_nJobRequest << std::endl;
    }
    if (m_nJobConfirm) {
        std::cout << "Bad confirm job balance = " << __FUNCTION__ << ", balance = " << m_nJobConfirm << std::endl;
    }
#endif 

    CErrorCode ec;
    GetSocket().shutdown(boost::asio::socket_base::shutdown_both, ec);
    GetSocket().close(ec);
    m_mapIndex.clear();
    g_pServer->Recycle(this);
    m_bCloseInternal = false;
}

SPA::UINT64 CServerSession::GetLatestTime() const {
    return (m_ccb.RecvTime > m_ccb.SendTime) ? m_ccb.RecvTime : m_ccb.SendTime;
}

unsigned int CServerSession::GetCountOfJoinedChatGroups() {
    CAutoLock sl(m_mutex);
    return (unsigned int) m_ccb.ChatGroups.size();
}

unsigned int CServerSession::GetJoinedGroupIds(unsigned int *pChatGroup, unsigned int count) {
    unsigned int n;
    if (pChatGroup == nullptr)
        count = 0;
    CAutoLock sl(m_mutex);
    if (count > (unsigned int) m_ccb.ChatGroups.size())
        count = (unsigned int) m_ccb.ChatGroups.size();
    for (n = 0; n < count; ++n) {
        pChatGroup[n] = m_ccb.ChatGroups[n];
    }
    return n;
}

void CServerSession::DropCurrentSlowRequest() {
    CAutoLock sl(m_mutex);
    m_bDropSlowRequest = true;
    if (m_ReqInfo.Size) {
        m_qRead.Pop(m_ReqInfo.Size);
        m_ReqInfo.Size = 0;
    }
}

unsigned int CServerSession::Write(const SPA::CStreamHeader &sh, const unsigned char *s, unsigned int nSize) {
    unsigned int ulLen;
    if (m_cs < csSslShaking)
        return 0;
    if (s == nullptr)
        nSize = 0;
    if (m_pSsl) {
        SPA::CScopeUQueue sb;
        sb << sh;
        sb->Push(s, nSize);
        return Write(sb->GetBuffer(), sb->GetSize());
    }
    if (m_qWrite.GetTailSize() < nSize + sizeof (sh) && m_qWrite.GetHeadPosition() >= nSize + sizeof (sh))
        m_qWrite.SetHeadPosition();
    if (m_bWBLocked) {
        m_qWrite << sh;
        m_qWrite.Push(s, nSize);
        return nSize + sizeof (sh);
    }
    ulLen = m_qWrite.GetSize();
    if (ulLen == 0) {
        ::memcpy(m_WriteBuffer, &sh, sizeof (sh));
        if (nSize + sizeof (sh) <= IO_BUFFER_SIZE) {
            if (nSize)
                ::memcpy(m_WriteBuffer + sizeof (sh), s, nSize);
            ulLen = nSize + sizeof (sh);
        } else {
            ::memcpy(m_WriteBuffer + sizeof (sh), s, IO_BUFFER_SIZE - sizeof (sh));
            ulLen = IO_BUFFER_SIZE;

            //remaining
            m_qWrite.Push(s + (IO_BUFFER_SIZE - sizeof (sh)), nSize - (IO_BUFFER_SIZE - sizeof (sh)));
        }
    } else {
        m_qWrite << sh;
        m_qWrite.Push(s, nSize);
        ulLen = m_qWrite.GetSize();
        if (ulLen > IO_BUFFER_SIZE)
            ulLen = IO_BUFFER_SIZE;
        m_qWrite.Pop(m_WriteBuffer, ulLen);
    }
    m_ccb.m_ulSent += ulLen;
    m_bWBLocked = ulLen;

    m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CServerSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
    return ulLen;
}

unsigned int CServerSession::Write(const unsigned char *s, unsigned int nSize) {
    unsigned int ulLen;
    if (m_cs < csSslShaking)
        return 0;
    if (s == nullptr)
        nSize = 0;
    if (m_pSsl) {
        SPA::CScopeUQueue su;
        if (s && nSize) {
            m_pSsl->Encrypt(s, nSize, *su);
        }
        s = su->GetBuffer();
        nSize = su->GetSize();
        if (m_bWBLocked) {
            if (m_qWrite.GetTailSize() < nSize && m_qWrite.GetHeadPosition() >= nSize)
                m_qWrite.SetHeadPosition();
            m_qWrite.Push(s, nSize);
            return nSize;
        }
        ulLen = m_qWrite.GetSize();
        if (ulLen == 0 && s && nSize > 0) {
            if (nSize <= IO_BUFFER_SIZE) {
                ::memcpy(m_WriteBuffer, s, nSize);
                ulLen = nSize;
            } else {
                ::memcpy(m_WriteBuffer, s, IO_BUFFER_SIZE);
                ulLen = IO_BUFFER_SIZE;

                //remaining
                m_qWrite.Push(s + IO_BUFFER_SIZE, nSize - IO_BUFFER_SIZE);
            }
        } else {
            m_qWrite.Push(s, nSize);
            ulLen = m_qWrite.GetSize();
            if (ulLen == 0)
                return nSize;
            if (ulLen > IO_BUFFER_SIZE)
                ulLen = IO_BUFFER_SIZE;
            m_qWrite.Pop(m_WriteBuffer, ulLen);
        }
    } else {
        if (m_bWBLocked) {
            if (m_qWrite.GetTailSize() < nSize && m_qWrite.GetHeadPosition() >= nSize)
                m_qWrite.SetHeadPosition();
            m_qWrite.Push(s, nSize);
            return nSize;
        }
        ulLen = m_qWrite.GetSize();
        if (ulLen == 0 && s && nSize > 0) {
            if (nSize <= IO_BUFFER_SIZE) {
                ::memcpy(m_WriteBuffer, s, nSize);
                ulLen = nSize;
            } else {
                ::memcpy(m_WriteBuffer, s, IO_BUFFER_SIZE);
                ulLen = IO_BUFFER_SIZE;

                //remaining
                m_qWrite.Push(s + IO_BUFFER_SIZE, nSize - IO_BUFFER_SIZE);
            }
        } else {
            m_qWrite.Push(s, nSize);
            ulLen = m_qWrite.GetSize();
            if (ulLen == 0)
                return nSize;
            if (ulLen > IO_BUFFER_SIZE)
                ulLen = IO_BUFFER_SIZE;
            m_qWrite.Pop(m_WriteBuffer, ulLen);
        }
    }
    m_ccb.m_ulSent += ulLen;
    m_bWBLocked = ulLen;
    m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CServerSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
    return nSize;
}

void CServerSession::Read() {
    if (m_bChatting || g_pServer->m_bStopped)
        return;
    if (m_cs < csSslShaking)
        return;
    if (m_bRBLocked || ((m_pUThread || m_nHttpCallCount) && m_qRead.GetIdleSize() < IO_BUFFER_SIZE))
        return;
    if (IsTooMany(m_mapIndex) || (m_qRead.GetSize() > 20 * IO_BUFFER_SIZE && (m_nHttpCallCount > 1 || QueryRequestsQueuedInternally() > 1)))
        return;
    m_bRBLocked = true;
    m_pSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), boost::bind(&CServerSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

unsigned int CServerSession::GetCurrentRequestLen() {
    m_mutex.lock();
    unsigned int len = m_ReqInfo.Size;
    m_mutex.unlock();
    return len;
}

unsigned short CServerSession::GetCurrentRequestID() {
    m_mutex.lock();
    unsigned short s = m_ReqInfo.RequestId;
    m_mutex.unlock();
    return s;
}

unsigned int CServerSession::GetSndBytesInQueue() {
    m_mutex.lock();
    unsigned int len = m_qWrite.GetSize();
    m_mutex.unlock();
    return len;
}

unsigned int CServerSession::GetSndBytesInQueueInternal() {
    return m_qWrite.GetSize();
}

unsigned int CServerSession::GetRcvBytesInQueue() {
    m_mutex.lock();
    unsigned int len = m_qRead.GetSize();
    m_mutex.unlock();
    return len;
}

unsigned int CServerSession::GetConnIndex() {
    if (m_cs < csClosing)
        return 0;
    return m_ulIndex;
}

unsigned int CServerSession::GetSvsID() {
    //m_mutex.lock();
    //unsigned int id = ServiceId;
    //m_mutex.unlock();
    return ServiceId;
}

bool CServerSession::Wait(unsigned int nTimeout) {
    CAutoLock al(m_mutex);
    return (m_cv.wait_for(al, MQ_FILE::ms(nTimeout)) == std::cv_status::no_timeout);
}

int CServerSession::GetErrorCode() {
    m_mutex.lock();
    int ec = m_ec.value();
    m_mutex.unlock();
    return ec;
}

std::string CServerSession::GetErrorMessage() {
    CAutoLock sl(m_mutex);
    switch (m_ec.value()) {
        case ERROR_NO_ERROR:
            return "ok";
        case ERROR_WRONG_SWITCH:
            return "bad service switch";
        case ERROR_AUTHENTICATION_FAILED:
            return "authentication failed";
        case ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE:
            return "no service found at server side";
        case ERROR_NOT_SWITCHED_YET:
            return "method SwitchTo not called yet";
        case ERROR_BAD_REQUEST:
            return "bad request";
        default:
            break;
    }
    return m_ec.message();
}

unsigned int CServerSession::SendReturnDataInternal(unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize) {
    if (ServiceId == SPA::sidHTTP) {
        g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
        return 0;
    }
    if (usReqId != SPA::idSwitchTo) {
        if (usReqId == m_ReqInfo.RequestId) {
            if (m_pServiceContext && m_pServiceContext->GetRandom()) {
                return SendReturnDataIndexInternal(m_indexCall, usReqId, pBuffer, ulBufferSize);
            }
        } else if (m_pServiceContext && m_pServiceContext->GetRandom())
            return BAD_OPERATION;
    }
    if (pBuffer == nullptr)
        ulBufferSize = 0;
    if (m_cs < csConnected || g_pServer->m_bStopped)
        return SOCKET_NOT_FOUND;
    if (usReqId != SPA::idCancel && m_bCanceled)
        return REQUEST_CANCELED;
    if (m_pQBatch) {
        SPA::CStreamHeader sh;
        sh.RequestId = usReqId;
        sh.Size = ulBufferSize;
        *m_pQBatch << sh;
        m_pQBatch->Push(pBuffer, ulBufferSize);
    } else if (!m_bZip) {
        SPA::CStreamHeader sh;
        sh.RequestId = usReqId;
        sh.Size = ulBufferSize;
        Write(sh, pBuffer, ulBufferSize);
    } else { //zipped
        if (m_pSsl) {
            SPA::CScopeUQueue sb;
            if (CompressResultTo(IsOld(), usReqId, m_zl, pBuffer, ulBufferSize, *sb)) {
                Write(sb->GetBuffer(), sb->GetSize());
            }
        } else {
            if (CompressResultTo(IsOld(), usReqId, m_zl, pBuffer, ulBufferSize, m_qWrite)) {
                Write(nullptr, 0);
            }
        }
    }
    if (m_ReqInfo.GetQueued() && usReqId == m_ReqInfo.RequestId) {
        assert(m_qa.IsValid());
        if (!m_bFail && !m_bDequeueTrans)
            m_pQLastIndex->Set(m_ClientQFile.Qs, m_qa);
        NotifyDequeued();
    }
    return ulBufferSize;
}

unsigned int CServerSession::SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize) {
    CAutoLock sl(m_mutex);
    return SendReturnDataInternal(usReqId, pBuffer, ulBufferSize);
}

unsigned int CServerSession::SendReturnDataIndex(SPA::UINT64 indexCall, unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize) {
    CAutoLock sl(m_mutex);
    return SendReturnDataIndexInternal(indexCall, usReqId, pBuffer, ulBufferSize);
}

unsigned int CServerSession::SendReturnDataIndexInternal(SPA::UINT64 indexCall, unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize) {
    auto it = m_mapIndex.find(indexCall);
    if (it == m_mapIndex.end())
        return BAD_OPERATION;
    std::shared_ptr<CResIndexImpl> ri = it->second;
    bool head = (it == m_mapIndex.begin());
    ri->SetHead(head);
    unsigned int res = ri->SendReturnData(usReqId, pBuffer, ulBufferSize);
    if (res >= RESULT_SENDING_FAILED) {
        m_mapIndex.clear();
        return res;
    }
    PutOntoWireInternal();
    return res;
}

void CServerSession::PutOntoWire() {
    CAutoLock sl(m_mutex);
    PutOntoWireInternal();
}

void CServerSession::PutOntoWireInternal() {
    bool bContinue = true;
    while (m_mapIndex.size() && bContinue) {
        bContinue = false;
        auto it = m_mapIndex.begin();
        std::shared_ptr<CResIndexImpl> ri = it->second;
        unsigned short reqId = ri->GetReqId();
        if ((reqId == SPA::idStartJob || reqId == SPA::idEndJob) && !::IsMainThread()) {
            g_pServer->m_IoService.post(boost::bind(&CServerSession::PutOntoWire, this));
            break;
        }
        bool partial = false;
        bool is_all = ri->IsAllCollected(&partial);
        SPA::UINT64 res = 0;
        if (is_all) {
            res = ri->SendAll();
        } else if (partial) {
            res = ri->SendPartial();
        }
        bool sent = ri->IsSent();
        if (sent) {
            m_mapIndex.erase(it);
            bContinue = true;
        }
        if (m_qWrite.GetSize() > 60 * IO_BUFFER_SIZE)
            break;
    }
}

unsigned int CServerSession::RetrieveRequestBuffer(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek) {
    m_mutex.lock();
    unsigned int ret = RetrieveRequestBufferInternally(pBuffer, ulBufferSize, bPeek);
    m_mutex.unlock();
    return ret;
}

const unsigned char* CServerSession::GetRequestBuffer() {
    //m_mutex.lock();
    const unsigned char *p = m_qRead.GetBuffer();
    //m_mutex.unlock();
    return p;
}

unsigned int CServerSession::RetrieveRequestBufferInternally(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek) {
    unsigned int ulGet = 0;
    if (pBuffer && ulBufferSize) {
        if (ulBufferSize > m_ReqInfo.Size) {
            ulBufferSize = m_ReqInfo.Size;
        }
        if (m_ReqInfo.RequestId == SPA::idSwitchTo) {
            bPeek = false;
            unsigned char *pHead = (unsigned char*) m_qRead.GetBuffer();
            ulGet = m_qRead.Pop((unsigned char*) pBuffer, ulBufferSize);
            ::memset(pHead, 0, ulBufferSize);
        } else {
            if (bPeek) {
                ::memcpy(pBuffer, m_qRead.GetBuffer(), ulBufferSize);
                ulGet = ulBufferSize;
            } else {
                ulGet = m_qRead.Pop((unsigned char*) pBuffer, ulBufferSize);
            }
        }
        if (!bPeek) {
            m_ReqInfo.Size -= ulGet;
        }
    }
    return ulGet;
}

unsigned int CServerSession::QueryRequestsQueued() {
    m_mutex.lock();
    unsigned int count = QueryRequestsQueuedInternally();
    m_mutex.unlock();
    return count;
}

unsigned int CServerSession::QueryRequestsQueuedInternally() {
    if (ServiceId == SPA::sidStartup)
        return 1;
    if (m_pHttpContext && m_pHttpContext->GetPS() != UHTTP::psComplete)
        return 1;
    unsigned int ulRtns = 0;
    if (m_ReqInfo.RequestId && m_ReqInfo.Size > 0)
        ulRtns++;
    unsigned int ulStartPos = m_ReqInfo.Size;
    while (ulStartPos + sizeof (SPA::CStreamHeader) <= m_qRead.GetSize()) {
        SPA::CStreamHeader *pStreamHeader = (SPA::CStreamHeader *)m_qRead.GetBuffer(ulStartPos);
        ulStartPos += (pStreamHeader->Size + sizeof (SPA::CStreamHeader));
        ulRtns++;
    }
    return ulRtns;
}

void CServerSession::OnNonBaseRequestArrive() {
    m_bDropSlowRequest = false;
    POnRequestArrive p = m_ccb.SvsContext.m_OnRequestArrive;
    POnFastRequestArrive pF = m_ccb.SvsContext.m_OnFastRequestArrive;
    USocket_Server_Handle index = MakeHandlerInternal();
    unsigned short reqId = m_ReqInfo.RequestId;
    unsigned long size = m_ReqInfo.Size;
    if (m_pServiceContext->GetRandom()) {
        if (m_ReqInfo.GetQueued()) {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        } else {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        }
    }
    CRAutoLock rsl(m_mutex, m_bChatting);
    if (p != nullptr) {
        p(index, reqId, size);
    }
    if (m_pServiceContext->IsSlowRequest(m_ReqInfo.RequestId)) {
        if (m_bDropSlowRequest) {
            return;
        }
        assert(m_pUThread == nullptr);
        m_pUThread = g_pServer->GetOneThread(m_pServiceContext->GetSvsContext().m_ta);
        m_pUThread->PostMessage(this, m_ReqInfo.RequestId, WM_ASK_FOR_PROCESSING, nullptr, 0);
        return;
    }
    if (pF != nullptr) {
        pF(index, reqId, size);
    }
}

void CServerSession::OnBaseRequestArrive() {
    switch (m_ReqInfo.RequestId) {
        case SPA::idDequeueBatchConfirmed:
        case SPA::idDequeueConfirmed:
        {
            unsigned int removed = 0;
            unsigned int qHandle;
            unsigned short reqId;
            MQ_FILE::CDequeueConfirmInfo dci;
            assert(m_ReqInfo.Size >= sizeof (MQ_FILE::CDequeueConfirmInfo));
            m_qRead >> dci;
            qHandle = dci.Handle;
            m_qa = dci.QA;
            reqId = dci.RequestId;
            m_ReqInfo.Size -= sizeof (MQ_FILE::CDequeueConfirmInfo);
            CQueueMap::iterator it = m_mapDequeue.find(qHandle);
            while (m_ReqInfo.Size >= sizeof (MQ_FILE::CDequeueConfirmInfo)) {
                assert(m_ReqInfo.RequestId == SPA::idDequeueBatchConfirmed);
                CVQAttr &vQA = it->second.first;
                vQA.push_back(m_qa);
                m_qRead >> dci;
                assert(dci.Handle == qHandle);
                assert(dci.QA.MessagePos < MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END);
                qHandle = dci.Handle;
                m_qa = dci.QA;
                reqId = dci.RequestId;
                m_ReqInfo.Size -= sizeof (MQ_FILE::CDequeueConfirmInfo);
            }
            assert(m_ReqInfo.Size == 0);
            switch (reqId) {
                case SPA::idStartJob:
#ifndef NDEBUG
                    ++m_nJobConfirm;
#endif
                    if (it != m_mapDequeue.end()) {
                        CVQAttr &vQA = it->second.first;
                        if (vQA.size() >= 2) {
                            CRAutoLock rsl(m_mutex, m_bChatting);
                            g_pServer->ConfirmQueue(qHandle, vQA.data(), vQA.size());
                        } else if (vQA.size() == 1) {
                            const MQ_FILE::QAttr &qa = vQA.front();
                            CRAutoLock rsl(m_mutex, m_bChatting);
                            g_pServer->ConfirmQueue(qHandle, qa.MessagePos, qa.MessageIndex, true);
                        }
                        if (vQA.size()) {
                            removed = RemoveDequeueCache(qHandle, vQA.back().MessageIndex);
                            vQA.clear();
                        }
                        vQA.push_back(m_qa);
                    }
                    assert(!m_bConfirmTrans);
                    m_bConfirmFail = dci.Fail;
                    m_bConfirmTrans = true;
                    break;
                case SPA::idEndJob:
#ifndef NDEBUG
                    --m_nJobConfirm;
#endif
                    assert(m_bConfirmTrans);
                    if (!m_bConfirmFail)
                        m_bConfirmFail = dci.Fail;
                    if (it != m_mapDequeue.end()) {
                        CVQAttr &vQA = it->second.first;
                        vQA.push_back(m_qa);
                        assert(vQA.size() >= 2);
                        {
                            CRAutoLock rsl(m_mutex, m_bChatting);
                            g_pServer->ConfirmQueueJob(qHandle, vQA.data(), vQA.size(), !m_bConfirmFail);
                        }
                        removed = RemoveDequeueCache(qHandle, m_qa.MessageIndex);
                        vQA.clear();
                    } else {
                        assert(false);
                    }
                    m_bConfirmTrans = false;
                    break;
                default:
                    if (m_bConfirmTrans) {
                        if (!m_bConfirmFail)
                            m_bConfirmFail = dci.Fail;
                        if (it != m_mapDequeue.end()) {
                            it->second.first.push_back(m_qa);
                            assert(it->second.first.size() >= 2);
                        }
                    } else {
                        if (dci.Fail) {
                            if (it != m_mapDequeue.end()) {
                                CVQAttr &vQA = it->second.first;
                                {
                                    CRAutoLock rsl(m_mutex, m_bChatting);
                                    if (vQA.size() >= 2) {
                                        g_pServer->ConfirmQueue(qHandle, vQA.data(), vQA.size());
                                    } else if (vQA.size() == 1) {
                                        const MQ_FILE::QAttr &qa = vQA.front();
                                        g_pServer->ConfirmQueue(qHandle, qa.MessagePos, qa.MessageIndex, true);
                                    }
                                    g_pServer->ConfirmQueue(qHandle, m_qa.MessagePos, m_qa.MessageIndex, !dci.Fail);
                                }
                                removed = RemoveDequeueCache(qHandle, m_qa.MessageIndex);
                                vQA.clear();
                            } else {
                                assert(false);
                            }
                        } else {
                            if (it != m_mapDequeue.end()) {
                                CVQAttr &vQA = it->second.first;
                                vQA.push_back(m_qa);
                                size_t size = it->second.second.size();
                                SPA::UINT64 posFront = vQA.front().MessagePos;
                                SPA::UINT64 posBack = vQA.back().MessagePos;
                                if (vQA.size() == size || (posBack - posFront) >= 2 * 1024 * 1024) {
                                    if (vQA.size() > 1) {
                                        CRAutoLock rsl(m_mutex, m_bChatting);
                                        g_pServer->ConfirmQueue(qHandle, vQA.data(), vQA.size());
                                    } else {
                                        const MQ_FILE::QAttr &qa = vQA.front();
                                        CRAutoLock rsl(m_mutex, m_bChatting);
                                        g_pServer->ConfirmQueue(qHandle, qa.MessagePos, qa.MessageIndex, true);
                                    }
                                    removed = RemoveDequeueCache(qHandle, m_qa.MessageIndex);
                                    vQA.clear();
                                }
                            } else {
                                assert(false);
                            }
                        }
                    }
                    break;
            }
#ifndef NDEBUG
            if (m_nJobConfirm > 1) {
                std::cout << "Bad confirm job balance = " << __FUNCTION__ << ", balance = " << m_nJobConfirm << std::endl;
            }
#endif
            return; //don't enable the callback OnBaseRequestCame for the request
        }
            break;
        case SPA::idStartBatching:
        {
            StartBatchingInternal();
            if (m_ClientQFile.Qs.qs.Header != 0 || m_ClientQFile.Qs.qs.Mid != 0 || m_ClientQFile.Qs.qs.End != 0)
                SendReturnDataInternal(SPA::idStartBatching, nullptr, 0);
        }
            break;
        case SPA::idCommitBatching:
        {
            if (m_ClientQFile.Qs.qs.Header != 0 || m_ClientQFile.Qs.qs.Mid != 0 || m_ClientQFile.Qs.qs.End != 0)
                SendReturnDataInternal(SPA::idCommitBatching, nullptr, 0);
            CommitBatchingInternal();
        }
            break;
        case SPA::idSetZipLevelAtSvr:
            assert(sizeof (int) == m_ReqInfo.Size);
        {
            int zl;
            m_qRead >> zl;
            m_ReqInfo.Size -= sizeof (zl);
            m_zl = (SPA::tagZipLevel)zl;
            SendReturnDataInternal(SPA::idSetZipLevelAtSvr, (const unsigned char*) &zl, sizeof (zl));
        }
            break;
        case SPA::idTurnOnZipAtSvr:
            assert(sizeof (bool) == m_ReqInfo.Size);
        {
            m_qRead >> m_bZip;
            m_ReqInfo.Size -= sizeof (m_bZip);
            bool zip = m_bZip;
            SendReturnDataInternal(SPA::idTurnOnZipAtSvr, (const unsigned char*) &zip, sizeof (zip));
        }
            break;
        case SPA::idSetSockOptAtSvr:
            assert(3 * sizeof (int) == m_ReqInfo.Size);
        {
            int optName;
            int optValue;
            int level;
            int hr = 0;
            m_qRead >> optName >> optValue >> level;
            optName = SPA::MapSockOption((SPA::tagSocketOption)optName);
            level = SPA::MapSockLevel((SPA::tagSocketLevel)level);
            m_ReqInfo.Size = 0;
            hr = ::setsockopt(GetSocket().native_handle(), level, optName, (const char*) &optValue, sizeof (optValue));
            SendReturnDataInternal(SPA::idSetSockOptAtSvr, (const unsigned char*) &hr, sizeof (hr));
        }
            break;
        case SPA::idGetSockOptAtSvr:
            assert(2 * sizeof (int) == m_ReqInfo.Size);
        {
            int optName;
            int level;
            int optValue = 0;
#ifdef WIN32_64
            static int len = sizeof (optValue);
#else
            static socklen_t len = sizeof (optValue);
#endif
            m_qRead >> optName >> level;
            m_ReqInfo.Size = 0;
            ::getsockopt(GetSocket().native_handle(), level, optName, (char*) &optValue, &len);
            SendReturnDataInternal(SPA::idGetSockOptAtSvr, (const unsigned char*) &optValue, sizeof (optValue));
        }
            break;
        case SPA::idStopQueue:
            m_qRead >> m_ClientQFile;
            m_pQLastIndex->Remove(m_ClientQFile.Qs);
            m_ReqInfo.Size = 0;
            ::memset(&m_ClientQFile, 0, sizeof (m_ClientQFile));
            m_bDequeueTrans = false;
            break;
        case SPA::idStartQueue:
            m_qRead >> m_ClientQFile;
            if ((m_ClientQFile.MinIndex & MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) == MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) {
                if (m_pQLastIndex) {
                    m_pQLastIndex->Remove(m_ClientQFile.Qs);
                }
            }
            m_ReqInfo.Size = 0;
            m_bDequeueTrans = false;
            break;
        case SPA::idEndJob:
#ifndef NDEBUG
            --m_nJobRequest;
            if (!m_bDequeueTrans) {
                std::cout << "Bad m_bDequeueTrans/SPA::idEndJob: " << m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
            }
#endif
            m_bDequeueTrans = false;
            break;
        case SPA::idStartJob:
#ifndef NDEBUG
            ++m_nJobRequest;
            if (m_bDequeueTrans) {
                std::cout << "Bad m_bDequeueTrans/SPA::idStartJob: " << m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
            }
#endif
            //this assert will not work as expected when a client calls the method Cancel
            //assert(!m_bDequeueTrans);
            m_bDequeueTrans = true;
            break;
        case SPA::idCancel:
        {
            AbortBatchingInternal();
        }
            //case SPA::idDoEcho:
        default:
        {
            unsigned short id;
            SPA::CScopeUQueue sb;
            sb->Push(m_qRead.GetBuffer(), m_ReqInfo.Size);
            m_qRead.Pop(m_ReqInfo.Size);
            m_ReqInfo.Size = 0;
            id = m_ReqInfo.RequestId;
            SendReturnDataInternal(id, sb->GetBuffer(), sb->GetSize());
        }
            break;
    }
#ifndef NDEBUG
    if (m_nJobRequest > 1) {
        std::cout << "Bad dequeue job balance = " << __FUNCTION__ << ", balance = " << m_nJobRequest << std::endl;
    }
#endif
    POnBaseRequestCame p = m_ccb.SvsContext.m_OnBaseRequestCame;
    if (p != nullptr) {
        USocket_Server_Handle index = MakeHandlerInternal();
        unsigned short reqId = m_ReqInfo.RequestId;
        CRAutoLock ral(m_mutex, m_bChatting);
        p(index, reqId);
    }
}

bool CServerSession::DoAuthentication(unsigned int ServiceId) {
    POnIsPermitted p = g_pServer->m_pOnIsPermitted;
    if (p != nullptr) {
        USocket_Server_Handle index = MakeHandlerInternal();
        CRAutoLock rsl(m_mutex, m_bChatting);
        return p(index, ServiceId);
    }
    return false;
}

void CServerSession::OnSwitchTo(unsigned int OldServiceId, unsigned int NewServiceId) {
    ServiceId = NewServiceId;
    POnSwitchTo p = m_ccb.SvsContext.m_OnSwitchTo;
    SetServerInfoInternal(&g_pServer->m_ServerInfo);
    std::time_t t;
    std::time(&t);
    m_ServerInfo.SwitchTime = (SPA::UINT64)t;
    if (p != nullptr) {
        USocket_Server_Handle index = MakeHandlerInternal();
        CRAutoLock ral(m_mutex, m_bChatting);
        p(index, OldServiceId, NewServiceId);
    }
    if (NewServiceId != SPA::sidHTTP) {
        unsigned int errCode = g_bRegistered ? ERROR_NO_ERROR : ERROR_EVALUATION;
        if (errCode == ERROR_EVALUATION) {
            if (m_pServiceContext->m_bRegisterred)
                errCode = ERROR_NO_ERROR;
        }
        SPA::CScopeUQueue sb;
        sb << errCode;
        sb << m_ServerInfo;
        SendReturnDataInternal(SPA::idSwitchTo, sb->GetBuffer(), sb->GetSize());
    }
}

void CServerSession::OnRA() {
    if (m_ReqInfo.RequestId == SPA::idSwitchTo) {
        SPA::CScopeUQueue sb(m_ReqInfo.GetOS(), m_ReqInfo.IsBigEndian());
        if (sb->GetMaxSize() < m_ReqInfo.Size)
            sb->ReallocBuffer(m_ReqInfo.Size + 16);
        unsigned int len = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
        sb->SetSize(len);
        sb >> m_ClientInfo;
        if (m_ClientInfo.ServiceId == ServiceId) {
            PostCloseInternal(ERROR_WRONG_SWITCH);
            return;
        }
        m_pServiceContext = g_pServer->SeekServiceContext(m_ClientInfo.ServiceId);
        if (m_pServiceContext == nullptr) {
            PostCloseInternal(ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE);
            return;
        } else {
            m_ccb.SvsContext = m_pServiceContext->GetSvsContext();
            m_pRoutingServiceContext = g_pServer->SeekServiceContext(m_pServiceContext->GetRoutingSvsId());
            if (m_pRoutingServiceContext != nullptr) {
                CRAutoLock ral(m_mutex, m_bChatting);
                m_pRoutingServiceContext->AddRoutee(this);
                m_pServiceContext->NotifyRouteeChanged((unsigned int) (m_pRoutingServiceContext->GetRouteeSize()));
            }
        }

        assert(m_ccb.UserId.size() == 0);

        sb >> m_ccb.UserId;
        sb >> m_ccb.Password;

        unsigned int oldServiceId = ServiceId;
        unsigned int svsId = m_ClientInfo.ServiceId;
        bool ok = true;
        if (svsId == SPA::sidStartup) {
            OnSwitchTo(oldServiceId, svsId);
        } else if ((g_pServer->GetSharedAM() && ServiceId != SPA::sidStartup) || DoAuthentication(svsId)) {
            OnSwitchTo(oldServiceId, svsId);
        } else {
            ok = false;
        }
        m_ccb.Password.resize(m_ccb.Password.size(), ' ');
        m_ccb.Password.clear();
        if (!ok)
            PostCloseInternal(ERROR_AUTHENTICATION_FAILED);
        return;
    }
    if (ServiceId == SPA::sidStartup) {
        PostCloseInternal(ERROR_NOT_SWITCHED_YET);
        return;
    }
    if (m_pHttpContext != nullptr && !m_ReqInfo.IsFake()) {
        UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
        UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
        if (pWebRequestProcessor != nullptr) {
            switch (m_ReqInfo.RequestId) {
                case SPA::idEnter:
                case SPA::idExit:
                case SPA::idSendUserMessage:
                case SPA::idSpeak:
                case SPA::ServerSide::idUserRequest:
                    assert(m_ReqInfo.Size >= (sizeof (SPA::INT64) + sizeof (UHTTP::tagSpError)));
                    m_qRead >> pWebRequestProcessor->m_CurrentIndex;
                    m_qRead >> pWebRequestProcessor->m_CurrentErrCode;
                    assert(pWebRequestProcessor->m_CurrentErrCode <= UHTTP::seAuthenticationFailed);
                    m_ReqInfo.Size -= (sizeof (SPA::INT64) + sizeof (UHTTP::tagSpError));
                    assert(pWebResponseProcessor->m_nReqCount > 0);
                    assert(pWebRequestProcessor->m_CurrentIndex > 0);
                    assert(pWebRequestProcessor->m_CurrentErrCode == UHTTP::seOk);
                    --pWebResponseProcessor->m_nReqCount;
                    if (pWebResponseProcessor->m_nReqCount == 0) {
                        UHTTP::CWebSocketMsg *p = m_pHttpContext->GetWebSocketMsg();
                        if (p)
                            p->ParseStatus = UHTTP::psInitial;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    switch (m_ReqInfo.RequestId) {
        case SPA::idEndMerge:
        case SPA::idStartMerge:
            assert(false);
            break;
        case SPA::idStartJob:
            if (m_pServiceContext && m_pServiceContext->GetRandom()) {
                m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                    if (p) delete p;
                });
            } else {
                OnBaseRequestArrive();
                if ((!m_bFail && !m_bDequeueTrans) || (m_qa.MessageIndex == 1)) {
                    m_pQLastIndex->Set(m_ClientQFile.Qs, m_qa);
                }
                NotifyDequeued();
            }
            break;
        case SPA::idEndJob:
            if (m_pServiceContext && m_pServiceContext->GetRandom()) {
                m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                    if (p) delete p;
                });
            } else {
                OnBaseRequestArrive();
                if (!m_bFail && !m_bDequeueTrans) {
                    m_pQLastIndex->Set(m_ClientQFile.Qs, m_qa);
                }
                NotifyDequeued();
            }
            break;
        case SPA::idDequeueConfirmed:
        case SPA::idDequeueBatchConfirmed:
        case SPA::idStartQueue:
        case SPA::idStopQueue:
        case SPA::idCancel:
        case SPA::idCommitBatching:
        case SPA::idStartBatching:
            OnBaseRequestArrive();
            break;
        case SPA::idTurnOnZipAtSvr:
        case SPA::idRouteeChanged:
        case SPA::idEncrypted:
        case SPA::idBatchZipped:
        case SPA::idGetSockOptAtSvr:
        case SPA::idSetSockOptAtSvr:
        case SPA::idDoEcho:
        case SPA::idPing:
        case SPA::idSetZipLevelAtSvr:
            if (m_pServiceContext && m_pServiceContext->GetRandom()) {
                if (m_ReqInfo.GetQueued()) {
                    m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                        if (p) delete p;
                    });
                } else {
                    m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this), [](CResIndexImpl * p) {
                        if (p) delete p;
                    });
                }
            }
            OnBaseRequestArrive();
            break;
        case SPA::idEnter:
        case SPA::idSpeakEx:
        case SPA::idSendUserMessageEx:
        case SPA::idExit:
        {
            POnChatRequestComing p = m_ccb.SvsContext.m_OnChatRequestComing;
            if (p) {
                USocket_Server_Handle index = MakeHandlerInternal();
                unsigned short reqId = m_ReqInfo.RequestId;
                unsigned int size = m_ReqInfo.Size;
                CRAutoLock rsl(m_mutex, m_bChatting);
                p(index, (SPA::tagChatRequestID)reqId, size);
            }
        }
            OnChatRequestArrive();
            break;
        case SPA::idSendUserMessage:
        case SPA::idSpeak:
        {
            POnChatRequestComing p = m_ccb.SvsContext.m_OnChatRequestComing;
            if (p) {
                USocket_Server_Handle index = MakeHandlerInternal();
                unsigned short reqId = m_ReqInfo.RequestId;
                unsigned int size = m_ReqInfo.Size;
                CRAutoLock rsl(m_mutex, m_bChatting);
                p(index, (SPA::tagChatRequestID)reqId, size);
            }
        }
            OnChatVariantRequestArrive();
            break;
        default:
            OnNonBaseRequestArrive();
            break;
    }
}

void CServerSession::SendChatResult(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
    SPA::CScopeUQueue sb;
    sb << senderAddr << (short) senderClientPort << sendUserId << senderServiceId;
    if (pBuffer == nullptr)
        size = 0;
    if (size > 0)
        sb->Push(pBuffer, size);
    SendReturnData(usReqId, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::SendChatResultInternal(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
    SPA::CScopeUQueue sb;
    sb << senderAddr << (short) senderClientPort << sendUserId << senderServiceId;
    if (pBuffer == nullptr)
        size = 0;
    if (size > 0)
        sb->Push(pBuffer, size);
    SendReturnDataInternal(usReqId, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::BounceBackMessage(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
    unsigned short port;
    std::string addr;
    unsigned int svsId;
    GetPeerName(addr, &port);
    std::wstring userId = m_ccb.UserId;
    svsId = ServiceId;
    SendChatResultInternal(addr.c_str(), port, userId.c_str(), svsId, usReqId, pBuffer, size);
}

SPA::UINT64 CServerSession::GetCallIndex() {
    SPA::UINT64 index = (~0);
    m_mutex.lock();
    if (m_pServiceContext->GetRandom())
        index = m_indexCall;
    m_mutex.unlock();
    return index;
}

void CServerSession::OnChatVariantRequestArrive() {
    unsigned int nCount;
    bool b = IsSameEndian();
    POnChatRequestCame crp = m_ccb.SvsContext.m_OnChatRequestCame;
    SPA::CScopeUQueue sb(m_ReqInfo.GetOS(), m_ReqInfo.IsBigEndian());
    if (m_ReqInfo.Size > sb->GetMaxSize())
        sb->ReallocBuffer(m_ReqInfo.Size + 16);
    if (m_ReqInfo.Size > 0) {
        nCount = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
        sb->SetSize(nCount);
    }
    if (m_pServiceContext && m_pServiceContext->GetRandom()) {
        if (m_ReqInfo.GetQueued()) {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        } else {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        }
    }
    SPA::UVariant vtMsg;
    switch (m_ReqInfo.RequestId) {
        case SPA::idSpeak:
        {
            unsigned short vt = 0;
            unsigned int nCount = 0;
            sb >> vt;
            assert(vt == (VT_UINT | VT_ARRAY));
            sb >> nCount;
            std::vector<unsigned int> vGroup, vFinal;
            if (nCount > 0) {
                unsigned int *p = (unsigned int*) sb->GetBuffer();
                vGroup.assign(p, p + nCount);
                sb->Pop(nCount * sizeof (unsigned int));
            }
            sb >> vtMsg;
            assert(sb->GetSize() == 0);
            if (vGroup.size() > 0) {
                USocket_Server_Handle index = MakeHandlerInternal();
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->Speak(this, vGroup.data(), (unsigned int) vGroup.size(), vtMsg);
                if (crp) {
                    crp(index, SPA::idSpeak);
                }
            }
            sb->SetSize(0);
            sb << vtMsg;
            Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vGroup.data(), (unsigned int) vGroup.size(), vFinal);
            nCount = (unsigned int) vFinal.size();
            sb->Push((const unsigned char*) &nCount, sizeof (nCount));
            if (nCount > 0)
                sb->Push((const unsigned char*) vFinal.data(), nCount * sizeof (unsigned int));

            if (ServiceId == SPA::sidHTTP) {
                UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
                if (m_pHttpContext)
                    pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
                if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
                    if (m_pSsl) {
                        SPA::CScopeUQueue sb;
                        pWebResponseProcessor->BounceBackSpeak(vFinal.data(), nCount, *sb);
                        Write(sb->GetBuffer(), sb->GetSize());
                    } else {
                        pWebResponseProcessor->BounceBackSpeak(vFinal.data(), nCount, m_qWrite);
                        Write(nullptr, 0);
                    }
                } else {
                    Connection::CConnectionContext::Speak(m_ccb.Id, vtMsg, vFinal.data(), nCount);
                }
            } else {
                BounceBackMessage(SPA::idSpeak, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        case SPA::idSendUserMessage:
        {
            std::wstring userId;
            sb >> userId;
            sb >> vtMsg;
            USocket_Server_Handle index = MakeHandlerInternal();
            {
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->SendUserMessage(this, userId.c_str(), vtMsg);
                if (crp) {
                    crp(index, SPA::idSendUserMessage);
                }
            }
            assert(sb->GetSize() == 0);
            sb->SetSize(0);
            sb << vtMsg;
            if (ServiceId == SPA::sidHTTP) {
                UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
                if (m_pHttpContext)
                    pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
                if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
                    if (m_pSsl) {
                        SPA::CScopeUQueue sb;
                        pWebResponseProcessor->BounceBackSendUserMessage(*sb);
                        Write(sb->GetBuffer(), sb->GetSize());
                    } else {
                        pWebResponseProcessor->BounceBackSendUserMessage(m_qWrite);
                        Write(nullptr, 0);
                    }
                } else {
                    Connection::CConnectionContext::SendUserMessage(m_ccb.Id, vtMsg, userId.c_str(), m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
                }
            } else {
                BounceBackMessage(SPA::idSendUserMessage, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        default:
            //not implemented
            assert(false);
            break;
    }
}

void CServerSession::OnChatRequestArrive() {
    unsigned int nCount;
    bool b = IsSameEndian();
    POnChatRequestCame crp = m_ccb.SvsContext.m_OnChatRequestCame;
    SPA::CScopeUQueue sb(m_ReqInfo.GetOS(), m_ReqInfo.IsBigEndian());
    if (m_ReqInfo.Size > sb->GetMaxSize())
        sb->ReallocBuffer(m_ReqInfo.Size + 16);
    if (m_ReqInfo.Size > 0) {
        nCount = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
        sb->SetSize(nCount);
    }
    if (m_pServiceContext && m_pServiceContext->GetRandom()) {
        if (m_ReqInfo.GetQueued()) {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this, m_qa), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        } else {
            m_mapIndex[m_indexCall] = std::shared_ptr<CResIndexImpl>(new CResIndexImpl(m_ReqInfo.RequestId, this), [](CResIndexImpl * p) {
                if (p) delete p;
            });
        }
    }
    switch (m_ReqInfo.RequestId) {
        case SPA::idEnter:
        {
            unsigned int n;
            unsigned short vt;
            std::vector<unsigned int> vExit, vNew;
            sb >> vt;
            assert(vt == (VT_ARRAY | VT_UINT));
            sb >> nCount;
            assert(nCount == sb->GetSize() / sizeof (unsigned int));
            std::vector<unsigned int> vChatGroup, vAll;
            g_pServer->GetJoinedGroupIds(vAll);
            Connection::CConnectionContextBase::ChatGroupsAnd((const unsigned int*) sb->GetBuffer(), nCount, vAll.data(), (unsigned int) vAll.size(), vChatGroup);
            Connection::CConnectionContextBase::ChatGroupsNew(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vChatGroup.data(), (unsigned int) vChatGroup.size(), vNew, vExit);
            //fake request exit
            nCount = (unsigned int) vExit.size();
            if (nCount > 0) {
                CRAutoLock ral(m_mutex, m_bChatting);
                Exit(vExit.data(), nCount);
            }

            //notify other clients
            nCount = (unsigned int) vNew.size();
            if (nCount > 0) {
                USocket_Server_Handle index = MakeHandlerInternal();
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->Enter(this, vNew.data(), nCount);
                if (crp) {
                    crp(index, SPA::idEnter);
                }
            }
            for (n = 0; n < nCount; ++n) {
                m_ccb.ChatGroups.push_back(vNew[n]);
            }
            Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
            if (sp)
                sp->ChatGroups = m_ccb.ChatGroups;
            //bounce back enter
            sb->SetSize(0);
            nCount = (unsigned int) vNew.size();
            sb->Push((const unsigned char*) vNew.data(), nCount * sizeof (unsigned int));
            if (ServiceId == SPA::sidHTTP) {
                UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
                if (m_pHttpContext)
                    pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
                if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
                    if (m_pSsl) {
                        SPA::CScopeUQueue sb;
                        pWebResponseProcessor->BounceBackEnter(vNew.data(), nCount, *sb);
                        Write(sb->GetBuffer(), sb->GetSize());
                    } else {
                        pWebResponseProcessor->BounceBackEnter(vNew.data(), nCount, m_qWrite);
                        Write(nullptr, 0);
                    }
                } else
                    Connection::CConnectionContext::Enter(m_ccb.Id, vNew.data(), nCount);
            } else
                BounceBackMessage(SPA::idEnter, sb->GetBuffer(), sb->GetSize());
        }
            break;
        case SPA::idSpeakEx:
            assert(sb->GetSize() >= sizeof (unsigned int));
        {
            unsigned int size;
            sb >> size;
            assert(sb->GetSize() >= size);
            std::vector<unsigned int> vGroup, vFinal;
            nCount = (sb->GetSize() - size) / sizeof (unsigned int);
            if (nCount > 0) {
                unsigned int *p = (unsigned int*) sb->GetBuffer(size);
                vGroup.assign(p, p + nCount);
                sb->Pop(nCount * sizeof (unsigned int), size);
            }
            if (vGroup.size() > 0) {
                USocket_Server_Handle index = MakeHandlerInternal();
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->SpeakEx(this, sb->GetBuffer(), size, vGroup.data(), (unsigned int) vGroup.size());
                if (crp) {
                    crp(index, SPA::idSpeakEx);
                }
            }
            Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vGroup.data(), (unsigned int) vGroup.size(), vFinal);
            sb->Insert((const unsigned char*) &size, sizeof (size));
            nCount = (unsigned int) vFinal.size();
            if (nCount > 0)
                sb->Push((const unsigned char*) vFinal.data(), nCount * sizeof (unsigned int));
            BounceBackMessage(SPA::idSpeakEx, sb->GetBuffer(), sb->GetSize());
        }
            break;
        case SPA::idSendUserMessageEx:
            assert(sb->GetSize() >= sizeof (unsigned int));
        {
            std::wstring userId;
            sb >> userId;
            USocket_Server_Handle index = MakeHandlerInternal();
            {
                CRAutoLock rsl(m_mutex, m_bChatting);
                g_pServer->SendUserMessage(this, userId.c_str(), sb->GetBuffer(), sb->GetSize());
                if (crp) {
                    crp(index, SPA::idSendUserMessageEx);
                }
            }
            BounceBackMessage(SPA::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
        }
            break;
        case SPA::idExit:
        {
            std::vector<unsigned int> v0;
            if (sb->GetSize() == 0) {
                v0 = m_ccb.ChatGroups;
                m_ccb.ChatGroups.clear();
            } else {
                assert(sb->GetSize() >= sizeof (unsigned int));
                unsigned int *p = (unsigned int*) sb->GetBuffer();
                nCount = sb->GetSize() / sizeof (unsigned int);
                std::vector<unsigned int> v(p, p + nCount), v1;
                Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), v.data(), (unsigned int) v.size(), v0);
                Connection::CConnectionContextBase::ChatGroupsNew(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), v0.data(), (unsigned int) v0.size(), v, v1);
                m_ccb.ChatGroups = v1;
                Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
                if (sp)
                    sp->ChatGroups = v1;
            }
            sb->SetSize(0);
            nCount = (unsigned int) v0.size();
            if (nCount > 0) {
                USocket_Server_Handle index = MakeHandlerInternal();
                {
                    CRAutoLock rsl(m_mutex, m_bChatting);
                    g_pServer->Exit(this, v0.data(), nCount);
                    if (crp) {
                        crp(index, SPA::idExit);
                    }
                }
                sb->Push((const unsigned char*) v0.data(), sizeof (unsigned int) *nCount);
            }
            if (m_cs >= csConnected) {
                if (ServiceId == SPA::sidHTTP) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = nullptr;
                    if (m_pHttpContext)
                        pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
                    if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
                        if (m_pSsl) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->BounceBackExit(v0.data(), nCount, *sb);
                            Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->BounceBackExit(v0.data(), nCount, m_qWrite);
                            Write(nullptr, 0);
                        }
                    } else {
                        Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
                        if (sp) {
                            Connection::CConnectionContext::Exit(m_ccb.Id.c_str(), v0.data(), nCount);
                        }
                    }
                } else
                    BounceBackMessage(SPA::idExit, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        default:
            //not implemented
            assert(false);
            break;
    }
}

bool CServerSession::ProcessWithLock() {
    CAutoLock sl(m_mutex);
    bool b = Process();
    Read();
    Write(nullptr, 0);
    return b;
}

bool CServerSession::GetPeerDequeueFailed() {
    CAutoLock al(m_mutex);
    return m_bFail;
}

bool CServerSession::IsSecure() {
    return bool(m_pSsl);
}

bool CServerSession::PreocessWebRequest(SPA::CUQueue &q) {
    bool Continue;
    UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
    const UHTTP::UHttpRequest &ur = pWebRequestProcessor->GetUHttpRequest();
    UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
    if (ur.ErrCode != UHTTP::seOk) {
        assert(pWebResponseProcessor->m_nReqCount > 0);
        pWebResponseProcessor->ProcessBadRequest(q, ur.ErrCode);
        return false;
    } else if (ur.SpRequest != UHTTP::srSwitchTo) {
        if (!ur.Id || (!m_pHttpContext->IsWebSocket() && !Connection::CConnectionContext::SeekConnectionContext(ur.Id))) {
            assert(pWebResponseProcessor->m_nReqCount > 0);
            pWebResponseProcessor->ProcessBadRequest(q, UHTTP::seAuthenticationFailed);
            return false;
        }
    }

    if (ur.SpRequest != UHTTP::srSwitchTo && !m_pHttpContext->IsWebSocket() && m_ccb.Id.size() == 0) {
        Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(ur.Id);
        if (sp) {
            m_ccb = *sp;
            m_ccb.RecvTime = (GetTimeTick() - g_pServer->m_tStart);
        }
    }

    switch (ur.SpRequest) {
        case UHTTP::srSwitchTo:
        {
            bool ok = true;
            std::string ipAddr;
            unsigned short port;
            m_ccb.UserId = ur.GetUserIdW();
            m_ccb.Password = ur.GetPwdW();
            UHTTP::UHttpRequest &bad = (UHTTP::UHttpRequest&)ur;
            bad.CleanPwd();
            assert(pWebResponseProcessor->m_nReqCount == 1);
            POnHttpAuthentication p = m_ccb.SvsContext.m_OnHttpAuthentication;
            if (p) {
                USocket_Server_Handle index = MakeHandlerInternal();
                {
                    if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
#ifdef WCHAR32
                        SPA::CScopeUQueue qUserId;
                        SPA::CScopeUQueue qPassword;
                        SPA::Utilities::ToUTF16(m_ccb.UserId.c_str(), (unsigned int) m_ccb.UserId.size(), *qUserId);
                        SPA::Utilities::ToUTF16(m_ccb.Password.c_str(), (unsigned int) m_ccb.Password.size(), *qPassword);
                        CRAutoLock ral(m_mutex, m_bChatting);
                        ok = p(index, (const wchar_t*) qUserId->GetBuffer(), (const wchar_t*) qPassword->GetBuffer());
#endif
                    } else {
                        CRAutoLock ral(m_mutex, m_bChatting);
                        ok = p(index, m_ccb.UserId.c_str(), m_ccb.Password.c_str());
                    }
                }
            }
            m_ccb.Password.resize(m_ccb.Password.size(), ' ');
            m_ccb.Password.clear();
            GetPeerName(ipAddr, &port);
            --pWebResponseProcessor->m_nReqCount;
            m_ccb.Id = pWebResponseProcessor->ProcessHttpSwitch(ok, ipAddr.c_str(), port, q);
            if (!m_pHttpContext->IsWebSocket()) {
                Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
                sp->SvsContext = m_ccb.SvsContext;
                sp->m_ulRead += m_ccb.m_ulRead;
                sp->m_ulSent += m_ccb.m_ulSent;
                m_ccb.Pt = sp->Pt;
                m_ccb.IsGet = sp->IsGet;
                m_ccb.IsOpera = sp->IsOpera;
            }
            Continue = (m_nHttpCallCount > 0);
        }
            break;
        case UHTTP::srPing:
            assert(pWebResponseProcessor->m_nReqCount == 1);
            --pWebResponseProcessor->m_nReqCount;
            pWebResponseProcessor->ProcessPing(q);
        {
            POnBaseRequestCame p = m_ccb.SvsContext.m_OnBaseRequestCame;
            if (p) {
                USocket_Server_Handle index = MakeHandlerInternal();
                CRAutoLock ral(m_mutex, m_bChatting);
                p(index, SPA::idPing);
            }
        }

            Continue = false;
            break;
        case UHTTP::srClose:
            assert(pWebResponseProcessor->m_nReqCount == 1);
            --pWebResponseProcessor->m_nReqCount;
            pWebResponseProcessor->ProcessClose(q);
            Continue = false;
            break;
        default:
        {
            const SPA::CUQueue &req = pWebRequestProcessor->GetBinaryRequests();
            m_qRead.Insert(req.GetBuffer(), req.GetSize(), m_ReqInfo.Size);
            m_nHttpCallCount += ur.GetReqCount();
            Continue = true;
        }
            break;
    }
    return Continue;
}

bool CServerSession::ProcessWebSocketRequest() {
    const unsigned char *end = m_pHttpContext->ParseWSMsg(m_qRead.GetBuffer(), m_qRead.GetSize());
    unsigned int len = (unsigned int) (end - m_qRead.GetBuffer());
    UHTTP::CWebSocketMsg *pWebSocketMsg = m_pHttpContext->GetWebSocketMsg();
    if (!pWebSocketMsg)
        return false;
    if ((m_pHttpContext->GetContentLength() + 64) > m_qRead.GetMaxSize())
        m_qRead.ReallocBuffer((unsigned int) m_pHttpContext->GetContentLength() + 64);
    if (pWebSocketMsg->ParseStatus == UHTTP::psComplete && len > 0 && pWebSocketMsg->IsFin()) {
        m_qRead.Pop(len);
        UHTTP::tagWSOpCode wsOpCode = pWebSocketMsg->GetOpCode();
        switch (wsOpCode) {
            case UHTTP::ocTextMsg:
            {
                const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
                SPA::CScopeUQueue su;
                bool ok = PreocessWebRequest(*su);
                pWebSocketMsg->Content.SetSize(0);
                Write(su->GetBuffer(), su->GetSize());
                UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
                if (pWebResponseProcessor->m_nReqCount == 0)
                    pWebSocketMsg->ParseStatus = UHTTP::psInitial;
                return ok;
            }
                break;
            case UHTTP::ocMsgContinuation:
                assert(false);
                return false;
                break;
            case UHTTP::ocConnectionClose:
                m_qRead.SetSize(0);
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->PrepareWSResponseMessage(nullptr, 0, wsOpCode, *sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->PrepareWSResponseMessage(nullptr, 0, wsOpCode, m_qWrite);
                }
                return true;
                break;
            case UHTTP::ocPing:
                assert(false);
                return false;
                break;
            case UHTTP::ocPong:
                return false;
                break;
            case UHTTP::ocBinaryMsg:
                assert(false);
                return false;
            default:
                return false;
                break;
        }
    } else {
        return false;
    }
    return true;
}

bool CServerSession::ProcessAjaxRequest() {
    SPA::CScopeUQueue su;
    m_pHttpContext->SetContent(m_qRead.GetBuffer(), (unsigned int) m_pHttpContext->GetContentLength());
    m_qRead.Pop((unsigned int) m_pHttpContext->GetContentLength());
    const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
    bool b = PreocessWebRequest(*su);
    switch (ur.SpRequest) {
        case UHTTP::srClose:
        {
            SPA::CStreamHeader reqInfo;
            reqInfo.MakeFake();
            reqInfo.RequestId = SPA::idExit;
            m_qRead << reqInfo;
            ++m_nHttpCallCount;
            b = true;
        }
            assert(ur.GetReqCount() == 1);
            Write(su->GetBuffer(), su->GetSize());
            break;
        case UHTTP::srSwitchTo:
        case UHTTP::srPing:
            assert(ur.GetReqCount() == 1);
            Write(su->GetBuffer(), su->GetSize());
            break;
        default:
            if (!b) {
                Write(su->GetBuffer(), su->GetSize());
            }
            break;
    }
    return b;
}

bool CServerSession::ProcessJavaScriptRequest() {
    SPA::CScopeUQueue su;
    const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
    bool b = PreocessWebRequest(*su);
    switch (ur.SpRequest) {
        case UHTTP::srClose:
        {
            SPA::CStreamHeader reqInfo;
            reqInfo.RequestId = SPA::idExit;
            reqInfo.MakeFake();
            m_qRead << reqInfo;
            ++m_nHttpCallCount;
            b = true;
        }
            assert(ur.GetReqCount() == 1);
            Write(su->GetBuffer(), su->GetSize());
            break;
        case UHTTP::srSwitchTo:
        case UHTTP::srPing:
            assert(ur.GetReqCount() == 1);
            Write(su->GetBuffer(), su->GetSize());
            break;
        default:
            if (m_pHttpContext->IsSpRequest() && !m_pHttpContext->GetResponseProgress().Chunked && ur.GetReqCount() > 1) {
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->StartChunkedResponse(*sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->StartChunkedResponse(m_qWrite);
                    Write(nullptr, 0);
                }
            }
            break;
    }
    return b;
}

bool CServerSession::ProcessHttpRequest() {
    if (m_pHttpContext == nullptr) {
        m_pHttpContext = UHTTP::CHttpContext::Lock();
    }
    if (m_pHttpContext->IsWebSocket())
        return ProcessWebSocketRequest();
    UHTTP::tagParseStatus ps = m_pHttpContext->GetPS();
    do {
        if (ps >= UHTTP::psHeaders)
            break;
        const char *start = (const char*) m_qRead.GetBuffer();
        const char *end = m_pHttpContext->ParseHeaders(start);
        ps = m_pHttpContext->GetPS();
        if (ps >= UHTTP::psHeaders) {
            unsigned int parsed = (unsigned int) (end - start);
            m_qRead.Pop(parsed);
            m_qRead.SetHeadPosition();
            m_qRead.SetNull();
            if (ServiceId == SPA::sidStartup) {
                SPA::CScopeUQueue sb;
                SPA::CSwitchInfo SwitchInfo;
                ::memset(&SwitchInfo, 0, sizeof (SwitchInfo));
                SwitchInfo.ServiceId = SPA::sidHTTP;
                SwitchInfo.MajorVersion = 2;
                sb << SwitchInfo;
                sb << m_ccb.UserId;
                sb << m_ccb.Password;
                {
                    CRAutoLock ral(m_mutex, m_bChatting);
                    FakeAClientRequest(SPA::idSwitchTo, sb->GetBuffer(), sb->GetSize());
                }

                if (m_pHttpContext->IsWebSocket()) {
                    if (m_pSsl) {
                        SPA::CScopeUQueue sb;
                        m_pHttpContext->PrepareResponse(nullptr, 0, *sb, UHTTP::hrfpAll);
                        Write(sb->GetBuffer(), sb->GetSize());
                    } else {
                        m_pHttpContext->PrepareResponse(nullptr, 0, m_qWrite, UHTTP::hrfpAll);
                        Write(nullptr, 0);
                    }
                } else {
                    m_pHttpContext->SetPostPS(UHTTP::psHeaders);
                    g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
                }
                return true;
            }
        } else if ((ps == UHTTP::psFailed && ServiceId == SPA::sidHTTP) || (ps >= UHTTP::psMethod && m_pHttpContext->GetResponseCode() >= 400)) {
            PostCloseInternal(0);
            return false;
        } else
            return false; //if headers are not available, stop here!
    } while (false);

    SPA::UINT64 content_length = m_pHttpContext->GetContentLength();
    if (content_length != UHTTP::CONTENT_LEN_UNKNOWN) {
        unsigned int ms = m_qRead.GetMaxSize();
        if (content_length > ms && MULTIPLE_CONTEXT_LENGTH > ms)
            m_qRead.ReallocBuffer(MULTIPLE_CONTEXT_LENGTH + 128);
        if (m_qRead.GetSize() < content_length && m_qRead.GetSize() < MULTIPLE_CONTEXT_LENGTH) {
            return false;
        }
    } else {

    }

    SPA::ServerSide::tagHttpMethod hm = m_pHttpContext->GetMethod();
    switch (hm) {
        case SPA::ServerSide::hmGet:
        {
            assert(m_qRead.GetSize() == 0);
            m_pHttpContext->SetPostPS();
            UHTTP::tagHttpRequestType request_type = m_pHttpContext->GetHttpRequestType();
            if (request_type == UHTTP::hrtDownloadAdapter) {//download spadapter.js
                unsigned int len;
                assert(m_qRead.GetSize() == 0);
                UHTTP::CUJsLoader loader(m_pHttpContext->GetUserAgent(),
                        m_pHttpContext->GetParams().Start,
                        m_pHttpContext->GetHost(),
                        m_pHttpContext->IsCrossDomain(),
                        IsSecure());
                const char *code = loader.GetSPACode(len);
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->PrepareResponse((const unsigned char*) code, len, *sb, UHTTP::hrfpAll);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->PrepareResponse((const unsigned char*) code, len, m_qWrite, UHTTP::hrfpAll);
                    Write(nullptr, 0);
                }
                return false;
            } else if (request_type == UHTTP::hrtDownloadLoader) { //download uloader.js
                assert(m_qRead.GetSize() == 0);
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->StartDownloadFile(m_pHttpContext->GetUrl().Start + 1, *sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->StartDownloadFile(m_pHttpContext->GetUrl().Start + 1, m_qWrite);
                    Write(nullptr, 0);
                }
                return false;
            } else if (request_type == UHTTP::hrtJsRequest) {
                assert(m_qRead.GetSize() == 0);
                return ProcessJavaScriptRequest();
            } else { //Non-SocketPro JavaScript request
                assert(m_qRead.GetSize() == 0);
                SPA::CStreamHeader reqInfo;
                reqInfo.RequestId = SPA::ServerSide::idGet;
                m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
                ++m_nHttpCallCount;
                return true;
            }
        }
            break;
        case SPA::ServerSide::hmPost:
            if (m_pHttpContext->GetCM() != SPA::ServerSide::cmUnknown || m_pHttpContext->GetTE() != SPA::ServerSide::teUnknown) {
                m_pHttpContext->SetResponseCode(501); //Not Implemented
                m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->PrepareResponse(nullptr, 0, *sb, UHTTP::hrfpAll);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->PrepareResponse(nullptr, 0, m_qWrite, UHTTP::hrfpAll);
                    Write(nullptr, 0);
                }
                return false;
            } else {
                if (m_pHttpContext->GetContentLength() <= m_qRead.GetSize()) {
                    m_pHttpContext->SetPostPS();
                    if (m_pHttpContext->GetHttpRequestType() == UHTTP::hrtJsRequest) {
                        bool ok = ProcessAjaxRequest();
                        if (!ok)
                            return false;
                    } else //Non-SocketPro JavaScript request
                    {
                        SPA::CStreamHeader reqInfo;
                        reqInfo.RequestId = SPA::ServerSide::idPost;
                        reqInfo.Size = (unsigned int) m_pHttpContext->GetContentLength();
                        m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
                        ++m_nHttpCallCount;
                    }
                } else {
                    return false;
                }
            }
            break;
        case SPA::ServerSide::hmPut:
        case SPA::ServerSide::hmOptions:
            if (m_pHttpContext->GetContentLength() != UHTTP::CONTENT_LEN_UNKNOWN) {
                SPA::CStreamHeader reqInfo;
                reqInfo.Size = (unsigned int) m_pHttpContext->GetContentLength();
                switch (hm) {
                    case SPA::ServerSide::hmPut:
                        reqInfo.RequestId = SPA::ServerSide::idPut;
                        break;
                    case SPA::ServerSide::hmOptions:
                        reqInfo.RequestId = SPA::ServerSide::idOptions;
                        break;
                }
                m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
                ++m_nHttpCallCount;
            } else {
                m_pHttpContext->SetResponseCode(501); //Not Implemented
                m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->PrepareResponse(nullptr, 0, *sb, UHTTP::hrfpAll);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->PrepareResponse(nullptr, 0, m_qWrite, UHTTP::hrfpAll);
                    Write(nullptr, 0);
                }
                return false;
            }
            break;
        case SPA::ServerSide::hmHead:
        case SPA::ServerSide::hmDelete:
        case SPA::ServerSide::hmTrace:
            if (m_pHttpContext->GetContentLength() == UHTTP::CONTENT_LEN_UNKNOWN || m_pHttpContext->GetContentLength() == 0) {
                SPA::CStreamHeader reqInfo;
                switch (hm) {
                    case SPA::ServerSide::hmHead:
                        reqInfo.RequestId = SPA::ServerSide::idHead;
                        break;
                    case SPA::ServerSide::hmDelete:
                        reqInfo.RequestId = SPA::ServerSide::idDelete;
                        break;
                    case SPA::ServerSide::hmTrace:
                        reqInfo.RequestId = SPA::ServerSide::idTrace;
                        break;
                }
                m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
                ++m_nHttpCallCount;
                break; //break only for the supported HTTP request
            }
        default:
            m_pHttpContext->SetResponseCode(501); //Not Implemented
            m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                m_pHttpContext->PrepareResponse(nullptr, 0, *sb, UHTTP::hrfpAll);
                Write(sb->GetBuffer(), sb->GetSize());
            } else {
                m_pHttpContext->PrepareResponse(nullptr, 0, m_qWrite, UHTTP::hrfpAll);
                Write(nullptr, 0);
            }
            return false;
            break;
    }
    return true;
}

bool CServerSession::Decompress() {
    unsigned int res;
    bool reset = false;
    bool defaultZipped = m_ReqInfo.IsDefaultZipped(false/*IsOld()*/);
    bool fastZip = m_ReqInfo.IsFastZipped(false/*IsOld()*/);

    if (m_ReqInfo.RequestId == SPA::idBatchZipped) {
        if (defaultZipped || fastZip) {
            SPA::CScopeUQueue sb;
            unsigned short ratio = m_ReqInfo.GetZipRatio();
            assert(ratio > 0);
            res = DecompressRequestTo(ratio, defaultZipped ? SPA::zlDefault : SPA::zlBestSpeed, m_qRead.GetBuffer(), m_ReqInfo.Size, *sb);
            assert(res > 0);
            m_qRead.Replace(0, m_ReqInfo.Size, sb->GetBuffer(), res);
        }
        m_ReqInfo.Size = 0;
        m_ReqInfo.RequestId = 0;
        reset = true;
    } else if (defaultZipped || fastZip) {
        SPA::CScopeUQueue sb;
        unsigned short ratio = m_ReqInfo.GetZipRatio();
        res = DecompressRequestTo(ratio, defaultZipped ? SPA::zlDefault : SPA::zlBestSpeed, m_qRead.GetBuffer(), m_ReqInfo.Size, *sb);
        assert(res > 0);
        m_qRead.Replace(0, m_ReqInfo.Size, sb->GetBuffer(), res);
        m_ReqInfo.Size = res;
    }
    return reset;
}

void CServerSession::NotifyDequeued() {
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idDequeueConfirmed;
    MQ_FILE::CDequeueConfirmInfo dci(m_qa, m_bFail, m_ReqInfo.RequestId);
    sh.Size = sizeof (dci);
    Write(sh, (const unsigned char*) &dci, sizeof (dci));
}

CServerSession::mutex CServerSession::m_mutexRouteRequestId;
SPA::CUQueue CServerSession::m_qRouteRequestId;

void CServerSession::NotifyFailRoutes(SPA::UINT64 receiver, CServiceContext *pServiceContext) {
    int total = 0;
    int count = 0;
    RouteMap *rms = nullptr;
    SPA::CScopeUQueue sb, sbOk, sbFail;
    SPA::CUQueue &q = *sb;
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idDequeueConfirmed;
    sh.Size = sizeof (MQ_FILE::CDequeueConfirmInfo);
    {
        CAutoLock al(m_mutexRouteRequestId);
        assert((m_qRouteRequestId.GetSize() % sizeof (RouteMap)) == 0);
        count = (int) (m_qRouteRequestId.GetSize() / sizeof (RouteMap));
        total = count;
        RouteMap *rms = (RouteMap *) m_qRouteRequestId.GetBuffer();
        for (int n = 0; n < count; ++n) {
            RouteMap &rm = rms[n];
            if (rm.Receiver == receiver)
                sbFail << rm;
            else
                sbOk << rm;
        }
        m_qRouteRequestId.SetSize(0);
        m_qRouteRequestId.Push(sbOk->GetBuffer(), sbOk->GetSize());
    }
    count = (int) (sbFail->GetSize() / sizeof (RouteMap));
    rms = (RouteMap *) sbFail->GetBuffer();
    for (int n = 0; n < count; ++n) {
        unsigned int index;
        RouteMap &rm = rms[n];
        CServerSession *pSession = GetSvrSession(rm.Sender, index);
        bool bValid = pServiceContext->IsRoutee(pSession);
        if (bValid && pSession && index == pSession->GetConnIndex()) {
            bool queued = (rm.Qa.MessageIndex != INVALID_NUMBER || rm.Qa.MessagePos != INVALID_NUMBER);
            if (queued) {
                assert(rm.Qa.MessageIndex != INVALID_NUMBER);
                assert(rm.Qa.MessagePos != INVALID_NUMBER);
                assert(rm.Qa.MessageIndex != 0);
                MQ_FILE::CDequeueConfirmInfo dci(rm.Qa, true, rm.RequestId);
                q.SetSize(0);
                q << sh << dci;
                pSession->m_mutex.lock();
                pSession->Write(q.GetBuffer(), q.GetSize());
                pSession->m_mutex.unlock();
#ifndef NDEBUG
                std::cout << "+++ Failed index=" << rm.Qa.MessageIndex << ", pos=" << rm.Qa.MessagePos << std::endl;
#endif
            } else {
#ifndef NDEBUG
                std::cout << "--- Failed index=" << rm.Qa.MessageIndex << ", pos=" << rm.Qa.MessagePos << std::endl;
#endif
                pSession->SendExceptionResult(L"Routee is disconnected", "SocketPro server", rm.RequestId, MB_ROUTEE_DISCONNECTED);
            }
        } else {
        }
    }
#ifndef NDEBUG
    m_mutexRouteRequestId.lock();
    std::cout << "Initial count = " << total << ", remain count=" << m_qRouteRequestId.GetSize() / sizeof (RouteMap) << std::endl;
    m_mutexRouteRequestId.unlock();
#endif
}

bool CServerSession::RemoveARouteMap(RouteMap &rm) {
    CAutoLock al(m_mutexRouteRequestId);
    assert((m_qRouteRequestId.GetSize() % sizeof (RouteMap)) == 0);
    unsigned int n, count = m_qRouteRequestId.GetSize() / sizeof (RouteMap);
    RouteMap *rms = (RouteMap *) m_qRouteRequestId.GetBuffer();
    for (n = 0; n < count; ++n) {
        if (rm == rms[n]) {
            rm.Qa = rms[n].Qa; //must be called first here
            m_qRouteRequestId.Pop((unsigned int) (sizeof (RouteMap)), sizeof (RouteMap) * n);
            return true;
        }
    }
    return false;
}

bool CServerSession::Route() {
    unsigned int index;
    unsigned int routeeSize = 0;
    if (m_ReqInfo.RequestId == SPA::idRoutingData) {
        unsigned short reqId;
        assert(!m_ReqInfo.GetQueued());
        SPA::UINT64 handle = *((SPA::UINT64*)m_qRead.GetBuffer(sizeof (reqId)));
        CServerSession *sender = GetSvrSession(handle, index);
        bool valid;
        {
            CRAutoLock ral(m_mutex, m_bChatting);
            valid = m_pServiceContext->IsRoutee(sender);
        }
        if (!valid) {
            m_qRead.Pop(m_ReqInfo.Size);
            m_ReqInfo.RequestId = 0;
            m_ReqInfo.Size = 0;
            SPA::CStreamHeader sh;
            sh.RequestId = SPA::idRoutePeerUnavailable;
            sh.Size = sizeof (handle);
            Write(sh, (const unsigned char*) &handle, sizeof (handle));
            return true;
        }

        if (sender && index == sender->GetConnIndex()) {
            routeeSize = sender->GetWritingBufferSize();
            if (routeeSize >= 5 * IO_BUFFER_SIZE) {
                sender->m_senderHandle = MakeHandlerInternal();
                return false;
            }

            m_senderHandle = 0;
            m_qRead >> reqId >> handle;

            m_ReqInfo.Size -= (sizeof (reqId) + sizeof (handle));
            m_ReqInfo.RequestId = reqId;

            SPA::CScopeUQueue sb;
            SPA::CUQueue &q = *sb;
            q << m_ReqInfo;
            assert(!m_ReqInfo.GetQueued());
            q.Push(m_qRead.GetBuffer(), m_ReqInfo.Size);
            m_qRead.Pop(m_ReqInfo.Size);
            m_ReqInfo.RequestId = 0;
            m_ReqInfo.Size = 0;
            {
                RouteMap rm;
                rm.Receiver = MakeHandlerInternal();
                rm.Sender = handle;
                rm.RequestId = reqId;
                if (RemoveARouteMap(rm)) {
                    SPA::CStreamHeader sh;
                    sh.RequestId = SPA::idDequeueConfirmed;
                    sh.Size = sizeof (MQ_FILE::CDequeueConfirmInfo);
                    MQ_FILE::CDequeueConfirmInfo dci(rm.Qa, false, rm.RequestId);
                    q << sh << dci;
                    CAutoLock al(sender->m_mutex);
                    sender->Write(q.GetBuffer(), q.GetSize());
                } else {
#ifndef NDEBUG
                    std::cout << "**** Map failed ****" << std::endl;
#endif
                }
            }
            return true;
        }
        assert(false);
        return false;
    } else if (!m_receiverHandle) {
        CRAutoLock rsl(m_mutex, m_bChatting);
        m_receiverHandle = m_pServiceContext->GetBestRoutee(routeeSize);
    }
    CServerSession *receiver = nullptr;
    if (m_receiverHandle) {
        receiver = GetSvrSession(m_receiverHandle, index);
        if (receiver && index != receiver->GetConnIndex()) {
            receiver = nullptr;
        }
    }

    if (!receiver) {
        m_bFail = true;
        NotifyDequeued();
        m_qRead.Pop(m_ReqInfo.Size);
        m_ReqInfo.Size = 0;

        if (m_ReqInfo.RequestId == SPA::idEndJob) {
#ifndef NDEBUG
            if (!m_bDequeueTrans) {
                std::cout << "Bad m_bDequeueTrans/SPA::idEndJob: " << m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
            }
#endif
            m_bDequeueTrans = false;
        }
        m_ReqInfo.RequestId = 0;
        if (!m_bDequeueTrans)
            m_receiverHandle = 0;

        //no routee available or closed
        return false;
    }

    if (!routeeSize) {
        CRAutoLock rsl(m_mutex, m_bChatting);
        routeeSize = receiver->GetWritingBufferSize();
    }

    USocket_Server_Handle h = MakeHandlerInternal();
    if (routeeSize < 5 * IO_BUFFER_SIZE) {
        SPA::CStreamHeader sh;
        SPA::CScopeUQueue sb;
        SPA::CUQueue &q = *sb;
        assert(m_receiverHandle);
        m_senderHandle = 0;
        {
            RouteMap rm;
            rm.Sender = h;
            if (m_ReqInfo.GetQueued()) {
                const MQ_FILE::QAttr *qa = (const MQ_FILE::QAttr*)m_qRead.GetBuffer();
                rm.Qa = *qa;
            }
            rm.RequestId = m_ReqInfo.RequestId;
            rm.Receiver = m_receiverHandle;
            CAutoLock al(m_mutexRouteRequestId); //dead lock within multi-main-threads environment
            if (m_qRouteRequestId.GetHeadPosition() >= IO_BUFFER_SIZE && m_qRouteRequestId.GetTailSize() <= sizeof (rm))
                m_qRouteRequestId.SetHeadPosition();
            m_qRouteRequestId << rm;
        }

        sh.RequestId = SPA::idRoutingData;
        sh.Size = m_ReqInfo.Size + sizeof (m_ReqInfo) + sizeof (h);

        q << sh << h << m_ReqInfo;
        if (m_ReqInfo.Size < IO_BUFFER_SIZE) {
            q.Push(m_qRead.GetBuffer(), m_ReqInfo.Size);
            receiver->Write(q.GetBuffer(), q.GetSize());
        } else {
            receiver->Write(q.GetBuffer(), q.GetSize());
            receiver->Write(m_qRead.GetBuffer(), m_ReqInfo.Size);
        }
        receiver->IncreaseRoutingRequestCount();
        switch (m_ReqInfo.RequestId) {
            case SPA::idStartJob:
#ifndef NDEBUG
                if (m_bDequeueTrans) {
                    std::cout << "Bad m_bDequeueTrans/SPA::idStartJob: " << m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
                }
#endif
                m_bDequeueTrans = true;
                break;
            case SPA::idEndJob:
#ifndef NDEBUG
                if (!m_bDequeueTrans) {
                    std::cout << "Bad m_bDequeueTrans/SPA::idEndJob: " << m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
                }
#endif
                m_bDequeueTrans = false;
                break;
            case SPA::idStartBatching:
                assert(!m_bRouteBatching);
                m_bRouteBatching = true;
                break;
            case SPA::idCommitBatching:
                assert(m_bRouteBatching);
                m_bRouteBatching = false;
                break;
            default:
                break;
        }
        if (!m_bDequeueTrans && !m_bRouteBatching)
            m_receiverHandle = 0;
        m_qRead.Pop(m_ReqInfo.Size);
        m_ReqInfo.RequestId = 0;
        m_ReqInfo.Size = 0;
        return true;
    } else
        m_senderHandle = h;

    //routee's writing buffer has too many data to be sent
    return false;
}

bool CServerSession::Process() {
    if (m_bChatting || m_pUThread) {
        return false;
    }
    bool bOk = false;
    if (ServiceId == SPA::sidStartup) {
        m_qRead.SetNull();
        UHTTP::CHttpContext *p = UHTTP::CHttpContext::Lock();
        const char *end = p->ParseHeaders((const char*) m_qRead.GetBuffer(), true);
        UHTTP::tagParseStatus ps = p->GetPS();
        UHTTP::CHttpContext::Unlock(p);
        if (ps >= UHTTP::psMethod) {
            if (!ProcessHttpRequest()) {
                return false;
            }
        }
    } else if (ServiceId == SPA::sidHTTP) {
        if (m_nHttpCallCount == 0) {
            if (!ProcessHttpRequest()) {
                return false;
            }
        }
    }

    //binary requests processing ......

    if (m_qRead.GetSize() < sizeof (m_ReqInfo)) {
        return bOk;
    }
    while (true) {
        if (ServiceId == SPA::sidHTTP && m_nHttpCallCount == 0) {
            if (m_qWrite.GetSize() < IO_BUFFER_SIZE && m_qRead.GetSize() >= sizeof (m_ReqInfo)) //this check reduces CPU and avoid extra buffer for m_qWrite
            {
                g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
            }
            return bOk;
        }

        if (m_ReqInfo.RequestId == 0 && m_qRead.GetSize() >= sizeof (m_ReqInfo)) {
            m_qRead >> m_ReqInfo;
            if (ServiceId != SPA::sidHTTP && m_ReqInfo.RequestId != SPA::idSwitchTo) {
                m_qRead.SetEndian(m_ReqInfo.IsBigEndian());
                m_qRead.SetOS(m_ReqInfo.GetOS());
            }
            if (m_ReqInfo.Size > m_qRead.GetMaxSize()) {
                m_qRead.ReallocBuffer(m_ReqInfo.Size + m_qRead.GetBlockSize() / 2);
            }
        }

        if (m_ReqInfo.RequestId == 0 || m_qRead.GetSize() < m_ReqInfo.Size) {
            return bOk;
        }

        if (m_ReqInfo.RequestId > SPA::idSwitchTo && m_pServiceContext && !m_pServiceContext->m_bRegisterred) {
            CServer::m_reg.AddCall(ServiceId, m_ReqInfo.GetOS(), m_ReqInfo.GetQueued(), m_ReqInfo.IsBigEndian());
        }

        m_bLastDequeue = false;
        if (m_pRoutingServiceContext != nullptr && IsRoutable(m_ReqInfo.RequestId) && (!m_pServiceContext->IsAlpah(m_ReqInfo.RequestId))) {
            if (Route()) {
                continue;
            } else {
                m_bChatting = true;
                m_mutex.unlock();
                if (m_pServiceContext->GetRouteeSize() == 0) {
                    m_mutex.lock();
                    m_bChatting = false;
                    SendExceptionResultInternal(L"The peer routee is not available", "Inside request processing loop", m_ReqInfo.RequestId, MB_QUEUE_FILE_NOT_AVAILABLE);
                    if (m_ReqInfo.GetQueued()) {
                        m_qRead >> m_qa;
                        m_ReqInfo.Size -= sizeof (m_qa);
                        m_bFail = true;
                        NotifyDequeued();
                    }
                    m_qRead.Pop(m_ReqInfo.Size);
                    m_ReqInfo.Size = 0;
                    m_ReqInfo.RequestId = 0;
                    continue;
                } else {
                    m_mutex.lock();
                    m_bChatting = false;
                    //Routee's writing buffer has too many data to be sent. Therefore, stop here
                    return false;
                }
            }
        }
        if (!m_bDequeueTrans) {
            m_bFail = false;
        }
        if (m_ReqInfo.GetQueued()) {
            m_qRead >> m_qa;
            m_bLastDequeue = ((m_qa.MessagePos & MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END) == MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END);
            if (m_bLastDequeue) {
                m_qa.MessagePos -= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
            }
            m_ReqInfo.Size -= sizeof (m_qa);
            MQ_FILE::QAttr seek = m_pQLastIndex->Seek(m_ClientQFile.Qs);

            //check if previous message index is already dequeued because of sudden socket disconnection, exception and others
            if (seek.MessageIndex != INVALID_NUMBER &&
                    seek.MessageIndex >= m_qa.MessageIndex &&
                    seek.MessagePos != INVALID_NUMBER &&
                    seek.MessagePos >= m_qa.MessagePos &&
                    m_qa.MessageIndex > 1) {
#ifndef NDEBUG
                std::cout << "++++ ReqId = " << m_ReqInfo.RequestId << ", len = " << m_ReqInfo.Size << ", msg index = " << m_qa.MessageIndex << ", pos = " << m_qa.MessagePos;
                std::cout << ", seek msg index = " << seek.MessageIndex << ", seek pos = " << seek.MessagePos << std::endl;
                if (m_ReqInfo.RequestId == SPA::idStartJob) {
                    ++m_nJobRequest;
                } else if (m_ReqInfo.RequestId == SPA::idEndJob) {
                    --m_nJobRequest;
                }
#endif
                m_bFail = false;
                SendExceptionResultInternal(L"The queued request already dequeued", "Inside request processing loop", m_ReqInfo.RequestId, MB_ALREADY_DEQUEUED);
                NotifyDequeued();
                m_qRead.Pop(m_ReqInfo.Size);
                m_ReqInfo.Size = 0;
                m_ReqInfo.RequestId = 0;
                continue;
            }
        }

        if (Decompress()) {
            continue;
        }
        m_indexCall = ++g_pServer->m_nRequestCount;

        if (m_ReqInfo.RequestId == SPA::idSwitchTo && m_ReqInfo.Size > ((MAX_USERID_CHARS + MAX_PASSWORD_CHARS) * sizeof (SPA::UTF16) + 2 * sizeof (unsigned int) + sizeof (m_ClientInfo))) //2*(510) + 2*sizeof(unsigned short) + sizeof(m_ClientInfo)
        {
            PostCloseInternal(ERROR_BAD_REQUEST);
            bOk = false;
            break;
        }

        if (ServiceId == SPA::sidStartup && (m_ReqInfo.Size > 1460 || m_ReqInfo.RequestId != SPA::idSwitchTo || m_ReqInfo.Size < (2 * sizeof (unsigned int) + sizeof (m_ClientInfo)))) {
            //std::cout << "ServiceId == SPA::sidStartup && m_ReqInfo.Size = " << m_ReqInfo.Size << ", id = " << m_ReqInfo.RequestId << std::endl;
            PostCloseInternal(ERROR_BAD_REQUEST);
            bOk = false;
            break;
        }

        //preprocess unziping, debatching or both

        try{
            OnRA();
        }

        catch(boost::system::system_error & err) {
            SendExceptionResultInternal(err.what(), "System runtime error inside request processing loop", m_ReqInfo.RequestId, MB_STL_EXCEPTION);
        }

        catch(SPA::CUException & err) {
            SendExceptionResultInternal(err.what(), err.GetStack().c_str(), m_ReqInfo.RequestId, err.GetErrCode());
        }

        catch(std::exception & err) {
            SendExceptionResultInternal(err.what(), "Inside request processing loop", m_ReqInfo.RequestId, MB_STL_EXCEPTION);
        }

        catch(...) {
            SendExceptionResultInternal(L"Unknown exception caught", "Inside request processing loop", m_ReqInfo.RequestId, MB_UNKNOWN_EXCEPTION);
        }

        if (ServiceId == SPA::sidHTTP) {
            --m_nHttpCallCount;
        }

        if (m_pUThread || m_cs < csConnected) {
            bOk = false;
            break;
        }

        if (!g_pServer->m_bStopped && !m_bCanceled && m_ReqInfo.GetQueued()) {
            if (m_pRoutingServiceContext) {
                if (!m_bFail && !m_bDequeueTrans && m_ReqInfo.RequestId > SPA::idReservedTwo) {
                    m_pQLastIndex->Set(m_ClientQFile.Qs, m_qa);
                }
            }
            if (m_ReqInfo.RequestId == SPA::idEndJob || m_qWrite.GetSize() >= 1460) {
                Write(nullptr, 0);
            }
        }
        bOk = true;
        m_ReqInfo.RequestId = 0;
        if (m_ReqInfo.Size > 0) {
            m_qRead.Pop(m_ReqInfo.Size);
            m_ReqInfo.Size = 0;
        }
    }
    return bOk;
}

void CServerSession::OnReadCompleted(const CErrorCode& Error, size_t nLen) {
    if (!Error) {
        unsigned int len = (unsigned int) nLen;
        CAutoLock sl(m_mutex);
        m_ccb.RecvTime = (GetTimeTick() - g_pServer->m_tStart);
        if (m_qRead.GetTailSize() < nLen && m_qRead.GetHeadPosition() >= nLen) {
            m_qRead.SetHeadPosition();
        }
        if (m_pSsl) {
            if (m_pSsl->Done()) {
                m_ccb.m_ulRead += m_pSsl->Decrypt(m_ReadBuffer, len, m_qRead);
            } else {
                m_bRBLocked = false;
                bool ok = m_pSsl->DoHandshake(m_ReadBuffer, len, m_qWrite);
                if (ok) {
                    Write(nullptr, 0);
                    if (m_pSsl->Done()) {
                        CErrorCode ec;
                        OnSslHandShake(ec);
                        unsigned int res = m_pSsl->Decrypt(nullptr, 0, m_qRead);
                    }
                    if (!m_qRead.GetSize()) {
                        Read();
                        return;
                    }
                } else {
                    m_ec.assign(SSL_ERROR_SSL, boost::asio::error::get_ssl_category());
                    CloseInternal();
                    return;
                }
            }
        } else {
            m_qRead.Push(m_ReadBuffer, len);
            m_ccb.m_ulRead += len;
        }
        m_qRead.SetNull();

        //clean password into memory
        if (ServiceId == SPA::sidStartup || (ServiceId == SPA::sidHTTP && m_ccb.Id.empty())) {
            ::memset(m_ReadBuffer, 0, IO_BUFFER_SIZE);
        }
        m_bCanceled = IsCanceledInternally();
        m_bRBLocked = false;
        Process();
        Read();
        Write(nullptr, 0);
    } else {
        CAutoLock sl(m_mutex);
        if (!m_bChatting) {
            m_ec = Error;
            CloseInternal();
        } else {
            g_pServer->m_IoService.post(boost::bind(&CServerSession::PostClose, this, Error.value()));
        }
    }
}

void CServerSession::OnWriteCompleted(const CErrorCode& Error, size_t bytes_transferred) {
    CAutoLock sl(m_mutex);
    m_ccb.SendTime = (GetTimeTick() - g_pServer->m_tStart);
    if (Error) {
        m_ec = Error;
        CloseInternal();
        return;
    }
    assert(m_bWBLocked >= (unsigned int) bytes_transferred);
    assert(m_bWBLocked <= (unsigned int) IO_BUFFER_SIZE);
    unsigned int len = m_qWrite.GetSize();
    if (len > 5 * IO_BUFFER_SIZE && len < 7 * IO_BUFFER_SIZE) {
        m_cv.notify_all();
    }
    if (ServiceId == SPA::sidHTTP) {
        do {
            if (m_pHttpContext == nullptr) {
                if (m_qRead.GetSize()) {
                    ProcessHttpRequest();
                }
                break;
            }
            if (m_pHttpContext->IsWebSocket()) {
                if (m_qRead.GetSize()) {
                    Process();
                }
                break;
            }
            if (m_pHttpContext->GetResponseProgress().Status == UHTTP::hrsCompleted) {
                UHTTP::CHttpContext::Unlock(m_pHttpContext);
                m_pHttpContext = nullptr;
                break;
            }
            if (m_qWrite.GetSize() < IO_BUFFER_SIZE) {
                m_qWrite.SetHeadPosition();
                if (m_pSsl) {
                    SPA::CScopeUQueue sb;
                    m_pHttpContext->DownloadFile(*sb);
                    Write(sb->GetBuffer(), sb->GetSize());
                } else {
                    m_pHttpContext->DownloadFile(m_qWrite);
                }
            }
        } while (false);
    } else {
        Process();
    }
    if (m_pServiceContext && m_pServiceContext->GetRandom())
        PutOntoWireInternal();
    if (m_bWBLocked > (unsigned int) bytes_transferred) {
        unsigned int ulLen = m_bWBLocked - (unsigned int) bytes_transferred;
        memmove(m_WriteBuffer, m_WriteBuffer + bytes_transferred, ulLen);
        m_bWBLocked = ulLen;
        unsigned int max_add = IO_BUFFER_SIZE - m_bWBLocked;
        if (max_add && m_qWrite.GetSize()) {
            if (max_add > m_qWrite.GetSize()) {
                max_add = m_qWrite.GetSize();
            }
            m_qWrite.Pop(m_WriteBuffer + ulLen, max_add);
            ulLen += max_add;
            m_bWBLocked = ulLen;
        }
        m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CServerSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
    } else {
        m_bWBLocked = 0;
        Write(nullptr, 0);
    }
    if (m_senderHandle && m_qWrite.GetSize() < IO_BUFFER_SIZE) {
        unsigned int index;
        CServerSession *pSession = GetSvrSession(m_senderHandle, index);
        if (pSession && index == pSession->GetConnIndex()) {
            g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, pSession));
        }
    }
    Read();
    if (m_qWrite.GetSize() < 1460 && len >= IO_BUFFER_SIZE) {
        POnResultsSent p = m_ccb.SvsContext.m_OnResultsSent;
        if (p) {
            bool chatting = false;
            USocket_Server_Handle index = MakeHandlerInternal();
            CRAutoLock ral(m_mutex, chatting);
            p(index);
        }
    }
}

const char* CServerSession::GetHTTPId() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return nullptr;
    return m_ccb.Id.c_str();
}

unsigned int CServerSession::GetHTTPCurrentMultiplaxHeaders(SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count) {
    if (HeaderValue == nullptr || count == 0)
        return 0;
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext || m_pHttpContext->GetCM() == SPA::ServerSide::cmUnknown)
        return 0;
    unsigned int n, size = 0;
    const UHTTP::CHeaderValue *pHV = m_pHttpContext->GetMultiplaxContext()->GetHeaderValue(size);
    for (n = 0; n < size && count > 0; ++n, --count, ++pHV) {
        HeaderValue[n].Header = pHV->Header.Start;
        HeaderValue[n].Value = pHV->Value.Start;
    }
    return n;
}

unsigned int CServerSession::GetHTTPRequestHeaders(SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count) {
    if (HeaderValue == nullptr || count == 0)
        return 0;
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return 0;
    unsigned int n, size = 0;
    const UHTTP::CHeaderValue *pHV = m_pHttpContext->GetRequestHeaders(size);
    for (n = 0; n < size && count > 0; ++n, --count, ++pHV) {
        HeaderValue[n].Header = pHV->Header.Start;
        HeaderValue[n].Value = pHV->Value.Start;
    }
    return n;
}

const char* CServerSession::GetHTTPPath() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return nullptr;
    return m_pHttpContext->GetUrl().Start;
}

SPA::UINT64 CServerSession::GetHTTPContentLength() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return 0;
    if (m_pHttpContext->GetMethod() != SPA::ServerSide::hmPost)
        return 0;
    if (m_pHttpContext->GetTE() != SPA::ServerSide::teUnknown || m_pHttpContext->GetCM() != SPA::ServerSide::cmUnknown)
        return 0;
    return m_pHttpContext->GetContentLength();
}

const char* CServerSession::GetHTTPQuery() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return nullptr;
    return m_pHttpContext->GetParams().Start;
}

bool CServerSession::DownloadFile(const char *filePath) {
    bool b;
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return false;
    if (m_pHttpContext->GetMethod() != SPA::ServerSide::hmGet)
        return false;
    if (m_pHttpContext->GetPS() != UHTTP::psComplete)
        return false;
    if (m_pSsl) {
        SPA::CScopeUQueue sb;
        b = m_pHttpContext->StartDownloadFile(filePath, *sb);
        Write(sb->GetBuffer(), sb->GetSize());
    } else {
        b = m_pHttpContext->StartDownloadFile(filePath, m_qWrite);
        m_qWrite.SetNull();
        Write(nullptr, 0);
    }
    if (m_pHttpContext->GetResponseProgress().Status == UHTTP::hrsCompleted) {
        UHTTP::CHttpContext::Unlock(m_pHttpContext);
        m_pHttpContext = nullptr;
    }
    return b;
}

SPA::ServerSide::tagHttpMethod CServerSession::GetHTTPMethod() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return SPA::ServerSide::hmUnknown;
    return m_pHttpContext->GetMethod();
}

bool CServerSession::HTTPKeepAlive() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return true;
    return m_pHttpContext->IsKeepAlive();
}

bool CServerSession::IsWebSocket() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return false;
    return m_pHttpContext->IsWebSocket();
}

unsigned int CServerSession::StartChunkResponse() {
    unsigned int start;
    CAutoLock sl(m_mutex);
    if (ServiceId != SPA::sidHTTP || !m_pHttpContext)
        return BAD_OPERATION;
    if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
        return BAD_OPERATION;
    UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
    if (rs != UHTTP::hrsInitial)
        return BAD_OPERATION;
    if (!m_pHttpContext->AddResponseHeader(UHTTP::TRANSFER_ENCODING.c_str(), UHTTP::CHUNKED.c_str()))
        return RESULT_SENDING_FAILED;
    if (m_pSsl) {
        SPA::CScopeUQueue sb;
        m_pHttpContext->StartChunkedResponse(*sb);
        start = sb->GetSize();
        Write(sb->GetBuffer(), sb->GetSize());
    } else {
        start = m_qWrite.GetSize();
        m_pHttpContext->StartChunkedResponse(m_qWrite);
        start = (m_qWrite.GetSize() - start);
        Write(nullptr, 0);
    }
    return start;
}

unsigned int CServerSession::SendChunk(const unsigned char *buffer, unsigned int len) {
    if (!buffer)
        len = 0;
    if (!len)
        return 0;
    unsigned int start;
    CAutoLock sl(m_mutex);
    if (ServiceId != SPA::sidHTTP || !m_pHttpContext)
        return BAD_OPERATION;
    if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
        return BAD_OPERATION;
    UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
    if (rs < UHTTP::hrsHeadersOnly || rs > UHTTP::hrsContent)
        return BAD_OPERATION;
    const char *chunk = m_pHttpContext->SeekResponseHeaderValue(UHTTP::TRANSFER_ENCODING.c_str());
    if (!chunk || !UHTTP::iequals(UHTTP::CHUNKED.c_str(), chunk))
        return BAD_OPERATION;
    if (m_pSsl) {
        SPA::CScopeUQueue sb;
        m_pHttpContext->SendChunkedData(buffer, len, *sb);
        start = sb->GetSize();
        Write(sb->GetBuffer(), sb->GetSize());
    } else {
        start = m_qWrite.GetSize();
        m_pHttpContext->SendChunkedData(buffer, len, m_qWrite);
        m_qWrite.SetNull();
        start = (m_qWrite.GetSize() - start);
        Write(nullptr, 0);
    }
    return start;
}

unsigned int CServerSession::EndChunkResponse(const unsigned char *buffer, unsigned int len) {
    if (!buffer)
        len = 0;
    unsigned int start;
    CAutoLock sl(m_mutex);
    if (ServiceId != SPA::sidHTTP || !m_pHttpContext)
        return BAD_OPERATION;
    if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
        return BAD_OPERATION;
    UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
    if (rs < UHTTP::hrsHeadersOnly || rs > UHTTP::hrsContent)
        return BAD_OPERATION;
    const char *chunk = m_pHttpContext->SeekResponseHeaderValue(UHTTP::TRANSFER_ENCODING.c_str());
    if (!chunk || !UHTTP::iequals(UHTTP::CHUNKED.c_str(), chunk))
        return BAD_OPERATION;
    if (m_pSsl) {
        SPA::CScopeUQueue sb;
        if (len)
            m_pHttpContext->SendChunkedData(buffer, len, *sb);
        m_pHttpContext->SendChunkedData(nullptr, 0, *sb);
        start = sb->GetSize();
        Write(sb->GetBuffer(), sb->GetSize());
    } else {
        start = m_qWrite.GetSize();
        if (len)
            m_pHttpContext->SendChunkedData(buffer, len, m_qWrite);
        m_pHttpContext->SendChunkedData(nullptr, 0, m_qWrite);
        m_qWrite.SetNull();
        start = (m_qWrite.GetSize() - start);
        Write(nullptr, 0);
    }
    return start;
}

bool CServerSession::IsCrossDomain() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return false;
    return m_pHttpContext->IsCrossDomain();
}

double CServerSession::GetHTTPVersion() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return 0.0;
    return m_pHttpContext->GetVersion();
}

bool CServerSession::HTTPGZipAccepted() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return false;
    return m_pHttpContext->IsGZipAccepted();
}

const char* CServerSession::GetHTTPUrl() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return nullptr;
    return m_pHttpContext->GetUrl().Start;
}

const char* CServerSession::GetHTTPHost() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return nullptr;
    return m_pHttpContext->GetHost();
}

SPA::ServerSide::tagTransport CServerSession::GetHTTPTransport() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return SPA::ServerSide::tUnknown;
    return m_pHttpContext->GetTransport();
}

SPA::ServerSide::tagTransferEncoding CServerSession::GetHTTPTransferEncoding() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return SPA::ServerSide::teUnknown;
    return m_pHttpContext->GetTE();

}

SPA::ServerSide::tagContentMultiplax CServerSession::GetHTTPContentMultiplax() {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext)
        return SPA::ServerSide::cmUnknown;
    return m_pHttpContext->GetCM();
}

bool CServerSession::SetHTTPResponseCode(unsigned int errCode) {
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext || m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
        return false;
    m_pHttpContext->SetResponseCode(errCode);
    return true;
}

bool CServerSession::SetHTTPResponseHeader(const char *uft8Header, const char *utf8Value) {
    if (uft8Header == nullptr || ::strlen(uft8Header) == 0)
        return false;
    if (utf8Value == nullptr)
        utf8Value = "";
    CAutoLock sl(m_mutex);
    if (!m_pHttpContext || m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
        return false;
    return m_pHttpContext->AddResponseHeader(uft8Header, utf8Value);
}

unsigned int CServerSession::HTTPCallbackA(const char *name, const char *str, unsigned int chars) {
    unsigned int res = 0;
    CAutoLock sl(m_mutex);
    if (ServiceId != SPA::sidHTTP)
        return BAD_OPERATION;
    if (m_pHttpContext && !m_pHttpContext->IsWebSocket()) {
        if (!m_pHttpContext->IsSpRequest())
            return BAD_OPERATION;
        res = Connection::CConnectionContext::SendResult(m_ccb.Id, m_ReqInfo.RequestId, str, chars, name);
    } else if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
        unsigned int start;
        SPA::CScopeUQueue su;
        Connection::CConnectionContext::SendWSResult(m_ReqInfo.RequestId, str, chars, *su, name);
        if (m_pSsl) {
            SPA::CScopeUQueue sb;
            m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, *sb);
            Write(sb->GetBuffer(), sb->GetSize());
        } else {
            start = m_qWrite.GetSize();
            m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, m_qWrite);
            res = m_qWrite.GetSize() - start;
        }
    } else
        return BAD_OPERATION;
    Write(nullptr, 0);
    return res;
}

unsigned int CServerSession::SendHTTPReturnDataA(const char *str, unsigned int chars) {
    unsigned int res = 0;
    static const char *empty = "";
    if (str == nullptr) {
        chars = 0;
        str = empty;
    } else if (chars == (~0))
        chars = (unsigned int) ::strlen(str);
    CAutoLock sl(m_mutex);
    if (ServiceId != SPA::sidHTTP)
        return BAD_OPERATION;
    if (m_ReqInfo.RequestId > SPA::idReservedTwo) {
        if (!m_pHttpContext || !m_pHttpContext->IsWebSocket())
            res = Connection::CConnectionContext::SendResult(m_ccb.Id, m_ReqInfo.RequestId, str, chars, (const char*) nullptr);
        else if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
            SPA::CScopeUQueue su;
            Connection::CConnectionContext::SendWSResult(m_ReqInfo.RequestId, str, chars, *su, (const char*) nullptr);
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, *sb);
                res = sb->GetSize();
                Write(sb->GetBuffer(), res);
            } else {
                unsigned int start = m_qWrite.GetSize();
                m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, m_qWrite);
                res = m_qWrite.GetSize() - start;
            }
        } else
            return BAD_OPERATION;
    } else {
        if (!m_pHttpContext)
            return BAD_OPERATION;
        UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
        if (pWebResponseProcessor) {
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                res = pWebResponseProcessor->ProcessUserRequest(str, *sb);
                Write(sb->GetBuffer(), sb->GetSize());
            } else {
                res = pWebResponseProcessor->ProcessUserRequest(str, m_qWrite);
            }
        } else {
            if (m_pSsl) {
                SPA::CScopeUQueue sb;
                m_pHttpContext->PrepareResponse((const unsigned char*) str, chars, *sb);
                res = sb->GetSize();
                Write(sb->GetBuffer(), res);
            } else {
                unsigned int start = m_qWrite.GetSize();
                m_pHttpContext->PrepareResponse((const unsigned char*) str, chars, m_qWrite);
                res = m_qWrite.GetSize() - start;
            }
        }
    }
    Write(nullptr, 0);
    return res;
}
