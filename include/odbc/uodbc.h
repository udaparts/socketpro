
#pragma once

#include "../definebase.h"
#include "../ucomm.h"

namespace SPA {
    namespace Odbc {
        static const unsigned int sidOdbc = (unsigned int)tagServiceID::sidODBC; //asynchronous ODBC service id

        //meta recordsets
        static const unsigned short idSQLColumnPrivileges = 0x7f00 + 100;
        static const unsigned short idSQLColumns = 0x7f00 + 101;
        static const unsigned short idSQLForeignKeys = 0x7f00 + 102;
        static const unsigned short idSQLPrimaryKeys = 0x7f00 + 103;
        static const unsigned short idSQLProcedureColumns = 0x7f00 + 104;
        static const unsigned short idSQLProcedures = 0x7f00 + 105;
        static const unsigned short idSQLSpecialColumns = 0x7f00 + 106;
        static const unsigned short idSQLStatistics = 0x7f00 + 107;
        static const unsigned short idSQLTablePrivileges = 0x7f00 + 108;
        static const unsigned short idSQLTables = 0x7f00 + 109;
        static const unsigned short idSQLGetInfo = 0x7f00 + 110;

        static const int ER_SUCCESS = 0;
        static const int ER_ERROR = -1;

        //error codes from async ODBC server library
        static const int ER_NO_DB_OPENED_YET = -1981;
        static const int ER_BAD_END_TRANSTACTION_PLAN = -1982;
        static const int ER_NO_PARAMETER_SPECIFIED = -1983;
        static const int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
        static const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
        static const int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
        static const int ER_NO_DB_NAME_SPECIFIED = -1987;
        static const int ER_ODBC_ENVIRONMENT_NOT_INITIALIZED = -1988;
        static const int ER_BAD_MANUAL_TRANSACTION_STATE = -1989;
        static const int ER_BAD_INPUT_PARAMETER_DATA_TYPE = -1990;
        static const int ER_BAD_PARAMETER_DIRECTION_TYPE = -1991;
        static const int ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = -1992;
    } //namespace Odbc
} //namespace SPA