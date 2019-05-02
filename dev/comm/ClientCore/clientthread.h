#ifndef _UMB_CLIENT_THREAD_H__
#define _UMB_CLIENT_THREAD_H__

#include "../../include/definebase.h"
#include "../../include/uclient.h"
#include "../shared/uthread.h"
#include <boost/shared_ptr.hpp>

class CClientSession;
class CSocketPool;
class CClientThread;

struct LockState {

    LockState(CClientThread *ct = nullptr, bool lock = false) : ClientThread(ct), Locked(lock) {
    }

    CClientThread *ClientThread;
    bool Locked;
};

typedef std::pair<boost::shared_ptr<CClientSession>, LockState> CSessionState;
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
    boost::shared_ptr<CClientSession> Lock();
    bool Unlock(USocket_Client_Handle h);
    unsigned int GetLocked();
    unsigned int GetConnectedSockets();
    unsigned int GetIdleSockets();
    unsigned int GetDisconnectedSockets();
    void DisconnectAll();
    USocket_Client_Handle FindAClosedSocket();
    bool Within(USocket_Client_Handle h);
    CSslContext& GetSslContext();
    PSocketPoolCallback GetSocketPoolCallback();
    void PostTimerMessage();
    CClientSession* SeekSmallQueue(CClientSession *session);
    std::vector<CClientSession*> FindQueuedSessions();
    SPA::UINT64 GetLockedEx();

public:
    static const SPA::UINT64 LOCKED_REQUEST_COUNT = 0xFFFFFFFFFFFF;

protected:
    virtual void OnThreadStarted();
    virtual void OnThreadEnded();
    static bool SortUnlocked(const CSessionState &p0, const CSessionState &p1);

private:
    void TimerHandler();

public:

#ifndef NOT_USE_OPENSSL
    static bool verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context& ctx);
    static CSslContext m_sslContext;
#endif

private:
    PSocketPoolCallback m_spc;
    CMapClientSession m_mapClientSession;
    CSocketPool *m_pSocketPool;
    boost::mutex m_ml;
    unsigned int m_session;
    unsigned int m_msTimerInterval;

    class MyTimerSet {
        MyTimerSet();
        ~MyTimerSet();
        static volatile long m_stop;
        static boost::thread m_thread;
        static MyTimerSet ms;
        static void ThreadFunc();
        static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line);
        static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line);
        static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line);
    };
    friend class CUCertImpl;
};

typedef CClientThread* PCClientThread;

#endif

