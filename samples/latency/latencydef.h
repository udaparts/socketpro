#ifndef ___SOCKETPRO_DEFINES_LATENCY_H__
#define ___SOCKETPRO_DEFINES_LATENCY_H__

#include "../../include/ucomm.h"

#define sidLatency  ((unsigned int)SPA::tagServiceID::sidReserved + 128)

#define idEchoInt1  ((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1)
#define idEchoInt2  (idEchoInt1 + 2)

#endif