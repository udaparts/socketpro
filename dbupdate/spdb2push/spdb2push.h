

#ifndef __UDAPARTS_DB2_UPDATE_BASE_H__
#define __UDAPARTS_DB2_UPDATE_BASE_H__

#include "../include/db2/sql.h"
#include "../include/db2/sqludf.h"

#ifdef __cplusplus
extern "C" {
#endif
    void SQL_API_FN SetConnectionString(SQLUDF_VARCHAR *connectionString,
            SQLUDF_INTEGER *ret,
            SQLUDF_SMALLINT *connectionStringNullInd,
            SQLUDF_SMALLINT *retNullInd,
            SQLUDF_TRAIL_ARGS);

    void SQL_API_FN NotifyDatabaseEvent(SQLUDF_INTEGER *eventType,
            SQLUDF_VARCHAR *queryFilter,
            SQLUDF_VARCHAR *groups,
            SQLUDF_VARCHAR *res,
            SQLUDF_SMALLINT *eventTypeNullInd,
            SQLUDF_SMALLINT *queryFilterNullInd,
            SQLUDF_SMALLINT *groupsNullInd,
            SQLUDF_SMALLINT *resNullInd,
            SQLUDF_TRAIL_ARGS);

    void SQL_API_FN GetConnections(SQLUDF_VARCHAR *res,
            SQLUDF_SMALLINT *resNullInd,
            SQLUDF_TRAIL_ARGS);
#ifdef __cplusplus
}
#endif

#endif