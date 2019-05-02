
#ifndef _UDAPARTS_ASYNC_SQLITE_HANDLER_H_
#define _UDAPARTS_ASYNC_SQLITE_HANDLER_H_

#include "sqlite/usqlite.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Sqlite::sidSqlite> CSqliteBase;
        typedef CAsyncDBHandler<SPA::Sqlite::sidSqlite> CSqlite;
    } //namespace ClientSide
} //namespace SPA

#endif
