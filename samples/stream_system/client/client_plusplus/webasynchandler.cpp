
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
	: CCachedBaseHandler<sidStreamSystem>(pClientSocket), m_ssIndex(0) {
}

SPA::UINT64 CWebAsyncHandler::QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DCanceled canceled) {
    ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
		int res;
        std::wstring errMsg;
        CMaxMinAvg m_m_a;
		ar >> index >> res >> errMsg >> m_m_a;
		std::pair<DMaxMinAvg, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapMMA[index];
			this->m_mapMMA.erase(index);
		}
		if (p.first)
			p.first(m_m_a, res, errMsg);
	};
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapMMA[index] = std::pair<DMaxMinAvg, DCanceled>(mma, canceled);
	}
	if (!SendRequest(idQueryMaxMinAvgs, index, filter, arh, [index, this]() {
		std::pair<DMaxMinAvg, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapMMA[index];
			this->m_mapMMA.erase(index);
		}
		if (p.second)
			p.second();
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapMMA.erase(index);
		return 0;
	}
	return index;
}

SPA::UINT64 CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs, DCanceled canceled) {
    ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
        unsigned int master_connections, slave_conenctions;
		ar >> index >> master_connections >> slave_conenctions;
		std::pair<DConnectedSessions, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapSession[index];
			this->m_mapSession.erase(index);
		}
		if (p.first)
			p.first(master_connections, slave_conenctions);
    };
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapSession[index] = std::pair<DConnectedSessions, DCanceled>(cs, canceled);
	}
	if (!SendRequest(idGetMasterSlaveConnectedSessions, index, arh, [index, this](){
		std::pair<DConnectedSessions, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapSession[index];
			this->m_mapSession.erase(index);
		}
		if (p.second)
			p.second();
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapSession.erase(index);
		return 0;
	}
	return index;
}

SPA::UINT64 CWebAsyncHandler::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DCanceled canceled) {
    ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
		int errCode;
        std::wstring errMsg;
        CInt64Array vId;
		ar >> index >> errCode >> errMsg >> vId;
		std::pair<DUploadEmployees, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapUpload[index];
			this->m_mapUpload.erase(index);
		}
		if (p.first)
			p.first(errCode, errMsg, vId);
    };
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapUpload[index] = std::pair<DUploadEmployees, DCanceled>(res, canceled);
	}
	if (!SendRequest(idUploadEmployees, index, vData, arh, [index, this](){
		std::pair<DUploadEmployees, DCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapUpload[index];
			this->m_mapUpload.erase(index);
		}
		if (p.second)
			p.second();
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapUpload.erase(index);
		return 0;
	}
	return index;
}
