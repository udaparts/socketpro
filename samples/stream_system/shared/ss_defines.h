
#include "../../../include/ucomm.h"
#include "../../../include/membuffer.h"
#include <vector>

#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//#defines for stream system
static const unsigned int sidStreamSystem = ((unsigned int) SPA::tagServiceID::sidReserved + 1210);

static const unsigned short idQueryMaxMinAvgs = (unsigned short) SPA::tagBaseRequestID::idReservedTwo + 1;
static const unsigned short idGetMasterSlaveConnectedSessions = idQueryMaxMinAvgs + 1;
static const unsigned short idUploadEmployees = idQueryMaxMinAvgs + 2;
static const unsigned short idGetRentalDateTimes = idQueryMaxMinAvgs + 3;

struct CMaxMinAvg {

    CMaxMinAvg() : Max(0.0), Min(0.0), Avg(0) {
    }
    double Max;
    double Min;
    double Avg;
};

static_assert(24 == sizeof (CMaxMinAvg), "sizeof(CMaxMinAvg) != 24");

struct CRentalDateTimes {

    CRentalDateTimes() : rental_id(0), Rental(0), Return(0), LastUpdate(0) {
    }
    SPA::INT64 rental_id;
    SPA::UINT64 Rental;
    SPA::UINT64 Return;
    SPA::UINT64 LastUpdate;
};

static_assert(32 == sizeof (CRentalDateTimes), "sizeof(CRentalDateTimes) != 32");
typedef std::vector<SPA::INT64> CInt64Array;

#endif
