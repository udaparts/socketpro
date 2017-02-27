

#ifndef __UDAPARTS_ORACLE_UPDATE_BASE_H__
#define __UDAPARTS_ORACLE_UPDATE_BASE_H__

#include "../include/oracle/oci.h"

#ifdef __cplusplus
extern "C" {
#endif
    int SetConnectionString(const char *connectionString);
    char* NotifyDatabaseEvent(OCIExtProcContext *ctx, int eventType, short ind, const char *queryFilter, short str_0, const char *groups, short str_1, short *ret_0, short *ret_1);
    char* GetConnections(OCIExtProcContext *ctx, short *ret_0, short *ret_1);
#ifdef __cplusplus
}
#endif

#endif