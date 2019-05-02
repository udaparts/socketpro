#include "stdafx.h"
#include "clientsession_win.h"
#include <ctype.h>
#include "../../pinc/uzip.h"
#include "../../pinc/getsysid.h"
#include <boost/algorithm/string/trim.hpp>
#include "../clientcore/socketpool.h"
#include <assert.h>
#include <boost/filesystem.hpp>

boost::posix_time::ptime CClientSession::m_tStart = boost::posix_time::microsec_clock::local_time();

std::vector<unsigned char*> CClientSession::m_aBuffer;
boost::mutex CClientSession::m_mutexBuffer;
std::string CClientSession::m_WorkingPath("");

unsigned char* CClientSession::GetIoBuffer() {
    unsigned char *s = NULL;
    {
        CAutoLock sl(m_mutexBuffer);
        size_t size = m_aBuffer.size();
        if (size) {
            s = m_aBuffer[size - 1];
            m_aBuffer.pop_back();
        }
    }
    if (s == NULL)
        s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + IO_EXTRA);
    return s;
}

void CClientSession::ReleaseIoBuffer(unsigned char *buffer) {
    if (buffer == NULL)
        return;
    m_mutexBuffer.lock();
    m_aBuffer.push_back(buffer);
    m_mutexBuffer.unlock();
}

boost::mutex CClientSession::m_mutexQLI;
std::vector<boost::shared_ptr<MQ_FILE::CMqFile> > CClientSession::m_vQRequest;
boost::shared_ptr<MQ_FILE::CQLastIndex> CClientSession::m_pQLastIndex;

CClientSession::CClientSession(CIoService &IoService, CClientThread *pClientThread)
: m_pIoService(&IoService), m_pSsl(NULL), m_ulRead(0), m_ulSent(0),
m_ReadBuffer(GetIoBuffer()), m_bRBLocked(false), m_WriteBuffer(GetIoBuffer()), m_bWBLocked(false), m_zl(SPA::zlDefault), m_pQBatch(NULL), m_bZip(false),
m_EncryptionMethod(SPA::NoEncryption), m_Resolver(IoService), m_nPort(0),
m_OnSocketClosed(NULL), m_OnHandShakeCompleted(NULL), m_OnSocketConnected(NULL), m_OnRequestProcessed(NULL),
m_pThread(pClientThread), m_ConnState(csClosed), m_nConnTimeout(SPA::ClientSide::DEFAULT_CONN_TIMEOUT), m_nRecvTimeout(SPA::ClientSide::DEFAULT_RECV_TIMEOUT),
m_OnSubscribe(NULL), m_OnUnsubscribe(NULL), m_OnBroadcastEx(NULL), m_OnBroadcast(NULL), m_OnPostUserMessageEx(NULL), m_OnPostUserMessage(NULL), m_OnServerException(NULL),
m_OnBaseRequestProcessed(NULL), m_OnAllRequestsProcessed(NULL), m_bAutoConn(false), m_nPoolId(m_pThread->GetPool()->GetPoolId()), m_bFail(false),
m_bDequeueTrans(false), m_bConfirmTrans(false), m_bConfirmFail(false), m_RouterHandle(0), m_nRouteeCount(0) {
    SocketPoolCallback spc = pClientThread->GetSocketPoolCallback();
    if (spc) {
        spc(m_nPoolId, SPA::ClientSide::speUSocketCreated, this);
    }
}

CClientSession::~CClientSession() {
    CAutoLock sl(m_mutex);
    if (IsContextSet()) {
        CErrorCode ec(0, boost::system::system_category());
        if (IsSslEnabled()) {
			// ????
            //don't call this here. Otherwise, there is an exception caught by boost framework
            //m_pSslSocket->shutdown(ec);
        }
        GetSocket()->shutdown(boost::asio::socket_base::shutdown_both, ec);
        GetSocket()->close(ec);
    }
    delete m_pSsl;
    m_pSocket.reset();
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    ReleaseIoBuffer(m_ReadBuffer);
    ReleaseIoBuffer(m_WriteBuffer);
    SocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speUSocketKilled, this);
    }
}

int CClientSession::GetErrorCode() {
    CAutoLock sl(m_mutex);
    return m_ec.value();
}

std::string CClientSession::GetErrorMessage() {
    CAutoLock sl(m_mutex);
    return m_ec.message();
}

bool CClientSession::IsContextSet() {
    return (m_pSsl != NULL || m_pSocket);
}

void CClientSession::TimerHandler() {
#ifdef WIN32_64
    int errorCode = WSAETIMEDOUT;
#else
    int errorCode = SO_RCVTIMEO;
#endif

    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::ptime lastOne = ((m_tRecv > m_tSend) ? m_tRecv : m_tSend);
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsProcessing())
        return;
    switch (m_ConnState) {
        case csSslShaking:
        case csConnecting:
            if (now > lastOne + boost::posix_time::milliseconds(m_nConnTimeout)) {
                m_ec.assign(errorCode, boost::asio::error::get_system_category());
                m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
            }
            break;
        case csSwitched:
        case csConnected:
            if (m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader) > 0 && (now > lastOne + boost::posix_time::milliseconds(m_nRecvTimeout))) {
                m_ec.assign(errorCode, boost::asio::error::get_system_category());
                m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
            } else if (now > lastOne + boost::posix_time::milliseconds((unsigned int) GetServerPingTimeInternal() * 1000 + m_pThread->GetTimerInterval())) {
                if (m_qReqIdWait.GetSize() == 0 || (m_qRequest && m_qRequest->GetMessageCount() > 0)) {
#ifdef WIN32_64
                    errorCode = WSAECONNRESET;
#else
                    errorCode = ECONNREFUSED;
#endif
                    m_ec.assign(errorCode, boost::asio::error::get_system_category());
                    m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
                }
            } else {

            }
            break;
        case csClosed:
            if (m_bAutoConn && m_strhost.size() > 0 && m_nPort > 0 && now > (lastOne + boost::posix_time::milliseconds(150))) {
                CloseInternal();
                m_ConnState = csConnecting;
                m_tRecv = boost::posix_time::microsec_clock::local_time();
                m_tSend = m_tRecv;
                m_pIoService->post(boost::bind(&CClientSession::ConenctInternally, this));
            }
            break;
        default:
            break;
    }
}

bool CClientSession::IsSslEnabled() {
    switch (m_EncryptionMethod) {
        case SPA::TLSv1:
            return true;
            break;
        default:
            break;
    }
    return false;
}

bool CClientSession::CheckQueueAvailable() {
    return (m_qRequest && m_qRequest->IsAvailable());
}

bool CClientSession::IsDequeueEnabled() {
    CAutoLock al(m_mutex);
    return (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->IsDequeueOk());
}

void CClientSession::OnClosed(int errCode) {
    if (CheckQueueAvailable()) {
        m_qRequest->ReleaseMessageAttributesInDequeuing();
        m_qRequest->Notify();
    }
    m_bZip = false;
    POnSocketClosed p = m_OnSocketClosed;
    if (p != NULL) {
        CRAutoLock sl(m_mutex);
        p(this, errCode);
    }
    m_cv.notify_all();
}

CClientThread* CClientSession::GetClientThread() {
    return m_pThread;
}

CSocketPool* CClientSession::GetSocketPool() {
    if (m_pThread != NULL)
        return m_pThread->GetPool();
    return NULL;
}

void* CClientSession::GetSSL() {
    CAutoLock al(m_mutex);
	// ????
	/*
    if (m_pSsl)
        return m_pSsl->
	*/
    return NULL;
}

CUCertImpl* CClientSession::GetUCert() {
    CAutoLock al(m_mutex);
    if (!m_pCert && m_pSsl)
        m_pCert.reset(new CUCertImpl(m_pSsl));
    return m_pCert.get();
}

bool CClientSession::Cancel(unsigned int requestsQueued) {
    CAutoLock sl(m_mutex);
    if (NULL != m_pQBatch)
        return false;
    if (CheckQueueAvailable()) {
        m_qRequest->Reset();
    } else {
        unsigned int count = m_qReqIdCancel.GetSize() / sizeof (SPA::CStreamHeader);
        for (unsigned int n = count - 1; n != (~0); --n) {
            if (m_qWrite.GetSize() < sizeof (SPA::CStreamHeader))
                break;
            SPA::CStreamHeader *pStreamHeader = (SPA::CStreamHeader*)m_qReqIdCancel.GetBuffer(n * sizeof (SPA::CStreamHeader));
            unsigned int total = pStreamHeader->Size + sizeof (SPA::CStreamHeader);
            if (total <= m_qWrite.GetSize()) {
                m_qWrite.SetSize(m_qWrite.GetSize() - total);
                m_qReqIdCancel.SetSize(m_qReqIdCancel.GetSize() - sizeof (SPA::CStreamHeader));
            } else
                break;
        }
    }
    requestsQueued = (~0);
    bool ok = SendRequestInternal(SPA::idCancel, &requestsQueued, sizeof (requestsQueued));
    return ok;
}

bool CClientSession::IsRandom() {
    CAutoLock al(m_mutex);
    return (m_ServerInfo.SockMinorVersion & RETURN_RESULT_RANDOM) > 0;
}

bool CClientSession::IsRouting() {
    CAutoLock al(m_mutex);
    return (m_ServerInfo.SockMinorVersion & IS_ROUTING_PARTNER) > 0;
}

