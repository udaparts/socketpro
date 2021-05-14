
#include "../include/definebase.h"
#ifdef OLD_IMPL
#include "server.h"
#elif defined(WIN32_64)
#include "../uservercore_win/server.h"
#else
#include "../uservercore/server.h"
#endif

#include "serverthread.h"
#define THREAD_SAFE_ALIVE_TIME	1000

extern std::vector<PThreadEvent> g_vThreadEvent;

CServerThread::CServerThread(unsigned int nMaxThreadIdleTimeBeforeSuicide, SPA::tagThreadApartment ta)
: SPA::CUCommThread(ta),
m_bBusy(false),
m_nMaxThreadIdleTimeBeforeSuicide(nMaxThreadIdleTimeBeforeSuicide + THREAD_SAFE_ALIVE_TIME) {
    m_tWorking = GetTimeTick();
}

CServerThread::~CServerThread() {
    CAutoLock sl(m_mutex);
    while (m_qThreadMessage.size()) {
        CUThreadMessage &message = m_qThreadMessage.front();
        SPA::CScopeUQueue::Unlock(message.m_pMessageBuffer);
        m_qThreadMessage.pop();
    }
}

unsigned int CServerThread::ProcessSlowRequest(CServerSession *pSession, unsigned short reqId) {
    int res = pSession->ExecuteSlowRequestFromThreadPool(reqId);
    return (unsigned int) res;
}

void CServerThread::Handle() {
    while (true) {
        m_sl.lock();
        m_bBusy = true;
        if (!m_qThreadMessage.size()) {
            m_bBusy = false;
            m_sl.unlock();
            return;
        }
        CUThreadMessage message = m_qThreadMessage.front();
        m_qThreadMessage.pop();
        m_sl.unlock();
        SPA::CUQueue& q = *message.m_pMessageBuffer;
        unsigned int res;
        PSession pSession;
        m_tWorking = GetTimeTick();
        unsigned int nMsgId = message.m_nMsgId;
        q >> pSession;
        switch (nMsgId) {
            case WM_ASK_FOR_PROCESSING:
                res = ProcessSlowRequest(pSession, message.m_uRequestId);
                break;
            default:
                res = 0;
                break;
        }
        message.m_nMsgId = WM_REQUEST_PROCESSED;
        q.SetSize(0);
        q << pSession << res;
        g_pServer->PostSproMessage(message);
        m_tWorking = GetTimeTick();
    }
}

bool CServerThread::IsBusy() {
    m_sl.lock();
    bool b = m_bBusy;
    m_sl.unlock();
    return b;
}

bool CServerThread::IsAliveSafe() {
    SPA::UINT64 tNow = GetTimeTick() - (m_nMaxThreadIdleTimeBeforeSuicide - THREAD_SAFE_ALIVE_TIME);
    bool b = (tNow < m_tWorking.load(std::memory_order_relaxed));
    return b;
}

void CServerThread::OnThreadStarted() {
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teStarted);
    }
}

void CServerThread::OnThreadEnded() {
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teKilling);
    }
}

bool CServerThread::PostMessage(CServerSession *pSession, unsigned short uRequestId, unsigned int nMsgId, const void *pBuffer, unsigned int nSize) {
    CUThreadMessage message(nMsgId, SPA::CScopeUQueue::Lock(), uRequestId);
    PSession session = pSession;
    *(message.m_pMessageBuffer) << session;
    if (pBuffer && nSize) {
        message.m_pMessageBuffer->Push((const unsigned char*) pBuffer, (unsigned int) nSize);
    }
    m_sl.lock();
    if (m_pThread == nullptr) {
        m_sl.unlock();
        return false;
    }
    m_qThreadMessage.push(std::move(message));
    if (m_qThreadMessage.size() == 1) {//if queue has two or more message we don't dispatch a handle
        boost::asio::post(GetIoService(), std::bind(&CServerThread::Handle, this));
    }
    m_sl.unlock();
    return true;
}
