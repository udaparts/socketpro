

#include "sqliteimpl.h"

std::shared_ptr<SPA::ServerSide::CSqliteService> g_pSqlite;

void WINAPI SetSqliteDBGlobalConnectionString(const wchar_t *dbConnection) {
    SPA::ServerSide::CSqliteImpl::SetDBGlobalConnectionString(dbConnection);
}

bool WINAPI InitServerLibrary(int param) {
    SPA::ServerSide::CSqliteImpl::SetInitialParam((unsigned int) param);
    g_pSqlite.reset(new SPA::ServerSide::CSqliteService(SPA::Sqlite::sidSqlite, SPA::taNone));
    if ((param & SPA::ServerSide::Sqlite::USE_SHARED_CACHE_MODE) == SPA::ServerSide::Sqlite::USE_SHARED_CACHE_MODE) {
        sqlite3_enable_shared_cache(1);
    }
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pSqlite.reset();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return SPA::Sqlite::sidSqlite;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (g_pSqlite && serviceId == SPA::Sqlite::sidSqlite)
        sc = g_pSqlite->GetSvsContext();
    else
        memset(&sc, 0, sizeof (sc));
    return sc;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId) {
    return 8; //The service only has seven slow requests
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
            return SPA::UDB::idExecuteBatch;
        default:
            break;
    }
    return 0;
}
