
#ifndef _UMYSQL_SOCKETPRO_H_
#define _UMYSQL_SOCKETPRO_H_

#include "../ucomm.h"

namespace SPA {
    namespace Mysql {
        static const unsigned int sidMysql = (unsigned int) sidReserved + 0x6FFFFFF1; //asynchronous MySQL/MariaDB service id

        //error codes from async MySQL/MariaDB server library
        static const int ER_NO_DB_OPENED_YET = 1981;
        static const int ER_BAD_END_TRANSTACTION_PLAN = 1982;
        static const int ER_NO_PARAMETER_SPECIFIED = 1983;
        static const int ER_BAD_PARAMETER_COLUMN_SIZE = 1984;
        static const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985;
        static const int ER_DATA_TYPE_NOT_SUPPORTED = 1986;
        static const int ER_BAD_MANUAL_TRANSACTION_STATE = 1987;
        static const int ER_UNABLE_TO_SWITCH_TO_DATABASE = 1988;
        static const int ER_SERVICE_COMMAND_ERROR = 1989;

        //The following defines are required by non-plugin version MySQL/MariaDB SQL-stream technology

        static const int ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1990;
        /**
         * Use MySQL/MariaDB embedded at SocketPro server side by default.
         * Use this constance value for the input parameter flags with the method of CAsyncDBHandler::Open at client side
         * to open a connection to remote MySQL/MariaDB server at SocketPro server instead of embedded MySQL/MariaDB
         */
        static const unsigned int USE_REMOTE_MYSQL = 0x1;
    } //namespace Mysql
} //namespace SPA
#endif