bool CClientSession::IsRoutingInternal() {
    return (m_ServerInfo.SockMinorVersion & IS_ROUTING_PARTNER) > 0;
}

unsigned short CClientSession::GetServerPingTime() {
    CAutoLock al(m_mutex);
    return GetServerPingTimeInternal();
}

unsigned short CClientSession::GetServerPingTimeInternal() {
    return (unsigned int) (m_ServerInfo.Param2 & 0xFFFF);
}

const SPA::CSwitchInfo* CClientSession::GetServerInfo() {
    CAutoLock al(m_mutex);
    return &m_ServerInfo;
}

unsigned int CClientSession::GetCurrentServiceId() {
    CAutoLock al(m_mutex);
    return m_ServerInfo.ServiceId;
}

const SPA::CSwitchInfo* CClientSession::GetClientInfo() {
    CAutoLock al(m_mutex);
    return &m_ClientInfo;
}

void CClientSession::SetClientInfo(SPA::CSwitchInfo si) {
    CAutoLock al(m_mutex);
    m_ClientInfo.Param2 = si.Param2;
    m_ClientInfo.SwitchTime = si.SwitchTime;
}

SPA::UINT64 CClientSession::GetBytesReceived() {
    CAutoLock sl(m_mutex);
    return m_ulRead;
}

SPA::UINT64 CClientSession::GetBytesSent() {
    CAutoLock sl(m_mutex);
    return m_ulSent;
}

bool CClientSession::IsSameThread() {
    return (boost::this_thread::get_id() == m_pThread->GetBoostThread()->get_id());
}

bool CClientSession::WaitAllInternal(CAutoLock &sl, unsigned int nTimeout) {
    bool b;
    boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(nTimeout);
    b = (m_pQBatch != NULL || m_ConnState < csConnected);
    if (b)
        return false;
    b = (m_qReqIdWait.GetSize() == 0);
    if (b) {
        m_qReqIdCancel.SetSize(0);
        return true;
    }
    if (IsSameThread())
        return false;
    do {
        b = (m_cv.timed_wait(sl, td) && m_ConnState >= csConnected);
    } while (b && m_qReqIdWait.GetSize() > 0);
    return b;
}

bool CClientSession::WaitAll(unsigned int nTimeout) {
    CAutoLock sl(m_mutex);
    return WaitAllInternal(sl, nTimeout);
}

unsigned int CClientSession::GetBytesInSendingBuffer() {
    m_mutex.lock();
    unsigned int len = m_qWrite.GetSize();
    m_mutex.unlock();
    return len;
}

unsigned int CClientSession::GetBytesInReceivingBuffer() {
    m_mutex.lock();
    unsigned int len = m_qRead.GetSize();
    m_mutex.unlock();
    return len;
}

bool CClientSession::IsBatching() {
    m_mutex.lock();
    bool b = (m_pQBatch != NULL);
    m_mutex.unlock();
    return b;
}

unsigned int CClientSession::GetBytesBatched() {
    m_mutex.lock();
    unsigned int len = 0;
    if (m_pQBatch != NULL)
        len = m_pQBatch->GetSize();
    m_mutex.unlock();
    return len;
}

unsigned int CClientSession::GetCountOfRequestsInQueue() {
    m_mutex.lock();
    unsigned int count = m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader);
    m_mutex.unlock();
    return count;
}

void CClientSession::OnConnectedInternal(int errCode) {
    ::memset(&m_ResultInfo, 0, sizeof (m_ResultInfo));
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = NULL;
    POnSocketConnected p = m_OnSocketConnected;
    m_qRead.SetSize(0);
    m_qWrite.SetSize(0);
    m_ulRead = 0;
    m_ulSent = 0;
    if (errCode == 0)
        m_ConnState = csConnected;
    m_nCancel = 0;
    m_qReqIdWait.SetSize(0);
    m_qReqIdCancel.SetSize(0);
    m_RouterHandle = 0;
    m_nRouteeCount = 0;
    m_bWBLocked = false;
    m_bRBLocked = false;
    if (p) {
        CRAutoLock sl(m_mutex);
        p(this, errCode);
    }

    SocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speConnected, this);
    }

    if (errCode == 0)
        m_pThread->GetPool()->Notify();

    m_cv.notify_all();
}

void CClientSession::SetEncryptionMethod(SPA::tagEncryptionMethod em) {
    m_mutex.lock();
    m_EncryptionMethod = em;
    m_mutex.unlock();
}

SPA::tagEncryptionMethod CClientSession::GetEncryptionMethod() {
    m_mutex.lock();
    SPA::tagEncryptionMethod em = m_EncryptionMethod;
    m_mutex.unlock();
    return em;
}

bool CClientSession::IsOpened() {
    m_mutex.lock();
    bool b = (m_ConnState >= csConnected);
    m_mutex.unlock();
    return b;
}

CSocket* CClientSession::GetSocket() {
    return m_pSocket.get();
}

void CClientSession::WriteIntoBatch(const void *pBuffer, unsigned int len) {
    m_pQBatch->Push((const unsigned char*) pBuffer, len);
}

bool CClientSession::StartBatching() {
    bool b = false;
    m_mutex.lock();
    do {
        if (m_pQBatch != NULL || ((m_ConnState < csConnected) && !CheckQueueAvailable()))
            break;
        if (m_qRequest && m_qRequest->GetJobSize())
            break;
        b = true;
        m_pQBatch = SPA::CScopeUQueue::Lock();
    } while (false);
    m_mutex.unlock();
    return b;
}

bool CClientSession::Enter(const unsigned int *pChatGroupId, unsigned int nCount) {
    if (pChatGroupId == NULL)
        nCount = 0;
    unsigned short vt = (VT_UINT | VT_ARRAY);
    SPA::CScopeUQueue sb;
    sb << vt;
    sb << nCount;
    sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    CAutoLock sl(m_mutex);
    return SendRequestInternal((unsigned short) SPA::idEnter, sb->GetBuffer(), sb->GetSize());
}

void CClientSession::Exit() {
    CAutoLock sl(m_mutex);
    SendRequestInternal((unsigned short) SPA::idExit, NULL, 0);
}

