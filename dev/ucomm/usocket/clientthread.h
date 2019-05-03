
#ifndef _UMB_CLIENT_THREAD_H__
#define _UMB_CLIENT_THREAD_H__

#include "../include/definebase.h"
#include "../include/uclient.h"
#include "../core_shared/shared/uthread.h"

class CClientSession;
class CSocketPool;
class CClientThread;

struct LockState {

    LockState(CClientThread *ct = nullptr, bool lock = false) : ClientThread(ct), Locked(lock) {
    }

    CClientThread *ClientThread;
    bool Locked;
};

#ifndef WINCE
typedef std::shared_ptr<CClientSession> CClientSessionPtr;
#else
typedef boost::shared_ptr<CClientSession> CClientSessionPtr;
#endif

typedef std::pair<CClientSessionPtr, LockState> CSessionState;
typedef std::vector<CSessionState> CMapClientSession;

class CClientThread : public SPA::CUCommThread {
public:
    CClientThread(PSocketPoolCallback spc, unsigned int session, CSocketPool *pSocketPool, SPA::tagThreadApartment ta);
    ~CClientThread();

public:
    unsigned int GetCountOfSessions();
    bool Start();
    bool Kill();
    bool IsBusy();
    CSocketPool* GetPool() const;
    unsigned int GetTimerInterval() const;
	CClientSessionPtr Lock();
    bool Unlock(USocket_Client_Handle h);
    unsigned int GetLocked();
    unsigned int GetConnectedSockets();
    unsigned int GetIdleSockets();
    unsigned int GetDisconnectedSockets();
    void DisconnectAll();
    USocket_Client_Handle FindAClosedSocket();
    bool Within(USocket_Client_Handle h);

    PSocketPoolCallback GetSocketPoolCallback();
    void PostTimerMessage();
    CClientSession* SeekSmallQueue(CClientSession *session);
    std::vector<CClientSession*> FindQueuedSessions();
    SPA::UINT64 GetLockedEx();

protected:
    virtual void OnThreadStarted();
    virtual void OnThreadEnded();
    static bool SortUnlocked(const CSessionState &p0, const CSessionState &p1);

private:
    void TimerHandler();

public:
    static const SPA::UINT64 LOCKED_REQUEST_COUNT = 0xFFFFFFFFFFFF;

private:
    PSocketPoolCallback m_spc;
    CMapClientSession m_mapClientSession;
    CSocketPool *m_pSocketPool;
#ifndef WINCE
	using mutex = std::mutex;
#else
	using mutex = boost::mutex;
#endif
    mutex m_ml;
    unsigned int m_session;
    unsigned int m_msTimerInterval;

public:
    class MyTimerSet {
	public:
#ifndef WINCE
		using thread = std::thread;
#else
		using thread = boost::thread;
#endif
        MyTimerSet();
        ~MyTimerSet();
        static volatile long m_stop;
        static thread *m_thread;
        static MyTimerSet ms;
        static void ThreadFunc();
    };
};

typedef CClientThread* PCClientThread;
#ifndef WINCE
typedef std::shared_ptr<CClientThread> CClientThreadPtr;
#else
typedef boost::shared_ptr<CClientThread> CClientThreadPtr;
#endif

#endif

