
#if defined(OLD_IMPL)
#include "stdafx.h"
#include "clientsession.h"
#elif defined(_WIN32_WCE) || defined(WIN32_64)
#include "../usocket/stdafx.h"
#include "../usocket/clientsession.h"
#else
#include "../ClientCoreUnix/stdafx.h"
#include "../ClientCoreUnix/clientsession.h"
#endif

#include "socketpool.h"
#include "../core_shared/pinc/getsysid.h"
#include <boost/filesystem.hpp>

#ifndef WINCE
std::mutex g_mutex;
#else
boost::mutex g_mutex;
#endif
std::vector<CSocketPool*> g_vSocketPool;
std::string g_localhost;

unsigned int GetDefaultSocketsPerThread() {
    return 2;
}

CSocketPool* FindSocketPool(unsigned int poolId) {
    size_t n, size = g_vSocketPool.size();
    for (n = 0; n < size; ++n) {
        CSocketPool *p = g_vSocketPool[n];
        if (p->GetPoolId() == poolId)
            return p;
    }
    return nullptr;
}

unsigned int WINAPI GetNumberOfSocketPools() {
    CAutoLock al(g_mutex);
    return (unsigned int) g_vSocketPool.size();
}

unsigned int WINAPI CreateSocketPool(PSocketPoolCallback spc, unsigned int maxSocketsPerThread, unsigned int maxThreads, bool bAvg, SPA::tagThreadApartment ta) {
#ifndef WIN32_64
    ta = SPA::taNone;
#endif
    if (maxSocketsPerThread == 0)
        maxSocketsPerThread = GetDefaultSocketsPerThread();
    if (maxThreads == 0)
        maxThreads = GetNumberOfCores();
    {
        CAutoLock al(g_mutex);
		StartTimerThread();
        if (!g_localhost.size()) {
            char str[256] = {0};
            ::gethostname(str, sizeof (str));
            g_localhost = str;
            std::transform(g_localhost.begin(), g_localhost.end(), g_localhost.begin(), ::tolower);
        }
        if (g_vSocketPool.size() == 0) {
            if (CClientSession::m_WorkingPath.size() == 0) {
#ifdef WINCE

#elif defined(WIN32_64)
                boost::filesystem::path cp = boost::filesystem::current_path();
                std::string path = cp.generic_string();
                std::replace(path.begin(), path.end(), '/', '\\');
                CRAutoLock ral(g_mutex);
                SetClientWorkDirectory(path.c_str());
#else
                boost::filesystem::path cp = boost::filesystem::current_path();
                CRAutoLock ral(g_mutex);
                SetClientWorkDirectory(cp.c_str());
#endif
            }
            std::string path = CClientSession::m_WorkingPath + SPA::GetAppName();
            CClientSession::m_pQLastIndex.reset(new MQ_FILE::CQLastIndex(path.c_str(), true));
        }
    }
    CSocketPool *p = new CSocketPool(spc, maxSocketsPerThread, maxThreads, bAvg, ta);
    CAutoLock al(g_mutex);
    g_vSocketPool.push_back(p);
    return p->GetPoolId();
}

bool WINAPI DestroySocketPool(unsigned int poolId) {
    CSocketPool *p = nullptr;
    {
        CAutoLock al(g_mutex);
        for (std::vector<CSocketPool*>::iterator it = g_vSocketPool.begin(), end = g_vSocketPool.end(); it != end; ++it) {
            if ((*it)->GetPoolId() != poolId)
                continue;
            p = (*it);
            p->SetKilling();
            g_vSocketPool.erase(it);
            break;
        }
    }
    bool b = (p != nullptr);
    delete p;
    g_mutex.lock();
    if (g_vSocketPool.size() == 0) {
        g_mutex.unlock();
        if (CClientSession::m_pQLastIndex) {
            CClientSession::m_pQLastIndex->Stop();
            CClientSession::m_pQLastIndex.reset();
        }
		yield();
    } else {
        g_mutex.unlock();
    }
    return b;
}

unsigned int WINAPI GetSocketsPerThread(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetSocketsPerThread();
}

unsigned int WINAPI GetThreadCount(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetThreadCount();
}

bool WINAPI IsAvg(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return true;
    return p->IsAvg();
}

bool WINAPI GetQueueAutoMergeByPool(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p) {
        return false;
    }
    return p->GetQueueAutoMerge();
}

void WINAPI SetQueueAutoMergeByPool(unsigned int poolId, bool autoMerge) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (p) {
        p->SetQueueAutoMerge(autoMerge);
    }
}

unsigned int WINAPI GetDisconnectedSockets(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetDisconnectedSockets();
}

USocket_Client_Handle WINAPI FindAClosedSocket(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return nullptr;
    return p->FindAClosedSocket();
}

bool WINAPI AddOneThreadIntoPool(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return false;
    return p->AddOneThread();
}

unsigned int WINAPI GetLockedSockets(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetLockedSockets();
}

unsigned int WINAPI GetIdleSockets(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetIdleSockets();
}

unsigned int WINAPI GetConnectedSockets(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return 0;
    return p->GetConnectedSockets();
}

bool WINAPI DisconnectAll(unsigned int poolId) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return false;
    return p->DisconnectAll();
}

USocket_Client_Handle WINAPI LockASocket(unsigned int poolId, unsigned int timeout, USocket_Client_Handle hSameThread) {
    CSocketPool *p;
    {
        CAutoLock al(g_mutex);
        p = FindSocketPool(poolId);
        if (nullptr == p)
            return nullptr;
    }
    return p->LockClientSession(timeout, hSameThread);
}

bool WINAPI UnlockASocket(unsigned int poolId, USocket_Client_Handle h) {
    CAutoLock al(g_mutex);
    CSocketPool *p = FindSocketPool(poolId);
    if (nullptr == p)
        return false;
    return p->UnlockClientSession(h);
}