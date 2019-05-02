
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

SPA::UINT64 GetTimeTick() {
    SPA::UINT64 now = 0;
#ifdef WIN32_64
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    FILETIME ft;
    ::SystemTimeToFileTime(&st, &ft);
    now = ft.dwHighDateTime;
    now <<= 32;
    now += ft.dwLowDateTime;
    now /= 10000;
#else
    struct timeval start;
    gettimeofday(&start, NULL);
    now = start.tv_sec * 1000 + start.tv_usec / 1000;
#endif
    return now;
}