
#include "stdafx.h"
#include "../include/dbupdate.h"
#include "dbupdateimpl.h"

unsigned int WINAPI SetSocketProConnectionString(const wchar_t *connectionString) {
    return CDBUpdateImpl::DBUpdate.SetSocketProConnectionString(connectionString ? connectionString : L"");
}

SPA::UINT64 WINAPI GetSocketProConnections(unsigned int *index, unsigned int size) {
    if (!index) {
        size = 0;
    }
    return CDBUpdateImpl::DBUpdate.GetSocketProConnections(index, size);
}

SPA::UINT64 WINAPI NotifySocketProDatabaseEvent(unsigned int *group, unsigned int count, SPA::UDB::tagUpdateEvent dbEvent, const wchar_t *queryFilter, unsigned int *index, unsigned int size) {
    if (!group || !count)
        return 0;
    if (!index)
        size = 0;
    return CDBUpdateImpl::DBUpdate.NotifySocketProDatabaseEvent(group, count, dbEvent, queryFilter ? queryFilter : L"", index, size);
}
