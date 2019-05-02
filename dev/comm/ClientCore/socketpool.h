
#ifndef __UMB_SOCKET_POOL_H__
#define __UMB_SOCKET_POOL_H__

#include <boost/atomic.hpp>

#ifdef OLD_IMPL
#include "clientthread.h"
#elif defined(_WIN32_WCE) || defined(WIN32_64)
#include "../ClientCoreWin/clientthread.h"
#else
#include "../ClientCoreUnix/clientthread.h"
#endif

class CSocketPool : boost::noncopyable {
public:
    CSocketPool(PSocketPoolCallback spc, unsigned int socketsPerThread, unsigned int threads, bool bAvg, SPA::tagThreadApartment ta);
    virtual ~CSocketPool();

public:
    bool IsAvg();
    unsigned int GetPoolId();
    unsigned int GetSocketsPerThread();
    unsigned int GetThreadCount();
    unsigned int GetDisconnectedSockets();
    bool AddOneThread();
    USocket_Client_Handle FindAClosedSocket();
    unsigned int GetLockedSockets();
    unsigned int GetIdleSockets();
    unsigned int GetConnectedSockets();
    bool DisconnectAll();
    USocket_Client_Handle LockClientSession(unsigned int timeout, USocket_Client_Handle ClientSession);
    bool UnlockClientSession(USocket_Client_Handle ClientSession);
    void Notify();
    bool IsKilling();
    void SetKilling();
    void PostTimerMessage();
    typedef std::vector<boost::shared_ptr<CClientThread> > CClientThreadVector;
    typedef CClientThread* PClientThread;
    bool GetQueueAutoMerge();
    void SetQueueAutoMerge(bool autoMerge);
    void OnClose(CClientSession *session);
    void OnFindClosed();

private:
    bool WaitUtil(CAutoLock &al, unsigned int timeout);
    void OnCloseInternal(CClientSession *session);
    void SortLock();
    void SortClosed();

private:
    PSocketPoolCallback m_spc;
    unsigned int m_SocketsPerThread;
    unsigned int m_threads;
    bool m_bAvg;
    unsigned int m_poolId;
    static unsigned int m_nPoolId;
    CClientThreadVector m_vThread;
    SPA::CUQueue m_qThread;
    boost::mutex m_mutex;
    CConditionVariable m_cv;
    SPA::tagThreadApartment m_ta;
    boost::atomic<bool> m_bKilling;
    boost::atomic<bool> m_bQueueAutoMerge;
    bool m_bDisconenctAll;
};

#endif


