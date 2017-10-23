
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
	: CCachedBaseHandler<sidStreamSystem>(pClientSocket), m_ssIndex(0) {
}

SPA::UINT64 CWebAsyncHandler::QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DMyCanceled canceled) {
	ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
		int res;
		std::wstring errMsg;
		CMaxMinAvg m_m_a;
		ar >> index >> res >> errMsg >> m_m_a;
		std::pair<DMaxMinAvg, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapMMA[index];
			this->m_mapMMA.erase(index);
		}
		if (p.first)
			p.first(index, m_m_a, res, errMsg);
	};
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapMMA[index] = std::pair<DMaxMinAvg, DMyCanceled>(mma, canceled);
	}
	if (!SendRequest(idQueryMaxMinAvgs, index, filter, arh, [index, this]() {
		std::pair<DMaxMinAvg, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapMMA[index];
			this->m_mapMMA.erase(index);
		}
		if (p.second)
			p.second(index);
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapMMA.erase(index);
		return 0;
	}
	return index;
}

SPA::UINT64 CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyCanceled canceled) {
	ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
		unsigned int master_connections, slave_conenctions;
		ar >> index >> master_connections >> slave_conenctions;
		std::pair<DConnectedSessions, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapSession[index];
			this->m_mapSession.erase(index);
		}
		if (p.first)
			p.first(index, master_connections, slave_conenctions);
	};
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapSession[index] = std::pair<DConnectedSessions, DMyCanceled>(cs, canceled);
	}
	if (!SendRequest(idGetMasterSlaveConnectedSessions, index, arh, [index, this]() {
		std::pair<DConnectedSessions, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapSession[index];
			this->m_mapSession.erase(index);
		}
		if (p.second)
			p.second(index);
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapSession.erase(index);
		return 0;
	}
	return index;
}

SPA::UINT64 CWebAsyncHandler::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DMyCanceled canceled) {
	ResultHandler arh = [this](CAsyncResult & ar) {
		SPA::UINT64 index;
		int errCode;
		std::wstring errMsg;
		CInt64Array vId;
		ar >> index >> errCode >> errMsg >> vId;
		std::pair<DUploadEmployees, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapUpload[index];
			this->m_mapUpload.erase(index);
		}
		if (p.first)
			p.first(index, errCode, errMsg, vId);
	};
	SPA::UINT64 index;
	{
		SPA::CAutoLock al(m_csSS);
		index = ++m_ssIndex;
		m_mapUpload[index] = std::pair<DUploadEmployees, DMyCanceled>(res, canceled);
	}
	if (!SendRequest(idUploadEmployees, index, vData, arh, [index, this]() {
		std::pair<DUploadEmployees, DMyCanceled> p;
		{
			SPA::CAutoLock al(this->m_csSS);
			p = this->m_mapUpload[index];
			this->m_mapUpload.erase(index);
		}
		if (p.second)
			p.second(index);
	})) {
		SPA::CAutoLock al(m_csSS);
		m_mapUpload.erase(index);
		return 0;
	}
	return index;
}
