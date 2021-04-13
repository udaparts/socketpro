#include "reqbase.h"

namespace tds {

	CReqBase::CReqBase() : ResponseHeader(tagPacketType::ptInitial, 0) {
	}

	const PacketHeader& CReqBase::GetResponseHeader() {
		return ResponseHeader;
	}
}