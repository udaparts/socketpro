#ifndef _UDAPARTS_ASYNC_DB2_HANDLER_H_
#define _UDAPARTS_ASYNC_DB2_HANDLER_H_

#include "odbcbase.h"
#include "db2/udb2.h"

namespace SPA {
    namespace ClientSide {
        typedef CBaseOdbc<SPA::Db2::sidDB2> CDb2;
        typedef CAsyncDBHandler<SPA::Db2::sidDB2> CDb2Base;
        typedef CSocketPool<CDb2> CDb2Pool;
    } //namespace ClientSide
} //namespace SPA

#endif