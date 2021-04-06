#include "pch.h"
#include "rawthread.h"

std::mutex g_mutexCvc;
PCertificateVerifyCallback g_cvc = nullptr;

void WINAPI SetCertVerifyCallback(PCertificateVerifyCallback cvc) {
	CAutoLock al(g_mutexCvc);
	g_cvc = cvc;
#ifndef WIN32_64

#endif
}

bool WINAPI SetVerify(const char *certFile) {
#ifdef WIN32_64
	return SPA::CCertificateImpl::SetVerifyLocation(certFile);
#else
	return CUCertImpl::SetVerifyLocation(certFile);
#endif
}

CRawThread::CRawThread(PDataArrive da, PSessionCallback sc, unsigned int session, SPA::tagThreadApartment ta) : SPA::CUCommThread(ta), m_da(da), m_sc(sc), m_session(session), m_id(0) {
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
#ifdef WIN32_64
	m_id = ::GetCurrentThreadId();
#else
	m_id = ::pthread_self();
#endif
	if (m_sc) {
		m_sc(this, tagSessionEvent::seThreadCreated, nullptr);
	}
}

void CRawThread::OnThreadEnded() {
	if (m_sc) {
		m_sc(this, tagSessionEvent::seKillingThread, nullptr);
	}
	m_id = 0;
}

UTHREAD_ID CRawThread::GetThreadId() {
	return m_id;
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
		PRawSession p = new CRawSession(GetIoService(), *this, m_da);
		m_mutex.lock();
		m_mapSession.push_back(CSessionState(p, ls));
		m_mutex.unlock();
	}
	return bStart;
}

unsigned int CRawThread::GetConnectedSessions() {
	unsigned int data = 0;
	CAutoLock al(m_mutex);
	for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
		if (it->first->IsConnected()) {
			++data;
		}
	}
	return data;
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
			PRawSession p = new CRawSession(GetIoService(), *this, m_da);
			m_mutex.lock();
			m_mapSession.push_back(CSessionState(p, ls));
			m_mutex.unlock();
			--session;
		}
	}
	return bStart;
}

bool CRawThread::Kill() {
	CloseAll();
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
		m_sc(this, tagSessionEvent::seThreadDestroyed, nullptr);
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
	CAutoLock al(m_mutex);
	for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
		auto session = it->first;
		if (!session->IsConnected()) {
			continue;
		}
		session->Close();
	}
}

bool CRawThread::IsStarted() {
	return SPA::CUCommThread::IsStarted();
}

USessionHandle CRawThread::FindAClosedSession() {
	CAutoLock al(m_mutex);
	for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
		auto session = it->first;
		if (!session->IsConnected()) {
			return session;
		}
	}
	return nullptr;
}

unsigned int CRawThread::ConnectAll(const char *strHost, unsigned int nPort, SPA::tagEncryptionMethod secure, bool b6) {
	unsigned int count = 0;
	{
		CAutoLock al(m_mutex);
		for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
			auto session = it->first;
			if (!session->IsConnected()) {
				if (session->Connect(strHost, nPort, secure, b6, false, 0)) {
					++count;
				}
			}
		}
	}
	return count;
}
