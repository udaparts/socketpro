
#ifndef _UMYSQL_SOCKETPRO_H_
#define _UMYSQL_SOCKETPRO_H_

#include "../ucomm.h"

namespace SPA {
    namespace Mysql {
        static const unsigned int sidMysql = (unsigned int) sidReserved + 0x6FFFFFF1; //asynchronous mysql service id

        /**
         * Use Mysql embedded at SocketPro server side by default.
         * Use this const value for the input parameter flags with the method of CAsyncDBHandler::Open at client side
         * to open a connection to remote Mysql server at SocketPro server instead of embedded Mysql
         */
        static const unsigned int USE_REMOTE_MYSQL = 0x1;


        //error codes from async mysql server library
        static const int ER_NO_DB_OPENED_YET = 1981;
        static const int ER_BAD_END_TRANSTACTION_PLAN = 1982;
        static const int ER_NO_PARAMETER_SPECIFIED = 1983;
        static const int ER_BAD_PARAMETER_COLUMN_SIZE = 1984;
        static const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985;
        static const int ER_DATA_TYPE_NOT_SUPPORTED = 1986;
        static const int ER_NO_DB_NAME_SPECIFIED = 1987;
        static const int ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1988;
        static const int ER_BAD_MANUAL_TRANSACTION_STATE = 1989;

    } //namespace Mysql
} //namespace SPA
#endif