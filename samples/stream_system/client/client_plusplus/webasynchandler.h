
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler < sidStreamSystem > {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef std::function<void(const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    typedef std::function<void(unsigned int, unsigned int) > DConnectedSessions;
    typedef std::function<void(int res, const std::wstring &errMsg, CInt64Array &vId) > DUploadEmployees;

public:
    SPA::UINT64 QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DCanceled canceled = nullptr);
	SPA::UINT64 GetMasterSlaveConnectedSessions(DConnectedSessions cs, DCanceled canceled = nullptr);
	SPA::UINT64 UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DCanceled canceled = nullptr);

private:
	CWebAsyncHandler(const CWebAsyncHandler &wah);
	CWebAsyncHandler& operator=(const CWebAsyncHandler &wah);

private:
	SPA::CUCriticalSection m_csSS;
	SPA::UINT64 m_ssIndex;
	std::unordered_map<UINT64, std::pair<DMaxMinAvg, DCanceled> > m_mapMMA;
	std::unordered_map<UINT64, std::pair<DConnectedSessions, DCanceled> > m_mapSession;
	std::unordered_map<UINT64, std::pair<DUploadEmployees, DCanceled> > m_mapUpload;
};
