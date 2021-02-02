#pragma once

#include "../spa_module.h"
#include "usqlite.h"

#ifndef _SQLITE_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _SQLITE_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

namespace SPA {
    namespace ServerSide {
        namespace Sqlite {
            /**
             * Don't use sqlite update hook at server side by default.
             * If update hook is enabled, all connected clients will be notified when there is a record insert, update or delete.
             * For details, refer to the documentation of sqlite function sqlite3_update_hook
             */
            static const unsigned int ENABLE_GLOBAL_SQLITE_UPDATE_HOOK = 0x1;

            /**
             * Don't use sqlite shared cache at server side by default
             */
            static const unsigned int USE_SHARED_CACHE_MODE = 0x2;

            /**
             * Sqlite server will return extended error codes to client by default
             */
            static const unsigned int DO_NOT_USE_EXTENDED_ERROR_CODE = 0x4;

            /**
             * Sqlite server will use utf8 encoding by default
             */
            static const unsigned int USE_UTF16_ENCODING = 0x8;
        } //namespace Sqlite
    } //namespace ServerSide
} //namespace SPA

#define MONITORED_TABLES   "monitored_tables"

#endif