bool CClientSession::SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    if (!pChatGroupId)
        nCount = 0;
    if (!message)
        size = 0;
    SPA::CScopeUQueue sb;
    sb << size;
    sb->Push(message, size);
    sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    CAutoLock sl(m_mutex);
    return SendRequestInternal((unsigned short) SPA::idSpeakEx, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::Speak(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
    if (!pChatGroupId || !nCount)
        return false;
    if (!message || size < sizeof (unsigned short))
        return false;
    SPA::CScopeUQueue sb;
    unsigned short vt = (VT_UINT | VT_ARRAY);
    sb << vt;
    sb << nCount;
    sb->Push((const unsigned char*) pChatGroupId, sizeof (unsigned int) *nCount);
    sb->Push(message, size);
    CAutoLock sl(m_mutex);
    return SendRequestInternal((unsigned short) SPA::idSpeak, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == NULL)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    CAutoLock sl(m_mutex);
    return SendRequestInternal((unsigned short) SPA::idSendUserMessage, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == NULL)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    CAutoLock sl(m_mutex);
    return SendRequestInternal((unsigned short) SPA::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SwitchToIntenal(unsigned int serviceId) {
    SPA::CScopeUQueue sb;
    if (m_ConnState < csConnected)
        return false;
    bool bQueue = CheckQueueAvailable();
    if (bQueue && m_ClientInfo.ServiceId == SPA::sidStartup) {
        m_qReqIdCancel.SetSize(0);
        m_qReqIdWait.SetSize(0);
    }
    if (bQueue && m_ClientInfo.ServiceId != SPA::sidStartup)
        return false;
    m_ClientInfo.ServiceId = serviceId;
    std::time_t t;
    std::time(&t);
    m_ClientInfo.SwitchTime = (SPA::UINT64)t;
    sb << m_ClientInfo;
    sb << m_strUserId;
    sb << GetPwd();
    bool ok = SendRequestInternal((unsigned short) SPA::idSwitchTo, sb->GetBuffer(), sb->GetSize());
    return ok;
}

bool CClientSession::SwitchTo(unsigned int serviceId) {
    if (serviceId < SPA::sidReserved &&
            serviceId != SPA::sidStartup &&
            serviceId != SPA::sidChat)
        return false;
    CAutoLock sl(m_mutex);
    return SwitchToIntenal(serviceId);
}

bool CClientSession::CommitBatching(bool bBatchingAtServerSide) {
    bool b = true;
    SPA::CUQueue *pBatch = NULL;
    CAutoLock sl(m_mutex);
    bool bQueue = CheckQueueAvailable();
    do {
        if (m_pQBatch == NULL || ((m_ConnState < csConnected) && !bQueue)) {
            b = false;
            break;
        }

        if (m_pQBatch->GetSize() == 0) {
            SPA::CScopeUQueue::Unlock(m_pQBatch);
            m_pQBatch = NULL;
            break;
        }

        if (bBatchingAtServerSide) {
            SPA::CStreamHeader reqInfo;
            reqInfo.RequestId = SPA::idStartBatching;
            m_pQBatch->Insert(&reqInfo, 0);
            reqInfo.RequestId = SPA::idCommitBatching;
            *m_pQBatch << reqInfo;
        }
        pBatch = m_pQBatch;
        m_pQBatch = NULL;
    } while (false);

    if (pBatch != NULL) {
        SPA::UINT64 index;
        unsigned int len = pBatch->GetSize();
        const unsigned char *pBuffer = pBatch->GetBuffer();
        while (len >= sizeof (SPA::CStreamHeader)) {
            SPA::CStreamHeader *pStreamHeader = (SPA::CStreamHeader*)pBuffer;
            if (!m_bZip && !bQueue)
                m_qReqIdCancel << *pStreamHeader;

            if (pStreamHeader->RequestId != SPA::idStartBatching &&
                    pStreamHeader->RequestId != SPA::idCommitBatching && !bQueue)
                m_qReqIdWait << *pStreamHeader;

            len -= sizeof (SPA::CStreamHeader);
            pBuffer += sizeof (SPA::CStreamHeader);
            len -= pStreamHeader->Size;
            pBuffer += pStreamHeader->Size;
        }
        assert(len == 0);
        if (m_bZip && !bQueue) { //don't support queue when zip is enabled
            if (bQueue) {
                SPA::CScopeUQueue sb;
                len = CompressRequestTo(SPA::idBatchZipped, m_zl, pBatch->GetBuffer(), pBatch->GetSize(), *sb);
                SPA::CStreamHeader *sh = (SPA::CStreamHeader *)sb->GetBuffer();
                sb->Pop((unsigned int) sizeof (SPA::CStreamHeader));
                {
                    CRAutoLock rsl(m_mutex);
                    index = m_qRequest->Enqueue(*sh, *sb);
                }
                if (m_ConnState > csConnected) {
                    WriteFromQueueFile();
                    Write(NULL, 0);
                }
            } else {
                len = CompressRequestTo(SPA::idBatchZipped, m_zl, pBatch->GetBuffer(), pBatch->GetSize(), m_qWrite);
                m_qReqIdCancel.Push(m_qWrite.GetBuffer(), sizeof (SPA::CStreamHeader));
                Write(NULL, 0);
            }
        } else {
            if (bQueue && m_ConnState > csConnected) {
                {
                    CRAutoLock rsl(m_mutex);
                    m_qRequest->BatchEnqueue(*pBatch);
                }
                WriteFromQueueFile();
                Write(NULL, 0);
            } else
                Write(pBatch->GetBuffer(), pBatch->GetSize());
        }
        SPA::CScopeUQueue::Unlock(pBatch);
    }
    return b;
}

bool CClientSession::AbortBatching() {
    m_mutex.lock();
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = NULL;
    m_mutex.unlock();
    return true;
}

bool CClientSession::IgnoreLastRequest(unsigned short reqId) {
    CAutoLock sl(m_mutex);
    SPA::CStreamHeader *p = (SPA::CStreamHeader*)m_qReqIdWait.GetBuffer();
    for (int count = (int) m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader), n = count - 1; n >= 0; --n) {
        if (p[n].RequestId == reqId) {
            m_qReqIdWait.Pop(sizeof (SPA::CStreamHeader), n * sizeof (SPA::CStreamHeader));
            return true;
        }
    }
    return false;
}

void CClientSession::SetZip(bool zip) {
    m_mutex.lock();
    m_bZip = zip;
    m_mutex.unlock();
}

bool CClientSession::GetZip() {
    m_mutex.lock();
    bool zip = m_bZip;
    m_mutex.unlock();
    return zip;
}

void CClientSession::SetZipLevel(SPA::tagZipLevel zl) {
    if (zl != SPA::zlBestSpeed && zl != SPA::zlDefault)
        return;
    m_mutex.lock();
    m_zl = zl;
    m_mutex.unlock();
}

SPA::tagZipLevel CClientSession::GetZipLevel() {
    m_mutex.lock();
    SPA::tagZipLevel zl = m_zl;
    m_mutex.unlock();
    return zl;
}

bool CClientSession::SendRoutingResultInternal(unsigned short reqId, const unsigned char *buffer, unsigned int len) {
    if (!buffer)
        len = 0;
    if (m_RouterHandle == 0 || m_ConnState < csConnected)
        return false;
    SPA::CScopeUQueue su;
    SPA::CUQueue &q = *su;
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idRoutingData;
    if (m_bZip) {
        SPA::CScopeUQueue sb;
        unsigned int res = CompressRequestTo(reqId, m_zl, buffer, len, *sb);
        sb->SetSize(res);
        sb >> sh;
        sh.RequestId = SPA::idRoutingData;
        sh.Size = sizeof (reqId) + sizeof (m_RouterHandle) + sb->GetSize();
        q << sh << reqId << m_RouterHandle;
        q.Push(sb->GetBuffer(), sb->GetSize());
    } else {
        sh.Size = sizeof (reqId) + sizeof (m_RouterHandle) + len;
        q << sh << reqId << m_RouterHandle;
        q.Push(buffer, len);
    }
    m_qReqIdCancel << sh;
    Write(q.GetBuffer(), q.GetSize());
    return true;
}

bool CClientSession::SendRouteeResult(unsigned short reqId, const unsigned char *buffer, unsigned int len) {
    CAutoLock sl(m_mutex);
    return SendRoutingResultInternal(reqId, buffer, len);
}

bool CClientSession::SendRequestInternal(unsigned short reqId, const void *pBuffer, unsigned int len) {
    SPA::UINT64 index;
    bool bQueue = CheckQueueAvailable();
    if (m_ConnState < csConnected) {
        if (!bQueue)
            return false;
    }
    if (pBuffer == NULL)
        len = 0;

    SPA::CStreamHeader reqInfo;
    reqInfo.RequestId = reqId;
    reqInfo.Size = len;
    if (len > 0) {
        SPA::CScopeUQueue sb;
        if (m_pQBatch != NULL) {
            sb << reqInfo;
            sb->Push((const unsigned char*) pBuffer, len);
            WriteIntoBatch(sb->GetBuffer(), sb->GetSize());
        } else {
            if (m_bZip && reqId != SPA::idSwitchTo) {
                unsigned int res = CompressRequestTo(reqId, m_zl, (const unsigned char*) pBuffer, len, *sb);
                sb->SetSize(res);
                //The first sizeof(reqInfo) of bytes contains request header info
                if (!bQueue) {
                    m_qReqIdCancel.Push(sb->GetBuffer(), sizeof (reqInfo));
                    m_qReqIdWait.Push(sb->GetBuffer(), sizeof (reqInfo));
                }
            } else {
                sb << reqInfo;
                if (!bQueue) {
                    m_qReqIdCancel << reqInfo;
                    m_qReqIdWait << reqInfo;
                }
                sb->Push((const unsigned char*) pBuffer, len);
            }
            if (bQueue && reqId != SPA::idSwitchTo) {
                SPA::CStreamHeader *p = (SPA::CStreamHeader*)sb->GetBuffer();
                sb->Pop((unsigned int) sizeof (SPA::CStreamHeader));
                CRAutoLock rsl(m_mutex);
                index = m_qRequest->Enqueue(*p, *sb);
            } else
                Write(sb->GetBuffer(), sb->GetSize());
        }
    } else {
        if (m_pQBatch != NULL)
            WriteIntoBatch((const unsigned char*) &reqInfo, sizeof (reqInfo));
        else {
            if (bQueue) {
                CRAutoLock rsl(m_mutex);
                index = m_qRequest->Enqueue(reqInfo, NULL, 0);
            } else {
                m_qReqIdCancel << reqInfo;
                m_qReqIdWait << reqInfo;
                Write((const unsigned char*) &reqInfo, sizeof (reqInfo));
            }
        }
    }

    if (m_ConnState > csConnected && bQueue && m_qRequest->GetMessageCount() > 0) {
        WriteFromQueueFile();
    }
    Write(NULL, 0);

    return true;
}

void CClientSession::WriteFromQueueFile() {
    if (m_ConnState <= csConnected)
        return;
    if (m_qWrite.GetSize() >= IO_BUFFER_SIZE)
        return;
    if (!m_qRequest)
        return;
    if (!m_qRequest->IsAvailable())
        return;
    if ((m_ServerInfo.SockMinorVersion & IS_ROUTING_PARTNER) == IS_ROUTING_PARTNER && m_nRouteeCount == 0)
        return;

    unsigned int pos = 0;
    unsigned int index = 0;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;

    std::vector<unsigned int> vSize = m_qRequest->DoBatchDequeue(*qAttr, *qRequests, 3 * IO_BUFFER_SIZE, 0);
    MQ_FILE::QAttr *qattr = (MQ_FILE::QAttr *)qAttr->GetBuffer();
    for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
        unsigned int total = *it;
        SPA::CStreamHeader *sh = (SPA::CStreamHeader*)qRequests->GetBuffer(pos);
        unsigned int size = sh->Size;
        assert((size + sizeof (SPA::CStreamHeader)) <= total);
        sh->Size += sizeof (MQ_FILE::QAttr);
        sh->SetQueued(true);
        m_qReqIdCancel << *sh;
        m_qReqIdWait << *sh;
        m_qWrite << *sh;
        m_qWrite << qattr[index];
        m_qWrite.Push(qRequests->GetBuffer(pos + sizeof (SPA::CStreamHeader)), size);
        pos += total;
        ++index;
    }
}

bool CClientSession::SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int len) {
    if (reqId < SPA::idReservedTwo)
        return false;
    if (!IsQueueStarted() && !IsSameThread()) {
        while (GetBytesInSendingBuffer() > 50 * 1024) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    }
    CAutoLock sl(m_mutex);
    return SendRequestInternal(reqId, pBuffer, len);
}

