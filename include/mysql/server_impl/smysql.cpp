#include "mysqlimpl.h"

std::shared_ptr<SPA::ServerSide::CMysqlService> g_pMysql;

bool WINAPI InitServerLibrary(int param) {
    SPA::ServerSide::CMysqlImpl::InitMySql();
    g_pMysql.reset(new SPA::ServerSide::CMysqlService(SPA::Mysql::sidMysql, SPA::tagThreadApartment::taNone));
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pMysql.reset();
    SPA::ServerSide::CMysqlImpl::UnloadMysql();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return SPA::Mysql::sidMysql;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (g_pMysql && serviceId == SPA::Mysql::sidMysql)
        sc = g_pMysql->GetSvsContext();
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
