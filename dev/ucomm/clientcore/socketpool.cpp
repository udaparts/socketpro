
#if defined(OLD_IMPL)
#include "stdafx.h"
#include "clientsession.h"
#elif defined(_WIN32_WCE)|| defined(WIN32_64)
#include "../usocket/stdafx.h"
#include "../usocket/clientsession.h"
#else
#include "../ClientCoreUnix/stdafx.h"
#include "../ClientCoreUnix/clientsession.h"
#endif

#include "socketpool.h"
#include <algorithm>
#include "../../include/uclient.h"

#ifndef WINCE
extern std::mutex g_mutex;
#else
extern boost::mutex g_mutex;
#endif

unsigned int CSocketPool::m_nPoolId = 0;

CSocketPool::CSocketPool(PSocketPoolCallback spc, unsigned int maxSocketsPerThread, unsigned int threads, bool bAvg, SPA::tagThreadApartment ta)
: m_spc(spc), m_SocketsPerThread(maxSocketsPerThread), m_threads(threads), m_bAvg(bAvg), m_ta(ta), m_bKilling(false), m_bQueueAutoMerge(false), m_bDisconenctAll(false) {
    g_mutex.lock();
    m_poolId = ++m_nPoolId;
    g_mutex.unlock();
    if (m_spc) {
        m_spc(m_poolId, SPA::ClientSide::speStarted, nullptr);
    }
    while (threads) {
        AddOneThread();
        --threads;
    }
}

bool CSocketPool::IsKilling() {
    return m_bKilling;
}

void CSocketPool::SetKilling() {
    m_bKilling = true;
}

CSocketPool::~CSocketPool() {
    m_bKilling = true;
    DisconnectAll();
    {
        CClientThreadVector vec;
        {
            CAutoLock al(m_mutex);
            vec = m_vThread;
            m_vThread.clear();
        }
    }
    if (m_spc)
        m_spc(m_poolId, SPA::ClientSide::speShutdown, nullptr);
}

bool CSocketPool::AddOneThread() {
    if (m_spc) {
        m_spc(m_poolId, SPA::ClientSide::speCreatingThread, nullptr);
    }
    boost::shared_ptr<CClientThread> p(new CClientThread(m_spc, m_SocketsPerThread, this, m_ta));
    bool b = p->Start();
    if (!b)
        return false;
    CAutoLock al(m_mutex);
    m_vThread.push_back(p);
    PClientThread thread = p.get();
    m_qThread << thread;
    return true;
}

unsigned int CSocketPool::GetPoolId() {
    return m_poolId;
}

unsigned int CSocketPool::GetSocketsPerThread() {
    return m_SocketsPerThread;
}

bool CSocketPool::IsAvg() {
    return m_bAvg;
}

void CSocketPool::SortClosed() {
    int count = (int) (m_qThread.GetSize() / sizeof (PClientThread));
    PClientThread *arr = (PClientThread *) m_qThread.GetBuffer();
    for (int i = 1; i < count; ++i) {
        PClientThread key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j]->GetConnectedSockets() > key->GetConnectedSockets()) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

USocket_Client_Handle CSocketPool::FindAClosedSocket() {
    CAutoLock al(m_mutex);
    if (m_bAvg)
        SortClosed();
    unsigned int count = m_qThread.GetSize() / sizeof (PClientThread);
    PClientThread *arr = (PClientThread *) m_qThread.GetBuffer();
    for (unsigned int it = 0; it < count; ++it) {
        USocket_Client_Handle h = arr[it]->FindAClosedSocket();
        if (h)
            return h;
    }
    return nullptr;
}

void CSocketPool::OnFindClosed() {
    std::vector<CClientSession*> vClosed;
    if (m_bKilling)
        return;
    CAutoLock al(m_mutex);
    if (!m_bQueueAutoMerge || m_bDisconenctAll)
        return;
    for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
        std::vector<CClientSession*> v = (*it)->FindQueuedSessions();
        for (std::vector<CClientSession*>::iterator it = v.begin(), end = v.end(); it != v.end(); ++it) {
            vClosed.push_back(*it);
        }
    }
    for (std::vector<CClientSession*>::iterator it = vClosed.begin(), end = vClosed.end(); it != vClosed.end(); ++it) {
        OnCloseInternal(*it);
    }
}

void CSocketPool::OnCloseInternal(CClientSession *session) {
    boost::shared_ptr<MQ_FILE::CMqFile> q = session->GetQueue();
    if (!q || !q->IsAvailable())
        return;
    if (q->GetJobSize() > 0)
        return;
    if (q->GetMessageCount() == 0)
        return;
    CClientSession *found = nullptr;
    CClientSession *start = session;
    while (start->m_to) {
        found = start->m_to;
        start = found;
    }
    if (!found) {
        for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
            CClientSession *s = (*it)->SeekSmallQueue(session);
            if (s) {
                if (!found) {
                    found = s;
                } else if (found->GetQueue()->GetMessageCount() > s->GetQueue()->GetMessageCount()) {
                    found = s;
                }
            }
        }
    }
    session->m_to = found;
    if (found) {
        PSocketPoolCallback spc = m_spc;
        //make sure the two callacks are called in sequence within multi-thread environment which simplifies adapter merge implementation
        if (spc) {
            spc(m_poolId, SPA::ClientSide::speQueueMergedFrom, session);
        }
        USocket_Client_Handle h = found;
        bool ok = PushQueueTo(session, &h, 1);
        assert(ok);
        if (spc) {
            spc(m_poolId, SPA::ClientSide::speQueueMergedTo, found);
        }
    }
}