void CClientSession::SetContext() {
	m_pSocket.reset(new CSocket(*m_pIoService));
    switch (m_EncryptionMethod) {
        case SPA::TLSv1:
            delete m_pSsl;
            m_pSsl = new SPA::CMsSsl(m_pThread->GetSslContext());
            break;
        default:
            break;
    }
}

#ifndef WIN32_64
boost::mutex g_dns_mutex;
#endif

void CClientSession::ConenctInternally() {
    CErrorCode ec;
    SocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        spc(m_nPoolId, SPA::ClientSide::speConnecting, this);
    }
    CAutoLock sl(m_mutex);
    SetContext();
    CResolver::query ipAddress(m_strhost, boost::lexical_cast<std::string > (m_nPort));
    CResolver r(*m_pIoService);
    {
#ifndef WIN32_64
        CAutoLock gsl(g_dns_mutex);
#endif
        nsIP::tcp::resolver::iterator iterator = r.resolve(ipAddress, ec);
        m_ec = ec;
        if (ec) {
            OnConnectedInternal(ec.value());
            CloseInternal(ec.value());
            return;
        }
        nsIP::tcp::endpoint endpoint(iterator->endpoint().address(), m_nPort);

        //sync connect doesn't create a new thread, but may slow down building a pool of socket connections
        /*
        CErrorCode ec;
        {
            GetSocket()->connect(endpoint, ec);
            CRAutoLock sl(m_mutex);
            OnConnected(ec, iterator);
        }
         */
        //async_coonect requires a new thread created
        GetSocket()->async_connect(endpoint, boost::bind(&CClientSession::OnConnected, this, boost::asio::placeholders::error, ++iterator));
    }
}

tagConnectionState CClientSession::GetConnectionState() {
    CAutoLock sl(m_mutex);
    return m_ConnState;
}

bool CClientSession::WaitConnected(CAutoLock &sl, unsigned int nTimeout) {
    if (IsSameThread())
        return false;
    if (m_ConnState >= csConnected)
        return true;
    boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(nTimeout);
    return (m_cv.timed_wait(sl, td) && m_ConnState >= csConnected);
}

bool CClientSession::Connect(const char *strHost, unsigned int nPort, bool bSync, bool b6) {
    {
        CAutoLock sl(m_mutex);
        CloseInternal();
        m_strhost = strHost;
        std::transform(m_strhost.begin(), m_strhost.end(), m_strhost.begin(), ::tolower);
        boost::trim(m_strhost);
        m_nPort = nPort;
        m_ConnState = csConnecting;
        m_tRecv = boost::posix_time::microsec_clock::local_time();
        m_tSend = m_tRecv;
        m_pIoService->post(boost::bind(&CClientSession::ConenctInternally, this));
    }
    if (bSync) {
        CAutoLock sl(m_mutex);
        return WaitConnected(sl, m_nConnTimeout);
    }
    return true;
}

void CClientSession::OnResolve(const CErrorCode &ec, CResolver::iterator ep) {
    CAutoLock sl(m_mutex);
    if (ec) {
        m_ec = ec;
        OnConnectedInternal(ec.value());
        CloseInternal(ec.value());
        return;
    } else
        m_ec.clear();
    nsIP::tcp::endpoint endpoint(ep->endpoint().address(), m_nPort);
	// ????
	/*
    if (IsSslEnabled())
        m_pSslSocket->lowest_layer().async_connect(endpoint, boost::bind(&CClientSession::OnConnected, this, boost::asio::placeholders::error, ++ep));
    else
	*/
	m_pSocket->async_connect(endpoint, boost::bind(&CClientSession::OnConnected, this, boost::asio::placeholders::error, ++ep));
}

boost::posix_time::ptime& CClientSession::GetLatestTime() {
    return (m_tRecv > m_tSend) ? m_tRecv : m_tSend;
}

void CClientSession::OnConnected(const CErrorCode &ec, CResolver::iterator ep) {
    CAutoLock sl(m_mutex);
    m_nCancel = 0;
    if (ec) {
        m_ec = ec;
        OnConnectedInternal(ec.value());
        CloseInternal(ec.value());
        return;
    } else
        m_ec.clear();

    m_tRecv = boost::posix_time::microsec_clock::local_time();
    m_tSend = m_tRecv;
    if (IsSslEnabled()) {
        m_ConnState = csSslShaking;
		// ????
        //m_pSslSocket->async_handshake(boost::asio::ssl::stream_base::client, boost::bind(&CClientSession::OnSslHandShake, this, boost::asio::placeholders::error));
    } else {
        OnConnectedInternal(ec.value());
        Read();
    }
}

void CClientSession::Write(const unsigned char *s, unsigned int nSize) {
    if (m_ConnState < csConnected)
        return;
    if (m_bWBLocked) {
        if (s)
            m_qWrite.Push(s, nSize);
        return;
    }
    unsigned int ulLen = m_qWrite.GetSize();
    if (ulLen == 0 && s != NULL && nSize > 0) {
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
            return;
        if (ulLen > IO_BUFFER_SIZE)
            ulLen = IO_BUFFER_SIZE;
        m_qWrite.Pop(m_WriteBuffer, ulLen);
    }
    m_ulSent += ulLen;
    m_bWBLocked = true;
    if (IsSslEnabled()) {
		// ????
		/*
        boost::asio::async_write(*m_pSslSocket,
                boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CClientSession::OnWriteCompleted, this, nsPlaceHolders::error));
		*/
    } else {
        boost::asio::async_write(*m_pSocket,
                boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CClientSession::OnWriteCompleted, this, nsPlaceHolders::error));
    }

    if (m_qWrite.GetHeadPosition() > 2 * IO_BUFFER_SIZE)
        m_qWrite.SetHeadPosition();
}

void CClientSession::Read() {
    if (m_bRBLocked || m_ConnState < csConnected)
        return;
    m_bRBLocked = true;
	// ????
	/*
    if (m_pSslSocket)
        m_pSslSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE),
            boost::bind(&CClientSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
    else
	*/
    m_pSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE),
        boost::bind(&CClientSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

void CClientSession::OnHandleShakeCompleted(int errCode) {
    POnHandShakeCompleted p = m_OnHandShakeCompleted;
    if (p != NULL) {
        CRAutoLock sl(m_mutex);
        p(this, errCode);
    }
    SocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speHandShakeCompleted, this);
    }
}

void CClientSession::OnSslHandShake(const CErrorCode& ec) {
    CAutoLock sl(m_mutex);
    m_ec = ec;
    if (!ec) {
        OnHandleShakeCompleted(ec.value());
        OnConnectedInternal(ec.value());
        Read();
    } else {
        OnHandleShakeCompleted(ec.value());
        CloseInternal(ec.value());
        m_cv.notify_all();
    }
}

void CClientSession::SetUserID(const wchar_t *strUserId) {
    m_mutex.lock();
    if (strUserId)
        m_strUserId = strUserId;
    else
        m_strUserId.clear();
    boost::trim(m_strUserId);
    m_mutex.unlock();
}

unsigned int CClientSession::GetUID(wchar_t *strUserId, unsigned int chars) {
    if (strUserId == NULL || chars == 0)
        return 0;
    m_mutex.lock();
    if (chars > (unsigned int) (m_strUserId.size() + 1))
        chars = (unsigned int) (m_strUserId.size() + 1);
    --chars;
    if (chars > 0) {
        ::memcpy(strUserId, m_strUserId.c_str(), sizeof (wchar_t) * chars);
        strUserId[chars] = 0;
    }
    m_mutex.unlock();
    return chars;
}

void CClientSession::SetPassword(const wchar_t *strPassword) {
    std::wstring pwd;
    if (strPassword) {
        pwd = strPassword;
        boost::trim(pwd);
    }
    SPA::CScopeUQueue su;
    SPA::Utilities::ToUTF8(pwd.c_str(), pwd.size(), *su);
    su->SetNull();
    SPA::UINT64 src = (SPA::UINT64)this;
    MQ_FILE::CMyContainer::Container.Set(src, (const char*) su->GetBuffer());
}

SPA::UINT64 CClientSession::GetSocketNativeHandle() {
    m_mutex.lock();
    CSocket *p = GetSocket();
    m_mutex.unlock();
    if (p == NULL)
        return (~0);
    return (SPA::UINT64) p->native();
}

void CClientSession::SetOnEnter(POnEnter p) {
    CAutoLock al(m_mutex);
    m_OnSubscribe = p;
}

void CClientSession::SetOnExit(POnExit p) {
    CAutoLock al(m_mutex);
    m_OnUnsubscribe = p;
}

void CClientSession::SetOnSpeakEx(POnSpeakEx p) {
    CAutoLock al(m_mutex);
    m_OnBroadcastEx = p;
}

void CClientSession::SetOnSpeak(POnSpeak p) {
    CAutoLock al(m_mutex);
    m_OnBroadcast = p;
}

void CClientSession::SetOnSendUserMessageEx(POnSendUserMessageEx p) {
    CAutoLock al(m_mutex);
    m_OnPostUserMessageEx = p;
}

