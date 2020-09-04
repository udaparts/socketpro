#include "stdafx.h"
#include "clientsession.h"
#include <ctype.h>
#include "../../pinc/uzip.h"
#include "../../pinc/getsysid.h"
#include <boost/algorithm/string/trim.hpp>
#include "../ClientCore/socketpool.h"
#include <assert.h>
#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

extern std::string g_localhost;

boost::posix_time::ptime CClientSession::m_tStart = boost::posix_time::microsec_clock::local_time();

std::vector<unsigned char*> CClientSession::m_aBuffer;
boost::mutex CClientSession::m_mutexBuffer;
std::string CClientSession::m_WorkingPath("");
extern boost::mutex g_mutexCvc;
extern PCertificateVerifyCallback g_cvc;

unsigned char* CClientSession::GetIoBuffer() {
    unsigned char *s = nullptr;
    {
        CAutoLock sl(m_mutexBuffer);
        size_t size = m_aBuffer.size();
        if (size) {
            s = m_aBuffer[size - 1];
            m_aBuffer.pop_back();
        }
    }
    if (s == nullptr)
        s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + 16);
    return s;
}

void CClientSession::ReleaseIoBuffer(unsigned char *buffer) {
    if (buffer == nullptr)
        return;
    m_mutexBuffer.lock();
    m_aBuffer.push_back(buffer);
    m_mutexBuffer.unlock();
}

boost::mutex CClientSession::m_mutexQLI;
std::vector<boost::shared_ptr<MQ_FILE::CMqFile> > CClientSession::m_vQRequest;
boost::shared_ptr<MQ_FILE::CQLastIndex> CClientSession::m_pQLastIndex;

CClientSession::CClientSession(CIoService &IoService, CClientThread *pClientThread)
:
#ifndef NDEBUG
m_nJobRequest(0),
m_nJobConfirm(0),
#endif
m_qRead(INIT_BUFFER_SIZE), m_qWrite(INIT_BUFFER_SIZE, BUFFER_BLOCK_SIZE),
m_qReqIdWait(INIT_BUFFER_SIZE, BUFFER_BLOCK_SIZE), m_qReqIdCancel(INIT_BUFFER_SIZE, BUFFER_BLOCK_SIZE),
m_pIoService(&IoService), m_pSocket(nullptr), m_ulRead(0), m_ulSent(0),
m_ReadBuffer(GetIoBuffer()), m_bRBLocked(false), m_WriteBuffer(GetIoBuffer()), m_bWBLocked(0),
m_zl(SPA::zlDefault), m_pQBatch(nullptr), m_bZip(false), m_EncryptionMethod(SPA::NoEncryption),
m_Resolver(IoService), m_nPort(0), m_OnSocketClosed(nullptr), m_OnHandShakeCompleted(nullptr),
m_OnSocketConnected(nullptr), m_OnRequestProcessed(nullptr), m_pThread(pClientThread),
m_ConnState(SPA::ClientSide::csClosed), m_nConnTimeout(SPA::ClientSide::DEFAULT_CONN_TIMEOUT),
m_nRecvTimeout(SPA::ClientSide::DEFAULT_RECV_TIMEOUT), m_nCancel(0), m_OnSubscribe(nullptr), m_OnUnsubscribe(nullptr),
m_OnBroadcastEx(nullptr), m_OnBroadcast(nullptr), m_OnPostUserMessageEx(nullptr), m_OnPostUserMessage(nullptr),
m_OnServerException(nullptr), m_OnBaseRequestProcessed(nullptr), m_OnAllRequestsProcessed(nullptr), m_bAutoConn(false),
m_nPoolId(m_pThread->GetPool()->GetPoolId()), m_bFail(false), m_bDequeueTrans(false), m_bConfirmTrans(false),
m_bConfirmFail(false), m_RouterHandle(0), m_nRouteeCount(0), m_bRegistered(true), m_b6(false),
m_bSync(false), m_routeeNotAvailable(0), m_bRoutingQueueIndexEnabled(false), m_bRoutingWait(false),
m_bSendWaiting(false), m_bWaiting(false), m_pCertContext(nullptr), m_bLastDequeue(false), m_nRcvBufferSize(0),
m_OnSubscribe2(nullptr), m_OnUnsubscribe2(nullptr), m_OnBroadcastEx2(nullptr), m_OnBroadcast2(nullptr),
m_OnPostUserMessageEx2(nullptr), m_OnPostUserMessage2(nullptr), m_to(nullptr), m_OnPostProcessing(nullptr) {
    m_tRecv = GetTimeTick();
    m_tSend = m_tRecv;
    PSocketPoolCallback spc = pClientThread->GetSocketPoolCallback();
    if (spc) {
        spc(m_nPoolId, SPA::ClientSide::speUSocketCreated, this);
    }
    ::memset(&m_hCreds, 0, sizeof (m_hCreds));
}

CClientSession::~CClientSession() {
    CAutoLock sl(m_mutex);
    FreeCredHandle();
    if (m_pCertContext) {
        ::CertFreeCertificateContext(m_pCertContext);
        m_pCertContext = nullptr;
    }
    if (IsContextSet()) {
        CErrorCode ec(0, boost::system::system_category());
        if (IsSslEnabled()) {

        }
        GetSocket()->shutdown(boost::asio::socket_base::shutdown_both, ec);
        GetSocket()->close(ec);
    }
    delete m_pSocket;
    m_pSocket = nullptr;
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    ReleaseIoBuffer(m_ReadBuffer);
    ReleaseIoBuffer(m_WriteBuffer);
    StopQueueInternal(false);
    PSocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
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
    return (m_pSocket != nullptr);
}

