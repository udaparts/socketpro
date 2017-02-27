
#ifndef __UDAPARTS_MYSQL_UPDATE_BASE_H__
#define __UDAPARTS_MYSQL_UPDATE_BASE_H__

#include "../../include/mysql/mysql.h"

#ifdef __cplusplus
extern "C" {
#endif
    my_bool SetConnectionString_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    void SetConnectionString_deinit(UDF_INIT *initid);
    long long SetConnectionString(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

    my_bool NotifyDatabaseEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    void NotifyDatabaseEvent_deinit(UDF_INIT *initid);
    char* NotifyDatabaseEvent(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

    my_bool GetConnections_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    void GetConnections_deinit(UDF_INIT *initid);
    char* GetConnections(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
#ifdef __cplusplus
}
#endif

#endif