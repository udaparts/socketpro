
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler<sidStreamSystem> {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef std::function<void(const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    typedef std::function<void(unsigned int, unsigned int) > DConnectedSessions;
	typedef std::function<void(int res, const std::wstring &errMsg, std::vector<SPA::INT64> &vId) > DUploadEmployees;

public:
    bool QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma);
    bool GetMasterSlaveConnectedSessions(DConnectedSessions cs);
	bool UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res);
};