void CClientSession::SetOnSendUserMessage(POnSendUserMessage p) {
    CAutoLock al(m_mutex);
    m_OnPostUserMessage = p;
}

SPA::tagOperationSystem CClientSession::GetPeerOs(bool *endian) {
    CAutoLock al(m_mutex);
    if (endian)
        *endian = m_ResultInfo.IsBigEndian();
    return m_ResultInfo.GetOS();
}

unsigned int CClientSession::CompressRequestTo(unsigned short reqId, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q) {
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
                    sh.OrTreat((unsigned char) (ratio / 256), false, false);
                    sh.SetRatio((unsigned char) (ratio % 256));
                    assert(sh.GetZipRatio() == ratio || sh.GetZipRatio() > SPA::CStreamHeader::ALL_ZIPPED);
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
                    assert(sh.GetZipRatio() == ratio);
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

unsigned int CClientSession::DecompressResultTo(unsigned short ratio, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q) {
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

bool CClientSession::Decompress() {
    unsigned int res;
    bool reset = false;
    bool defaultZipped = m_ResultInfo.IsDefaultZipped();
    bool fastZip = m_ResultInfo.IsFastZipped();

    if (m_ResultInfo.RequestId == SPA::idBatchZipped) {
        if (defaultZipped || fastZip) {
            SPA::CScopeUQueue sb;
            unsigned short ratio = m_ResultInfo.GetZipRatio();
            assert(ratio > 0);
            res = DecompressResultTo(ratio, defaultZipped ? SPA::zlDefault : SPA::zlBestSpeed, m_qRead.GetBuffer(), m_ResultInfo.Size, *sb);
            assert(res > 0);
            m_qRead.Replace(0, m_ResultInfo.Size, sb->GetBuffer(), res);
        }
        m_ResultInfo.Size = 0;
        m_ResultInfo.RequestId = 0;
        RemoveRequestId(SPA::idBatchZipped);
        reset = true;
    } else if (defaultZipped || fastZip) {
        SPA::CScopeUQueue sb;
        unsigned short ratio = m_ResultInfo.GetZipRatio();
        res = DecompressResultTo(ratio, defaultZipped ? SPA::zlDefault : SPA::zlBestSpeed, m_qRead.GetBuffer(), m_ResultInfo.Size, *sb);
        assert(res > 0);
        m_qRead.Replace(0, m_ResultInfo.Size, sb->GetBuffer(), res);
        m_ResultInfo.Size = res;
    }
    return reset;
}

void CClientSession::PostCloseInternal(int error) {
    CAutoLock sl(m_mutex);
    CloseInternal(error);
}

void CClientSession::CloseInternal(int nError) {
    if (m_ConnState == csClosed) {
        return;
    }
    m_tRecv = boost::posix_time::microsec_clock::local_time();
    m_tSend = m_tRecv;
    SocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speClosingSocket, this);
    }

    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = NULL;

    if (IsContextSet()) {
        CErrorCode ec(nError, boost::system::system_category());
        if (IsSslEnabled()) {
			// ????
            //don't call this here. Otherwise, there is thread deadlock from .NET adapter
            //m_pSslSocket->shutdown(ec);
        }
        GetSocket()->shutdown(boost::asio::socket_base::shutdown_both, ec);
        GetSocket()->close(ec);
    }
    m_ConnState = csClosed;
    m_qRead.SetSize(0);
    m_qWrite.SetSize(0);
    m_qReqIdCancel.SetSize(0);
    m_qReqIdWait.SetSize(0);
    m_pCert.reset();
    m_vQTrans.clear();
    m_bConfirmTrans = false;
    m_bConfirmFail = false;
    OnClosed(nError);

    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speSocketClosed, this);
    }
    m_ClientInfo.ServiceId = SPA::sidStartup;
}

void CClientSession::Close() {
    CAutoLock sl(m_mutex);
    CloseInternal();
}

bool CClientSession::GetSockAddr(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars) {
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

bool CClientSession::GetPeerName(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars) {
    CErrorCode ec;
    unsigned short n;
    boost::asio::ip::tcp::endpoint ep;
    CAutoLock sl(m_mutex);
    ep = m_pSocket->remote_endpoint(ec);
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

bool CClientSession::StartQueue(const char *qName, bool secure, bool dequeueShared, unsigned int ttl) {
    CAutoLock sl(m_mutex);
    return StartQueueInternal(qName, secure, dequeueShared, ttl);
}

unsigned int CClientSession::GetConnTimeout() {
    CAutoLock sl(m_mutex);
    return m_nConnTimeout;
}

unsigned int CClientSession::GetRecvTimeout() {
    CAutoLock sl(m_mutex);
    return m_nRecvTimeout;
}

void CClientSession::SetRecvTimeout(unsigned int timeout) {
    if (timeout < 1000)
        timeout = 1000;
    m_mutex.lock();
    m_nRecvTimeout = timeout;
    m_mutex.unlock();
}

void CClientSession::SetConnTimeout(unsigned int timeout) {
    if (timeout < 1000)
        timeout = 1000;
    m_mutex.lock();
    m_nConnTimeout = timeout;
    m_mutex.unlock();
}

void CClientSession::SetAutoConn(bool autoConnecting) {
    m_mutex.lock();
    m_bAutoConn = autoConnecting;
    m_mutex.unlock();
}

bool CClientSession::GetAutoConn() {
    CAutoLock sl(m_mutex);
    return m_bAutoConn;
}

bool CClientSession::Find(const std::string &rawName) {
    CAutoLock sl(m_mutexQLI);
    std::vector<boost::shared_ptr<MQ_FILE::CMqFile> >::iterator it, end = m_vQRequest.end();
    for (it = m_vQRequest.begin(); it != end; ++it) {
        if ((*it)->GetMQFileName().find(rawName) == 0)
            return true;
    }
    return false;
}

std::wstring CClientSession::GetPwd() {
    SPA::UINT64 src = (SPA::UINT64)this;
    std::string str = MQ_FILE::CMyContainer::Container.Get(src);
    SPA::CScopeUQueue su;
    SPA::Utilities::ToWide(str.c_str(), str.size(), *su);
    su->SetNull();
    return (const wchar_t*)su->GetBuffer();
}

bool CClientSession::StartQueueInternal(const char *qName, bool secure, bool dequeueShared, unsigned int ttl) {
    if (m_qRequest || !qName)
        return false;
    secure = (secure || m_EncryptionMethod != SPA::NoEncryption);
    std::wstring pwd = GetPwd();
    if (secure && pwd.size() == 0)
        return false;
    std::string fn(qName);
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute()) {
#ifdef WIN32_64
        fn = CClientSession::m_WorkingPath + fn;
#else
        fn = CClientSession::m_WorkingPath + fn;
#endif
    }

    boost::trim(fn);
    if (fn.size() == 0)
        return false;
#ifdef WIN32_64
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
#endif

    if (Find(fn))
        return false;

    if (secure) {
        std::wstring id = m_strUserId;
        std::transform(id.begin(), id.end(), id.begin(), ::tolower);
        m_qRequest = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFileEx(fn.c_str(), 0, ttl, true, id.c_str(), pwd.c_str(), m_pQLastIndex.get(), true, dequeueShared));
    } else {
        m_qRequest = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFile(fn.c_str(), 0, ttl, true, false, true, dequeueShared));
    }
    if (!m_qRequest->IsAvailable()) {
        m_qRequest.reset();
        return false;
    } else {
        if (m_ClientInfo.ServiceId > SPA::sidStartup && m_ConnState > csConnected)
            SendStartQueueMessage();
        CAutoLock sl(m_mutexQLI);
        m_vQRequest.push_back(m_qRequest);
    }
    return (!!m_qRequest);
}

void CClientSession::SendStopQueueMessage() {
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idStopQueue;
    SPA::CScopeUQueue sb;
    sb << m_qRequest->GetMQInitInfo();
    sh.Size = sb->GetSize();
    sb->Insert((const unsigned char*) &sh, sizeof (sh));
    Write(sb->GetBuffer(), sb->GetSize());
}

