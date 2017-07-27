
#include "stdafx.h"
#include "spdb2push.h"
#include "../include/dbupdate.h"

void SQL_API_FN SetConnectionString(SQLUDF_VARCHAR *connectionString, SQLUDF_INTEGER *ret, SQLUDF_SMALLINT *connectionStringNullInd, SQLUDF_SMALLINT *retNullInd, SQLUDF_TRAIL_ARGS) {
    std::wstring s;
    if (*connectionStringNullInd == 0) {
        s = SPA::Utilities::ToWide(connectionString);
    }
    *ret = (sqlint32) SetSocketProConnectionString(s.c_str());
    *retNullInd = 0;
}

void SQL_API_FN NotifyDatabaseEvent(SQLUDF_INTEGER *et, SQLUDF_VARCHAR *queryFilter, SQLUDF_VARCHAR *pGroup, SQLUDF_VARCHAR *res, SQLUDF_SMALLINT *eventTypeNullInd, SQLUDF_SMALLINT *queryFilterNullInd, SQLUDF_SMALLINT *groupsNullInd, SQLUDF_SMALLINT *resNullInd, SQLUDF_TRAIL_ARGS) {
    std::string sqlFilter;
    if (*queryFilterNullInd == 0)
        sqlFilter = queryFilter;
    std::string groups;
    if (*groupsNullInd == 0)
        groups = pGroup;
    int eventType = -1;
    if (*eventTypeNullInd == 0)
        eventType = (int) (*et);
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
    for (unsigned int n = 0; n < count; ++n) {
        res[n] = (index[n] ? '0' : '1');
    }
    res[count] = 0;
    *resNullInd = 0;
}

void SQL_API_FN GetConnections(SQLUDF_VARCHAR *res, SQLUDF_SMALLINT *resNullInd, SQLUDF_TRAIL_ARGS) {
    SPA::CScopeUQueue sb;
    unsigned int *index = (unsigned int *) sb->GetBuffer();
    unsigned int count = sb->GetMaxSize() / sizeof (unsigned int);
    SPA::UINT64 ret = GetSocketProConnections(index, count);
    unsigned int suc = (unsigned int) ret;
    unsigned int fail = (unsigned int) (ret >> 32);
    if (count > suc + fail)
        count = suc + fail;
    for (unsigned int n = 0; n < count; ++n) {
        res[n] = (index[n] ? '0' : '1');
    }
    res[count] = 0;
    *resNullInd = 0;
}