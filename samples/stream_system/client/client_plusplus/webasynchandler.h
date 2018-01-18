
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler < sidStreamSystem > {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef CAsyncServiceHandler::DDiscarded DMyDiscarded;

public:
    typedef std::function<void(const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    bool QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DMyDiscarded discarded = nullptr);

    typedef std::function<void(unsigned int m_connections, unsigned int s_connections) > DConnectedSessions;
    bool GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyDiscarded discarded = DMyDiscarded());

    typedef std::function<void(int res, const std::wstring &errMsg, CInt64Array &vId) > DUploadEmployees;
    bool UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DMyDiscarded discarded = nullptr);

    typedef std::function<void(const CRentalDateTimes &dates, int res, const std::wstring &errMsg) > DRentalDateTimes;
    bool GetRentalDateTimes(SPA::INT64 rentalId, DRentalDateTimes rdt, DMyDiscarded discarded = nullptr);

private:
    CWebAsyncHandler(const CWebAsyncHandler &wah);
    CWebAsyncHandler& operator=(const CWebAsyncHandler &wah);
};
