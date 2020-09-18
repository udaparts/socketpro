#ifndef ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__
#define ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__

#include "../latencydef.h"
#include "../../../include/aclientw.h"

using namespace SPA::ClientSide;

typedef CASHandler<sidLatency> CLatencyHandler;
typedef CSocketPool<CLatencyHandler> CLatencyPool;

#endif
