
#include "../../../include/ucomm.h"
#include "../../../include/membuffer.h"

#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//#defines for stream system
static const unsigned int sidStreamSystem = (SPA::sidReserved + 121);

static const unsigned short idQueryMaxMinAvgs = SPA::idReservedTwo + 1;


struct CMaxMinAvg {
    SPA::INT64 Max;
    SPA::INT64 Min;
    double Avg;
};

#endif