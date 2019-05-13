#ifdef WINCE
#elif WIN32_64
#include "../usocket_win/clientsession.h"
#else
#include "../usocket/clientsession.h"
#endif
#include "socketpool.h"

#ifndef WINCE
extern std::mutex g_mutex;
std::mutex g_mutexCvc;
#else
extern boost::mutex g_mutex;
boost::mutex g_mutexCvc;
#endif

extern std::vector<CSocketPool*> g_vSocketPool;

volatile long CClientThread::MyTimerSet::m_stop = 0;

PCertificateVerifyCallback g_cvc = nullptr;

void WINAPI SetCertificateVerifyCallback(PCertificateVerifyCallback cvc) {
    CAutoLock al(g_mutexCvc);
    g_cvc = cvc;
#ifndef NOT_USE_OPENSSL
    if (g_cvc != nullptr) {
        CClientThread::m_sslContext.set_verify_mode(boost::asio::ssl::verify_peer);
        CClientThread::m_sslContext.set_verify_callback(boost::bind(&CClientThread::verify_certificate_cb, _1, _2));
    } else {
        CClientThread::m_sslContext.set_verify_mode(boost::asio::ssl::verify_none);
        //CClientThread::m_sslContext.set_verify_callback(nullptr);
    }
#endif
}

SPA::CUCommThread::thread* CClientThread::MyTimerSet::m_thread = nullptr;

void StartTimerThread() {
    if (!CClientThread::MyTimerSet::m_thread) {
        CClientThread::MyTimerSet::m_thread = new boost::thread(boost::bind(CClientThread::MyTimerSet::ThreadFunc));
        sleep(boost::posix_time::milliseconds(10));
    }
}

void StopTimerThread() {
    CClientThread::MyTimerSet::m_stop = 1;
    if (CClientThread::MyTimerSet::m_thread) {
        CClientThread::MyTimerSet::m_thread->join();
        delete CClientThread::MyTimerSet::m_thread;
        CClientThread::MyTimerSet::m_thread = nullptr;
    }
}

CClientThread::MyTimerSet::MyTimerSet() {
#ifndef NOT_USE_OPENSSL
    CRYPTO_set_dynlock_create_callback(dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
#endif
}

CClientThread::MyTimerSet::~MyTimerSet() {
    StopTimerThread();
#ifndef NOT_USE_OPENSSL
    CRYPTO_set_dynlock_create_callback(nullptr);
    CRYPTO_set_dynlock_lock_callback(nullptr);
    CRYPTO_set_dynlock_destroy_callback(nullptr);
#endif
}

void CClientThread::MyTimerSet::ThreadFunc() {
    while (!m_stop) {
        {
            size_t n, size;
            CAutoLock al(g_mutex);
            size = g_vSocketPool.size();
            for (n = 0; n < size; ++n) {
                CSocketPool *pool = g_vSocketPool[n];
                pool->Notify();
                if (pool->IsKilling())
                    continue;
                pool->PostTimerMessage();
            }
        }
        sleep(boost::posix_time::milliseconds(100));
    }
    m_stop = 0;
}

CClientThread::MyTimerSet CClientThread::MyTimerSet::ms;

#ifndef NOT_USE_OPENSSL
CSslContext CClientThread::m_sslContext(CSslContext::tls_client);

struct CRYPTO_dynlock_value* CClientThread::MyTimerSet::dyn_create_function(const char *file, int line) {
    boost::mutex *p = new boost::mutex;
    return (struct CRYPTO_dynlock_value*) p;
}

void CClientThread::MyTimerSet::dyn_lock_function(int mode, struct CRYPTO_dynlock_value *lock, const char *file, int line) {
    boost::mutex *p = (boost::mutex *)lock;
    if (mode & CRYPTO_LOCK) {
        p->lock();
    } else {
        p->unlock();
    }
}

void CClientThread::MyTimerSet::dyn_destroy_function(struct CRYPTO_dynlock_value *lock, const char *file, int line) {
    boost::mutex *p = (boost::mutex *)lock;
    delete p;
}

bool CClientThread::verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context& ctx) {
    CAutoLock al(g_mutexCvc);
    if (g_cvc) {
        X509_STORE_CTX *cts = ctx.native_handle();
        X509* cert = ::X509_STORE_CTX_get_current_cert(cts);
        int depth = ::X509_STORE_CTX_get_error_depth(cts);
        int errCode = X509_STORE_CTX_get_error(cts);
        const char *errMsg = X509_verify_cert_error_string(errCode);
        CCertificateImplPtr pCert(new CUCertImpl(cert));
        return g_cvc(preverified, depth, errCode, errMsg, pCert.get());
    }
    return true;
}

