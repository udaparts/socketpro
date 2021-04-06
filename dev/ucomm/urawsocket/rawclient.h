#ifndef _U_RAW_CLIENT_SOCKET_HEADER_H_
#define _U_RAW_CLIENT_SOCKET_HEADER_H_

#include "../include/ucomm.h"

static const unsigned int LARGE_SENDING_BUFFER = 0x40000; //256 kilo bytes

struct USessionBase {
public:

	virtual ~USessionBase() {
	}

	virtual bool Connect(const char *strHost, unsigned int nPort, SPA::tagEncryptionMethod secure, bool b6, bool bSync, unsigned int timeout) = 0;
	virtual bool Shutdown(SPA::tagShutdownType st) = 0;
	virtual int GetErrorCode(char *em, unsigned int len) = 0;
	virtual bool IsConnected() = 0;
	virtual void Close() = 0;
	virtual int Send(const unsigned char *data, unsigned int bytes) = 0;
};
typedef USessionBase *USessionHandle;

struct IRawThread {
	virtual ~IRawThread() {
	}
	virtual bool IsStarted() = 0;
	virtual bool IsBusy() = 0;
	virtual unsigned int GetSessions() = 0;
	virtual bool AddSession() = 0;
	virtual USessionHandle FindAClosedSession() = 0;
	virtual unsigned int GetConnectedSessions() = 0;
	virtual bool Start() = 0;
	virtual bool Kill() = 0;
	virtual USessionHandle Lock(unsigned int timeout) = 0;
	virtual bool Unlock(USessionHandle session) = 0;
	virtual void CloseAll() = 0;
	virtual unsigned int ConnectAll(const char *strHost, unsigned int nPort, SPA::tagEncryptionMethod secure, bool b6) = 0;
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
	seHandShakeCompleted,
	seLocked,
	seUnlocked,
	seThreadDestroyed,
	seSessionClosed,
	seSessionDestroyed,
	seTimer
};

enum class tagSessionState {
	ssClosed = 0,
	ssSslShaked,
	ssConnected
};

typedef void(CALLBACK *PSessionCallback) (PIRawThread, tagSessionEvent, USessionHandle);
typedef void(CALLBACK *PDataArrive) (USessionHandle, const unsigned char*, unsigned int);

PIRawThread WINAPI CreateSessions(PDataArrive da, PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta);


#endif
