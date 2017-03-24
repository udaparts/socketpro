
#pragma once

#include "../ucomm.h"
#include "../udatabase.h"
#include <sqlext.h>

namespace SPA {
    namespace Odbc {
        static const unsigned int sidOdbc = (unsigned int) sidReserved + 0x6FFFFFF2; //asynchronous ODBC service id

        //meta recordsets
        static const unsigned short idSQLColumnPrivileges = SPA::UDB::idEndRows + 100;
        static const unsigned short idSQLColumns = SPA::UDB::idEndRows + 101;
        static const unsigned short idSQLForeignKeys = SPA::UDB::idEndRows + 102;
        static const unsigned short idSQLPrimaryKeys = SPA::UDB::idEndRows + 103;
        static const unsigned short idSQLProcedureColumns = SPA::UDB::idEndRows + 104;
        static const unsigned short idSQLProcedures = SPA::UDB::idEndRows + 105;
        static const unsigned short idSQLSpecialColumns = SPA::UDB::idEndRows + 106;
        static const unsigned short idSQLStatistics = SPA::UDB::idEndRows + 107;
        static const unsigned short idSQLTablePrivileges = SPA::UDB::idEndRows + 108;
        static const unsigned short idSQLTables = SPA::UDB::idEndRows + 109;
        static const unsigned short idSQLGetInfo = SPA::UDB::idEndRows + 110;

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

    } //namespace Odbc
} //namespace SPA