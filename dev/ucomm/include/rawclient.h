#ifndef _U_RAW_CLIENT_SOCKET_HEADER_H_
#define _U_RAW_CLIENT_SOCKET_HEADER_H_

#include "../include/ucomm.h"

namespace SPA {

    struct ISession {
    public:

        virtual ~ISession() {
        }
    };
    typedef ISession *SessionHandle;

    struct ISessionPool {

        virtual ~ISessionPool() {
        }
    };

    typedef ISessionPool *SessionPoolHandle;

    enum class tagSessionPoolEvent {
        seUnknown = -1,
        seStarted = 0,
        seCreatingThread,
        seThreadCreated,
        seConnected,
        seKillingThread,
        seShutdown,
        seSessionCreated,
        seSslShaking,
        seLocked,
        seUnlocked,
        seThreadDestroyed,
        seSessionClosed,
        seSessionDestroyed,
        seTimer
    };

    enum class tagSessionState {
        ssClosed = 0,
        ssSslShaking,
        ssConnected
    };

    typedef void(CALLBACK *PSessionCallback) (SessionPoolHandle, tagSessionPoolEvent, SessionHandle);
    typedef void(CALLBACK *PDataArrive) (SessionPoolHandle, SessionHandle, const unsigned char*, unsigned int);

}; //namespace SPA

#ifdef __cplusplus
extern "C" {
#endif

    SPA::SessionPoolHandle WINAPI CreateASessionPool(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta = SPA::tagThreadApartment::taNone);
    void WINAPI DestroyASessionPool(SPA::SessionPoolHandle sph);
    void WINAPI SetCertVerifyCallback(PCertificateVerifyCallback cvc);
    bool WINAPI SetVerify(const char *certFile);

    //session pool
    unsigned int WINAPI SPH_GetSessions(SPA::SessionPoolHandle sph);
    bool WINAPI SPH_AddSession(SPA::SessionPoolHandle sph);
    SPA::SessionHandle WINAPI SPH_FindAClosedSession(SPA::SessionPoolHandle sph);
    unsigned int WINAPI SPH_GetConnectedSessions(SPA::SessionPoolHandle sph);
    SPA::SessionHandle WINAPI SPH_Lock(SPA::SessionPoolHandle sph, unsigned int msTimeout);
    bool WINAPI SPH_Unlock(SPA::SessionPoolHandle sph, SPA::SessionHandle session);
    void WINAPI SPH_CloseAll(SPA::SessionPoolHandle sph);
    unsigned int WINAPI SPH_ConnectAll(SPA::SessionPoolHandle sph, const char *host, unsigned int port, SPA::tagEncryptionMethod secure = SPA::tagEncryptionMethod::NoEncryption, bool v6 = false);
    bool WINAPI SPH_Kill(SPA::SessionPoolHandle sph);

    //session
    bool WINAPI SH_Connect(SPA::SessionHandle session, const char *host, unsigned int port, SPA::tagEncryptionMethod secure = SPA::tagEncryptionMethod::NoEncryption, bool v6 = false, bool sync = false, unsigned int msTimeout = 30000);
    bool WINAPI SH_Shutdown(SPA::SessionHandle session, SPA::tagShutdownType st);
    int WINAPI SH_GetErrorCode(SPA::SessionHandle session, char *em, unsigned int len);
    bool WINAPI SH_IsConnected(SPA::SessionHandle session);
    void WINAPI SH_Close(SPA::SessionHandle session);
    int WINAPI SH_Send(SPA::SessionHandle session, const unsigned char *data, unsigned int bytes);
    SPA::IUcert* WINAPI SH_GetUCert(SPA::SessionHandle session);
    unsigned int WINAPI SH_GetOutBufferSize(SPA::SessionHandle session);
    bool WINAPI SH_GetPeerName(SPA::SessionHandle session, unsigned int *port, char *addr, unsigned int chars);


#ifdef __cplusplus
}
#endif

#endif

