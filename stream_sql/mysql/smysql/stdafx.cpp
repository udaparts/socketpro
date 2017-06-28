
#include "stdafx.h"
#include "streamingserver.h"
#include "umysql_udf.h"

my_bool PublishDBEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 13;
    if (args->arg_count != 2 || args->arg_type[0] != INT_RESULT || args->arg_type[1] != STRING_RESULT) {
#ifdef WIN32_64
        strcpy_s(message, 1024, "PublishDBEvent() requires database event type number and a filter string");
#else
        strcpy(message, "PublishDBEvent() requires database event type number and a filter string");
#endif
        return 1;
    }
    args->maybe_null[0] = 0;
    args->maybe_null[1] = 0;
    return 0;
}

void PublishDBEvent_deinit(UDF_INIT *initid) {

}

long long PublishDBEvent(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    long long eventType = *((long long*) (args->args[0]));
    std::string s = std::to_string(eventType);
    s += "/";
    s.append(args->args[1], args->lengths[1]);
    SPA::UVariant vt(s.c_str());
    if (g_pStreamingServer) {
        return SPA::ServerSide::CSocketProServer::PushManager::Publish(vt, &SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1) ? 1 : 0;
    }
    return 0;
}
