#include "stdafx.h"
#include "serverthread_win.h"
#include "server.h"

#define THREAD_SAFE_ALIVE_TIME	1000

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
    size_t size;
    unsigned int res;
    unsigned int nMsgId;
    SPA::CUThreadMessage message;
    {
        CAutoLock sl(m_mutex);
        m_bBusy = true;
        size = m_qThreadMessage.size();
    }
    PSession pSession;

    while (size) {
        {
            CAutoLock sl(m_mutex);
            message = m_qThreadMessage.front();
            m_qThreadMessage.pop();
        }

        nMsgId = message.m_nMsgId;
        *(message.m_pMessageBuffer) >> pSession;
        m_tWorking = boost::posix_time::microsec_clock::local_time();

        switch (nMsgId) {
            case WM_ASK_FOR_PROCESSING:
                res = ProcessSlowRequest(pSession, message);
                break;
            default:
                break;
        }

        m_tWorking = boost::posix_time::microsec_clock::local_time();

        {
            CAutoLock sl(m_mutex);
            size = m_qThreadMessage.size();
        }

        message.m_nMsgId = WM_REQUEST_PROCESSED;
        message.m_pMessageBuffer->SetSize(0);
        *message.m_pMessageBuffer << pSession;
        *message.m_pMessageBuffer << res;
        g_pServer->PostSproMessage(message);
    }

    {
        CAutoLock sl(m_mutex);
        m_bBusy = false;
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

bool CServerThread::PostMessage(CServerSession *pSession, unsigned short uRequestId, unsigned int nMsgId, const void *pBuffer, unsigned int nSize) {
    SPA::CUThreadMessage message;
    message.m_nMsgId = nMsgId;
    message.m_uRequestId = uRequestId;
    message.m_pMessageBuffer = SPA::CScopeUQueue::Lock();

    PSession session = pSession;
    *(message.m_pMessageBuffer) << session;

    if (pBuffer && nSize)
        message.m_pMessageBuffer->Push((const unsigned char*) pBuffer, (unsigned int) nSize);

    CAutoLock sl(m_mutex);
    if (m_pThread == NULL)
        return false;
    m_qThreadMessage.push(message);
    if (m_qThreadMessage.size() == 1) //if queue has two or more message we don't dispatch a handle
        GetIoService().post(boost::bind(&CServerThread::Handle, this));
    return true;
}

