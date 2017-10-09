
#include "stdafx.h"
#include "webasynchandler.h"

CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket)
: CCachedBaseHandler<sidStreamSystem>(pClientSocket) {
}

bool CWebAsyncHandler::QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma) {
    return false;
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