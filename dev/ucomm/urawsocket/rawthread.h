#ifndef _U_CLIENT_RAW_THREAD_H_
#define _U_CLIENT_RAW_THREAD_H_

#include "../core_shared/shared/uthread.h"
#include "rawsession.h"

extern std::mutex g_mutexCvc;
extern PCertificateVerifyCallback g_cvc;

namespace SPA {

    class U_MODULE_HIDDEN CRawThread : public ISessionPool, public CUCommThread {
		CScopeUQueue m_sb;

    public:
        CRawThread(PDataArrive da, PSessionCallback sc, unsigned int sessions, tagThreadApartment ta);
        CRawThread(const CRawThread &rt) = delete;
        ~CRawThread();

        struct U_MODULE_HIDDEN LockState {

            LockState(CRawThread *rt = nullptr, bool lock = false) : RawThread(rt), Locked(lock) {
            }

            CRawThread *RawThread;
            bool Locked;
        };

        typedef std::pair<PRawSession, LockState> CSessionState;
        typedef std::vector<CSessionState> CMapSession;

    public:
        UTHREAD_ID GetThreadId();
        CRawThread& operator=(const CRawThread &rt) = delete;
        bool IsBusy();
        unsigned int GetSessions();
        bool AddSession();
        SessionHandle FindAClosedSession();
        unsigned int GetConnectedSessions();
        bool Start(unsigned int sessions);
        bool Kill();
        SessionHandle Lock(unsigned int timeout);
        bool Unlock(SessionHandle session);
        PSessionCallback GetSessionCallback();
        void CloseAll();
        bool IsStarted();
        unsigned int ConnectAll(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6);

#ifndef NOT_USE_OPENSSL
        CSslContext& GetSslContext();
        static bool verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context& ctx);
        static CSslContext m_sslContext;

        struct MyTimerSet {
            MyTimerSet();
            ~MyTimerSet();
            static MyTimerSet ms;
            static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line);
            static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line);
            static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line);
        };
#endif

    protected:
        virtual void OnThreadStarted();
        virtual void OnThreadEnded();

	public:
		SPA::CUQueue &m_buff;

    private:
        PDataArrive m_da;
        PSessionCallback m_sc;
        CMapSession m_mapSession;
        std::atomic<UTHREAD_ID> m_id;
        CConditionVariable m_cv;
    };

}; //namespace SPA;

#endif