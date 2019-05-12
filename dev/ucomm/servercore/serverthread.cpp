
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
    m_tWorking = boost::posix_time::microsec_clock::local_time();
}

unsigned int CServerThread::ProcessSlowRequest(CServerSession *pSession, SPA::CUThreadMessage ThreadMessage) {
    int res = pSession->ExecuteSlowRequestFromThreadPool(ThreadMessage.m_uRequestId);
    return (unsigned int) res;
}

void CServerThread::Handle() {
    while (true) {
        m_mutex.lock();
        m_bBusy = true;
        if (!m_qThreadMessage.size()) {
            m_bBusy = false;
            m_mutex.unlock();
            return;
        }
        SPA::CUThreadMessage message = m_qThreadMessage.front();
        m_qThreadMessage.pop();
        m_mutex.unlock();
        unsigned int res;
        PSession pSession;
        unsigned int nMsgId = message.m_nMsgId;
        *(message.m_pMessageBuffer) >> pSession;
        m_mutex.lock();
        m_tWorking = boost::posix_time::microsec_clock::local_time();
        m_mutex.unlock();
        switch (nMsgId) {
            case WM_ASK_FOR_PROCESSING:
                res = ProcessSlowRequest(pSession, message);
                break;
            default:
                res = 0;
                break;
        }
        m_mutex.lock();
        m_tWorking = boost::posix_time::microsec_clock::local_time();
        m_mutex.unlock();
        message.m_nMsgId = WM_REQUEST_PROCESSED;
        message.m_pMessageBuffer->SetSize(0);
        *message.m_pMessageBuffer << pSession;
        *message.m_pMessageBuffer << res;
        g_pServer->PostSproMessage(message);
    }
}

bool CServerThread::IsBusy() {
    CAutoLock sl(m_mutex);
    return m_bBusy;
}

bool CServerThread::IsAliveSafe() {
    boost::posix_time::ptime tNow = boost::posix_time::microsec_clock::local_time() - boost::posix_time::milliseconds(m_nMaxThreadIdleTimeBeforeSuicide - THREAD_SAFE_ALIVE_TIME);
    CAutoLock sl(m_mutex);
    bool b = (tNow < m_tWorking);
    return b;
}

void CServerThread::OnThreadStarted() {
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::teStarted);
    }
}

void CServerThread::OnThreadEnded() {
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::teKilling);
    }
}

bool CServerThread::PostMessage(CServerSession *pSession, unsigned short uRequestId, unsigned int nMsgId, const void *pBuffer, unsigned int nSize) {
    SPA::CUThreadMessage message(nMsgId, SPA::CScopeUQueue::Lock(), uRequestId);

    PSession session = pSession;
    *(message.m_pMessageBuffer) << session;

    if (pBuffer && nSize)
        message.m_pMessageBuffer->Push((const unsigned char*) pBuffer, (unsigned int) nSize);

    CAutoLock sl(m_mutex);
    if (m_pThread == nullptr)
        return false;
    m_qThreadMessage.push(message);
    if (m_qThreadMessage.size() == 1) //if queue has two or more message we don't dispatch a handle
        GetIoService().post(boost::bind(&CServerThread::Handle, this));
    return true;
}

