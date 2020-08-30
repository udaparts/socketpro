
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
: CCachedBaseHandler<sidStreamSystem>(pClientSocket) {
}

bool CWebAsyncHandler::QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DMyDiscarded discarded) {
    DResultHandler arh = [mma](CAsyncResult & ar) {
        int res;
        std::wstring errMsg;
        CMaxMinAvg m_m_a;
        ar >> res >> errMsg >> m_m_a;
        if (mma)
            mma(m_m_a, res, errMsg);
    };
    return SendRequest(idQueryMaxMinAvgs, arh, discarded, filter);
}

bool CWebAsyncHandler::GetRentalDateTimes(SPA::INT64 rentalId, DRentalDateTimes rdt, DMyDiscarded discarded) {
    DResultHandler arh = [rdt](CAsyncResult & ar) {
        int res;
        std::wstring errMsg;
        CRentalDateTimes rDateTime;
        ar >> rDateTime >> res >> errMsg;
        if (rdt)
            rdt(rDateTime, res, errMsg);
    };
    return SendRequest(idGetRentalDateTimes, arh, discarded, rentalId);
}

bool CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyDiscarded discarded) {
    DResultHandler arh = [cs](CAsyncResult & ar) {
        unsigned int master_connections, slave_conenctions;
        ar >> master_connections >> slave_conenctions;
        if (cs)
            cs(master_connections, slave_conenctions);
    };
    return SendRequest(idGetMasterSlaveConnectedSessions, arh, discarded);
}

bool CWebAsyncHandler::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DMyDiscarded discarded) {
    DResultHandler arh = [res](CAsyncResult & ar) {
        int errCode;
        std::wstring errMsg;
        CInt64Array vId;
        ar >> errCode >> errMsg >> vId;
        if (res)
            res(errCode, errMsg, vId);
    };
    return SendRequest(idUploadEmployees, arh, discarded, vData);
}
