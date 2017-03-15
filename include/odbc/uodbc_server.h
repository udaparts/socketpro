
#ifndef _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _ASYNC_ODBC_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#include "../spa_module.h"
#include "uodbc.h"

#ifdef WIN32_64

#include <sqlext.h>

#else


#endif

#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetOdbcDBGlobalConnectionString(const wchar_t *dbConnection);

#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetOdbcDBGlobalConnectionString)(const wchar_t *dbConnection);

namespace SPA {
    namespace ServerSide {
        namespace Odbc {
            
        } //namespace Odbc
    } //namespace ServerSide
} //namespace SPA


#endif