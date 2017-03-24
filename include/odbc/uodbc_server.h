
#ifndef _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#include "../spa_module.h"
#include "uodbc.h"

#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetOdbcDBGlobalConnectionString(const wchar_t *dbConnection);

#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetOdbcDBGlobalConnectionString)(const wchar_t *dbConnection);

#endif