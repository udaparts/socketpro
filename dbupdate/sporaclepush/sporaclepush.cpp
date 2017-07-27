
#include "stdafx.h"
#include "sporaclepush.h"
#include "../include/dbupdate.h"

#ifdef WIN32_64
#if defined(WIN64) || defined(_WIN64)
#pragma comment(lib, "../lib/x64/oci.lib")
#else
#pragma comment(lib, "../lib/x86/oci.lib")
#endif
#endif

int SetConnectionString(const char *connectionString) {
    std::wstring s;
    if (connectionString) {
        s = SPA::Utilities::ToWide(connectionString);
    }
    return SetSocketProConnectionString(s.c_str());
}

char* NotifyDatabaseEvent(OCIExtProcContext *ctx, int eventType, short ind, const char *queryFilter, short str_0, const char *pGroup, short str_1, short *ret_0, short *ret_1) {
    if (OCI_IND_NULL == ind)
        eventType = -1;
    std::string sqlFilter;
    if (OCI_IND_NULL != str_0)
        sqlFilter = queryFilter;
    std::string groups;
    if (OCI_IND_NULL != str_1)
        groups = pGroup;
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
    SPA::UINT64 ret = NotifySocketProDatabaseEvent(vGroup.data(), (unsigned int) vGroup.size(), (SPA::UDB::tagUpdateEvent)eventType, SPA::Utilities::ToWide(sqlFilter.c_str()).c_str(), index, sb->GetMaxSize() / sizeof (unsigned int));
    unsigned int suc = (unsigned int) ret;
    unsigned int fail = (unsigned int) (ret >> 32);
    if (count > suc + fail)
        count = suc + fail;
    char *res = (char*) OCIExtProcAllocCallMemory(ctx, count);
    *ret_0 = (short) OCI_IND_NOTNULL;
    *ret_1 = (short) count;
    for (unsigned int n = 0; n < count; ++n) {
        res[n] = (index[n] ? '0' : '1');
    }
    return res;
}

char* GetConnections(OCIExtProcContext *ctx, short *ret_0, short *ret_1) {
    SPA::CScopeUQueue sb;
    unsigned int *index = (unsigned int *) sb->GetBuffer();
    unsigned int count = sb->GetMaxSize() / sizeof (unsigned int);
    SPA::UINT64 ret = GetSocketProConnections(index, sb->GetMaxSize() / sizeof (unsigned int));
    unsigned int suc = (unsigned int) ret;
    unsigned int fail = (unsigned int) (ret >> 32);
    if (count > suc + fail)
        count = suc + fail;
    char *res = (char*) OCIExtProcAllocCallMemory(ctx, count);
    *ret_0 = (short) OCI_IND_NOTNULL;
    *ret_1 = (short) count;
    for (unsigned int n = 0; n < count; ++n) {
        res[n] = (index[n] ? '0' : '1');
    }
    return res;
}
