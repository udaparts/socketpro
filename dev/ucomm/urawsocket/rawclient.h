#ifndef _U_RAW_CLIENT_SOCKET_HEADER_H_
#define _U_RAW_CLIENT_SOCKET_HEADER_H_

#include "../include/ucomm.h"

namespace SPA {
	static const unsigned int LARGE_SENDING_BUFFER = 0x40000; //256 kilo bytes

	struct USessionBase {
	public:

		virtual ~USessionBase() {
		}

		virtual bool Connect(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6, bool bSync, unsigned int timeout) = 0;
		virtual bool Shutdown(tagShutdownType st) = 0;
		virtual int GetErrorCode(char *em, unsigned int len) = 0;
		virtual bool IsConnected() = 0;
		virtual void Close() = 0;
		virtual int Send(const unsigned char *data, unsigned int bytes) = 0;
		virtual IUcert* GetUCert() = 0;
		virtual unsigned int GetSendBufferSize() = 0;
	};
	typedef USessionBase *USessionHandle;

	struct IRawThread {
		virtual ~IRawThread() {
		}
		virtual bool IsBusy() = 0;
		virtual unsigned int GetSessions() = 0;
		virtual bool AddSession() = 0;
		virtual USessionHandle FindAClosedSession() = 0;
		virtual unsigned int GetConnectedSessions() = 0;
		virtual USessionHandle Lock(unsigned int timeout) = 0;
		virtual bool Unlock(USessionHandle session) = 0;
		virtual void CloseAll() = 0;
		virtual unsigned int ConnectAll(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6) = 0;
	};

	typedef IRawThread *PIRawThread;

	enum class tagSessionEvent {
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

	typedef void(CALLBACK *PSessionCallback) (PIRawThread, tagSessionEvent, USessionHandle);
	typedef void(CALLBACK *PDataArrive) (USessionHandle, const unsigned char*, unsigned int);

}; //namespace SPA

SPA::PIRawThread WINAPI CreateSessions(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta);

void WINAPI SetCertVerifyCallback(PCertificateVerifyCallback cvc);
bool WINAPI SetVerify(const char *certFile);

#endif
