#include "reqbase.h"

namespace tds {
	CReqBase::CReqBase() : ResponseHeader(tagPacketType::ptInitial, 0) {
	}

	CReqBase::~CReqBase() {
	}

	const PacketHeader& CReqBase::GetResponseHeader() const {
		return ResponseHeader;
	}
}