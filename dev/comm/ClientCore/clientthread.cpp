#include "stdafx.h"
#include "clientthread.h"
#include "clientsession.h"
#include "socketpool.h"
#include "../shared/ucertimpl.h"
#include <boost/scoped_ptr.hpp>

extern boost::mutex g_Mutex;

extern boost::mutex g_mutex;
extern std::vector<CSocketPool*> g_vSocketPool;

boost::mutex g_mutexCvc;

#ifndef NOT_USE_OPENSSL
CSslContext CClientThread::m_sslContext(CSslContext::tls_client);
#else
CSslContext CClientThread::m_sslContext;
#endif

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

CClientThread::MyTimerSet::MyTimerSet() {
    CRYPTO_set_dynlock_create_callback(dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
}

CClientThread::MyTimerSet::~MyTimerSet() {
    CRYPTO_set_dynlock_create_callback(nullptr);
    CRYPTO_set_dynlock_lock_callback(nullptr);
    CRYPTO_set_dynlock_destroy_callback(nullptr);
    m_stop = 1;
}

void CClientThread::MyTimerSet::ThreadFunc() {
    while (!m_stop) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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
    }
    m_stop = 0;
}

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

CClientThread::MyTimerSet CClientThread::MyTimerSet::ms;

boost::thread CClientThread::MyTimerSet::m_thread(boost::bind(CClientThread::MyTimerSet::ThreadFunc));

#ifndef NOT_USE_OPENSSL

bool CClientThread::verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context& ctx) {
    CAutoLock al(g_mutexCvc);
    if (g_cvc) {
        X509_STORE_CTX *cts = ctx.native_handle();
        X509* cert = ::X509_STORE_CTX_get_current_cert(cts);
        int depth = ::X509_STORE_CTX_get_error_depth(cts);
        int errCode = X509_STORE_CTX_get_error(cts);
        const char *errMsg = X509_verify_cert_error_string(errCode);
        boost::scoped_ptr<CUCertImpl> pCert(new CUCertImpl(cert));
        return g_cvc(preverified, depth, errCode, errMsg, pCert.get());
    }
    return true;
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
    CAutoLock al(m_mutex);
    m_spc = nullptr;
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

CSslContext& CClientThread::GetSslContext() {
    return m_sslContext;
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

std::vector<CClientSession*> CClientThread::FindQueuedSessions() {
    std::vector<CClientSession*> vSession;
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened())
            continue;
        boost::shared_ptr<MQ_FILE::CMqFile> q = it->first->GetQueue();
        if (q && q->IsAvailable() && q->GetJobSize() == 0 && q->GetMessageCount() > 0) {
            vSession.push_back(it->first.get());
        }
    }
    return vSession;
}

CClientSession* CClientThread::SeekSmallQueue(CClientSession* session) {
    CClientSession *p = nullptr;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        CClientSession *s = it->first.get();
        if (s == session || s->GetConnectionState() < SPA::ClientSide::csSwitched)
            continue;
        if (s->m_hn == session->m_hn)
            continue;
        boost::shared_ptr<MQ_FILE::CMqFile> q = s->GetQueue();
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
            boost::shared_ptr<CClientSession> p(new CClientSession(GetIoService(), this));
            m_mutex.lock();
            m_mapClientSession.push_back(CSessionState(p, ls));
            m_mutex.unlock();
            --session;
        }
    }
    return bStart;
}

unsigned int CClientThread::GetLocked() {
    unsigned int data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            m_ml.lock();
            if (it->second.Locked)
                ++data;
            m_ml.unlock();
        }
    }
    return data;
}

SPA::UINT64 CClientThread::GetLockedEx() {
    SPA::UINT64 data = 0;
    //CAutoLock al(m_mutex);
    for (CMapClientSession::iterator it = m_mapClientSession.begin(), end = m_mapClientSession.end(); it != end; ++it) {
        if (it->first->IsOpened()) {
            CAutoLock alml(m_ml);
            if (it->second.Locked)
                data += LOCKED_REQUEST_COUNT;
            boost::shared_ptr<MQ_FILE::CMqFile> q = it->first->GetQueue();
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
        boost::shared_ptr<MQ_FILE::CMqFile> q = p0.first->GetQueue();
        if (q)
            p0_count = q->GetMessageCount();
        else
            p0_count = p0.first->GetCountOfRequestsInQueue();
    }
    SPA::UINT64 p1_count;
    if (p1.second.Locked) {
        p1_count = (~0);
    } else {
        boost::shared_ptr<MQ_FILE::CMqFile> q = p1.first->GetQueue();
        if (q)
            p1_count = q->GetMessageCount();
        else
            p1_count = p0.first->GetCountOfRequestsInQueue();
    }
    return (p0_count < p1_count);
}

boost::shared_ptr<CClientSession> CClientThread::Lock() {
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
    return boost::shared_ptr<CClientSession > ();
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