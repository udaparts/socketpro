
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
    return 18; //The service only has seventeen slow requests
}

unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index) {
    //The following seven requests are slow ones
    switch (index) {
        case 0:
            return SPA::UDB::idOpen;
        case 1:
            return SPA::UDB::idBeginTrans;
        case 2:
            return SPA::UDB::idEndTrans;
        case 3:
            return SPA::UDB::idExecute;
        case 4:
            return SPA::UDB::idClose;
        case 5:
            return SPA::UDB::idPrepare;
        case 6:
            return SPA::UDB::idExecuteParameters;
        case 7:
            return SPA::Odbc::idSQLColumnPrivileges;
        case 8:
            return SPA::Odbc::idSQLColumns;
        case 9:
            return SPA::Odbc::idSQLForeignKeys;
        case 10:
            return SPA::Odbc::idSQLPrimaryKeys;
            break;
        case 11:
            return SPA::Odbc::idSQLProcedureColumns;
        case 12:
            return SPA::Odbc::idSQLProcedures;
        case 13:
            return SPA::Odbc::idSQLSpecialColumns;
        case 14:
            return SPA::Odbc::idSQLStatistics;
        case 15:
            return SPA::Odbc::idSQLTablePrivileges;
        case 16:
            return SPA::Odbc::idSQLTables;
		case 17:
            return SPA::UDB::idExecuteBatch;
        default:
            break;
    }
    return 0;
}