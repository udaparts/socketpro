
#pragma once

#include "../ucomm.h"

namespace SPA {
    namespace Postgres {
        //asynchronous postgreSQL service id
        static const unsigned int sidPostgres = (unsigned int) tagServiceID::sidReserved + 0x6FFFFFF4;

        //two Open flag options specifically for PostgreSQL
        static const unsigned int ROWSET_META_FLAGS_REQUIRED = 0x40000000;
        static const unsigned int USE_SINGLE_ROW_MODE = 0x20000000;

        //error codes & others from PostgreSQL
        static const int ER_NO_DB_OPENED_YET = -1981;
        static const int ER_BAD_END_TRANSTACTION_PLAN = -1982;
        static const int ER_NO_PARAMETER_SPECIFIED = -1983;
        static const int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
        static const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
        static const int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
        static const int ER_BAD_TRANSTACTION_STAGE = -1987;

    } //namespace Postgres
} //namespace SPA
