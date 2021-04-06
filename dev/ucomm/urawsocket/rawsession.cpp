#include "pch.h"
#include "rawsession.h"
#include "rawthread.h"
#include <boost/lexical_cast.hpp>

SPA::CSpinLock CRawSession::m_csBuffer;
std::vector<unsigned char*> CRawSession::m_aBuffer;

unsigned char* CRawSession::GetIoBuffer() {
	unsigned char *s = nullptr;
	{
		CAutoLock sl(m_csBuffer);
		size_t size = m_aBuffer.size();
		if (size) {
			s = m_aBuffer[size - 1];
			m_aBuffer.pop_back();
		}
	}
	if (s == nullptr)
		s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + IO_ENCRYPTION_PADDING); //add extra space for encryption padding
	return s;
}

void CRawSession::ReleaseIoBuffer(unsigned char *buffer) {
	if (buffer == nullptr)
		return;
	m_csBuffer.lock();
	m_aBuffer.push_back(buffer);
	m_csBuffer.unlock();
}

CRawSession::CRawSession(CIoService &IoService, CRawThread &rt, PDataArrive da) : m_qWrite(*m_sbWrite), m_io(IoService), m_rt(rt), m_da(da), m_socket(IoService),
	m_nPort(0), m_b6(false), m_bSync(false), m_ss(tagSessionState::ssClosed), m_secure(SPA::tagEncryptionMethod::NoEncryption), m_ReadBuffer(GetIoBuffer()),
	m_bRBLocked(false), m_WriteBuffer(GetIoBuffer()), m_bWBLocked(0), m_bWaiting(false)
{
#ifdef WIN32_64
	::memset(&m_hCreds, 0, sizeof(m_hCreds));
#else

#endif
	auto sc = rt.GetSessionCallback();
	if (sc) {
		sc(&rt, tagSessionEvent::seSessionCreated, this);
	}
}

CRawSession::~CRawSession() {
	auto sc = m_rt.GetSessionCallback();
	if (sc) {
		//bool chatting = false;
		//CRAutoLock sl(m_mutex, chatting);
		sc(&m_rt, tagSessionEvent::seSessionDestroyed, this);
	}
	ReleaseIoBuffer(m_WriteBuffer);
	ReleaseIoBuffer(m_ReadBuffer);
}

void CRawSession::PostProcessing(unsigned int hint, SPA::UINT64 data) {
	m_io.post(boost::bind(&CRawSession::OnPostProcessing, this, hint, data));
}

bool CRawSession::IsSameThread() {
#ifdef WIN32_64
	auto id = ::GetCurrentThreadId();
#else
	auto id = ::pthread_self();
#endif
	return (id == m_rt.GetThreadId());
}

void CRawSession::OnPostProcessing(unsigned int hint, SPA::UINT64 data) {
	
	switch (hint)
	{
	case HINT_CLOSE:
		Close();
		break;
	case HINT_CONNECT:
	{
		PSessionCallback sc = m_rt.GetSessionCallback();
		tagSessionEvent se = tagSessionEvent::seUnknown;
		CErrorCode ec;
		{
			std::unique_lock<std::mutex> sl(m_mutex);
			do {
				CAutoLock sl(m_cs);
				CResolver::query ipAddr(m_b6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), m_strhost, boost::lexical_cast<std::string> (m_nPort));
				CResolver r(m_io);
				CResolver::iterator iterator = r.resolve(ipAddr, ec);
				if (ec || iterator == CResolver::iterator()) {
					m_ec = ec;
					m_ss = tagSessionState::ssClosed;
					se = tagSessionEvent::seSessionClosed;
					break;
				}
				m_socket.connect(iterator->endpoint(), ec);
				if (ec) {
					m_ec = ec;
					m_ss = tagSessionState::ssClosed;
					se = tagSessionEvent::seSessionClosed;
					break;
				}
				m_qWrite.SetSize(0);
				if (m_secure == SPA::tagEncryptionMethod::TLSv1) {

				}
				se = tagSessionEvent::seConnected;
				m_ss = tagSessionState::ssConnected;
				m_ec.clear();
				Read();
			} while (false);
			if (m_bWaiting) {
				m_cv.notify_all();
			}
		}
		if (sc) {
			sc(&m_rt, se, this);
		}
	}
		break;
	default:
		assert(false);
		break;
	}
}

bool CRawSession::Shutdown(SPA::tagShutdownType st) {
	CAutoLock sl(m_cs);
	if (m_ss < tagSessionState::ssSslShaked) {
		m_ec.assign(boost::system::errc::not_connected, boost::system::generic_category());
		return false;
	}
	m_ec.clear();
	m_socket.shutdown((nsIP::tcp::socket::shutdown_type)st, m_ec);
	return (!m_ec.failed());
}

bool CRawSession::Connect(const char *strHost, unsigned int nPort, SPA::tagEncryptionMethod secure, bool b6, bool bSync, unsigned int timeout) {
	std::unique_lock<std::mutex> sl(m_mutex);
	{
		CAutoLock sl(m_cs);
		if (m_ss > tagSessionState::ssClosed) {
			m_ec.assign(boost::system::errc::already_connected, boost::system::generic_category());
			return false;
		}
		m_strhost = strHost ? strHost : "";
		m_nPort = nPort;
		m_bSync = bSync;
		m_b6 = b6;
		m_secure = secure;
		PostProcessing(HINT_CONNECT, 0);
	}
	if (bSync) {
		if (IsSameThread()) {
			m_ec.assign(boost::system::errc::not_supported, boost::system::generic_category());
			return false;
		}
		if (m_ss >= tagSessionState::ssConnected)
			return true;
		m_bWaiting = true;
		bool b = false;
		do {
			if (m_cv.wait_for(sl, ms(timeout)) == std::cv_status::timeout) {
				m_ec.assign(boost::system::errc::timed_out, boost::system::generic_category());
				break;
			}
			b = (m_ss >= tagSessionState::ssConnected);
		} while (false);
		m_bWaiting = false;
		return b;
	}
	return true;
}