void CClientSession::SendStartQueueMessage() {
    if (m_ConnState != csSwitched)
        return;
    SPA::CStreamHeader sh;
    sh.RequestId = SPA::idStartQueue;
    SPA::CScopeUQueue sb;
    sb << m_qRequest->GetMQInitInfo();
    sh.Size = sb->GetSize();
    sb->Insert((const unsigned char*) &sh, sizeof (sh));

    sh.RequestId = SPA::idPing;
    sh.Size = 0;
    sb << sh;
    m_qReqIdWait << sh;
    Write(sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::IsQueueSecured() {
    CAutoLock al(m_mutex);
    return (m_qRequest && m_qRequest->IsSecure());
}

void CClientSession::ResetQueue() {
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable())
        m_qRequest->Reset();
}

SPA::UINT64 CClientSession::RemoveQueuedRequestsByTTL() {
    {
        CAutoLock al(m_mutex);
        if (!m_qRequest || !m_qRequest->IsAvailable())
            return 0;
    }
    return m_qRequest->RemoveByTTL();
}

unsigned int CClientSession::GetTTL() {
    CAutoLock al(m_mutex);
    if (!m_qRequest || !m_qRequest->IsAvailable())
        return 0;
    return m_qRequest->GetTTL();
}

bool CClientSession::IsDequeueShared() {
    CAutoLock al(m_mutex);
    return (m_qRequest && m_qRequest->IsDequeueShared());
}

SPA::UINT64 CClientSession::AppendQueue(boost::shared_ptr<MQ_FILE::CMqFile> q) {
    if (!q || !q->IsAvailable())
        return 0;
    CAutoLock al(m_mutex);
    if (!m_qRequest || !m_qRequest->IsAvailable())
        return 0;
    return m_qRequest->Append(*(q.get()));
}

void CClientSession::SendFromPersistantQueue() {
    CAutoLock al(m_mutex);
    if (m_ConnState > csConnected) {
        WriteFromQueueFile();
        Write(NULL, 0);
    }
}

bool CClientSession::PushQueueTo(const std::vector<CClientSession*> &vClients) {
    SPA::UINT64 count;
    if (vClients.size() == 0)
        return false;
    std::vector<MQ_FILE::CMqFile*> vQ;
    std::vector<CClientSession*>::const_iterator it, end = vClients.end();

    {
        CAutoLock al(m_mutex);
        if (!m_qRequest || !m_qRequest->IsAvailable())
            return false;
        for (it = vClients.begin(); it != end; ++it) {
            vQ.push_back((*it)->m_qRequest.get());
        }
        count = m_qRequest->GetMessageCount();
    }

    if (count) {
        MQ_FILE::CMqFile* &ref = vQ.front();
        if (m_qRequest->AppendTo(&ref, (unsigned int) vQ.size())) {
            for (it = vClients.begin(); it != end; ++it) {
                (*it)->SendFromPersistantQueue();
            }
            return true;
        }
        return false;
    }
    return true;
}

SPA::UINT64 CClientSession::GetJobSize() {
    CAutoLock al(m_mutex);
    return (m_qRequest && m_qRequest->GetJobSize());
}

const char* CClientSession::GetQueueName() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return NULL;
    return m_qRequest->GetRawName().c_str();
}

const char* CClientSession::GetQueueFileName() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return NULL;
    return m_qRequest->GetMQFileName().c_str();
}

bool CClientSession::IsQueueStarted() {
    CAutoLock al(m_mutex);
    return CheckQueueAvailable();
}

unsigned int CClientSession::GetMessagesInDequeuing() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->GetMessagesInDequeuing();
}

unsigned int CClientSession::PeekQueuedRequests(SPA::CQueuedRequestInfo *qri, unsigned int count) {
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable())
        return m_qRequest->PeekRequests(qri, count);
    return 0;
}

unsigned int CClientSession::CancelQueuedRequests(const unsigned short *ids, unsigned int count) {
    if (!ids || !count)
        return 0;
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable()) {
        return m_qRequest->CancelQueuedRequests(ids, count);
    }
    return 0;
}

SPA::UINT64 CClientSession::CancelQueuedRequests(SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable()) {
        return m_qRequest->CancelQueuedRequests(startIndex, endIndex);
    }
    return 0;
}

bool CClientSession::AbortJob() {
    CAutoLock al(m_mutex);
    if (m_pQBatch)
        return false;
    if (m_qRequest && m_qRequest->IsAvailable()) {
        return m_qRequest->AbortJob();
    }
    return false;
}

bool CClientSession::StartJob() {
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable()) {
        WriteFromQueueFile();
        Write(NULL, 0);
        return (m_qRequest->StartJob() != INVALID_NUMBER);
    }
    return false;
}

bool CClientSession::EndJob() {
    CAutoLock al(m_mutex);
    if (m_pQBatch)
        return false;
    if (m_qRequest && m_qRequest->EndJob() != INVALID_NUMBER) {
        WriteFromQueueFile();
        Write(NULL, 0);
        return true;
    }
    return false;
}

SPA::UINT64 CClientSession::GetMessageCount() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->GetMessageCount();
}

SPA::UINT64 CClientSession::GetQueueSize() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->GetMQSize();
}

SPA::UINT64 CClientSession::GetLastQueueMessageTime() {
    CAutoLock al(m_mutex);
    if (!m_qRequest || !m_qRequest->IsAvailable())
        return 0;
    return m_qRequest->GetLastTime();
}

SPA::UINT64 CClientSession::GetQueueLastIndex() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->GetLastMessageIndex();
}

void CClientSession::StopQueue(bool permanent) {
    bool qAvailable = false;
    CAutoLock al(m_mutex);
    qAvailable = CheckQueueAvailable();
    StopQueueInternal(permanent);
    if (qAvailable && m_ConnState >= csConnected && m_ClientInfo.ServiceId != SPA::sidStartup) {
        SendStopQueueMessage();
    }
}

void CClientSession::StopQueueInternal(bool permanent) {
    std::string qFileName;
    if (CheckQueueAvailable()) {
        qFileName = m_qRequest->GetMQFileName();
        CAutoLock al(m_mutexQLI);
        std::vector<boost::shared_ptr<MQ_FILE::CMqFile> >::iterator it = std::find(m_vQRequest.begin(), m_vQRequest.end(), m_qRequest);
        if (it != m_vQRequest.end())
            m_vQRequest.erase(it);
    }
    m_qRequest.reset();
    if (qFileName.size()) {
#ifdef WINCE
        SPA::CScopeUQueue su;
        SPA::Utilities::ToWide(qFileName.c_str(), qFileName.size(), *su);
        DeleteFile((const wchar_t*)su->GetBuffer());
#else
        ::remove(qFileName.c_str());
#endif
    }
}

bool CClientSession::DequeuedResult() {
    m_mutex.lock();
    bool q = m_ResultInfo.GetQueued();
    m_mutex.unlock();
    return q;
}

void CClientSession::Shutdown(nsIP::tcp::socket::shutdown_type nHow) {
    CErrorCode ec;
    CAutoLock sl(m_mutex);
    if (IsContextSet()) {
        do {
            if (!IsSslEnabled())
                break;

			// ????
            //m_pSslSocket->shutdown(ec);
        } while (false);
        GetSocket()->shutdown(nHow, ec);
    }
}

void CClientSession::SetOnSocketClosed(POnSocketClosed p) {
    m_mutex.lock();
    m_OnSocketClosed = p;
    m_mutex.unlock();
}

void CClientSession::SetOnHandShakeCompleted(POnHandShakeCompleted p) {
    m_mutex.lock();
    m_OnHandShakeCompleted = p;
    m_mutex.unlock();
}

void CClientSession::SetOnSocketConnected(POnSocketConnected p) {
    m_mutex.lock();
    m_OnSocketConnected = p;
    m_mutex.unlock();
}

void CClientSession::SetOnRequestProcessed(POnRequestProcessed p) {
    m_mutex.lock();
    m_OnRequestProcessed = p;
    m_mutex.unlock();
}

unsigned short CClientSession::GetCurrentRequestID() {
    m_mutex.lock();
    unsigned short s = m_ResultInfo.RequestId;
    m_mutex.unlock();
    return s;
}

void CClientSession::SetOnServerException(POnServerException p) {
    m_mutex.lock();
    m_OnServerException = p;
    m_mutex.unlock();
}

void CClientSession::SetOnBaseRequestProcessed(POnBaseRequestProcessed p) {
    m_mutex.lock();
    m_OnBaseRequestProcessed = p;
    m_mutex.unlock();
}

void CClientSession::SetOnAllRequestsProcessed(POnAllRequestsProcessed p) {
    m_mutex.lock();
    m_OnAllRequestsProcessed = p;
    m_mutex.unlock();
}

unsigned int CClientSession::GetCurrentResultSize() {
    m_mutex.lock();
    unsigned int size = m_ResultInfo.Size;
    m_mutex.unlock();
    return size;
}

unsigned int CClientSession::RetrieveResultInternal(unsigned char *pBuffer, unsigned int size) {
    if (pBuffer == NULL || size == 0)
        return 0;
    if (size > m_ResultInfo.Size)
        size = m_ResultInfo.Size;
    if (size > 0) {
        size = m_qRead.Pop((unsigned char*) pBuffer, size);
        m_ResultInfo.Size -= size;
    }
    return size;
}

unsigned int CClientSession::RetrieveResult(unsigned char *pBuffer, unsigned int size) {
    CAutoLock sl(m_mutex);
    return RetrieveResultInternal(pBuffer, size);
}

bool CClientSession::RemoveRequestId(unsigned short nRequestId) {
    SPA::CStreamHeader *pId;
    if (m_qReqIdWait.GetSize() < sizeof (SPA::CStreamHeader))
        return false;
    pId = (SPA::CStreamHeader *) m_qReqIdWait.GetBuffer();
    if ((m_ServerInfo.SockMinorVersion & RETURN_RESULT_RANDOM) > 0) {
        unsigned int n, count = m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader);
        for (n = 0; n < count; ++n) {
            if (pId[n].RequestId == nRequestId) {
                m_qReqIdWait.Pop((unsigned int) sizeof (SPA::CStreamHeader), (unsigned int) (n * sizeof (SPA::CStreamHeader)));
                return true;
            }
        }
    } else {
        if (pId->RequestId == nRequestId) {
            m_qReqIdWait.Pop((unsigned int) sizeof (SPA::CStreamHeader)); //remove the request id
            return true;
        }
    }
    return false;
}

bool CClientSession::IsSameEndian() {
    return (m_ResultInfo.IsBigEndian() == SPA::IsBigEndian());
}

