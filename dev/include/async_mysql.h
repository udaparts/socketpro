
#ifndef _UDAPARTS_ASYNC_MYSQL_HANDLER_H_
#define _UDAPARTS_ASYNC_MYSQL_HANDLER_H_

#include "mysql/umysql.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Mysql::sidMysql> CMysqlBase;
        typedef CAsyncDBHandler<SPA::Mysql::sidMysql> CMysql;
    } //namespace ClientSide
} //namespace SPA

#endif