bool ExecuteSpc(unsigned int id, CClientSession *session, PSocketPoolCallback spc) {
    if (spc) {
#ifdef WIN32_64
        __try{
            //there is unwinded exception here
            spc(id, SPA::ClientSide::speTimer, session);
        }

        __except(EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
#else
        spc(id, SPA::ClientSide::speTimer, session);
#endif
    }
    return true;
}

void CClientSession::TimerHandler() {
    if (!ExecuteSpc(m_nPoolId, this, m_pThread->GetSocketPoolCallback()))
        return;
#ifdef WIN32_64
    int errorCode = WSAETIMEDOUT;
#else
    int errorCode = SO_RCVTIMEO;
#endif
    SPA::UINT64 now = GetTimeTick();
    SPA::UINT64 lastOne = ((m_tRecv > m_tSend) ? m_tRecv : m_tSend);
    CAutoLock al(m_mutex);
    switch (m_ConnState) {
            //case SPA::ClientSide::csSslShaking:
        case SPA::ClientSide::csConnecting:
            if (now > lastOne + m_nConnTimeout) {
                m_ec.assign(errorCode, boost::asio::error::get_system_category());
                m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
            }
            break;
        case SPA::ClientSide::csSwitched:
        {
            Read();
            WriteFromQueueFile();
            Write(nullptr, 0);
        }
        case SPA::ClientSide::csConnected:
            if (m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader) > 0 && (now > lastOne + m_nRecvTimeout)) {
                m_ec.assign(errorCode, boost::asio::error::get_system_category());
                m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
            } else if (now > lastOne + (unsigned int) GetServerPingTimeInternal() * 1000 + m_pThread->GetTimerInterval()) {
                if (m_qReqIdWait.GetSize() == 0 || (m_qRequest && m_qRequest->GetMessageCount() > 0)) {
#ifdef WIN32_64
                    errorCode = WSAECONNRESET;
#else
                    errorCode = ECONNREFUSED;
#endif
                    m_ec.assign(errorCode, boost::asio::error::get_system_category());
                    m_pIoService->post(boost::bind(&CClientSession::PostCloseInternal, this, errorCode));
                }
            } else if (!m_bRegistered) {
                ::srand((unsigned int) time(nullptr));
                int rc = ::rand();
                if ((rc % 120) == 1) {
#ifdef WIN32_64
                    ::MessageBoxW(nullptr, L"Thank you very much for evaluation on SocketPro!\r\n\r\nPlease go to www.udaparts.com for registration.", L"UDAParts", MB_ICONINFORMATION);
#else
                    std::cout << "Thank you very much for evaluation on SocketPro!" << std::endl;
                    std::cout << "Please go to www.udaparts.com for registration." << std::endl;
                    boost::this_thread::sleep(boost::posix_time::milliseconds(rc));
#endif
                }
            }
            break;
        case SPA::ClientSide::csClosed:
            if (m_bAutoConn && m_strhost.size() > 0 && m_nPort > 0 && now > (lastOne + 350)) {
                CloseInternal();
                m_ConnState = SPA::ClientSide::csConnecting;
                m_tRecv = GetTimeTick();
                m_tSend = m_tRecv;
                m_pIoService->post(boost::bind(&CClientSession::ConnectInternally, this));
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
    return (m_qRequest && m_qRequest->IsDequeueOk());
}

void CClientSession::OnClosed(int errCode) {
    if (CheckQueueAvailable()) {
        DoConfirmDequeue();
        m_qRequest->ReleaseMessageAttributesInDequeuing();
        m_qRequest->Notify();
    }
    m_bZip = false;
    POnSocketClosed p = m_OnSocketClosed;
    {
        CRAutoLock sl(m_mutex);
        if (p != nullptr) {
            p(this, errCode);
        }
        m_pThread->GetPool()->Notify();
    }
    if (m_bWaiting)
        m_cv.notify_all();
}

CSocketPool* CClientSession::GetSocketPool() {
    if (m_pThread != nullptr)
        return m_pThread->GetPool();
    return nullptr;
}

void* CClientSession::GetSSL() {
    CAutoLock al(m_mutex);
    if (m_pSspi)
        return m_pSspi->GetCtxHandle();
    return nullptr;
}

SPA::CCertificateImpl* CClientSession::GetUCert() {
    CAutoLock al(m_mutex);
    return m_pCert.get();
}

bool CClientSession::SendInterruptRequest(SPA::UINT64 options) {
    SPA::CStreamHeader reqInfo;
    reqInfo.RequestId = SPA::idInterrupt;
    reqInfo.Size = sizeof (options);
    CAutoLock sl(m_mutex);
    if (m_ConnState < SPA::ClientSide::csSwitched) {
        return false;
    }
    m_qReqIdCancel << reqInfo;
    Write(reqInfo, (const unsigned char*) &options, sizeof (options));
    return true;
}

bool CClientSession::Cancel(unsigned int requestsQueued) {
    CAutoLock sl(m_mutex);
    if (m_pQBatch) {
        return false;
    }
    if (CheckQueueAvailable()) {
        if (m_qRequest->Empty() == INVALID_NUMBER) {
            return false;
        }
    }
    m_nCancel = 0;
    unsigned int count = m_qReqIdCancel.GetSize() / sizeof (SPA::CStreamHeader);
    for (unsigned int n = count - 1; n != ((unsigned int) (~0)); --n) {
        if (m_qWrite.GetSize() < sizeof (SPA::CStreamHeader))
            break;
        SPA::CStreamHeader *pStreamHeader = (SPA::CStreamHeader*)m_qReqIdCancel.GetBuffer(n * sizeof (SPA::CStreamHeader));
        bool stopped = false;
        switch (pStreamHeader->RequestId) {
            case SPA::idInterrupt:
            case SPA::idCancel:
            case SPA::idDequeueConfirmed:
            case SPA::idDequeueBatchConfirmed:
            case SPA::idRoutingData:
            case SPA::idStartBatching:
            case SPA::idCommitBatching:
            case SPA::idStopQueue:
                stopped = true;
                break;
            default:
                break;
        }
        if (stopped) {
            break;
        }
        unsigned int total = pStreamHeader->Size + sizeof (SPA::CStreamHeader);
        if (total <= m_qWrite.GetSize()) {
            m_qWrite.SetSize(m_qWrite.GetSize() - total);
            m_qReqIdCancel.SetSize(m_qReqIdCancel.GetSize() - sizeof (SPA::CStreamHeader));
            ++m_nCancel;
        } else {
            break;
        }
    }
    requestsQueued = (~0);
    SPA::CStreamHeader sh;
    sh.Size = sizeof (requestsQueued);
    sh.RequestId = SPA::idCancel;
    m_qReqIdWait << sh;
    m_qReqIdCancel << sh;
    Write(sh, (unsigned char*) &requestsQueued, sizeof (requestsQueued));
    return true;
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
    //CAutoLock sl(m_mutex);
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
    bool b = (m_pQBatch != nullptr || m_ConnState < SPA::ClientSide::csConnected);
    if (b)
        return false;
    b = (m_qReqIdWait.GetSize() == 0);
    if (b) {
        m_qReqIdCancel.SetSize(0);
        return true;
    }
    if (IsSameThread()) {
        if (m_bSendWaiting)
            return false;
        boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time() + boost::posix_time::milliseconds(nTimeout);
        do {
            {
                CRAutoLock rl(m_mutex);
                m_pIoService->poll(m_ec);
            }
            if (m_ConnState < SPA::ClientSide::csConnected)
                break;
            if (m_qReqIdWait.GetSize() == 0)
                break;
            if (end <= boost::posix_time::microsec_clock::local_time())
                break;
            b = true;
        } while (true);
        return (m_ConnState >= SPA::ClientSide::csConnected && m_qReqIdWait.GetSize() == 0);
    }
    boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(nTimeout);
    do {
        m_bWaiting = true;
        b = (m_cv.timed_wait(sl, td) && m_ConnState >= SPA::ClientSide::csConnected);
        m_bWaiting = false;
    } while (b && m_qReqIdWait.GetSize() > 0);
    return b;
}

bool CClientSession::WaitAll(unsigned int nTimeout) {
    m_mutex.lock();
    bool routing = (m_RouterHandle > 0);
    m_mutex.unlock();
    if (routing) {
        bool ok = true;
        CErrorCode ec;
        m_bRoutingWait = true;
        do {
            size_t size = m_pIoService->poll(ec);
            if (ec) {
                CAutoLock sl(m_mutex);
                m_ec = ec;
                ok = false;
                break;
            } else {
                CAutoLock sl(m_mutex);
                if (m_routeeNotAvailable == m_RouterHandle) {
                    ok = false;
                    break;
                }
                if (m_ConnState < SPA::ClientSide::csConnected) {
                    ok = false;
                    break;
                }
                if (m_qWrite.GetSize() <= IO_BUFFER_SIZE)
                    break;
            }
        } while (true);
        m_bRoutingWait = false;
        return ok;
    }
    CAutoLock sl(m_mutex);
    m_bRoutingWait = false;
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
    unsigned int len = m_nRcvBufferSize;
    m_mutex.unlock();
    return len;
}

bool CClientSession::IsBatching() {
    bool b;
    m_mutex.lock();
    if (m_pQBatch && m_ConnState >= SPA::ClientSide::csSwitched)
        b = true;
    else
        b = false;
    m_mutex.unlock();
    return b;
}

unsigned int CClientSession::GetBytesBatched() {
    m_mutex.lock();
    unsigned int len = 0;
    if (m_pQBatch != nullptr)
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
#ifndef NDEBUG
    m_nBalance = 0;
    if (m_qRequest && m_qRequest->IsAvailable())
        m_nBalance = m_qRequest->GetMessageCount();
#endif
    ::memset(&m_ResultInfo, 0, sizeof (m_ResultInfo));
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
    POnSocketConnected p = m_OnSocketConnected;
    m_nRcvBufferSize = 0;
    m_ulRead = 0;
    m_ulSent = 0;
    if (errCode == 0)
        m_ConnState = SPA::ClientSide::csConnected;
    m_RouterHandle = 0;
    m_nRouteeCount = 0;
    m_bRoutingQueueIndexEnabled = false;
    m_qBatchDequeueConfirm.SetSize(0);
    PSocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    {
        CRAutoLock sl(m_mutex);
        if (p) {
            p(this, errCode);
        }
        if (spc) {
            spc(m_nPoolId, SPA::ClientSide::speConnected, this);
        }
        if (errCode == 0) {
            m_pThread->GetPool()->Notify();
        }
    }
    if (errCode == 0 && m_bWaiting) {
        m_cv.notify_all();
    }
    m_routeeNotAvailable = 0;
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
    bool b = (m_ConnState >= SPA::ClientSide::csConnected);
    return b;
}

CSocket* CClientSession::GetSocket() {
    return m_pSocket;
}

bool CClientSession::StartBatching() {
    bool b = false;
    m_mutex.lock();
    do {
        if (m_pQBatch != nullptr || ((m_ConnState < SPA::ClientSide::csConnected) && !CheckQueueAvailable()))
            break;
        if (m_qRequest && m_qRequest->GetJobSize())
            break;
        b = true;
        m_pQBatch = SPA::CScopeUQueue::Lock();
        SPA::CStreamHeader reqInfo;
        reqInfo.RequestId = SPA::idStartBatching;
        *m_pQBatch << reqInfo;
    } while (false);
    m_mutex.unlock();
    return b;
}

bool CClientSession::Enter(const unsigned int *pChatGroupId, unsigned int nCount) {
    if (pChatGroupId == nullptr)
        nCount = 0;
    unsigned short vt = (VT_UINT | VT_ARRAY);
    SPA::CScopeUQueue sb;
    sb << vt;
    sb << nCount;
    sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, (unsigned short) SPA::idEnter, sb->GetBuffer(), sb->GetSize());
}

void CClientSession::Exit() {
    CAutoLock sl(m_mutex);
    SendRequestInternal(sl, (unsigned short) SPA::idExit, nullptr, 0);
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
    return SendRequestInternal(sl, (unsigned short) SPA::idSpeakEx, sb->GetBuffer(), sb->GetSize());
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
    return SendRequestInternal(sl, (unsigned short) SPA::idSpeak, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == nullptr)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, (unsigned short) SPA::idSendUserMessage, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) {
    if (userId == nullptr)
        return false;
    SPA::CScopeUQueue sb;
    sb << userId;
    sb->Push(message, size);
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, (unsigned short) SPA::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
}

bool CClientSession::SwitchToIntenal(CAutoLock &al, unsigned int serviceId) {
    SPA::CScopeUQueue sb;
    if (m_ConnState < SPA::ClientSide::csConnected)
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
    bool ok = SendRequestInternal(al, (unsigned short) SPA::idSwitchTo, sb->GetBuffer(), sb->GetSize());
    return ok;
}

bool CClientSession::SwitchTo(unsigned int serviceId) {
    if (serviceId < SPA::sidReserved &&
            serviceId != SPA::sidStartup &&
            serviceId != SPA::sidChat &&
            serviceId != SPA::sidFile &&
            serviceId != SPA::sidODBC)
        return false;
    CAutoLock sl(m_mutex);
    return SwitchToIntenal(sl, serviceId);
}

bool CClientSession::CommitBatching(bool bBatchingAtServerSide) {
    bool b = true;
    SPA::CUQueue *pBatch = nullptr;
    CAutoLock sl(m_mutex);
    bool bQueue = CheckQueueAvailable();
    do {
        if (m_pQBatch == nullptr || ((m_ConnState < SPA::ClientSide::csConnected) && !bQueue)) {
            b = false;
            break;
        }
        if (m_pQBatch->GetSize() <= sizeof (SPA::CStreamHeader)) {
            SPA::CScopeUQueue::Unlock(m_pQBatch);
            m_pQBatch = nullptr;
            break;
        }
        if (bBatchingAtServerSide) {
            SPA::CStreamHeader reqInfo;
            reqInfo.RequestId = SPA::idCommitBatching;
            *m_pQBatch << reqInfo;
        } else {
            m_pQBatch->Pop((unsigned int) sizeof (SPA::CStreamHeader));
        }
        pBatch = m_pQBatch;
        m_pQBatch = nullptr;
    } while (false);

    if (pBatch != nullptr) {
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
                    m_qRequest->Enqueue(*sh, *sb);
                    if (m_ConnState == SPA::ClientSide::csClosed) {
                        CRAutoLock ral(m_mutex);
                        m_pThread->GetPool()->OnClose(this);
                    }
                }
                if (m_ConnState > SPA::ClientSide::csConnected) {
                    WriteFromQueueFile();
                    Write(nullptr, 0);
                }
            } else {
                if (m_qWrite.GetTailSize() < len + sizeof (SPA::CStreamHeader) && m_qWrite.GetHeadPosition() > len + sizeof (SPA::CStreamHeader))
                    m_qWrite.SetHeadPosition();
                if (!m_pSspi) {
                    len = CompressRequestTo(SPA::idBatchZipped, m_zl, pBatch->GetBuffer(), pBatch->GetSize(), m_qWrite);
                    m_qReqIdCancel.Push(m_qWrite.GetBuffer(), sizeof (SPA::CStreamHeader));
                    Write(nullptr, 0);
                } else {
                    SPA::CScopeUQueue sb;
                    len = CompressRequestTo(SPA::idBatchZipped, m_zl, pBatch->GetBuffer(), pBatch->GetSize(), *sb);
                    m_qReqIdCancel.Push(sb->GetBuffer(), sizeof (SPA::CStreamHeader));
                    Write(sb->GetBuffer(), sb->GetSize());
                }
            }
        } else {
            if (m_pSspi) {
                if (bQueue && m_ConnState > SPA::ClientSide::csConnected) {
                    m_qRequest->BatchEnqueue(*pBatch);
                    WriteFromQueueFile();
                    Write(nullptr, 0);
                } else {
                    Write(pBatch->GetBuffer(), pBatch->GetSize());
                }
            } else {
                if (bQueue && m_ConnState > SPA::ClientSide::csConnected) {
                    m_qRequest->BatchEnqueue(*pBatch);
                    WriteFromQueueFile();
                    Write(nullptr, 0);
                } else if (m_qWrite.GetSize() == 0) {
                    m_qWrite.Swap(*pBatch);
                    Write(nullptr, 0);
                } else {
                    Write(pBatch->GetBuffer(), pBatch->GetSize());
                }
            }
        }
        SPA::CScopeUQueue::Unlock(pBatch);
    }
    return b;
}

