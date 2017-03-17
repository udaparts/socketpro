
#include "odbcimpl.h"

std::shared_ptr<SPA::ServerSide::COdbcService> g_pOdbc;

void WINAPI SetOdbcDBGlobalConnectionString(const wchar_t *dbConnection) {
    SPA::ServerSide::COdbcImpl::SetGlobalConnectionString(dbConnection);
}

bool WINAPI InitServerLibrary(int param) {
    if (!SPA::ServerSide::COdbcImpl::SetODBCEnv(param))
        return false;
    g_pOdbc.reset(new SPA::ServerSide::COdbcService(SPA::Odbc::sidOdbc, SPA::taNone));
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pOdbc.reset();
    SPA::ServerSide::COdbcImpl::FreeODBCEnv();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return SPA::Odbc::sidOdbc;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (g_pOdbc && serviceId == SPA::Odbc::sidOdbc)
        sc = g_pOdbc->GetSvsContext();
    else
        memset(&sc, 0, sizeof (sc));
    return sc;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId) {
    return 7; //The service only has seven slow requests
}

unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index) {
    //The following seven requests are slow ones
    switch (index) {
        case 0:
            return SPA::UDB::idOpen;
            break;
        case 1:
            return SPA::UDB::idBeginTrans;
            break;
        case 2:
            return SPA::UDB::idEndTrans;
            break;
        case 3:
            return SPA::UDB::idExecute;
            break;
        case 4:
            return SPA::UDB::idClose;
            break;
        case 5:
            return SPA::UDB::idPrepare;
            break;
        case 6:
            return SPA::UDB::idExecuteParameters;
            break;
        default:
            break;
    }
    return 0;
}