#include "reqbase.h"

namespace tds {
	CReqBase::CReqBase() : ResponseHeader(tagPacketType::ptInitial, 0) {
		ResponseHeader.Spid = 0;
		ResponseHeader.Length = 0;
	}

	CReqBase::~CReqBase() {
	}

	const PacketHeader& CReqBase::GetResponseHeader() const {
		return ResponseHeader;
	}

	bool CReqBase::IsDone() const{
		return (ResponseHeader.Type != tagPacketType::ptInitial);
	}
}