
#include "stdafx.h"
#include "spmysqlpush.h"
#include "../include/dbupdate.h"

my_bool SetConnectionString_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 13;
    if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT) {
        strcpy(message, "SetConnectionString() requires an array of remote SocketPro server connection strings separated by the char '|'");
        return 1;
    }
    args->maybe_null[0] = 0;
    return 0;
}

void SetConnectionString_deinit(UDF_INIT *initid) {

}

long long SetConnectionString(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    std::string conn = args->args[0];
    return SetSocketProConnectionString(SPA::Utilities::ToWide(conn.c_str()).c_str());
}

my_bool NotifyDatabaseEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 1024;
    if (args->arg_count != 3 || args->arg_type[0] != INT_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != STRING_RESULT) {
        strcpy(message, "NotifyDatabaseEvent() requires database event type number, a filter string, and a chat group integer id string separated by the char ','");
        return 1;
    }
    args->maybe_null[0] = 0;
    args->maybe_null[1] = 0;
    args->maybe_null[2] = 0;
    return 0;
}

void NotifyDatabaseEvent_deinit(UDF_INIT *initid) {


}

char* NotifyDatabaseEvent(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error) {
    long long eventType = *((long long*) (args->args[0]));
    std::string sqlFilter(args->args[1], args->lengths[1]);
    std::string groups(args->args[2], args->lengths[2]);
    const char *start = groups.c_str();
    const char *end = start + groups.size();
    std::vector<unsigned int> vGroup;
    while (end > start) {
        unsigned int gid = (unsigned int) std::atoi(start);
        if (gid) {
            vGroup.push_back(gid);
        }
        const char *str = strchr(start, ',');
        if (!str) {
            break;
        }
        start = str + 1;
    }
    SPA::CScopeUQueue sb;
    unsigned int *index = (unsigned int *) sb->GetBuffer();
    unsigned int count = sb->GetMaxSize() / sizeof (unsigned int);
    SPA::UINT64 res = NotifySocketProDatabaseEvent(vGroup.data(), (unsigned int) vGroup.size(), (SPA::UDB::tagUpdateEvent) eventType, SPA::Utilities::ToWide(sqlFilter.c_str()).c_str(), index, count);
    unsigned int suc = (unsigned int) res;
    unsigned int fail = (unsigned int) (res >> 32);
    if (count > suc + fail)
        count = suc + fail;
    for (unsigned int n = 0; n < count; ++n) {
        result[n] = (index[n] ? '0' : '1');
    }
    result[count] = 0;
    *is_null = 0;
    *length = count;
    return result;
}

my_bool GetConnections_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 1024;

    return 0;
}

void GetConnections_deinit(UDF_INIT *initid) {


}

char* GetConnections(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error) {
    SPA::CScopeUQueue sb;
    unsigned int *index = (unsigned int *) sb->GetBuffer();
    unsigned int count = sb->GetMaxSize() / sizeof (unsigned int);
    SPA::UINT64 res = GetSocketProConnections(index, count);
    unsigned int suc = (unsigned int) res;
    unsigned int fail = (unsigned int) (res >> 32);
    if (count > suc + fail)
        count = suc + fail;
    for (unsigned int n = 0; n < count; ++n) {
        result[n] = (index[n] ? '0' : '1');
    }
    result[count] = 0;
    *is_null = 0;
    *length = count;
    return result;
}