void CSocketPool::OnClose(CClientSession *session) {
    if (m_bKilling)
        return;
    CAutoLock al(m_mutex);
    if (!m_bQueueAutoMerge || m_bDisconenctAll)
        return;
    OnCloseInternal(session);
}

unsigned int CSocketPool::GetLockedSockets() {
    unsigned int count = 0;
    CAutoLock al(m_mutex);
    for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
        count += (*it)->GetLocked();
    }
    return count;
}

unsigned int CSocketPool::GetIdleSockets() {
    unsigned int count = 0;
    CAutoLock al(m_mutex);
    for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
        count += (*it)->GetIdleSockets();
    }
    return count;
}

unsigned int CSocketPool::GetConnectedSockets() {
    unsigned int count = 0;
    CAutoLock al(m_mutex);
    for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
        count += (*it)->GetConnectedSockets();
    }
    return count;
}

bool CSocketPool::DisconnectAll() {
    m_mutex.lock();
    std::vector<boost::shared_ptr<CClientThread> > temp(m_vThread);
    m_bDisconenctAll = true;
    m_mutex.unlock();

    for (CClientThreadVector::iterator it = temp.begin(), end = temp.end(); it != end; ++it) {
        (*it)->DisconnectAll();
    }

    //hack -- remove nix crash and no crash on windows found
    //boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    m_mutex.lock();
    m_bDisconenctAll = false;
    m_mutex.unlock();
    return true;
}

unsigned int CSocketPool::GetDisconnectedSockets() {
    size_t n, size;
    unsigned int count = 0;
    CAutoLock al(m_mutex);
    size = m_vThread.size();
    for (n = 0; n < size; ++n) {
        count += (m_vThread[n])->GetDisconnectedSockets();
    }
    return count;
}

void CSocketPool::PostTimerMessage() {
    size_t n, size;
    if (m_bKilling)
        return;
    CAutoLock al(m_mutex);
    size = m_vThread.size();
    for (n = 0; n < size; ++n) {
        (m_vThread[n])->PostTimerMessage();
    }
}

unsigned int CSocketPool::GetThreadCount() {
    m_mutex.lock();
    unsigned int size = (unsigned int) m_vThread.size();
    m_mutex.unlock();
    return size;
}

bool CSocketPool::WaitUtil(CAutoLock &al, unsigned int timeout) {
#ifndef WINCE
	using namespace std::chrono_literals;
	return (m_cv.wait_for(al, timeout * 1ms) == std::cv_status::no_timeout);
#else
    boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
    return m_cv.timed_wait(al, td);
#endif
}

void CSocketPool::SortLock() {
    int count = (int) (m_qThread.GetSize() / sizeof (PClientThread));
    PClientThread *arr = (PClientThread *) m_qThread.GetBuffer();
    for (int i = 1; i < count; ++i) {
        PClientThread key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j]->GetLockedEx() > key->GetLockedEx()) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

USocket_Client_Handle CSocketPool::LockClientSession(unsigned int timeout, USocket_Client_Handle ClientSession) {
    CClientSession *h;
    SPA::UINT64 diff;
    unsigned int open;
    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    if (ClientSession) {
        CAutoLock al(m_mutex);
        do {
            open = 0;
            h = nullptr;
            bool in = false;
            CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end();
            for (; it != end; ++it) {
                if ((*it)->Within(ClientSession)) {
                    h = ((*it)->Lock()).get();
                    in = true;
                    open += (*it)->GetConnectedSockets();
                    break;
                }
            }
            if (!in || h || !timeout)
                return (USocket_Client_Handle) h;
            diff = (boost::posix_time::microsec_clock::local_time() - start).total_milliseconds();
            if (diff >= timeout)
                break;
            else
                timeout -= (unsigned int) diff;
        } while (open && WaitUtil(al, timeout));
    } else {
        CAutoLock al(m_mutex);
        do {
            if (m_bAvg)
                SortLock();
            open = 0;
            unsigned int count = m_qThread.GetSize() / sizeof (PClientThread);
            PClientThread *arr = (PClientThread *) m_qThread.GetBuffer();
            for (unsigned int it = 0; it < count; ++it) {
                h = (arr[it]->Lock()).get();
                if (h)
                    return (USocket_Client_Handle) h;
                open += arr[it]->GetConnectedSockets();
            }
            if (!timeout)
                return nullptr;
            diff = (boost::posix_time::microsec_clock::local_time() - start).total_milliseconds();
            if (diff >= timeout)
                break;
            else
                timeout -= (unsigned int) diff;
        } while (open && WaitUtil(al, timeout));
    }
    return nullptr;
}

void CSocketPool::Notify() {
    m_cv.notify_all();
}

bool CSocketPool::GetQueueAutoMerge() {
    return m_bQueueAutoMerge;
}

void CSocketPool::SetQueueAutoMerge(bool autoMerge) {
    m_bQueueAutoMerge = autoMerge;
}

bool CSocketPool::UnlockClientSession(USocket_Client_Handle ClientSession) {
    if (!ClientSession)
        return false;
    m_mutex.lock();
    for (CClientThreadVector::iterator it = m_vThread.begin(), end = m_vThread.end(); it != end; ++it) {
        if ((*it)->Within(ClientSession)) {
            (*it)->Unlock(ClientSession);
            m_cv.notify_all();
            break;
        }
    }
    m_mutex.unlock();
    return false;
}