bool CClientSession::AbortBatching() {
    m_mutex.lock();
    SPA::CScopeUQueue::Unlock(m_pQBatch);
    m_pQBatch = nullptr;
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
    if (m_RouterHandle == 0 || m_ConnState < SPA::ClientSide::csConnected || m_RouterHandle == m_routeeNotAvailable)
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

bool CClientSession::SendRequestInternal(CAutoLock &al, unsigned short reqId, const void *pBuffer, unsigned int len) {
#ifdef WINCE
    static const unsigned int BLOCK_COUNT = 8;
#else
    static const unsigned int BLOCK_COUNT = 64;
#endif
    bool bQueue = CheckQueueAvailable();
    if (m_ConnState < SPA::ClientSide::csConnected) {
        if (!bQueue)
            return false;
    }
#ifndef NDEBUG
    if (reqId > SPA::idSwitchTo)
        ++m_nBalance;
#endif
    if (m_qWrite.GetSize() > BLOCK_COUNT * BUFFER_BLOCK_SIZE) {
        if (!IsSameThread()) {
            m_bSendWaiting = true;
            WaitAllInternal(al, (~0));
            m_bSendWaiting = false;
        }
    }

    SPA::CStreamHeader reqInfo;
    reqInfo.RequestId = reqId;
    reqInfo.Size = len;
    if (len > 0) {
        if (m_pQBatch != nullptr) {
            *m_pQBatch << reqInfo;
            m_pQBatch->Push((const unsigned char*) pBuffer, len);
        } else {
            if (m_bZip && reqId != SPA::idSwitchTo) {
                SPA::CScopeUQueue sb;
                if (m_qWrite.GetTailSize() < len + sizeof (reqInfo) && m_qWrite.GetHeadPosition() > len + sizeof (reqInfo))
                    m_qWrite.SetHeadPosition();
                unsigned int res = CompressRequestTo(reqId, m_zl, (const unsigned char*) pBuffer, len, *sb);
                sb->SetSize(res);
                //The first sizeof(reqInfo) of bytes contains request header info
                if (!bQueue) {
                    m_qReqIdCancel.Push(sb->GetBuffer(), sizeof (reqInfo));
                    m_qReqIdWait.Push(sb->GetBuffer(), sizeof (reqInfo));
                }
                if (bQueue) {
                    SPA::CStreamHeader *p = (SPA::CStreamHeader*)sb->GetBuffer();
                    sb->Pop((unsigned int) sizeof (SPA::CStreamHeader));
                    m_qRequest->Enqueue(*p, *sb);
                    if (!m_qRequest->GetJobSize() && m_ConnState < SPA::ClientSide::csConnected)
                        m_pIoService->post(boost::bind(&CSocketPool::OnClose, m_pThread->GetPool(), this));
                } else {
                    Write(sb->GetBuffer(), sb->GetSize());
                }
            } else {
                if (!bQueue) {
                    m_qReqIdCancel << reqInfo;
                    m_qReqIdWait << reqInfo;
                }
                if (bQueue && reqId != SPA::idSwitchTo) {
                    m_qRequest->Enqueue(reqInfo, (const unsigned char*) pBuffer, len);
                    if (!m_qRequest->GetJobSize() && m_ConnState < SPA::ClientSide::csConnected)
                        m_pIoService->post(boost::bind(&CSocketPool::OnClose, m_pThread->GetPool(), this));
                } else {
                    Write(reqInfo, (const unsigned char*) pBuffer, len);
                }
            }
        }
    } else {
        if (m_pQBatch != nullptr) {
            *m_pQBatch << reqInfo;
        } else {
            if (bQueue) {
                m_qRequest->Enqueue(reqInfo, nullptr, 0);
                if (!m_qRequest->GetJobSize() && m_ConnState < SPA::ClientSide::csConnected)
                    m_pIoService->post(boost::bind(&CSocketPool::OnClose, m_pThread->GetPool(), this));
            } else {
                m_qReqIdCancel << reqInfo;
                m_qReqIdWait << reqInfo;
                if (m_pSspi) {
                    Write(reqInfo, nullptr, 0);
                } else {
                    m_qWrite << reqInfo;
                }
            }
        }
    }
    if (bQueue) {
        WriteFromQueueFile();
    }
    Write(nullptr, 0);
#ifndef NDEBUG
    if (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->GetMessageCount() < m_nBalance) {
        //assert(false);
    }
#endif
    return true;
}

void CClientSession::WriteFromQueueFile() {
    if (m_ConnState <= SPA::ClientSide::csConnected)
        return;
    if (m_qWrite.GetSize() >= IO_BUFFER_SIZE)
        return;
    if (!m_qRequest)
        return;
    if ((m_ServerInfo.SockMinorVersion & IS_ROUTING_PARTNER) == IS_ROUTING_PARTNER && m_nRouteeCount == 0)
        return;
    if (m_qWrite.GetTailSize() < 4 * IO_BUFFER_SIZE) {
        m_qWrite.SetHeadPosition();
    }
    unsigned int pos = 0;
    unsigned int index = 0;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;

    std::vector<unsigned int> vSize = m_qRequest->DoBatchDequeue(*qAttr, *qRequests, 3 * IO_BUFFER_SIZE, 0);
    const MQ_FILE::QAttr *qattr = (const MQ_FILE::QAttr *)qAttr->GetBuffer();
    unsigned int records = (unsigned int) vSize.size();
    for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
        unsigned int total = *it;
        SPA::CStreamHeader *sh = (SPA::CStreamHeader*)qRequests->GetBuffer(pos);
        unsigned int size = sh->Size;
        assert((size + sizeof (SPA::CStreamHeader)) <= total);
        sh->Size += sizeof (MQ_FILE::QAttr);
        sh->SetQueued(true);
        m_qReqIdCancel << *sh;
        m_qReqIdWait << *sh;
        if (m_pSspi) {
            SPA::CScopeUQueue sb;
            sb << *sh;
            assert(qattr[index].MessageIndex && qattr[index].MessageIndex != (~0));
            if (records == index + 1) {
                MQ_FILE::QAttr qa = qattr[index];
                qa.MessagePos |= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
                sb << qa;
            } else {
                sb << qattr[index];
            }
            sb->Push(qRequests->GetBuffer(pos + sizeof (SPA::CStreamHeader)), size);
            Write(sb->GetBuffer(), sb->GetSize());
        } else {
            m_qWrite << *sh;
            m_qWrite << qattr[index];
            m_qWrite.Push(qRequests->GetBuffer(pos + sizeof (SPA::CStreamHeader)), size);
        }
        pos += total;
        ++index;
    }
}

bool CClientSession::SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int len) {
    if (reqId <= SPA::idReservedTwo)
        return false;
    if (!pBuffer)
        len = 0;
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, reqId, pBuffer, len);
}

void CClientSession::SetContext() {
    delete m_pSocket;
    m_pSocket = new CSocket(*m_pIoService);
#ifdef WIN32_64
#ifndef _WIN32_WCE
#define LOOPBACK_FAST_PATH ((DWORD)0x98000010)
    int optionValue = 1;
    DWORD NumOfBytes = 0;
    WSAIoctl(m_pSocket->native_handle(), LOOPBACK_FAST_PATH, &optionValue, sizeof (optionValue), nullptr, 0, &NumOfBytes, nullptr, nullptr);
#endif
#endif
}

#ifndef WIN32_64
boost::mutex g_mutexResover;
#endif

void CClientSession::ConnectInternally() {
    CErrorCode ec;
    PSocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        spc(m_nPoolId, SPA::ClientSide::speConnecting, this);
    }
    CAutoLock sl(m_mutex);
    SetContext();
#ifdef WIN32_64
    CResolver::query ipAddress(m_b6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), m_strhost, boost::lexical_cast<std::string > (m_nPort));
#else
    CResolver::query ipAddress(m_b6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), m_strhost, boost::lexical_cast<std::string > (m_nPort), boost::asio::ip::resolver_query_base::numeric_service);
#endif
    CResolver r(*m_pIoService);
    {
        nsIP::tcp::resolver::iterator iterator;
        {
#ifndef WIN32_64
            //it seems that resolve is not thread-safe on linux platforms
            CAutoLock al(g_mutexResover);
#endif
            {
                CRAutoLock rsl(m_mutex);
                iterator = r.resolve(ipAddress, ec);
            }
        }
        m_ec = ec;
        if (ec || iterator == nsIP::tcp::resolver::iterator()) {
            OnConnectedInternal(ec.value());
            CloseInternal(ec.value());
            return;
        }
        //async_coonect requires a new thread created
        GetSocket()->async_connect(iterator->endpoint(), boost::bind(&CClientSession::OnConnected, this, boost::asio::placeholders::error, iterator));
    }
}

SPA::ClientSide::tagConnectionState CClientSession::GetConnectionState() {
    return m_ConnState;
}

bool CClientSession::WaitConnected(CAutoLock &sl, unsigned int nTimeout) {
    if (IsSameThread())
        return false;
    if (m_ConnState >= SPA::ClientSide::csConnected)
        return true;
    boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(nTimeout);
    m_bWaiting = true;
    bool b = (m_cv.timed_wait(sl, td) && m_ConnState >= SPA::ClientSide::csConnected);
    m_bWaiting = false;
    return b;
}

bool CClientSession::Connect(const char *strHost, unsigned int nPort, bool bSync, bool b6) {
    {
        CAutoLock sl(m_mutex);
        CloseInternal();
        m_strhost = strHost;
        std::transform(m_strhost.begin(), m_strhost.end(), m_strhost.begin(), ::tolower);
        boost::trim(m_strhost);
        m_nPort = nPort;
        m_ConnState = SPA::ClientSide::csConnecting;
        m_tRecv = GetTimeTick();
        m_tSend = m_tRecv;
        m_b6 = b6;
        m_bSync = bSync;
    }
    m_pIoService->post(boost::bind(&CClientSession::ConnectInternally, this));

    //can't use the sync connecting, which may lead to dead lock
    //ConnectInternally();

    if (bSync) {
        CAutoLock sl(m_mutex);
        return WaitConnected(sl, m_nConnTimeout);
    }
    return true;
}

