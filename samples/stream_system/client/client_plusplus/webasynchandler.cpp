
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
: CCachedBaseHandler<sidStreamSystem>(pClientSocket) {
}

bool CWebAsyncHandler::QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma) {
    return SendRequest(idQueryMaxMinAvgs, sql, [mma](CAsyncResult & ar) {
        int res;
        std::wstring errMsg;
                CMaxMinAvg m_m_a;
                ar >> res >> errMsg >> m_m_a;
        if (mma) {
            mma(m_m_a, res, errMsg);
        }
    });
}

bool CWebAsyncHandler::GetMasterSlaveConnectedSessions(DConnectedSessions cs) {
    return SendRequest(idGetMasterSlaveConnectedSessions, [cs](CAsyncResult & ar) {
        unsigned int master_connections, slave_conenctions;
        ar >> master_connections >> slave_conenctions;
        if (cs) {
            cs(master_connections, slave_conenctions);
        }
    });
}

void CWebAsyncHandler::OnResultReturned(unsigned short reqId, SPA::CUQueue &mc) {
    switch (reqId) {
        case idQueryMaxMinAvgs:
            break;
        default:
            CCachedBaseHandler<sidStreamSystem>::OnResultReturned(reqId, mc);
            break;
    }
}
