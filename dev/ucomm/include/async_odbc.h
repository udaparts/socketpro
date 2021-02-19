#ifndef _UDAPARTS_ASYNC_ODBC_HANDLER_H_
#define _UDAPARTS_ASYNC_ODBC_HANDLER_H_

#include "odbcbase.h"

namespace SPA {
    namespace ClientSide {
        typedef CBaseOdbc COdbc;
        typedef CSocketPool<COdbc> COdbcPool;
    } //namespace ClientSide
} //namespace SPA

#endif
