
#include "includes.h"
#include <ctype.h>

unsigned int GetNumberOfCores() {
#ifndef WINCE
    return std::thread::hardware_concurrency();
#else
    return boost::thread::hardware_concurrency();
#endif
}

void ChangeUInt32Endian(unsigned int *pGroup, unsigned int count) {
    unsigned int n;
    for (n = 0; n < count; ++n) {
        SPA::CUQueue::ChangeEndian((unsigned char*) (pGroup + n), sizeof (unsigned int));
    }
}

SPA::CUQueue& operator<<(SPA::CUQueue &mc, const SPA::CSwitchInfo &si) {
    mc << si.SockMajorVersion;
    mc << si.ServiceId;
    mc << si.MajorVersion;
    mc << si.MinorVersion;
    mc << si.SockMinorVersion;
    mc << si.Param0;
    mc << si.Param1;
    mc << si.Param2;
    mc << si.SwitchTime;
    return mc;
}

SPA::CUQueue& operator>>(SPA::CUQueue &mc, SPA::CSwitchInfo &si) {
    mc >> si.SockMajorVersion;
    mc >> si.ServiceId;
    mc >> si.MajorVersion;
    mc >> si.MinorVersion;
    mc >> si.SockMinorVersion;
    mc >> si.Param0;
    mc >> si.Param1;
    mc >> si.Param2;
    mc >> si.SwitchTime;
    return mc;
}
