

#ifndef __UDAPARTS_DATABASE_UPDATE_BASE_H__
#define __UDAPARTS_DATABASE_UPDATE_BASE_H__

#include "../../include/udatabase.h"

#ifdef __cplusplus
extern "C" {
#endif
    unsigned int WINAPI SetSocketProConnectionString(const wchar_t *connectionString);
    SPA::UINT64 WINAPI NotifySocketProDatabaseEvent(unsigned int *group, unsigned int count, SPA::UDB::tagUpdateEvent dbEvent, const wchar_t *queryFilter, unsigned int *index, unsigned int size);
    SPA::UINT64 WINAPI GetSocketProConnections(unsigned int *index, unsigned int size);
#ifdef __cplusplus
}
#endif

#endif