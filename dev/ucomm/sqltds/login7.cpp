#include "login7.h"

namespace tds {

	bool CLogin7::GetClientMessage(SPA::CUQueue &buffer) {
		return false;
	}

	void CLogin7::OnResponse(const unsigned char *data, unsigned int bytes) {
		SPA::CScopeUQueue sb;
		sb->Push(data, bytes);
		SPA::CUQueue &buff = *sb;
		buff >> ResponseHeader;
		ResponseHeader.Length = tds::ChangeEndian(ResponseHeader.Length);
	}

}