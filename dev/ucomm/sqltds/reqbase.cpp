#include "reqbase.h"
#include "tdschannel.h"
#include <chrono>
using namespace std::chrono_literals;

namespace tds
{

    CReqBase::CReqBase(CTdsChannel& channel) : m_channel(channel), m_buffer(*m_sb),
		m_tt(tagTokenType::ttZero), ResponseHeader(tagPacketType::ptInitial, 0), m_bWaiting(false) {
		if (m_buffer.GetMaxSize() >= SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
			m_buffer.ReallocBuffer(SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
		}
    }

    CReqBase::~CReqBase() {
    }

    const PacketHeader & CReqBase::GetResponseHeader() const {
        return ResponseHeader;
    }

    bool CReqBase::IsDone() {
		CAutoLock al(m_cs);
		return IsDoneInternal();
    }

	bool CReqBase::HasMore() {
		CAutoLock al(m_cs);
		return HasMoreInternal();
	}

	UINT64 CReqBase::GetCount() {
		CAutoLock al(m_cs);
		return GetCountInternal();
	}

	bool CReqBase::HasMoreInternal() const {
		return ((m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore);
	}

	UINT64 CReqBase::GetCountInternal() const {
		if ((m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
			return m_Done.RowCount;
		}
		return INVALID_NUMBER;
	}

	bool CReqBase::IsDoneInternal() const {
		return (ResponseHeader.Status == tagPacketStatus::psEOM);
	}

    void CReqBase::Reset() {
		CAutoLock al(m_cs);
        ::memset(&ResponseHeader, 0, sizeof (ResponseHeader));
		memset(&m_Done, 0, sizeof(m_Done));
    }

	bool CReqBase::ParseDone() {
		if (m_buffer.GetSize() >= sizeof(m_Done)) {
			{
				CAutoLock al(m_cs);
				m_buffer >> m_Done;
			}
			m_tt = tagTokenType::ttZero;
			return true;
		}
		return false;
	}

	int CReqBase::GetSQLError(SPA::CDBString& errMsg) {
		CAutoLock al(m_cs);
		bool bad = (m_errCode.Class > 10);
		unsigned int ec = m_errCode.Class;
		if (bad) {
			ec |= 0x80;
		}
		ec <<= 8;
		ec += (m_errCode.State << 2);
		ec <<= 16;
		ec += m_errCode.SQLErrorNumber;
		errMsg = m_errCode.ErrorMessage;
		return (int)ec;
	}

	bool CReqBase::ParseError() {
		if (m_buffer.GetSize() > 2) {
			unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
			if (len + sizeof(len) <= m_buffer.GetSize()) {
				{
					CAutoLock al(m_cs);
					TokenInfo& ti = m_errCode;
					m_buffer >> len >> ti.SQLErrorNumber >> ti.State >> ti.Class;
					m_buffer >> len;
					const char16_t* str = (const char16_t*)m_buffer.GetBuffer();
					ti.ErrorMessage.assign(str, str + len);
					m_buffer.Pop(((unsigned int)len) << 1);
					unsigned char byteLen;
					m_buffer >> byteLen;
					str = (const char16_t*)m_buffer.GetBuffer();
					ti.ServerName.assign(str, str + byteLen);
					m_buffer.Pop(((unsigned int)byteLen) << 1);
					m_buffer >> ti.ProcessNameLength >> ti.LineNumber;
				}
				m_tt = tagTokenType::ttZero;
				return true;
			}
		}
		return false;
	}

	bool CReqBase::Wait(unsigned int milliseconds) {
		CAutoLock al(m_cs);
		if (IsDoneInternal() && !HasMoreInternal()) {
			return true;
		}
		m_bWaiting = true;
		bool ok = (m_cv.wait_for(al, milliseconds * 1ms) == std::cv_status::no_timeout);
		m_bWaiting = false;
		return ok;
	}

	int CReqBase::Send(const unsigned char* buffer, unsigned int bytes, unsigned int milliseconds, bool sync) {
		{
			CAutoLock al(m_csSend);
			{
				CAutoLock al(m_cs);
				m_errCode.Reset();
				ResponseHeader.Status = tagPacketStatus::psNormal;
				m_Done.Status = tagDoneStatus::dsMore;
			}
			int fail = m_channel.Send(this, buffer, bytes);
			if (fail || !sync) {
				return fail;
			}
		}
		CAutoLock al(m_cs);
		m_bWaiting = true;
		bool ok = (m_cv.wait_for(al, milliseconds * 1ms) == std::cv_status::no_timeout);
		m_bWaiting = false;
		return ok ? 0 : -1;
	}

	void CReqBase::OnResponse(const unsigned char *data, unsigned int bytes) {
		assert(bytes >= sizeof(ResponseHeader));
		{
			CAutoLock al(m_cs);
			memcpy(&ResponseHeader, data, sizeof(ResponseHeader));
			ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
			assert(ResponseHeader.Length == bytes);
			ResponseHeader.Spid = ChangeEndian(ResponseHeader.Spid);
		}
		data += sizeof(ResponseHeader);
		bytes -= sizeof(ResponseHeader);
		m_buffer.Push(data, bytes);
		try {
			ParseStream();
		}
		catch (SPA::CUException &ex) {
#ifndef NDEBUG
			std::cout << "serialization error: " << ex.what() << "\n";
#endif
		}
		catch (std::exception &ex) {
#ifndef NDEBUG
			std::cout << "stl error: " << ex.what() << "\n";
#endif
		}
		catch (...) {
#ifndef NDEBUG
			std::cout << "Unknown error\n";
#endif
		}
		CAutoLock al(m_cs);
		if (m_bWaiting && IsDoneInternal() && !HasMoreInternal()) {
			m_cv.notify_all();
		}
	}
}