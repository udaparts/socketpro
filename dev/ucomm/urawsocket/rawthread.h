#ifndef _U_CLIENT_RAW_THREAD_H_
#define _U_CLIENT_RAW_THREAD_H_

#include "../core_shared/shared/uthread.h"
#include "rawsession.h"

class U_MODULE_HIDDEN CRawThread : public IRawThread, public SPA::CUCommThread {

public:
	CRawThread(PSessionCallback sc, unsigned int session, SPA::tagThreadApartment ta);
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
	CRawThread& operator=(const CRawThread &rt) = delete;
	bool IsBusy();
	unsigned int GetSessions();
	bool AddSession();
	USessionHandle FindAClosedSession();
	unsigned int GetConnectedSessions();
	bool Start();
	bool Kill();
	USessionHandle Lock(unsigned int timeout);
	bool Unlock(USessionHandle session);
	PSessionCallback GetSessionCallback();
	void CloseAll();
	bool IsStarted();

protected:
	virtual void OnThreadStarted();
	virtual void OnThreadEnded();

private:
	PSessionCallback m_sc;
	unsigned int m_session;
	CMapSession m_mapSession;
};

#endif