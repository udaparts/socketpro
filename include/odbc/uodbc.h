
#pragma once

#include "../ucomm.h"

namespace SPA {
    namespace Odbc {
        static const unsigned int sidOdbc = (unsigned int) sidReserved + 0x6FFFFFF2; //asynchronous ODBC service id

		static const int ER_SUCCESS = 0;
		static const int ER_SUCCESS_WITH_INFO = 1; 
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
    } //namespace Odbc
} //namespace SPA