CSslContext& CClientThread::GetSslContext() {
    return m_sslContext;
}

#endif


//we don't use a mutex to lock m_mapClientSession because it is controlled by thread pool

CClientThread::CClientThread(PSocketPoolCallback spc, unsigned int session, CSocketPool *pSocketPool, SPA::tagThreadApartment ta)
: SPA::CUCommThread(ta),
m_spc(spc),
m_pSocketPool(pSocketPool),
m_session(session),
m_msTimerInterval(500) {

}

CClientThread::~CClientThread() {
    Kill();
    {
        CMapClientSession map;
        {
            CAutoLock al(m_mutex);
            map = m_mapClientSession;
            m_mapClientSession.clear();
        }
    }
}

void CClientThread::OnThreadStarted() {
    if (m_spc) {
        m_spc(GetPool()->GetPoolId(), SPA::ClientSide::speThreadCreated, nullptr);
    }
}

void CClientThread::OnThreadEnded() {
    if (m_spc) {
        m_spc(GetPool()->GetPoolId(), SPA::ClientSide::speKillingThread, nullptr);
    }
}

unsigned int CClientThread::GetTimerInterval() const {
    return m_msTimerInterval;
}

CSocketPool* CClientThread::GetPool() const {
    return m_pSocketPool;
}

void CClientThread::PostTimerMessage() {
    GetIoService().post(boost::bind(&CClientThread::TimerHandler, this));
}

void CClientThread::TimerHandler() {
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        it->first->TimerHandler();
    }
}

unsigned int CClientThread::GetCountOfSessions() {
    //CAutoLock al(m_mutex);
    return (unsigned int) (m_mapClientSession.size());
}

bool CClientThread::Kill() {
    bool ok = CUCommThread::Kill();
    if (m_spc) {
        m_spc(m_pSocketPool->GetPoolId(), SPA::ClientSide::speThreadKilled, nullptr);
    }
    return ok;
}

bool CClientThread::IsBusy() {
    return (GetIoService().poll_one(m_ec) > 0);
}

bool CClientThread::Start() {
    bool bStart = CUCommThread::Start();
    if (bStart) {
        unsigned int session = m_session;
        while (session) {
            LockState ls(this);
            CClientSessionPtr p(new CClientSession(GetIoService(), this));
            m_mutex.lock();
            m_mapClientSession.push_back(CSessionState(p, ls));
            m_mutex.unlock();
            --session;
        }
    }
    return bStart;
}

SPA::UINT64 CClientThread::GetLockedEx() {
    SPA::UINT64 data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            CAutoLock alml(m_ml);
            if (it->second.Locked)
                data += LOCKED_REQUEST_COUNT;
            MQ_FILE::CFilePtr q = it->first->GetQueue();
            if (q) {
                data += q->GetMessageCount();
            } else {
                data += it->first->GetCountOfRequestsInQueue();
            }
        } else {
            data += (LOCKED_REQUEST_COUNT * 4);
        }
    }
    return data;
}

unsigned int CClientThread::GetLocked() {
    unsigned int data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            CAutoLock alml(m_ml);
            if (it->second.Locked)
                ++data;
        }
    }
    return data;
}

PSocketPoolCallback CClientThread::GetSocketPoolCallback() {
    return m_spc;
}

bool CClientThread::Within(USocket_Client_Handle h) {
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first.get() == h) {
            return true;
        }
    }
    return false;
}

unsigned int CClientThread::GetConnectedSockets() {
    unsigned int data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            ++data;
        }
    }
    return data;
}

