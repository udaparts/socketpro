
#ifndef _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#include "uodbc.h"
#include "../spa_module.h"


#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetOdbcDBGlobalConnectionString(const wchar_t *dbConnection);
    bool WINAPI DoODBCAuthentication(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *odbcDriver, const wchar_t *dsn);

#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetOdbcDBGlobalConnectionString)(const wchar_t *dbConnection);
typedef bool (WINAPI *PDoODBCAuthentication)(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *odbcDriver, const wchar_t *dsn);

#endif