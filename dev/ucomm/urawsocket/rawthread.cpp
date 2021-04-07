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

namespace SPA {

	CRawThread::CRawThread(PDataArrive da, PSessionCallback sc, unsigned int sessions, tagThreadApartment ta) : CUCommThread(ta), m_da(da), m_sc(sc), m_id(0) {
		if (m_sc) {
			m_sc(this, tagSessionPoolEvent::seStarted, nullptr);
		}
		Start(sessions);
	}

	CRawThread::~CRawThread() {
		Kill();
		if (m_sc) {
			m_sc(this, tagSessionPoolEvent::seShutdown, nullptr);
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
			m_sc(this, tagSessionPoolEvent::seThreadCreated, nullptr);
		}
	}

	void CRawThread::OnThreadEnded() {
		if (m_sc) {
			m_sc(this, tagSessionPoolEvent::seKillingThread, nullptr);
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

	bool CRawThread::Start(unsigned int sessions) {
		if (!IsStarted()) {
			if (m_sc) {
				m_sc(this, tagSessionPoolEvent::seCreatingThread, nullptr);
			}
		}
		bool bStart = CUCommThread::Start();
		if (bStart) {
			while (sessions) {
				LockState ls(this);
				PRawSession p = new CRawSession(GetIoService(), *this, m_da);
				m_mutex.lock();
				m_mapSession.push_back(CSessionState(p, ls));
				m_mutex.unlock();
				--sessions;
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
			m_sc(this, tagSessionPoolEvent::seThreadDestroyed, nullptr);
		}
		return ok;
	}

	SessionHandle CRawThread::Lock(unsigned int timeout) {
		PRawSession s = nullptr;
		PSessionCallback sc = nullptr;
		unsigned int actives;
		{
			CAutoLock al(m_mutex);
			do {
				actives = 0;
				for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
					auto session = it->first;
					if (session->IsConnected()) {
						++actives;
						if (!it->second.Locked) {
							it->second.Locked = true;
							sc = m_sc;
							s = session;
							break;
						}
					}
					else {
						it->second.Locked = false;
					}
				}
			} while (actives && !s && m_cv.wait_for(al, ms(timeout)) != std::cv_status::timeout);
		}
		if (sc) {
			sc(this, tagSessionPoolEvent::seLocked, s);
		}
		return s;
	}

	bool CRawThread::Unlock(SessionHandle session) {
		if (!session) {
			return false;
		}
		SessionHandle s = nullptr;
		PSessionCallback sc = nullptr;
		{
			CAutoLock al(m_mutex);
			for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
				if (session == it->first) {
					it->second.Locked = false;
					sc = m_sc;
					s = session;
					m_cv.notify_all();
					break;
				}
			}
		}
		if (sc) {
			sc(this, tagSessionPoolEvent::seUnlocked, session);
		}
		return (s != nullptr);
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
		return CUCommThread::IsStarted();
	}

	SessionHandle CRawThread::FindAClosedSession() {
		CAutoLock al(m_mutex);
		for (CMapSession::iterator it = m_mapSession.begin(), end = m_mapSession.end(); it != end; ++it) {
			auto session = it->first;
			if (!session->IsConnected()) {
				return session;
			}
		}
		return nullptr;
	}

	unsigned int CRawThread::ConnectAll(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6) {
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

}; //namespace SPA;