void CClientSession::OnChatRequest(unsigned short nRequestId, SPA::CScopeUQueue &sb) {
    unsigned int nCount;
    SPA::ClientSide::CMessageSender sender;
    std::string senderIpAddress;
    std::wstring receiverUserId;
    std::wstring senderUserId;
    sb >> senderIpAddress >> sender.Port >> senderUserId >> sender.ServiceId;
    sender.IpAddress = senderIpAddress.c_str();

#ifdef WCHAR32
    SPA::CScopeUQueue suUTF16;
    if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
        SPA::CUQueue &qUTF16 = *suUTF16;
        SPA::Utilities::ToUTF16(senderUserId.c_str(), (unsigned int) senderUserId.size(), qUTF16);
        sender.UserId = (const wchar_t*)qUTF16.GetBuffer();
    } else
#endif
        sender.UserId = senderUserId.c_str();
    sender.SelfMessage = (sender.Port == GetSocket()->local_endpoint().port() && GetSocket()->local_endpoint().address().to_string() == senderIpAddress);
    switch (nRequestId) {
        case SPA::idEnter:
            nCount = sb->GetSize();
        {
            POnEnter p = m_OnSubscribe;
            if (p != NULL) {
                unsigned int *pGroup = (unsigned int*) sb->GetBuffer();
                nCount /= sizeof (unsigned int);
                CRAutoLock sl(m_mutex);
                p(this, sender, pGroup, nCount);
            }
        }
            break;
        case SPA::idExit:
            nCount = sb->GetSize();
        {
            POnExit p = m_OnUnsubscribe;
            if (p != NULL) {
                unsigned int *pGroup = (unsigned int*) sb->GetBuffer();
                nCount /= sizeof (unsigned int);
                CRAutoLock sl(m_mutex);
                p(this, sender, pGroup, nCount);
            }
        }
            break;
        case SPA::idSendUserMessage:
        {
            POnSendUserMessage p = m_OnPostUserMessage;
            if (p != NULL) {
                CRAutoLock sl(m_mutex);
                p(this, sender, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        case SPA::idSendUserMessageEx:
        {
            POnSendUserMessageEx p = m_OnPostUserMessageEx;
            if (p != NULL) {
                CRAutoLock sl(m_mutex);
                p(this, sender, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        case SPA::idSpeak:
        {
            SPA::UVariant vtMsg;
            sb >> vtMsg >> nCount;
            SPA::CScopeUQueue suMsg;
            suMsg << vtMsg;
            unsigned int *group = (unsigned int *) sb->GetBuffer();
            POnSpeak p = m_OnBroadcast;
            assert(sb->GetSize() == sizeof (unsigned int) *nCount);
            if (p != NULL) {
                CRAutoLock sl(m_mutex);
                p(this, sender, group, nCount, suMsg->GetBuffer(), suMsg->GetSize());
            }
        }
            break;
        case SPA::idSpeakEx:
        {
            unsigned int size;
            sb >> size;
            assert(sb->GetSize() >= size);
            nCount = (sb->GetSize() - size) / sizeof (unsigned int);
            unsigned int *pGroup = (unsigned int*) sb->GetBuffer(size);
            POnSpeakEx p = m_OnBroadcastEx;
            if (p != NULL) {
                CRAutoLock sl(m_mutex);
                p(this, sender, pGroup, nCount, sb->GetBuffer(), size);
            }
        }
            break;
        default:
            assert(false);
            break;
    }
    sb->SetSize(0);
}

void CClientSession::SetVQtrans() {
    bool ok;
    ok = m_qRequest->ConfirmDequeue(&m_vQTrans.front(), m_vQTrans.size(), m_bConfirmFail);
}

boost::shared_ptr<MQ_FILE::CMqFile> CClientSession::GetQueue() {
    CAutoLock sl(m_mutex);
    return m_qRequest;
}

void CClientSession::OnBaseRequestProcessed(unsigned short nRequestId, unsigned int nLen) {
    bool ok;
    SPA::CScopeUQueue sb(m_ResultInfo.GetOS(), m_ResultInfo.IsBigEndian());
    POnServerException se = m_OnServerException;
    POnBaseRequestProcessed brp = m_OnBaseRequestProcessed;
    if (nLen > 0) {
        if (nLen > sb->GetMaxSize())
            sb->ReallocBuffer(nLen);
        nLen = RetrieveResultInternal((unsigned char*) sb->GetBuffer(), nLen);
        sb->SetSize(nLen);
    }
    sb->SetNull();
    switch (nRequestId) {
        case SPA::idRouteeChanged:
            sb >> m_nRouteeCount;
            break;
        case SPA::idSetZipLevelAtSvr:
        case SPA::idTurnOnZipAtSvr:
        case SPA::idGetSockOptAtSvr:
        case SPA::idSetSockOptAtSvr:
            sb->SetSize(0);
            break;
        case SPA::idStopQueue:
            sb >> m_ServerQFile;
            m_pQLastIndex->Remove(m_ServerQFile.Qs);
            break;
        case SPA::idStartQueue:
            sb >> m_ServerQFile;
            break;
        case SPA::idEnableClientDequeue:
        {
            bool enableDequeue;
            sb >> enableDequeue;
            if (CheckQueueAvailable()) {
                m_qRequest->EnableDequeue(enableDequeue);
            }
        }
            break;
        case SPA::idDequeueConfirmed:
        {
            unsigned short reqId;
            MQ_FILE::CDequeueConfirmInfo dci;
            assert(sb->GetSize() == sizeof (MQ_FILE::CDequeueConfirmInfo));
            sb >> dci;
            m_qa = dci.QA;
            m_bConfirmFail = dci.Fail;
            reqId = dci.RequestId;
            bool bQueue = CheckQueueAvailable();
            if (bQueue) {
                switch (reqId) {
                    case SPA::idStartJob:
                        assert(m_vQTrans.size() == 0);
                        assert(!m_bConfirmTrans);
                        m_vQTrans.push_back(m_qa);
                        m_bConfirmTrans = true;
                        nRequestId = SPA::idStartJob;
                        RemoveRequestId(nRequestId);
                        break;
                    case SPA::idEndJob:
                        assert(m_bConfirmTrans);
                        m_vQTrans.push_back(m_qa);
                        SetVQtrans();
                        m_bConfirmTrans = false;
                        m_vQTrans.clear();
                        nRequestId = SPA::idEndJob;
                        RemoveRequestId(nRequestId);

                        if (brp && !m_bConfirmFail && m_qRequest->GetMessageCount() == 0) {
                            CRAutoLock sl(m_mutex);
                            brp(this, SPA::idAllMessagesDequeued);
                        }

                        //std::cout << "Dequeue commit trans index = " << m_qa.MessageIndex << std::endl;
                        if (m_ConnState >= csSwitched) {
                            WriteFromQueueFile();
                        }
                        Write(NULL, 0);

                        break;
                    default:
                        if (m_bConfirmTrans) {
                            m_vQTrans.push_back(m_qa);
                        } else {
                            bool fail = m_bConfirmFail;
                            SPA::UINT64 pos = m_qa.MessagePos;
                            SPA::UINT64 idx = m_qa.MessageIndex;
                            {
                                CRAutoLock rsl(m_mutex);
                                ok = m_qRequest->ConfirmDequeue(pos, idx, fail);
                            }

                            if (brp != NULL && !fail && m_qRequest.get() != NULL && m_qRequest->IsAvailable() && m_qRequest->GetMessageCount() == 0) {
                                CRAutoLock sl(m_mutex);
                                brp(this, SPA::idAllMessagesDequeued);
                            }
                        }
                        break;
                }
            }
        }
            return; //don't do callback for this base request
            break;
        case SPA::idReservedOne:
        {
            unsigned int errCode;
            sb >> errCode;
            sb >> m_ServerInfo;
            nRequestId = SPA::idSwitchTo;
        }
            break;
        case SPA::idSwitchTo:
        {
            unsigned int errCode;
            sb >> errCode;
            sb >> m_ServerInfo;
            m_nRouteeCount = m_ServerInfo.Param0;
            m_ClientInfo.ServiceId = m_ServerInfo.ServiceId;
            m_ConnState = csSwitched;
            if (CheckQueueAvailable())
                SendStartQueueMessage();
        }
            break;
        case SPA::idServerException:
        {
            unsigned short requestId;
            sb >> requestId;
            if (se) {
                std::wstring errMsg;
                std::string errWhere;
                unsigned int errCode;
                sb >> errMsg;
                sb >> errCode;
                sb >> errWhere;
                CRAutoLock sl(m_mutex);
                se(this, requestId, errMsg.c_str(), errWhere.c_str(), errCode);
            } else
                sb->SetSize(0);
            RemoveRequestId(requestId);
        }
            break;
        case SPA::idCancel:
            //ignore the number of requests canceled
            sb->SetSize(0);
        {
            SPA::CStreamHeader *pStreamHeader;
            while (m_qReqIdWait.GetSize() >= sizeof (SPA::CStreamHeader)) {
                pStreamHeader = (SPA::CStreamHeader *)m_qReqIdWait.GetBuffer();
                m_qReqIdWait.Pop(sizeof (SPA::CStreamHeader));
                if (pStreamHeader->RequestId == SPA::idCancel)
                    break;
            }
            while (m_qReqIdCancel.GetSize() >= sizeof (SPA::CStreamHeader)) {
                pStreamHeader = (SPA::CStreamHeader *)m_qReqIdCancel.GetBuffer();
                m_qReqIdCancel.Pop(sizeof (SPA::CStreamHeader));
                if (pStreamHeader->RequestId == SPA::idCancel)
                    break;
            }
        }
            break;
        case SPA::idEnter:
        case SPA::idSpeakEx:
        case SPA::idSendUserMessageEx:
        case SPA::idExit:
        case SPA::idSpeak:
        case SPA::idSendUserMessage:
            OnChatRequest(nRequestId, sb);
            return;
            break;
        default:
            break;
    }
    if (brp) {
        CRAutoLock sl(m_mutex);
        brp(this, nRequestId);
    }
    assert(sb->GetSize() == 0);
}

void CClientSession::OnRequestProcessed(unsigned short nRequestId, unsigned int nLen) {
    POnRequestProcessed p = m_OnRequestProcessed;
    if (p != NULL) {
        CRAutoLock sl(m_mutex);
        p(this, nRequestId, nLen);
    }
}

void CClientSession::SetPeerDequeueFailed(bool fail) {
    m_mutex.lock();
    m_bFail = fail;
    m_mutex.unlock();
}

bool CClientSession::GetPeerDequeueFailed() {
    CAutoLock al(m_mutex);
    return m_bFail;
}

bool CClientSession::IsRouteeRequest() {
    CAutoLock al(m_mutex);
    return (m_RouterHandle > 0);
}

void CClientSession::NotifyDequeued(unsigned int qHandle) {
    SPA::CScopeUQueue su;
    if (m_RouterHandle) {
        MQ_FILE::CDequeueConfirmInfo dci(m_qa, m_bFail, m_ResultInfo.RequestId);
        su << dci;
        SendRoutingResultInternal(SPA::idDequeueConfirmed, su->GetBuffer(), su->GetSize());
    } else {
        SPA::CStreamHeader sh;
        sh.RequestId = SPA::idDequeueConfirmed;
        MQ_FILE::CDequeueConfirmInfo dci(qHandle, m_qa, m_bFail, m_ResultInfo.RequestId);
        sh.Size = sizeof (dci);
        su << sh << dci;
        m_qReqIdCancel << sh;
        Write(su->GetBuffer(), su->GetSize());
    }
}

void CClientSession::NotifyDequeuedStartQueueTrans(unsigned int qHandle) {

    POnBaseRequestProcessed p = m_OnBaseRequestProcessed;
    if (p != NULL) {
        CRAutoLock sl(m_mutex);
        p(this, SPA::idStartJob);
    }
    NotifyDequeued(qHandle);
}

void CClientSession::NotifyDequeuedCommitQueueTrans(unsigned int qHandle) {
    POnBaseRequestProcessed p = m_OnBaseRequestProcessed;
    if (p != NULL) {
        CRAutoLock sl(m_mutex);
        p(this, SPA::idEndJob);
    }
    NotifyDequeued(qHandle);
}

void CClientSession::OnReadCompleted(const CErrorCode& Error, size_t nLen) {
    unsigned short sReqId;
    unsigned int len = (unsigned int) nLen;
    CAutoLock sl(m_mutex);
    if (m_ConnState < csConnected) {
        return;
    }
    if (len > 0) {
        m_ulRead += len;
        m_qRead.Push(m_ReadBuffer, len);
        m_tRecv = boost::posix_time::microsec_clock::local_time();
    }
    m_ec = Error;
    if (Error) {
        CloseInternal(Error.value());
        return;
    }
    m_bRBLocked = false;
    bool b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader) || m_ResultInfo.RequestId != 0);
    while (b) {
        if (m_ResultInfo.RequestId == 0) {
            m_qRead >> m_ResultInfo;
            m_qRead.SetOS(m_ResultInfo.GetOS());
        }
        b = (m_qRead.GetSize() >= m_ResultInfo.Size);
        if (!b)
            break;

        sReqId = m_ResultInfo.RequestId;
        if (sReqId == SPA::idRoutingData) {
            m_qRead >> m_RouterHandle;
            m_qRead >> m_ResultInfo;
            assert(m_qRead.GetSize() >= m_ResultInfo.Size);
            sReqId = m_ResultInfo.RequestId;
            if (m_ResultInfo.GetQueued()) {
                m_qRead >> m_qa;
                m_ResultInfo.Size -= sizeof (m_qa);
            }
            Decompress();
        } else {
            m_RouterHandle = 0;
            if (Decompress()) {
                continue;
            }
        }

        bool queued = m_ResultInfo.GetQueued();
        if (b) {
            if (sReqId < SPA::idReservedTwo && !queued)
                OnBaseRequestProcessed(sReqId, m_ResultInfo.Size);
            else {
                unsigned int qHandle = 0;
                if (queued) {
                    MQ_FILE::QAttr seek;
                    if (m_RouterHandle == 0) {
                        m_qRead >> qHandle >> m_qa;
                        m_ResultInfo.Size -= (sizeof (qHandle) + sizeof (m_qa));
                        seek = m_pQLastIndex->Seek(m_ServerQFile.Qs);
                    }

                    //check if previous message index is already dequeued because of sudden socket disconnection, exception and others
                    if (seek.MessageIndex != INVALID_NUMBER &&
                            seek.MessageIndex >= m_qa.MessageIndex &&
                            seek.MessagePos != INVALID_NUMBER &&
                            seek.MessagePos >= m_qa.MessagePos && m_qa.MessageIndex > 1) {
                        m_bFail = false;
                        NotifyDequeued(qHandle);
                        m_qRead.Pop(m_ResultInfo.Size);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        continue;
                    } else if (m_ResultInfo.RequestId == SPA::idStartJob) {
                        m_bDequeueTrans = true;
                        m_bFail = false;
                        NotifyDequeuedStartQueueTrans(qHandle);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        continue;
                    } else if (m_ResultInfo.RequestId == SPA::idEndJob) {
                        NotifyDequeuedCommitQueueTrans(qHandle);
                        m_bDequeueTrans = false;
                        m_qRead.Pop(m_ResultInfo.Size);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        if (!m_bFail && !m_RouterHandle)
                            m_pQLastIndex->Set(m_ServerQFile.Qs, m_qa);
                        continue;
                    } else if (!m_bDequeueTrans) {
                        m_bFail = false;
                    }

                }
                OnRequestProcessed(sReqId, m_ResultInfo.Size);
                if (queued) {
                    if ((!m_bFail && !m_bDequeueTrans || (m_ResultInfo.RequestId == SPA::idStartJob && m_qa.MessageIndex == 1)) && !m_RouterHandle)
                        m_pQLastIndex->Set(m_ServerQFile.Qs, m_qa);
                    NotifyDequeued(qHandle);
                }
            }
        } else
            break;
        RemoveRequestId(sReqId);
        if (m_ResultInfo.Size > 0)
            m_qRead.Pop(m_ResultInfo.Size);
        ::memset(&m_ResultInfo, 0, sizeof (m_ResultInfo));
        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
    }
    Read();
    if (!m_qReqIdWait.GetSize()) {
        m_qReqIdCancel.SetSize(0);
        POnAllRequestsProcessed p = m_OnAllRequestsProcessed;
        CRAutoLock rsl(m_mutex);
        if (p)
            p(this, sReqId);
        m_cv.notify_all();
    }
}

void CClientSession::OnWriteCompleted(const CErrorCode& Error) {
    CAutoLock sl(m_mutex);
    if (m_ConnState < csConnected) {
        return;
    }
    if (!Error) {
        m_ec.clear();
        m_tSend = boost::posix_time::microsec_clock::local_time();
        m_bWBLocked = false;
        bool bQueue = CheckQueueAvailable();
        if (bQueue && m_ConnState >= csSwitched) {
            WriteFromQueueFile();
        }
        Write(NULL, 0);
    } else {
        m_ec = Error;
        CloseInternal(Error.value());
    }
}

bool CClientSession::DoEcho() {
    CAutoLock sl(m_mutex);
    return SendRequestInternal(SPA::idDoEcho, NULL, 0);
}

bool CClientSession::SetSockOpt(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    int name = SPA::MapSockOption(optName);
    int lvl = SPA::MapSockLevel(level);
    CAutoLock sl(m_mutex);
    if (m_ConnState < csConnected)
        return false;
    int res = ::setsockopt(GetSocket()->native_handle(), lvl, name, (const char*) &optValue, sizeof (optValue));
    return (res == 0);
}

bool CClientSession::SetSockOptAtSvr(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    SPA::CScopeUQueue su;
    su << (int) optName << optValue << (int) level;
    CAutoLock sl(m_mutex);
    return SendRequestInternal(SPA::idSetSockOptAtSvr, su->GetBuffer(), su->GetSize());
}

bool CClientSession::TurnOnZipAtSvr(bool enableZip) {
    CAutoLock sl(m_mutex);
    return SendRequestInternal(SPA::idTurnOnZipAtSvr, &enableZip, sizeof (enableZip));
}

bool CClientSession::SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) {
    int level = zipLevel;
    CAutoLock sl(m_mutex);
    return SendRequestInternal(SPA::idSetZipLevelAtSvr, &level, sizeof (level));
}

unsigned int CClientSession::GetRouteeCount() {
    CAutoLock sl(m_mutex);
    return m_nRouteeCount;
}