SPA::UINT64 CClientSession::GetLatestTime() {
    return (m_tRecv > m_tSend) ? m_tRecv : m_tSend;
}

void CClientSession::OnConnected(const CErrorCode &ec, CResolver::iterator ep) {
    boost::asio::socket_base::keep_alive option(false);
    boost::asio::ip::tcp::no_delay nodelay(true);
    CAutoLock sl(m_mutex);
    m_pCert.reset();
#ifndef NDEBUG
    m_nJobRequest = 0;
    m_nJobConfirm = 0;
#endif
    m_nCancel = 0;
    m_qRead.SetSize(0);
    m_qWrite.SetSize(0);
    m_qReqIdCancel.SetSize(0);
    m_qReqIdWait.SetSize(0);
    m_bRBLocked = false;
    m_bWBLocked = 0;
    GetSocket()->set_option(option, m_ec);
    GetSocket()->set_option(nodelay, m_ec);
    if (ec) {
        m_ec = ec;
        OnConnectedInternal(ec.value());
        CloseInternal(ec.value());
        return;
    } else {
        m_ec.clear();
        nsIP::address addr = ep->endpoint().address();
        std::string saddr = addr.to_string();
        if (saddr == "127.0.0.1" || saddr == "::1") {
            m_hn = g_localhost;
        } else if (m_strhost == g_localhost) {
            m_hn = g_localhost;
        } else {
            int res;
            char hostname[NI_MAXHOST] = {0};
            if (ep->endpoint().address().is_v4()) {
                struct sockaddr_in saGNI;
                saGNI.sin_family = AF_INET;
                saGNI.sin_addr.s_addr = inet_addr(saddr.c_str());
                saGNI.sin_port = htons((u_short) m_nPort);
                res = ::getnameinfo((struct sockaddr *) &saGNI, sizeof (struct sockaddr), hostname, sizeof (hostname), nullptr, 0, NI_NAMEREQD);
            } else {
                nsIP::address_v6::bytes_type bytes = addr.to_v6().to_bytes();
                struct sockaddr_in6 saGNI;
                saGNI.sin6_family = AF_INET6;
                memcpy(saGNI.sin6_addr.u.Byte, bytes.data(), bytes.size());
                saGNI.sin6_port = htons((u_short) m_nPort);
                res = ::getnameinfo((struct sockaddr *) &saGNI, sizeof (struct sockaddr_in6), hostname, sizeof (hostname), nullptr, 0, NI_NAMEREQD);
            }
            if (res == 0) {
                std::string s = hostname;
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                if (saddr.size() > s.size() && saddr.find(s) != std::string::npos) {
                    m_hn = saddr;
                } else if (s.find(g_localhost) == 0 && s.size() > g_localhost.size() && s[g_localhost.size()] == '.') {
                    m_hn = g_localhost;
                } else {
                    m_hn = s;
                }
            } else {
                m_hn = saddr;
            }
        }
        m_hn += '@';
#ifdef _WIN32_WCE
        char str[16] = {0};
        sprintf(str, "%u", m_nPort);
        m_hn += str;
#else
        m_hn += std::to_string((SPA::UINT64)m_nPort);
#endif
    }

    m_tRecv = GetTimeTick();
    m_tSend = m_tRecv;
    if (IsSslEnabled()) {
        m_ConnState = SPA::ClientSide::csSslShaking;
        SECURITY_STATUS ss = OpenCred();
        if (ss == SEC_E_OK) {
            m_pSspi.reset(new SPA::CSspi(true, &m_hCreds, false));
            if (m_pSspi->DoHandshake(nullptr, 0, m_qWrite)) {
                Write(nullptr, 0);
            } else {
                ss = m_pSspi->GetLastStatus();
            }
        }
        if (ss != SEC_E_OK) {
            CloseInternal(ss);
            return;
        }
    } else {
        OnConnectedInternal(ec.value());
    }
    Read();
}

void CClientSession::Write(const SPA::CStreamHeader &sh, const unsigned char *s, unsigned int nSize) {
    if (!s)
        nSize = 0;
    if (m_ConnState < SPA::ClientSide::csConnected)
        return;
    if (m_qWrite.GetTailSize() < (nSize + sizeof (sh)) && m_qWrite.GetHeadPosition() > (nSize + sizeof (sh)))
        m_qWrite.SetHeadPosition();
    if (m_pSspi) {
        SPA::CScopeUQueue su;
        su << sh;
        su->Push(s, nSize);
        return Write(su->GetBuffer(), su->GetSize());
    }
    if (m_bWBLocked) {
        m_qWrite << sh;
        m_qWrite.Push(s, nSize);
        return;
    }
    unsigned int ulLen = m_qWrite.GetSize();
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
    m_ulSent += ulLen;
    m_bWBLocked = ulLen;
    m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CClientSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

void CClientSession::Write(const unsigned char *s, unsigned int nSize) {
    unsigned int ulLen;
    if (!s)
        nSize = 0;
    if (m_ConnState < SPA::ClientSide::csSslShaking)
        return;
    if (m_qWrite.GetTailSize() < nSize && nSize < m_qWrite.GetHeadPosition())
        m_qWrite.SetHeadPosition();

    if (m_pSspi) {
        SPA::CScopeUQueue su;
        if (s && nSize) {
            m_pSspi->Encrypt(s, nSize, *su);
        }
        s = su->GetBuffer();
        nSize = su->GetSize();
        if (m_bWBLocked) {
            m_qWrite.Push(s, nSize);
            return;
        }
        ulLen = m_qWrite.GetSize();
        if (ulLen == 0 && s != nullptr && nSize > 0) {
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
    } else {
        if (m_bWBLocked) {
            m_qWrite.Push(s, nSize);
            return;
        }
        ulLen = m_qWrite.GetSize();
        if (ulLen == 0 && s != nullptr && nSize > 0) {
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
    }
    m_ulSent += ulLen;
    m_bWBLocked = ulLen;
    m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CClientSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

void CClientSession::Read() {
    if (m_pThread && m_pThread->GetPool()->IsKilling())
        return;
    if (m_bRBLocked || m_ConnState < SPA::ClientSide::csSslShaking || m_bRoutingWait)
        return;
    m_bRBLocked = true;
    m_pSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), boost::bind(&CClientSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

void CClientSession::OnHandleShakeCompleted(int errCode) {
    POnHandShakeCompleted p = m_OnHandShakeCompleted;
    if (p != nullptr) {
        CRAutoLock sl(m_mutex);
        p(this, errCode);
    }
    PSocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speHandShakeCompleted, this);
    }
}

void CClientSession::OnSslHandShake(const CErrorCode& ec) {
    OnHandleShakeCompleted(ec.value());
    OnConnectedInternal(ec.value());
    if (!ec) {
        Read();
    } else {
        CloseInternal(ec.value());
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
    if (strUserId == nullptr || chars == 0)
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
#if defined(__ANDROID__) || defined(ANDROID)
    SPA::UINT64 src = (SPA::UINT64)this;
    std::string s = SPA::Utilities::ToUTF8(pwd.c_str(), pwd.size());
    MQ_FILE::CMyContainer::Container.Set(src, s.c_str());
#else
    SPA::CScopeUQueue su;
    SPA::Utilities::ToUTF8(pwd.c_str(), pwd.size(), *su);
    SPA::UINT64 src = (SPA::UINT64)this;
    MQ_FILE::CMyContainer::Container.Set(src, (const char*) su->GetBuffer());
#endif
}

SPA::UINT64 CClientSession::GetSocketNativeHandle() {
    m_mutex.lock();
    CSocket *p = GetSocket();
    m_mutex.unlock();
    if (p == nullptr)
        return (~0);
    return (SPA::UINT64) p->native_handle();
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
    //CAutoLock al(m_mutex);
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
    if (m_ConnState == SPA::ClientSide::csClosed) {
        return;
    }
#ifndef NDEBUG
    if (m_nJobRequest) {
        std::cout << "Bad dequeue job balance = " << __FUNCTION__ << ", balance = " << m_nJobRequest << std::endl;
    }
    if (m_nJobConfirm) {
        std::cout << "Bad confirm job balance = " << __FUNCTION__ << ", balance = " << m_nJobConfirm << std::endl;
    }
#endif
    m_tRecv = GetTimeTick();
    m_tSend = m_tRecv;
    PSocketPoolCallback spc = m_pThread->GetSocketPoolCallback();
    if (spc) {
        CRAutoLock sl(m_mutex);
        spc(m_nPoolId, SPA::ClientSide::speClosingSocket, this);
    }
    if (IsContextSet()) {
        CErrorCode ec(nError, boost::system::system_category());
        if (IsSslEnabled()) {

        }
        GetSocket()->shutdown(boost::asio::socket_base::shutdown_both, ec);
        GetSocket()->close(ec);
    }
    SPA::ClientSide::tagConnectionState ss = m_ConnState;
    m_ConnState = SPA::ClientSide::csClosed;
    m_pCert.reset();
    m_vQTrans.clear();
    m_bConfirmTrans = false;
    m_bConfirmFail = false;
    OnClosed(nError);
    {
        if (spc) {
            CRAutoLock sl(m_mutex);
            spc(m_nPoolId, SPA::ClientSide::speSocketClosed, this);
        }
        if (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->GetJobSize() == 0 && m_qRequest->GetMessageCount() > 0 && ss > SPA::ClientSide::csConnected) {
            CRAutoLock sl(m_mutex);
            m_pThread->GetPool()->OnClose(this);
        }
    }
    m_ClientInfo.ServiceId = SPA::sidStartup;
    FreeCredHandle();
    if (m_pCertContext) {
        ::CertFreeCertificateContext(m_pCertContext);
        m_pCertContext = nullptr;
    }
    m_pSspi.reset();
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
#if defined(__ANDROID__) || defined(ANDROID)
    return SPA::Utilities::ToWide(str.c_str(), str.size());
#else
    SPA::CScopeUQueue su;
    SPA::Utilities::ToWide(str.c_str(), str.size(), *su);
    return (const wchar_t*) su->GetBuffer();
#endif
}

bool CClientSession::StartQueueInternal(const char *qName, bool secure, bool dequeueShared, unsigned int ttl) {
    if ((m_qRequest && m_qRequest->IsAvailable()) || !qName || !::strlen(qName))
        return false;
    //secure = (secure || m_EncryptionMethod != SPA::NoEncryption);
    std::string pwdA = MQ_FILE::CMyContainer::Container.Get(0);
    if (secure) {
        secure = pwdA.size() ? true : false;
    }
#if defined(__ANDROID__) || defined(ANDROID)
    std::wstring pwd = SPA::Utilities::ToWide(pwdA.c_str(), pwdA.size());
#else
    SPA::CScopeUQueue tempSQ;
    SPA::Utilities::ToWide(pwdA.c_str(), pwdA.size(), *tempSQ);
    std::wstring pwd = (const wchar_t*) tempSQ->GetBuffer();
#endif
    std::string fn(qName);
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute())
        fn = CClientSession::m_WorkingPath + fn;
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
#ifdef WIN32_64
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
#endif
    if (secure) {
        std::string id = SPA::GetSysId();
        std::transform(id.begin(), id.end(), id.begin(), ::tolower);
#if defined(__ANDROID__) || defined(ANDROID)
        std::wstring wid = SPA::Utilities::ToWide(id.c_str(), id.size());
#else
        tempSQ->SetSize(0);
        SPA::Utilities::ToWide(id.c_str(), id.size(), *tempSQ);
        std::wstring wid = (const wchar_t*) tempSQ->GetBuffer();
#endif
        m_qRequest = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFileEx(fn.c_str(), ttl, SPA::oSystemMemoryCached, wid.c_str(), pwd.c_str(), m_pQLastIndex.get(), true, dequeueShared));
    } else {
        m_qRequest = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFile(fn.c_str(), ttl, SPA::oSystemMemoryCached, false, true, dequeueShared));
    }
#ifndef WINCE
    if (m_qRequest)
        m_qRequest->SetOptimistic(SPA::oMemoryCached);
#endif
    if (Find(fn))
        m_qRequest->StopQueue(SPA::qsDuplicateName);
    if (!m_qRequest->IsAvailable()) {
        return false;
    } else {
#ifndef NDEBUG
        m_nBalance += m_qRequest->GetMessageCount();
#endif
        if (m_ClientInfo.ServiceId > SPA::sidStartup && m_ConnState > SPA::ClientSide::csConnected)
            SendStartQueueMessage();
        CAutoLock sl(m_mutexQLI);
        m_vQRequest.push_back(m_qRequest);
    }
    return true;
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
    if (m_ConnState != SPA::ClientSide::csSwitched)
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

void CClientSession::EnableRoutingQueueIndex(bool enable) {
    CAutoLock al(m_mutex);
    m_bRoutingQueueIndexEnabled = enable;
}

bool CClientSession::IsRoutingQueueIndexEnabled() {
    CAutoLock al(m_mutex);
    return m_bRoutingQueueIndexEnabled;
}

void CClientSession::ResetQueue() {
    CAutoLock al(m_mutex);
    if (m_qRequest && m_qRequest->IsAvailable())
        m_qRequest->Reset();
}

SPA::UINT64 CClientSession::RemoveQueuedRequestsByTTL() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->RemoveByTTL();
}

unsigned int CClientSession::GetTTL() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return 0;
    return m_qRequest->GetTTL();
}

