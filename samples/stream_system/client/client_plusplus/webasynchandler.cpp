
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
: CCachedBaseHandler<sidStreamSystem>(pClientSocket) {
}

bool CWebAsyncHandler::QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DCanceled canceled) {
    ResultHandler arh = [mma](CAsyncResult & ar) {
        int res;
        std::wstring errMsg;
        CMaxMinAvg m_m_a;
        ar >> res >> errMsg >> m_m_a;
        if (mma)
            mma(m_m_a, res, errMsg);
    };
    return SendRequest(idQueryMaxMinAvgs, filter, arh, canceled);
}

bool CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs, DCanceled canceled) {
    ResultHandler arh = [cs](CAsyncResult & ar) {
        unsigned int master_connections, slave_conenctions;
        ar >> master_connections >> slave_conenctions;
        if (cs)
            cs(master_connections, slave_conenctions);
    };
    return SendRequest(idGetMasterSlaveConnectedSessions, arh, canceled);
}

bool CWebAsyncHandler::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DCanceled canceled) {
    ResultHandler arh = [res](CAsyncResult & ar) {
        int errCode;
        std::wstring errMsg;
        CInt64Array vId;
        ar >> errCode >> errMsg >> vId;
        if (res)
            res(errCode, errMsg, vId);
    };
    return SendRequest(idUploadEmployees, vData, arh, canceled);
}
