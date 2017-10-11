
#include "../../../include/ucomm.h"
#include "../../../include/membuffer.h"
#include <vector>

#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//#defines for stream system
static const unsigned int sidStreamSystem = (SPA::sidReserved + 1210);

static const unsigned short idQueryMaxMinAvgs = SPA::idReservedTwo + 1;
static const unsigned short idGetMasterSlaveConnectedSessions = SPA::idReservedTwo + 2;
static const unsigned short idUploadEmployees = SPA::idReservedTwo + 3;

struct CMaxMinAvg {
    double Max;
    double Min;
    double Avg;
};

static SPA::CUQueue& operator<<(SPA::CUQueue &q, const std::vector<SPA::INT64> &v) {
    q << (unsigned int) v.size();
    q.Push((const unsigned char*) v.data(), (unsigned int) (v.size() * sizeof (SPA::INT64)));
    return q;
}

static SPA::CUQueue& operator>>(SPA::CUQueue &q, std::vector<SPA::INT64> &v) {
    unsigned int count;
    v.clear();
    q >> count;
    auto data = (const SPA::INT64*)q.GetBuffer();
    v.assign(data, data + count);
    q.Pop(count * sizeof (SPA::INT64));
    return q;
}

#endif