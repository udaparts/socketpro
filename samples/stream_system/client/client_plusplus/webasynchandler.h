
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler < sidStreamSystem > {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
	typedef CCachedBaseHandler::DDiscarded DMyDiscarded;
    typedef std::function<void(const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    typedef std::function<void(unsigned int m_connections, unsigned int s_connections) > DConnectedSessions;
    typedef std::function<void(int res, const std::wstring &errMsg, CInt64Array &vId) > DUploadEmployees;
    typedef std::function<void(const CRentalDateTimes &dates, int res, const std::wstring &errMsg) > DRentalDateTimes;

public:
	bool QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DMyDiscarded discarded = nullptr);
	bool GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyDiscarded discarded = nullptr);
	bool UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DMyDiscarded discarded = nullptr);
	bool GetRentalDateTimes(SPA::INT64 rentalId, DRentalDateTimes rdt, DMyDiscarded discarded = nullptr);

private:
    CWebAsyncHandler(const CWebAsyncHandler &wah);
    CWebAsyncHandler& operator=(const CWebAsyncHandler &wah);
};
