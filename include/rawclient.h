#ifndef _U_RAW_CLIENT_SOCKET_HEADER_H_
#define _U_RAW_CLIENT_SOCKET_HEADER_H_

#include "../include/ucomm.h"

namespace SPA {
    static const unsigned int LARGE_SENDING_BUFFER = 0x40000; //256 kilo bytes

    struct ISession {
    public:

        virtual ~ISession() {
        }

        virtual bool Connect(const char *host, unsigned int port, tagEncryptionMethod secure = tagEncryptionMethod::NoEncryption, bool v6 = false, bool sync = false, unsigned int timeout = 30000) = 0;
        virtual bool Shutdown(tagShutdownType st) = 0;
        virtual int GetErrorCode(char *em, unsigned int len) = 0;
        virtual bool IsConnected() = 0;
        virtual void Close() = 0;
        virtual int Send(const unsigned char *data, unsigned int bytes) = 0;
        virtual IUcert* GetUCert() = 0;
        virtual unsigned int GetOutBufferSize() = 0;
        virtual bool GetPeerName(unsigned int *port, char *addr, unsigned int chars) = 0;
    };
    typedef ISession *SessionHandle;

    struct ISessionPool {

        virtual ~ISessionPool() {
        }
        virtual bool IsBusy() = 0;
        virtual unsigned int GetSessions() = 0;
        virtual bool AddSession() = 0;
        virtual SessionHandle FindAClosedSession() = 0;
        virtual unsigned int GetConnectedSessions() = 0;
        virtual SessionHandle Lock(unsigned int timeout) = 0;
        virtual bool Unlock(SessionHandle session) = 0;
        virtual void CloseAll() = 0;
        virtual unsigned int ConnectAll(const char *host, unsigned int port, tagEncryptionMethod secure = tagEncryptionMethod::NoEncryption, bool v6 = false) = 0;
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
    typedef void(CALLBACK *PDataArrive) (SessionHandle, const unsigned char*, unsigned int);

}; //namespace SPA

SPA::SessionPoolHandle WINAPI CreateASessionPool(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta = SPA::tagThreadApartment::taNone);

void WINAPI SetCertVerifyCallback(PCertificateVerifyCallback cvc);
bool WINAPI SetVerify(const char *certFile);

#endif

