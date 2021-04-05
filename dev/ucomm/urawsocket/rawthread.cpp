#include "pch.h"
#include "rawthread.h"

CRawThread::CRawThread(PSessionCallback sc, unsigned int session, SPA::tagThreadApartment ta) : SPA::CUCommThread(ta), m_sc(sc), m_session(session) {
	if (m_sc) {
		m_sc(this, tagSessionEvent::seStarted, nullptr);
	}
}

CRawThread::~CRawThread() {
	Kill();
	if (m_sc) {
		m_sc(this, tagSessionEvent::seShutdown, nullptr);
	}
}

bool CRawThread::IsBusy() {
	return (GetIoService().poll_one(m_ec) > 0);
}

void CRawThread::OnThreadStarted() {
	if (m_sc) {
		m_sc(this, tagSessionEvent::seThreadCreated, nullptr);
	}
}

void CRawThread::OnThreadEnded() {
	if (m_sc) {
		m_sc(this, tagSessionEvent::seKillingThread, nullptr);
	}
}

PSessionCallback CRawThread::GetSessionCallback() {
	return m_sc;
}

unsigned int CRawThread::GetSessions() {
	CAutoLock al(m_mutex);
	return (unsigned int)m_mapSession.size();
}

bool CRawThread::AddSession() {
	if (!IsStarted()) {
		return false;
	}
	bool bStart = CUCommThread::Start();
	if (bStart) {
		LockState ls(this);
		PRawSession p = new CRawSession(GetIoService(), this);
		m_mutex.lock();
		m_mapSession.push_back(CSessionState(p, ls));
		m_mutex.unlock();
	}
	return bStart;
}

USessionHandle CRawThread::FindAClosedSession() {
	return nullptr;
}

unsigned int CRawThread::GetConnectedSessions() {
	return 0;
}

bool CRawThread::Start() {
	if (!IsStarted()) {
		if (m_sc) {
			m_sc(this, tagSessionEvent::seCreatingThread, nullptr);
		}
	}
	bool bStart = CUCommThread::Start();
	if (bStart) {
		unsigned int session = m_session;
		while (session) {
			LockState ls(this);
			PRawSession p = new CRawSession(GetIoService(), this);
			m_mutex.lock();
			m_mapSession.push_back(CSessionState(p, ls));
			m_mutex.unlock();
			--session;
		}
	}
	return bStart;
}

bool CRawThread::Kill() {
	bool ok = CUCommThread::Kill();
	{
		CMapSession map;
		{
			CAutoLock al(m_mutex);
			map.swap(m_mapSession);
		}
		for (auto it = map.begin(), end = map.end(); it != end; ++it) {
			delete it->first;
		}
	}
	if (ok && m_sc) {
		m_sc(this, tagSessionEvent::seThreadKilled, nullptr);
	}
	return ok;
}

USessionHandle CRawThread::Lock(unsigned int timeout) {
	return nullptr;
}

bool CRawThread::Unlock(USessionHandle session) {
	return false;
}

void CRawThread::CloseAll() {

}

bool CRawThread::IsStarted() {
	return SPA::CUCommThread::IsStarted();
}