#include "reqbase.h"
#include <chrono>
using namespace std::chrono_literals;

namespace tds
{

    CReqBase::CReqBase() : m_buffer(*m_sb), m_tt(tagTokenType::ttZero), ResponseHeader(tagPacketType::ptInitial, 0) {
		if (m_buffer.GetMaxSize() >= SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
			m_buffer.ReallocBuffer(SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
		}
    }

    CReqBase::~CReqBase() {
    }

    const PacketHeader & CReqBase::GetResponseHeader() const {
        return ResponseHeader;
    }

    bool CReqBase::IsDone() const {
        return (ResponseHeader.Status == tagPacketStatus::psEOM);
    }

	bool CReqBase::HasMore() const {
		return ((m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore);
	}

	UINT64 CReqBase::GetCount() const {
		if ((m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
			return m_Done.RowCount;
		}
		return INVALID_NUMBER;
	}

    void CReqBase::Reset() {
        ::memset(&ResponseHeader, 0, sizeof (ResponseHeader));
		m_vInfo.clear();
		memset(&m_Done, 0, sizeof(m_Done));
    }

	bool CReqBase::ParseDone() {
		if (m_buffer.GetSize() >= sizeof(m_Done)) {
			m_buffer >> m_Done;
			m_tt = tagTokenType::ttZero;
			return true;
		}
		return false;
	}

	bool CReqBase::ParseCollation(CollationChange &cc) {
		if (m_buffer.GetSize() >= sizeof(cc) + sizeof(unsigned char) + sizeof(unsigned char)) {
			unsigned char b;
			m_buffer >> b;
			assert(b == 5);
			m_buffer >> cc.NewValue;
			m_buffer >> b;
			if (b) {
				m_buffer >> cc.OldValue;
			}
			m_tt = tagTokenType::ttZero;
			return true;
		}
		return false;
	}

	void CReqBase::ParseStringChange(tagEnvchangeType type, StringEventChange& sec) {
		unsigned char b;
		m_buffer >> b;
		sec.Type = type;
		const char16_t *str = (const char16_t *)m_buffer.GetBuffer();
		sec.NewValue.assign(str, str + b);
		m_buffer.Pop(((unsigned int)b) << 1);
		m_buffer >> b;
		if (b) {
			str = (const char16_t *)m_buffer.GetBuffer();
			sec.OldValue.assign(str, str + b);
			m_buffer.Pop(((unsigned int)b) << 1);
		}
	}

	bool CReqBase::ParseErrorInfo() {
		if (m_buffer.GetSize() > 2) {
			unsigned short len = *(unsigned short *)m_buffer.GetBuffer();
			if (len + sizeof(len) <= m_buffer.GetSize()) {
				TokenInfo ti;
				m_buffer >> len >> ti.SQLErrorNumber >> ti.State >> ti.Class;
				m_buffer >> len;
				const char16_t *str = (const char16_t *)m_buffer.GetBuffer();
				ti.ErrorMessage.assign(str, str + len);
				m_buffer.Pop(((unsigned int)len) << 1);
				unsigned char byteLen;
				m_buffer >> byteLen;
				str = (const char16_t *)m_buffer.GetBuffer();
				ti.ServerName.assign(str, str + byteLen);
				m_buffer.Pop(((unsigned int)byteLen) << 1);
				m_buffer >> ti.ProcessNameLength >> ti.LineNumber;
				m_vInfo.push_back(ti);
				m_tt = tagTokenType::ttZero;
				return true;
			}
		}
		return false;
	}

	bool CReqBase::Wait(unsigned int milliseconds) {
		CAutoLock al(m_cs);
		if (IsDone() && !HasMore()) {
			return true;
		}
		return (m_cv.wait_for(al, milliseconds * 1ms) == std::cv_status::no_timeout);
	}

	void CReqBase::OnResponse(const unsigned char *data, unsigned int bytes) {
		assert(bytes >= sizeof(ResponseHeader));
		CAutoLock al(m_cs);
		memcpy(&ResponseHeader, data, sizeof(ResponseHeader));
		ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
		assert(ResponseHeader.Length == bytes);
		ResponseHeader.Spid = ChangeEndian(ResponseHeader.Spid);
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
		if (IsDone() && !HasMore()) {
			m_cv.notify_all();
		}
	}
}