bool CRawSession::IsConnected() {
	return (m_ss >= tagSessionState::ssConnected);
}

int CRawSession::GetErrorCode(char *em, unsigned int len) {
	CAutoLock sl(m_cs);
	int ec = m_ec.value();
	if (!ec) {
		if (em) {
			*em = 0;
		}
	}
	else if (em && len) {
		m_ec.message(em, len);
	}
	return ec;
}

void CRawSession::OnReadCompleted(const CErrorCode& ec, size_t nLen) {
	if (ec || !nLen) {
		m_cs.lock();
		m_ec = ec;
		m_cs.unlock();
		Close();
	}
	else {
		if (m_secure == SPA::tagEncryptionMethod::TLSv1) {

		}
		m_da(this, m_ReadBuffer, (unsigned int)nLen);
		CAutoLock sl(m_cs);
		m_bRBLocked = false;
		Read();
	}
}

void CRawSession::Read() {
	if (m_bRBLocked || m_ss < tagSessionState::ssConnected)
		return;
	m_bRBLocked = true;
	m_socket.async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), boost::bind(&CRawSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

void CRawSession::OnWriteCompleted(const CErrorCode& ec, size_t bytes_transferred) {
	do {
		m_cs.lock();
		if (m_ss < tagSessionState::ssConnected) {
			m_cs.unlock();
			break;
		}
		if (ec) {
			m_ec = ec;
			m_cs.unlock();
			Close();
		}
		else {
			if (m_bWBLocked > bytes_transferred) {
				//m_bWBLocked -= (unsigned int)bytes_transferred;
				unsigned int ulLen = (unsigned int)(m_bWBLocked - bytes_transferred);
				::memmove(m_WriteBuffer, m_WriteBuffer + bytes_transferred, ulLen);
				m_bWBLocked = ulLen;
				m_socket.async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CRawSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
			}
			else {
				assert(m_bWBLocked == bytes_transferred);
				m_bWBLocked = 0;
				if (m_qWrite.GetSize()) {
					SendInternal(nullptr, 0);
				}
			}
			m_cs.unlock();
		}
	} while (false);
}

unsigned int CRawSession::GetSendBufferSize() {
	CAutoLock sl(m_cs);
	return m_qWrite.GetSize();
}

int CRawSession::SendInternal(const unsigned char *s, unsigned int nSize) {
	if (m_ss < tagSessionState::ssConnected) {
		m_ec.assign(boost::system::errc::connection_aborted, boost::system::generic_category());
		return m_ec.value();
	}
	if (m_bWBLocked) {
		m_qWrite.Push(s, nSize);
		unsigned len = m_qWrite.GetSize();
		return (len >= LARGE_SENDING_BUFFER) ? len : 0;
	}
	unsigned int ulLen = m_qWrite.GetSize();
	if (ulLen == 0 && s && nSize) {
		if (nSize <= IO_BUFFER_SIZE) {
			::memcpy(m_WriteBuffer, s, nSize);
			ulLen = nSize;
		}
		else {
			::memcpy(m_WriteBuffer, s, IO_BUFFER_SIZE);
			ulLen = IO_BUFFER_SIZE;

			//remaining
			m_qWrite.Push(s + IO_BUFFER_SIZE, nSize - IO_BUFFER_SIZE);
		}
	}
	else {
		m_qWrite.Push(s, nSize);
		ulLen = m_qWrite.GetSize();
		if (ulLen == 0)
			return 0;
		if (ulLen > IO_BUFFER_SIZE)
			ulLen = IO_BUFFER_SIZE;
		m_qWrite.Pop(m_WriteBuffer, ulLen);
	}
	if (m_secure == SPA::tagEncryptionMethod::TLSv1) {

	}
	m_bWBLocked = ulLen;
	m_socket.async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CRawSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
	return (int) ulLen;
}

int CRawSession::Send(const unsigned char *data, unsigned int bytes) {
	CAutoLock sl(m_cs);
	return SendInternal(data, bytes);
}

void CRawSession::Close() {
	PSessionCallback sc = nullptr;
#ifdef WIN32_64
	auto id = ::GetCurrentThreadId();
#else
	auto id = ::pthread_self();
#endif
	m_cs.lock();
	do {
		if (m_ss == tagSessionState::ssClosed) {
			break;
		}
		if (id == m_rt.GetThreadId()) {
			CErrorCode ec;
			m_socket.shutdown(nsIP::tcp::socket::shutdown_type::shutdown_both, ec);
			m_socket.close(ec);
			m_ss = tagSessionState::ssClosed;
			sc = m_rt.GetSessionCallback();
		}
		else {
			PostProcessing(HINT_CLOSE, 0);
		}
	} while (false);
	m_cs.unlock();
	if (sc) {
		sc(&m_rt, tagSessionEvent::seSessionClosed, this);
	}
}

#ifdef WIN32_64
void CRawSession::FreeCredHandle() {
	if (m_hCreds.dwLower || m_hCreds.dwUpper) {
		::FreeCredentialsHandle(&m_hCreds);
		::memset(&m_hCreds, 0, sizeof(m_hCreds));
	}
}
#else

#endif