unsigned int CClientThread::GetDisconnectedSockets() {
    unsigned int data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (!it->first->IsOpened()) {
            ++data;
        }
    }
    return data;
}

unsigned int CClientThread::GetIdleSockets() {
    unsigned int data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        CClientSession *p = it->first.get();
        if (p->IsOpened() && p->GetCountOfRequestsInQueue() == 0) {
            ++data;
        }
    }
    return data;
}

bool CClientThread::SortUnlocked(const CSessionState &p0, const CSessionState &p1) {
    SPA::UINT64 p0_count;
    if (p0.second.Locked) {
        p0_count = (~0);
    } else {
        MQ_FILE::CFilePtr q = p0.first->GetQueue();
        if (q)
            p0_count = q->GetMessageCount();
        else
            p0_count = p0.first->GetCountOfRequestsInQueue();
    }
    SPA::UINT64 p1_count;
    if (p1.second.Locked) {
        p1_count = (~0);
    } else {
        MQ_FILE::CFilePtr q = p1.first->GetQueue();
        if (q)
            p1_count = q->GetMessageCount();
        else
            p1_count = p0.first->GetCountOfRequestsInQueue();
    }
    return (p0_count < p1_count);
}

CClientSessionPtr CClientThread::Lock() {
    //CAutoLock al(m_mutex);
    std::sort(m_mapClientSession.begin(), m_mapClientSession.end(), SortUnlocked);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->GetConnectionState() >= SPA::ClientSide::csSwitched) {
            m_ml.lock();
            if (!it->second.Locked) {
                it->second.Locked = true;
                m_ml.unlock();
                if (m_spc) {
                    //CRAutoLock al(m_mutex);
                    m_spc(m_pSocketPool->GetPoolId(), SPA::ClientSide::speLocked, it->first.get());
                }
                return it->first;
            } else
                m_ml.unlock();
        } else {
            m_ml.lock();
            it->second.Locked = false;
            m_ml.unlock();
        }
    }
    return nullptr;
}

USocket_Client_Handle CClientThread::FindAClosedSocket() {
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (!it->first->IsOpened()) {
            return it->first.get();
        }
    }
    return nullptr;
}

std::vector<CClientSession*> CClientThread::FindQueuedSessions() {
    std::vector<CClientSession*> vSession;
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened())
            continue;
        MQ_FILE::CFilePtr q = it->first->GetQueue();
        if (q && q->IsAvailable() && q->GetJobSize() == 0 && q->GetMessageCount() > 0) {
            vSession.push_back(it->first.get());
        }
    }
    return vSession;
}

CClientSession* CClientThread::SeekSmallQueue(CClientSession* session) {
    CClientSession *p = nullptr;
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        CClientSession *s = it->first.get();
        if (s == session || s->GetConnectionState() < SPA::ClientSide::csSwitched)
            continue;
        if (s->m_hn == session->m_hn)
            continue;
        MQ_FILE::CFilePtr q = s->GetQueue();
        if (q && q->IsAvailable() && q->GetJobSize() == 0) {
            if (!p) {
                p = s;
            } else if (p->GetQueue()->GetMessageCount() > q->GetMessageCount()) {
                p = s;
            }
        }
    }
    return p;
}

void CClientThread::DisconnectAll() {
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            m_ml.lock();
            it->second.Locked = false;
            m_ml.unlock();
            //GetIoService().post(boost::bind(&CClientSession::Close, it->first.get()));
            //CRAutoLock ral(m_mutex);
            it->first->Close();
        }
    }
}

bool CClientThread::Unlock(USocket_Client_Handle p) {
    if (!p)
        return false;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (p == it->first.get()) {
            m_ml.lock();
            it->second.Locked = false;
            m_ml.unlock();
            m_pSocketPool->Notify();
            if (m_spc) {
                //CRAutoLock al(m_mutex);
                m_spc(m_pSocketPool->GetPoolId(), SPA::ClientSide::speUnlocked, it->first.get());
            }
            return true;
        }
    }
    return false;
}
