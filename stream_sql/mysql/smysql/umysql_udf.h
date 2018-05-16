
#ifndef _UMYSQL_UDF_SOCKETPRO_H_
#define _UMYSQL_UDF_SOCKETPRO_H_

#ifdef HAVE_PSI_SOCKET_INTERFACE
#undef HAVE_PSI_SOCKET_INTERFACE
#endif

#include "mysql_com.h"

#define STREAMING_DB_TRIGGER_PREFIX  L"sp_streaming_db_trigger_"

#ifdef __cplusplus
extern "C" {
#endif

    bool PublishDBEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    void PublishDBEvent_deinit(UDF_INIT *initid);
    long long PublishDBEvent(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

#ifdef __cplusplus
}
#endif

#endif
