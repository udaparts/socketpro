
#include "stdafx.h"
#include "../../../include/udatabase.h"
#include "../../../include/aserverw.h"
#include "umysql_udf.h"
#include "streamingserver.h"

bool PublishDBEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 1024;
    if (args->arg_count < 5 || args->arg_type[0] != INT_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != STRING_RESULT || args->arg_type[3] != STRING_RESULT) {
#ifdef WIN32_64
        strcpy_s(message, 1023, "PublishDBEvent() requires database event type number, host, database, and one or more other values");
#else
        strcpy(message, "PublishDBEvent() requires database event type number, host, database, and one or more other values");
#endif
        return 1;
    }
    return 0;
}

void PublishDBEvent_deinit(UDF_INIT *initid) {

}

long long PublishDBEvent(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    if (!g_pStreamingServer)
        return 0;
    VARIANT *p = nullptr;
    long long eventType = *((long long*) (args->args[0]));
    unsigned int count = args->arg_count;
    SPA::UVariant vtArray;
    vtArray.vt = (VT_ARRAY | VT_VARIANT);
    SAFEARRAYBOUND sab[] = {count + 1, 0};
    vtArray.parray = SafeArrayCreate(VT_VARIANT, 1, sab);
    SafeArrayAccessData(vtArray.parray, (void**) &p);
    ::memset(p, 0, sizeof (VARIANT) * count);

    p[0].vt = VT_I4;
    p[0].intVal = (int) eventType;

    {
        char *s = nullptr;
        char host_name[128] = {0};
        int res = ::gethostname(host_name, sizeof (host_name));
        unsigned int len = (unsigned int) strlen(host_name);
        SAFEARRAYBOUND sab1[] = {len, 0};
        p[1].vt = (VT_ARRAY | VT_I1);
        p[1].parray = SafeArrayCreate(VT_I1, 1, sab1);
        SafeArrayAccessData(p[1].parray, (void**) &s);
        memcpy(s, host_name, len);
        SafeArrayUnaccessData(p[1].parray);
        ++p;
    }

    for (unsigned int n = 1; n < count; ++n) {
        bool bNull = (args->args[n]) ? false : true;
        if (bNull && args->maybe_null[n]) {
            p[n].vt = VT_NULL;
            continue;
        }
        Item_result type = args->arg_type[n];
        switch (type) {
            case STRING_RESULT:
            {
                char *s = nullptr;
                unsigned int len = (unsigned int) args->lengths[n];
                p[n].vt = (VT_ARRAY | VT_I1);
                SAFEARRAYBOUND sab1[] = {len, 0};
                p[n].parray = SafeArrayCreate(VT_I1, 1, sab1);
                SafeArrayAccessData(p[n].parray, (void**) &s);
                memcpy(s, args->args[n], len);
                SafeArrayUnaccessData(p[n].parray);
            }
                break;
            case REAL_RESULT:
                p[n].vt = VT_R8;
                p[n].dblVal = *((double*) (args->args[n]));
                break;
            case INT_RESULT:
                p[n].vt = VT_I8;
                p[n].llVal = *((SPA::INT64*) (args->args[n]));
                break;
            case DECIMAL_RESULT:
            {
                unsigned long len = args->lengths[n];
                if (len < 20) {
                    SPA::ParseDec(args->args[n], p[n].decVal);
                } else {
                    SPA::ParseDec_long(args->args[n], p[n].decVal);
                }
                p[n].vt = VT_DECIMAL;
            }
                break;
            default:
                assert(false);
                break;
        }
    }
    SafeArrayUnaccessData(vtArray.parray);
    if (g_pStreamingServer) {
        return CSocketProServer::PushManager::Publish(vtArray, &SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1) ? 1 : 0;
    }
    return 0;
}

bool SetSQLStreamingPlugin_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 1024;
    if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT) {
#ifdef WIN32_64
        strcpy_s(message, 1023, "SetSQLStreamingPlugin() requires a DB connection string to set triggers for publishing table update events");
#else
        strcpy(message, "SetSQLStreamingPlugin() requires a DB connection string to set triggers for publishing table update events");
#endif
        return 1;
    }
    return 0;
}

void SetSQLStreamingPlugin_deinit(UDF_INIT *initid) {

}

long long SetSQLStreamingPlugin(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    if (!g_pStreamingServer)
        return 0;
    unsigned int len = (unsigned int) args->lengths[0];
    char *str = args->args[0];
    std::wstring dbConn = SPA::Utilities::ToWide(str, len);
    dbConn += L";server=localhost";
    CMysqlImpl impl;
    int res = 0, ms = 0;
    std::wstring errMsg;
    impl.Open(dbConn, 0, res, errMsg, ms);
    if (res) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Configuring streaming DB failed when connecting to local database (errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
    }
    if (!CMysqlImpl::SetPublishDBEvent(impl))
        return 0;
    return (CMysqlImpl::CreateTriggers(impl, CSetGlobals::Globals.cached_tables) ? 1 : 0);
}
