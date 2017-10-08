
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
	: CCachedBaseHandler<sidStreamSystem, CWebAsyncHandler>(pClientSocket) {
}

bool CWebAsyncHandler::QueryMaxMinAvgs(const wchar_t *table, DMaxMinAvg mma) {
    return false;
}

void CWebAsyncHandler::OnResultReturned(unsigned short reqId, SPA::CUQueue &mc) {
	switch (reqId) {
	case idQueryMaxMinAvgs:
		break;
	default:
		CCachedBaseHandler<sidStreamSystem, CWebAsyncHandler>::OnResultReturned(reqId, mc);
		break;
	}
}