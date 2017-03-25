
#ifndef _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#include "uodbc.h"
#include "../spa_module.h"


#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetOdbcDBGlobalConnectionString(const wchar_t *dbConnection);

#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetOdbcDBGlobalConnectionString)(const wchar_t *dbConnection);

#endif