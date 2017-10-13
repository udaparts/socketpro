
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
	: CCachedBaseHandler<sidStreamSystem>(pClientSocket) {
}

bool CWebAsyncHandler::QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma) {
	return SendRequest(idQueryMaxMinAvgs, filter, [mma](CAsyncResult & ar) {
		int res;
		std::wstring errMsg;
		CMaxMinAvg m_m_a;
		ar >> res >> errMsg >> m_m_a;
		if (mma)
			mma(m_m_a, res, errMsg);
	});
}

bool CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs) {
	return SendRequest(idGetMasterSlaveConnectedSessions, [cs](CAsyncResult & ar) {
		unsigned int master_connections, slave_conenctions;
		ar >> master_connections >> slave_conenctions;
		if (cs)
			cs(master_connections, slave_conenctions);
	});
}

bool CWebAsyncHandler::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res) {
	return SendRequest(idUploadEmployees, vData, [res](CAsyncResult & ar) {
		int errCode;
		std::wstring errMsg;
		CInt64Array vId;
		ar >> errCode >> errMsg >> vId;
		if (res)
			res(errCode, errMsg, vId);
	});
}
