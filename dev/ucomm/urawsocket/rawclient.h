#ifndef _U_RAW_CLIENT_SOCKET_HEADER_H_
#define _U_RAW_CLIENT_SOCKET_HEADER_H_

#include "../include/commutil.h"

struct USessionBase {
public:

	virtual ~USessionBase() {
	}
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
};

typedef IRawThread *PIRawThread;

enum class tagSessionEvent {
	seUnknown = -1,
	seStarted = 0,
	seCreatingThread,
	seThreadCreated,
	seConnecting,
	seConnected,
	seKillingThread,
	seShutdown,
	seSessionCreated,
	seHandShakeCompleted,
	seLocked,
	seUnlocked,
	seThreadKilled,
	seClosingSession,
	seSessionClosed,
	seSessionKilled,
	seTimer
};

typedef void(CALLBACK *PSessionCallback) (PIRawThread, tagSessionEvent, USessionHandle);

PIRawThread WINAPI CreateSessions(PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta);

#endif