bool CClientSession::IsDequeueShared() {
    CAutoLock al(m_mutex);
    return (m_qRequest && m_qRequest->IsDequeueShared());
}

SPA::tagOptimistic CClientSession::GetOptimistic() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return SPA::oSystemMemoryCached;
    return m_qRequest->IsOptimistic();
}

void CClientSession::SetOptimistic(SPA::tagOptimistic bOptimistic) {
    CAutoLock al(m_mutex);
    if (m_qRequest)
        m_qRequest->SetOptimistic(bOptimistic);
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
    if (m_ConnState > SPA::ClientSide::csConnected) {
        WriteFromQueueFile();
        Write(nullptr, 0);
    }
}

bool CClientSession::PushQueueTo(const std::vector<CClientSession*> &vClients) {
    if (vClients.size() == 0)
        return false;
    std::vector<MQ_FILE::CMqFile*> vQ;
    std::vector<CClientSession*>::const_iterator it, end = vClients.end();
    CAutoLock al(m_mutex);
    if (!m_qRequest || !m_qRequest->IsAvailable())
        return false;
    for (it = vClients.begin(); it != end; ++it) {
        vQ.push_back((*it)->m_qRequest.get());
    }
    SPA::UINT64 count = m_qRequest->GetMessageCount();
    if (count) {
        MQ_FILE::CMqFile* &ref = vQ.front();
        if (m_qRequest->AppendTo(&ref, (unsigned int) vQ.size())) {
            for (it = vClients.begin(); it != end; ++it) {
#ifndef NDEBUG
                (*it)->m_nBalance += count;
#endif
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
    if (m_qRequest)
        return m_qRequest->GetJobSize();
    return 0;
}

const char* CClientSession::GetQueueName() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return nullptr;
    return m_qRequest->GetRawName().c_str();
}

const char* CClientSession::GetQueueFileName() {
    CAutoLock al(m_mutex);
    if (!m_qRequest)
        return nullptr;
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

void CClientSession::FreeCredHandle() {
    if (m_hCreds.dwLower || m_hCreds.dwUpper) {
        ::FreeCredentialsHandle(&m_hCreds);
        ::memset(&m_hCreds, 0, sizeof (m_hCreds));
    }
}

SECURITY_STATUS CClientSession::OpenCred() {
    FreeCredHandle();
    SECURITY_STATUS Status = SEC_E_OK;
    SCHANNEL_CRED SchannelCred;
    ::memset(&SchannelCred, 0, sizeof (SchannelCred));

    PCCERT_CONTEXT pCertContext = nullptr;
    if (m_pSelfCert) {
        pCertContext = m_pSelfCert->GetCertContext();
    }
    if (pCertContext) {
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;
    }
    SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
#ifdef _WIN32_WCE
    SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3TLS1_CLIENTS;
    SchannelCred.dwFlags = (SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_REVOCATION_CHECK_CHAIN);
#else
    SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3TLS1_X_CLIENTS;
    SchannelCred.dwFlags = (SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_REVOCATION_CHECK_CHAIN/* | SCH_SEND_ROOT_CERT*/);
#endif
    SECURITY_STATUS ss = ::AcquireCredentialsHandle(nullptr,
            UNISP_NAME,
            SECPKG_CRED_OUTBOUND,
            nullptr,
            &SchannelCred,
            nullptr,
            nullptr,
            &m_hCreds,
            nullptr);
    return ss;
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
        Write(nullptr, 0);
        return (m_qRequest->StartJob() != INVALID_NUMBER);
    }
    return false;
}

bool CClientSession::EndJob() {
    CAutoLock al(m_mutex);
    if (m_pQBatch)
        return false;
    if (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->EndJob() != INVALID_NUMBER) {
        WriteFromQueueFile();
        Write(nullptr, 0);
        if (m_ConnState == SPA::ClientSide::csClosed) {
            CRAutoLock ral(m_mutex);
            m_pThread->GetPool()->OnClose(this);
        } else if (m_ConnState >= SPA::ClientSide::csConnected) {
            CRAutoLock ral(m_mutex);
            m_pThread->GetPool()->OnFindClosed();
        }
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
    if (!m_qRequest)
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
    if (qAvailable) {
        if (!permanent) {
            DoConfirmDequeue();
        } else {
            m_qConfirm.SetSize(0);
        }
    }
    if (qAvailable && m_ConnState >= SPA::ClientSide::csConnected && m_ClientInfo.ServiceId != SPA::sidStartup) {
        SendStopQueueMessage();
    }
    StopQueueInternal(permanent);
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
    if (permanent && qFileName.size()) {
#ifdef WINCE
        SPA::CScopeUQueue su;
        SPA::Utilities::ToWide(qFileName.c_str(), qFileName.size(), *su);
        DeleteFile((const wchar_t*) su->GetBuffer());
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
    if (pBuffer == nullptr || size == 0)
        return 0;
    if (size > m_ResultInfo.Size)
        size = m_ResultInfo.Size;
    if (size > 0) {
        size = m_qRead.Pop((unsigned char*) pBuffer, size);
        m_ResultInfo.Size -= size;
    }
    return size;
}

const unsigned char* CClientSession::GetResultBuffer() {
    const unsigned char *p;
    //m_mutex.lock();
    if (m_ConnState >= SPA::ClientSide::csConnected)
        p = m_qRead.GetBuffer();
    else
        p = nullptr;
    //m_mutex.unlock();
    return p;
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
#ifndef NDEBUG
                if (nRequestId > SPA::idReservedTwo) {
                    if (!m_nBalance) {
                        assert(false);
                    } else {
                        --m_nBalance;
                        if (m_nBalance == 0) {
                            int n;
                            n = 0;
                        }
                    }
                }
                if (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->GetMessageCount() < m_nBalance) {
                    assert(false);
                }
#endif
                return true;
            }
        }
    } else {

        if (pId->RequestId == nRequestId) {
            m_qReqIdWait.Pop((unsigned int) sizeof (SPA::CStreamHeader)); //remove the request id
#ifndef NDEBUG
            if (nRequestId > SPA::idReservedTwo) {
                if (!m_nBalance) {
                    assert(false);
                } else {
                    --m_nBalance;
                    if (m_nBalance == 0) {
                        int n;
                        n = 0;
                    }
                }
            }
            if (m_qRequest && m_qRequest->IsAvailable() && m_qRequest->GetMessageCount() < m_nBalance) {
                //assert(false);
            }
#endif
            if (m_qReqIdCancel.GetSize() > 0) {
                pId = (SPA::CStreamHeader *) m_qReqIdCancel.GetBuffer();
                if (pId->RequestId == nRequestId) {
                    m_qReqIdCancel.Pop((unsigned int) sizeof (SPA::CStreamHeader)); //remove the request id
                }
            }
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
    std::wstring senderUserId;
#if defined(WINCE) || defined(UNDER_CE) || defined(_WIN32_WCE)
    unsigned int bytes;
    //!!!! some devices have trouble in loading unicode string so that we have to use the hack code
    sb >> senderIpAddress >> sender.Port >> bytes;
    SPA::CScopeUQueue sbCe;
    sbCe->Push(sb->GetBuffer(), bytes);
    sb->Pop(bytes);
    sbCe->SetNull();
    sb >> sender.ServiceId;
    sender.UserId = (const wchar_t*) sbCe->GetBuffer();
#else
    sb >> senderIpAddress >> sender.Port >> senderUserId >> sender.ServiceId;
    sender.UserId = senderUserId.c_str();
#endif

#ifdef WCHAR32
    SPA::CScopeUQueue suUTF16;
    if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
#if defined(__ANDROID__) || defined(ANDROID)
        sender.UserId = (const wchar_t*) SPA::Utilities::ToUTF16(senderUserId.c_str(), (unsigned int) senderUserId.size()).c_str();
#else
        SPA::CUQueue &qUTF16 = *suUTF16;
        SPA::Utilities::ToUTF16(senderUserId.c_str(), (unsigned int) senderUserId.size(), qUTF16);
        sender.UserId = (const wchar_t*) qUTF16.GetBuffer();
#endif
    }
#endif
    sender.IpAddress = senderIpAddress.c_str();
    CErrorCode ec;
    boost::asio::ip::tcp::endpoint ep = GetSocket()->local_endpoint(ec);
    if (ec) return;
    sender.SelfMessage = (sender.Port == ep.port() && ep.address().to_string() == senderIpAddress);
    switch (nRequestId) {
        case SPA::idEnter:
            nCount = sb->GetSize();
        {
            POnEnter p = m_OnSubscribe;
            POnEnter2 p2 = m_OnSubscribe2;
            if (p || p2) {
                unsigned int *pGroup = (unsigned int*) sb->GetBuffer();
                nCount /= sizeof (unsigned int);
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, pGroup, nCount);
                else if (p2)
                    p2(this, &sender, pGroup, nCount);
            }
        }
            break;
        case SPA::idExit:
            nCount = sb->GetSize();
        {
            POnExit p = m_OnUnsubscribe;
            POnExit2 p2 = m_OnUnsubscribe2;
            if (p || p2) {
                unsigned int *pGroup = (unsigned int*) sb->GetBuffer();
                nCount /= sizeof (unsigned int);
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, pGroup, nCount);
                else if (p2)
                    p2(this, &sender, pGroup, nCount);
            }
        }
            break;
        case SPA::idSendUserMessage:
        {
            POnSendUserMessage p = m_OnPostUserMessage;
            POnSendUserMessage2 p2 = m_OnPostUserMessage2;
            if (p || p2) {
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, sb->GetBuffer(), sb->GetSize());
                else if (p2)
                    p2(this, &sender, sb->GetBuffer(), sb->GetSize());
            }
        }
            break;
        case SPA::idSendUserMessageEx:
        {
            POnSendUserMessageEx p = m_OnPostUserMessageEx;
            POnSendUserMessageEx2 p2 = m_OnPostUserMessageEx2;
            if (p || p2) {
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, sb->GetBuffer(), sb->GetSize());
                else if (p2)
                    p2(this, &sender, sb->GetBuffer(), sb->GetSize());
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
            POnSpeak2 p2 = m_OnBroadcast2;
            assert(sb->GetSize() == sizeof (unsigned int) *nCount);
            if (p || p2) {
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, group, nCount, suMsg->GetBuffer(), suMsg->GetSize());
                else if (p2)
                    p2(this, &sender, group, nCount, suMsg->GetBuffer(), suMsg->GetSize());
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
            POnSpeakEx2 p2 = m_OnBroadcastEx2;
            if (p || p2) {
                CRAutoLock sl(m_mutex);
                if (p)
                    p(this, sender, pGroup, nCount, sb->GetBuffer(), size);
                else if (p2)
                    p2(this, &sender, pGroup, nCount, sb->GetBuffer(), size);
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
    unsigned int removed = m_qRequest->ConfirmDequeueJob(&m_vQTrans.front(), m_vQTrans.size(), m_bConfirmFail);
}

boost::shared_ptr<MQ_FILE::CMqFile> CClientSession::GetQueue() {
    return m_qRequest;
}

void CClientSession::OnBaseRequestProcessed(unsigned short nRequestId, unsigned int nLen) {
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
        case SPA::idRoutePeerUnavailable:
            sb >> m_routeeNotAvailable;
            break;
        case SPA::idRouteeChanged:
            sb >> m_nRouteeCount;
            if (m_qRequest && m_nRouteeCount == 0 && m_ConnState >= SPA::ClientSide::csSwitched) {
                WriteFromQueueFile(); //router requires this call for fast wakeup
                Write(nullptr, 0);
            }
            break;
        case SPA::idSetZipLevelAtSvr:
        case SPA::idTurnOnZipAtSvr:
        case SPA::idGetSockOptAtSvr:
        case SPA::idSetSockOptAtSvr:
            sb->SetSize(0);
            break;
        case SPA::idStopQueue:
            sb >> m_ServerQFile;
            if (m_pQLastIndex)
                m_pQLastIndex->Remove(m_ServerQFile.Qs);
            break;
        case SPA::idStartQueue:
            sb >> m_ServerQFile;
            if ((m_ServerQFile.MinIndex & MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) == MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) {
                if (m_pQLastIndex) {
                    m_pQLastIndex->Remove(m_ServerQFile.Qs);
                }
            }
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
        case SPA::idDequeueBatchConfirmed:
        case SPA::idDequeueConfirmed:
        {
            unsigned short reqId;
            MQ_FILE::CDequeueConfirmInfo dci;
            if (nRequestId == SPA::idDequeueConfirmed) {
                assert(sb->GetSize() == sizeof (MQ_FILE::CDequeueConfirmInfo));
                sb >> dci;
#ifndef NDEBUG
                if (m_qRequest->GetMessageCount() < m_nBalance + 1 + m_qConfirm.GetSize() / sizeof (dci)) {
                    MQ_FILE::CDequeueConfirmInfo *start = (MQ_FILE::CDequeueConfirmInfo *) m_qConfirm.GetBuffer();
                    //assert(false);
                }
#endif
                assert(dci.QA.MessageIndex && dci.QA.MessageIndex != (~0));
                m_qa = dci.QA;
            } else {
#ifndef NDEBUG
                if (m_qRequest->GetMessageCount() < m_nBalance + sb->GetSize() / sizeof (dci) + m_qConfirm.GetSize() / sizeof (dci)) {
                    MQ_FILE::CDequeueConfirmInfo *start = (MQ_FILE::CDequeueConfirmInfo *) m_qConfirm.GetBuffer();
                    assert(false);
                }
#endif
                while (sb->GetSize() > 0) {
                    assert(sb->GetSize() >= sizeof (dci));
                    sb >> dci;
                    assert(dci.QA.MessageIndex && dci.QA.MessageIndex != (~0));
                    if (sb->GetSize() > 0)
                        m_qConfirm << dci;
                    else
                        m_qa = dci.QA;
                }
            }
            if (dci.Fail && (m_ServerInfo.SockMinorVersion & IS_ROUTING_PARTNER) == IS_ROUTING_PARTNER) {
                RemoveRequestId(dci.RequestId);
            }
            reqId = dci.RequestId;
            bool bQueue = CheckQueueAvailable();
            if (bQueue) {
                switch (reqId) {
                    case SPA::idStartJob:
#ifndef NDEBUG
                        ++m_nJobConfirm;
#endif
                        if (m_ConnState < SPA::ClientSide::csConnected)
                            return;
                        m_bConfirmFail = dci.Fail;
                        //the following asserts will not work as expected when the method Cancel is called
                        //assert(m_vQTrans.size() == 0);
                        //assert(!m_bConfirmTrans);
                        m_vQTrans.clear();
                        m_vQTrans.push_back(m_qa);
                        m_bConfirmTrans = true;
                        nRequestId = SPA::idStartJob;
                        RemoveRequestId(nRequestId);
                        break;
                    case SPA::idEndJob:
#ifndef NDEBUG
                        --m_nJobConfirm;
#endif
                        if (m_ConnState < SPA::ClientSide::csConnected || m_vQTrans.size() == 0)
                            return;
                        if (!m_bConfirmFail)
                            m_bConfirmFail = dci.Fail;
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
                        if (m_ConnState >= SPA::ClientSide::csSwitched) {
                            WriteFromQueueFile();
                        }
                        Write(nullptr, 0);
                        break;
                    default:
                        if (m_bConfirmTrans) {
                            if (!m_bConfirmFail)
                                m_bConfirmFail = dci.Fail;
                            m_vQTrans.push_back(m_qa);
                        } else {
                            m_qConfirm << dci;
#ifndef NDEBUG
                            if (m_qRequest->GetMessageCount() < m_nBalance + m_qConfirm.GetSize() / sizeof (dci)) {
                                MQ_FILE::CDequeueConfirmInfo *start = (MQ_FILE::CDequeueConfirmInfo *) m_qConfirm.GetBuffer();
                                //assert(false);
                            }
#endif
                            bool fail = dci.Fail;
                            if (fail || ComputeQueueDistance() >= 128 * 1024) {
                                DoConfirmDequeue();
                            }
                            if (brp != nullptr && !fail && m_qRequest && m_qRequest->GetMessageCount() == 0) {
                                CRAutoLock sl(m_mutex);
                                brp(this, SPA::idAllMessagesDequeued);
                            }
                        }
                        break;
                }
#ifndef NDEBUG
                if (m_nJobConfirm > 1) {
                    std::cout << "Bad confirm job balance = " << __FUNCTION__ << ", balance = " << m_nJobConfirm << std::endl;
                }
#endif
                if (dci.Fail) {
                    WriteFromQueueFile();
                    Write(nullptr, 0);
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
            m_bRegistered = (errCode == ERROR_NO_ERROR) ? true : false;
            sb >> m_ServerInfo;
            m_nRouteeCount = m_ServerInfo.Param0;
            m_ClientInfo.ServiceId = m_ServerInfo.ServiceId;
            m_ConnState = SPA::ClientSide::csSwitched;
            m_to = nullptr;
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
            m_qConfirm.SetSize(0);
            m_vQTrans.clear();
            m_bConfirmTrans = false;
        {
            SPA::CStreamHeader *pStreamHeader;
            while (m_qReqIdWait.GetSize() >= sizeof (SPA::CStreamHeader)) {
                pStreamHeader = (SPA::CStreamHeader *)m_qReqIdWait.GetBuffer();
                m_qReqIdWait.Pop(sizeof (SPA::CStreamHeader));
                if (pStreamHeader->RequestId == SPA::idCancel)
                    break;
                ++m_nCancel;
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
    if (nullptr != p) {
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
#ifndef NDEBUG
    if (m_nJobRequest > 1) {
        std::cout << "Bad dequeue job balance = " << __FUNCTION__ << ", balance = " << m_nJobRequest << std::endl;
    }
#endif
    if (m_RouterHandle) {
        switch (m_ResultInfo.RequestId) {
            case SPA::idStartJob:
            case SPA::idEndJob:
            {
                MQ_FILE::CDequeueConfirmInfo dci(m_qa, m_bFail, m_ResultInfo.RequestId);
                SendRoutingResultInternal(SPA::idDequeueConfirmed, (const unsigned char*) &dci, sizeof (dci));
            }
                break;
            default:
                //we don't do dequeue confirmation at routee's side. Instead, we do dequeue confirmation at server side
                break;
        }

    } else {
        MQ_FILE::CDequeueConfirmInfo dci(qHandle, m_qa, m_bFail, m_ResultInfo.RequestId);
        if (m_bDequeueTrans || m_bFail) {
            if (m_qBatchDequeueConfirm.GetSize()) {
                SPA::CStreamHeader sh;
                sh.RequestId = SPA::idDequeueBatchConfirmed;
                sh.Size = m_qBatchDequeueConfirm.GetSize();
                Write(sh, m_qBatchDequeueConfirm.GetBuffer(), m_qBatchDequeueConfirm.GetSize());
                m_qReqIdCancel << sh;
                if (m_qReqIdCancel.GetSize() > m_qReqIdWait.GetSize()) {
                    m_qReqIdCancel.Pop(m_qReqIdCancel.GetSize() - m_qReqIdWait.GetSize());
                }
                m_qBatchDequeueConfirm.SetSize(0);
            }
            SPA::CStreamHeader sh;
            sh.RequestId = SPA::idDequeueConfirmed;
            sh.Size = sizeof (dci);
            Write(sh, (const unsigned char*) &dci, sizeof (dci));
            m_qReqIdCancel << sh;
            if (m_qReqIdCancel.GetSize() > m_qReqIdWait.GetSize()) {
                m_qReqIdCancel.Pop(m_qReqIdCancel.GetSize() - m_qReqIdWait.GetSize());
            }
        } else {
            m_qBatchDequeueConfirm << dci;
            if (m_bLastDequeue) {
                SPA::CStreamHeader sh;
                sh.RequestId = SPA::idDequeueBatchConfirmed;
                sh.Size = m_qBatchDequeueConfirm.GetSize();
                Write(sh, m_qBatchDequeueConfirm.GetBuffer(), m_qBatchDequeueConfirm.GetSize());
                m_qReqIdCancel << sh;
                if (m_qReqIdCancel.GetSize() > m_qReqIdWait.GetSize()) {
                    m_qReqIdCancel.Pop(m_qReqIdCancel.GetSize() - m_qReqIdWait.GetSize());
                }
                m_qBatchDequeueConfirm.SetSize(0);
            }
        }
    }
}

void CClientSession::NotifyDequeuedStartQueueTrans(unsigned int qHandle) {
    POnBaseRequestProcessed p = m_OnBaseRequestProcessed;
    if (p != nullptr) {

        CRAutoLock sl(m_mutex);
        p(this, SPA::idStartJob);
    }
    NotifyDequeued(qHandle);
    assert(m_ResultInfo.RequestId == SPA::idStartJob);
}

void CClientSession::NotifyDequeuedCommitQueueTrans(unsigned int qHandle) {
    POnBaseRequestProcessed p = m_OnBaseRequestProcessed;
    if (p != nullptr) {

        CRAutoLock sl(m_mutex);
        p(this, SPA::idEndJob);
    }
    NotifyDequeued(qHandle);
    assert(m_ResultInfo.RequestId == SPA::idEndJob);
}

void CClientSession::OnReadCompleted(const CErrorCode& Error, size_t nLen) {
    unsigned short sReqId = 0;
    unsigned int len = (unsigned int) nLen;
    if (Error) {
        CAutoLock sl(m_mutex);
        CloseInternal(Error.value());
        return;
    }
    if (m_ConnState < SPA::ClientSide::csSslShaking) {
        return;
    }
    if (m_qRead.GetTailSize() < len && m_qRead.GetHeadPosition() >= len)
        m_qRead.SetHeadPosition();
    if (m_pSspi) {
        if (m_pSspi->GetHandshakeState() == SPA::hsDone) {
            nLen = m_qRead.GetSize();
            if (!m_pSspi->Decrypt(m_ReadBuffer, len, m_qRead)) {
                CAutoLock sl(m_mutex);
                CloseInternal(m_pSspi->GetLastStatus());
                return;
            }
            m_ulRead += (unsigned int) (m_qRead.GetSize() - nLen);
        } else {
            m_bRBLocked = false;
            CAutoLock sl(m_mutex);
            m_tRecv = GetTimeTick();
            m_qRead.Push(m_ReadBuffer, len);
            bool ok = m_pSspi->DoHandshake(m_qRead.GetBuffer(), m_qRead.GetSize(), m_qWrite);
            if (ok) {
                if (m_pSspi->GetLastStatus() != SEC_E_INCOMPLETE_MESSAGE) {
                    m_qRead.SetSize(0);
                }
                if (m_pSspi->GetHandshakeState() == SPA::hsDone) {

                    g_mutexCvc.lock();
                    PCertificateVerifyCallback cvc = g_cvc;
                    g_mutexCvc.unlock();
                    if (cvc) {
                        DWORD dwFlags = 0;
                        PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
                        CERT_ENHKEY_USAGE EnhkeyUsage;
                        CERT_USAGE_MATCH CertUsage;
                        CERT_CHAIN_PARA ChainPara;
                        EnhkeyUsage.cUsageIdentifier = 0;
                        EnhkeyUsage.rgpszUsageIdentifier = nullptr;
                        CertUsage.dwType = USAGE_MATCH_TYPE_AND;
                        CertUsage.Usage = EnhkeyUsage;
                        ChainPara.cbSize = sizeof (CERT_CHAIN_PARA);
                        ChainPara.RequestedUsage = CertUsage;
                        PCCERT_CONTEXT pCertContext = m_pSspi->GetCertContext();
                        if (!::CertGetCertificateChain(nullptr, // use the default chain engine
                                pCertContext, // pointer to the end certificate
                                NULL, // use the default time
                                SPA::CCertificateImpl::CertStore.GetCertStore(), // search no additional stores
                                &ChainPara, // use AND logic and enhanced key usage 
                                dwFlags,
                                NULL, // currently reserved
                                &pChainContext)) // return a pointer to the chain created
                        {
                            //ec.assign(::GetLastError(), boost::system::get_system_category());
                            CloseInternal((int) ::GetLastError());
                            return;
                        } else {
                            int errCode = 0;
                            const char *errMsg = SPA::CCertificateImpl::VerifyOne(pCertContext, &errCode, SPA::CCertificateImpl::CertStore.GetCertStore());
                            bool ok = (errCode == CERT_TRUST_NO_ERROR || ::strlen(errMsg) == 0);
                            PCERT_SIMPLE_CHAIN chains = pChainContext->rgpChain[0];
                            DWORD total = chains->cElement;
                            for (DWORD n = 0; n < total; ++n) {
                                PCERT_CHAIN_ELEMENT one = chains->rgpElement[n];
                                boost::scoped_ptr<SPA::CCertificateImpl> pCert(new SPA::CCertificateImpl(one->pCertContext, ""));
                                ok = cvc(ok, n, one->TrustStatus.dwErrorStatus, SPA::CCertificateImpl::MapErrorMessage(one->TrustStatus.dwErrorStatus), pCert.get());
                                if (!ok)
                                    break;
                            }
                            ::CertFreeCertificateChain(pChainContext);
                            if (!ok) {
                                CloseInternal((int) ::GetLastError());
                                return;
                            }
                        }
                    }
                    CErrorCode ec;
                    m_pCert.reset(new SPA::CCertificateImpl(m_pSspi, ""));
                    OnSslHandShake(ec);
                    return;
                } else if (SEC_I_INCOMPLETE_CREDENTIALS == m_pSspi->GetLastStatus()) {
                    if (m_certCN.size()) {
                        m_pCertContext = ::CertFindCertificateInStore(SPA::CCertificateImpl::CertStore.GetCertStore(), X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_STR, CA2CT(m_certCN.c_str()), nullptr);
                        if (m_pCertContext) {
                            m_pSelfCert.reset(new SPA::CCertificateImpl(m_pCertContext, ""));
                        } else {
                            m_pSelfCert.reset();
                        }
                    }
                    SECURITY_STATUS ss = OpenCred();
                    if (ss == SEC_E_OK) {
                        m_pSspi->ResetCredHandle(&m_hCreds);
                        ok = m_pSspi->DoHandshake(m_qRead.GetBuffer(), m_qRead.GetSize(), m_qWrite);
                        if (!ok) {
                            CloseInternal(m_pSspi->GetLastStatus());
                            return;
                        } else {

                        }
                    } else {
                        CloseInternal(ss);
                        return;
                    }
                } else {
                }
                Write(nullptr, 0);
                Read();
                return;
            } else {
                CloseInternal(m_pSspi->GetLastStatus());
                return;
            }

        }
    } else {
        m_ulRead += len;
        m_qRead.Push(m_ReadBuffer, len);
    }
    m_bRBLocked = false;
    CAutoLock sl(m_mutex);
    bool notify = (m_qReqIdWait.GetSize() > 0);
    m_tRecv = GetTimeTick();
    if (m_bRoutingWait)
        return;
    bool b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader) || m_ResultInfo.RequestId != 0);
    m_nRcvBufferSize = m_qRead.GetSize();
    while (b) {
        m_nRcvBufferSize = m_qRead.GetSize();
        if (m_bSendWaiting && (m_qWrite.GetSize() < BUFFER_BLOCK_SIZE / 2 || m_qReqIdWait.GetSize() / sizeof (SPA::CStreamHeader) <= 2)) {
            m_bSendWaiting = false;
            m_cv.notify_all();
        }
        m_bLastDequeue = false;
        if (m_ResultInfo.RequestId == 0) {
            m_qRead >> m_ResultInfo;
            m_qRead.SetOS(m_ResultInfo.GetOS());
        }
        b = (m_qRead.GetSize() >= m_ResultInfo.Size);
        if (!b) {
            if ((m_ResultInfo.Size + sizeof (m_ResultInfo)) > m_qRead.GetMaxSize())
                m_qRead.ReallocBuffer(m_ResultInfo.Size + sizeof (m_ResultInfo));
            break;
        }
        sReqId = m_ResultInfo.RequestId;
        if (sReqId == SPA::idRoutingData) {
            SPA::UINT64 handle;
            assert(m_qRead.GetSize() >= sizeof (m_ResultInfo) + sizeof (m_RouterHandle));
            m_qRead >> handle;
            if (handle != m_RouterHandle) {
                m_routeeNotAvailable = 0;
                m_RouterHandle = handle;
            }
            assert(m_RouterHandle != 0);
            m_qRead >> m_ResultInfo;
            sReqId = m_ResultInfo.RequestId;
            if (m_ResultInfo.GetQueued()) {
                assert(m_ResultInfo.Size >= sizeof (m_qa));
                m_qRead >> m_qa;
                m_bLastDequeue = ((m_qa.MessagePos & MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END) == MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END);
                if (m_bLastDequeue)
                    m_qa.MessagePos -= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
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
            if (sReqId <= SPA::idReservedTwo && !queued && sReqId != SPA::idInterrupt) {
                assert(m_RouterHandle == 0);
                if (!notify && m_bWaiting && sReqId == SPA::idCancel) {
                    notify = true;
                }
                OnBaseRequestProcessed(sReqId, m_ResultInfo.Size);
            } else {
                unsigned int qHandle = 0;
                if (queued && m_pQLastIndex) {
                    MQ_FILE::QAttr seek;
                    if (m_RouterHandle == 0) {
                        if (m_ResultInfo.Size < (sizeof (qHandle) + sizeof (m_qa))) {

                        }
                        m_qRead >> qHandle >> m_qa;
                        m_bLastDequeue = ((m_qa.MessagePos & MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END) == MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END);
                        if (m_bLastDequeue)
                            m_qa.MessagePos -= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
                        m_ResultInfo.Size -= (sizeof (qHandle) + sizeof (m_qa));
                        if ((m_ServerQFile.MinIndex & MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) != MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) {
                            seek = m_pQLastIndex->Seek(m_ServerQFile.Qs);
                        }
                    }

                    //check if previous message index is already dequeued because of sudden socket disconnection, exception and others
                    if ((m_RouterHandle == 0 || m_bRoutingQueueIndexEnabled) && //we don't detect resending if it is for routing
                            seek.MessageIndex != INVALID_NUMBER &&
                            seek.MessageIndex >= m_qa.MessageIndex &&
                            seek.MessagePos != INVALID_NUMBER &&
                            seek.MessagePos >= m_qa.MessagePos && m_qa.MessageIndex > 1) {
#ifndef NDEBUG
                        std::cout << "++++ ReqId = " << m_ResultInfo.RequestId << ", len = " << m_ResultInfo.Size << ", msg index = " << m_qa.MessageIndex << ", pos = " << m_qa.MessagePos;
                        std::cout << ", seek msg index = " << seek.MessageIndex << ", seek pos = " << seek.MessagePos << std::endl;
                        if (m_ResultInfo.RequestId == SPA::idStartJob) {
                            ++m_nJobRequest;
                        } else if (m_ResultInfo.RequestId == SPA::idEndJob) {
                            --m_nJobRequest;
                        }
#endif
                        m_bFail = false;
                        NotifyDequeued(qHandle);
                        assert(m_qRead.GetSize() >= m_ResultInfo.Size);
                        m_qRead.Pop(m_ResultInfo.Size);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        continue;
                    } else if (m_ResultInfo.RequestId == SPA::idStartJob) {
#ifndef NDEBUG
                        ++m_nJobRequest;
#endif
                        m_bDequeueTrans = true;
                        m_bFail = false;
                        NotifyDequeuedStartQueueTrans(qHandle);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        continue;
                    } else if (m_ResultInfo.RequestId == SPA::idEndJob) {
#ifndef NDEBUG
                        --m_nJobRequest;
#endif
                        NotifyDequeuedCommitQueueTrans(qHandle);
                        m_bDequeueTrans = false;
                        assert(m_qRead.GetSize() >= m_ResultInfo.Size);
                        m_qRead.Pop(m_ResultInfo.Size);
                        m_ResultInfo.Size = 0;
                        m_ResultInfo.RequestId = 0;
                        b = (m_qRead.GetSize() >= sizeof (SPA::CStreamHeader));
                        if (!m_bFail && !m_RouterHandle && m_pQLastIndex && (m_ServerQFile.MinIndex & MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) != MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX)
                            m_pQLastIndex->Set(m_ServerQFile.Qs, m_qa);
                        continue;
                    } else if (!m_bDequeueTrans) {
                        m_bFail = false;
                    }

                }
                if (m_ResultInfo.RequestId == SPA::idStartBatching || m_ResultInfo.RequestId == SPA::idCommitBatching) {
                    assert(m_RouterHandle != 0);
                    SendRoutingResultInternal(m_ResultInfo.RequestId, (const unsigned char*) nullptr, 0);
                } else {
                    assert(m_qRead.GetSize() >= m_ResultInfo.Size);
                    OnRequestProcessed(sReqId, m_ResultInfo.Size);
                }
                if (queued) {
                    if (((!m_bFail && !m_bDequeueTrans) || (m_ResultInfo.RequestId == SPA::idStartJob && m_qa.MessageIndex == 1)) &&
                            !m_RouterHandle && m_pQLastIndex &&
                            (m_ServerQFile.MinIndex & MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) != MQ_FILE::CQueueInitialInfo::QUEUE_SHARED_INDEX) {
                        m_pQLastIndex->Set(m_ServerQFile.Qs, m_qa);
                    }
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
    if (m_ConnState < SPA::ClientSide::csConnected)
        return;
    if (!m_qReqIdWait.GetSize()) {
        m_qReqIdCancel.SetSize(0);
        if (notify) {
            POnAllRequestsProcessed p = m_OnAllRequestsProcessed;
            if (p) {
                CRAutoLock rsl(m_mutex);
                p(this, sReqId);
            }
            if (m_qConfirm.GetSize() > 0 && m_qRequest) {
                DoConfirmDequeue();
            }
            if (m_bWaiting)
                m_cv.notify_all();
        }
    }
}

SPA::UINT64 CClientSession::ComputeQueueDistance() {
    SPA::UINT64 min = 0, max = 0;
    MQ_FILE::CDequeueConfirmInfo *start = (MQ_FILE::CDequeueConfirmInfo*)m_qConfirm.GetBuffer();
    unsigned int count = m_qConfirm.GetSize() / sizeof (MQ_FILE::CDequeueConfirmInfo);
    for (unsigned int n = 0; n < count; ++n, ++start) {
        if (start->QA.MessagePos > max)
            max = start->QA.MessagePos;
        if (!min || start->QA.MessagePos < min)
            min = start->QA.MessagePos;
    }
    return max - min;
}

bool CClientSession::SortQueueConfirm(const MQ_FILE::CDequeueConfirmInfo &dci0, const MQ_FILE::CDequeueConfirmInfo &dci1) {
    return (dci0.QA.MessageIndex < dci1.QA.MessageIndex);
}

void CClientSession::DoConfirmDequeue() {
    if (m_qConfirm.GetSize() > 0) {
        MQ_FILE::CDequeueConfirmInfo *start = (MQ_FILE::CDequeueConfirmInfo*)m_qConfirm.GetBuffer();
        assert((m_qConfirm.GetSize() % sizeof (MQ_FILE::CDequeueConfirmInfo)) == 0);
        unsigned int count = m_qConfirm.GetSize() / sizeof (MQ_FILE::CDequeueConfirmInfo);
        std::sort(start, start + count, SortQueueConfirm);
        unsigned int res = m_qRequest->DoConfirmDequeue(m_qConfirm);
#ifndef NDEBUG
        if (count != res) {
            assert(false);
        }
        if (m_qRequest->GetMessageCount() < m_nBalance) {
            //assert(false);
        }
#endif
        m_qConfirm.SetSize(0);
    }
}

void CClientSession::OnWriteCompleted(const CErrorCode& Error, size_t bytes_transferred) {
    CAutoLock sl(m_mutex);
    m_tSend = GetTimeTick();
    if (m_ConnState < SPA::ClientSide::csSslShaking) {
        return;
    }
    if (!Error) {
        assert(m_bWBLocked >= bytes_transferred);
        bool bQueue = CheckQueueAvailable();
        if (bQueue && m_ConnState >= SPA::ClientSide::csSwitched) {
            WriteFromQueueFile();
        }
        if (m_bWBLocked > bytes_transferred) {
            //m_bWBLocked -= (unsigned int)bytes_transferred;
            unsigned int ulLen = (unsigned int) (m_bWBLocked - bytes_transferred);
            memmove(m_WriteBuffer, m_WriteBuffer + bytes_transferred, ulLen);
            m_bWBLocked = ulLen;
            unsigned int max_add = (unsigned int) (IO_BUFFER_SIZE - m_bWBLocked);
            if (max_add && m_qWrite.GetSize()) {
                if (max_add > m_qWrite.GetSize()) {
                    max_add = m_qWrite.GetSize();
                }
                m_qWrite.Pop(m_WriteBuffer + ulLen, max_add);
                ulLen += max_add;
                m_bWBLocked = ulLen;
            }
            m_pSocket->async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CClientSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        } else {
            m_bWBLocked = 0;
            Write(nullptr, 0);
        }
        Read();
    } else {
        m_ec = Error;
        m_pIoService->post(boost::bind(&CClientSession::Close, this));
    }
}

bool CClientSession::DoEcho() {
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, SPA::idDoEcho, nullptr, 0);
}

bool CClientSession::SetSockOpt(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    int name = SPA::MapSockOption(optName);
    int lvl = SPA::MapSockLevel(level);
    CAutoLock sl(m_mutex);
    if (m_ConnState < SPA::ClientSide::csConnected)
        return false;
    int res = ::setsockopt(GetSocket()->native_handle(), lvl, name, (const char*) &optValue, sizeof (optValue));
    return (res == 0);
}

bool CClientSession::SetSockOptAtSvr(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level) {
    SPA::CScopeUQueue su;
    su << (int) optName << optValue << (int) level;
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, SPA::idSetSockOptAtSvr, su->GetBuffer(), su->GetSize());
}

bool CClientSession::TurnOnZipAtSvr(bool enableZip) {
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, SPA::idTurnOnZipAtSvr, &enableZip, sizeof (enableZip));
}

bool CClientSession::SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) {
    int level = zipLevel;
    CAutoLock sl(m_mutex);
    return SendRequestInternal(sl, SPA::idSetZipLevelAtSvr, &level, sizeof (level));
}

unsigned int CClientSession::GetRouteeCount() {
    CAutoLock sl(m_mutex);
    return m_nRouteeCount;
}

void CClientSession::OnPostProcessing(unsigned int hint, SPA::UINT64 data) {
    m_mutex.lock();
    POnPostProcessing pp = m_OnPostProcessing;
    m_mutex.unlock();
    if (pp) {
        pp(this, hint, data);
    }
}

void CClientSession::PostProcessing(unsigned int hint, SPA::UINT64 data) {
    m_pIoService->post(boost::bind(&CClientSession::OnPostProcessing, this, hint, data));
}

void CClientSession::SetOnPostProcessing(POnPostProcessing p) {
    m_mutex.lock();
    m_OnPostProcessing = p;
    m_mutex.unlock();
}
