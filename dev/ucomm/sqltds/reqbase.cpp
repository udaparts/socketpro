#include "reqbase.h"
#include <chrono>
using namespace std::chrono_literals;

namespace tds
{

    CReqBase::CReqBase(SPA::CBaseHandler& channel) : m_channel(channel), m_buffer(*m_sb),
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

	bool CReqBase::Wait(unsigned int milliseconds) {
		CAutoLock al(m_cs);
		if (IsDone() && !HasMore()) {
			return true;
		}
		m_bWaiting = true;
		bool ok = (m_cv.wait_for(al, milliseconds * 1ms) == std::cv_status::no_timeout);
		m_bWaiting = false;
		return ok;
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
		if (m_bWaiting && IsDone() && !HasMore()) {
			m_cv.notify_all();
		}
	}
}