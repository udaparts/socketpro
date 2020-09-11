#ifndef ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__
#define ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__

#include "../latencydef.h"
#include "../../../include/aclientw.h"

typedef SPA::ClientSide::CASHandler<sidLatency> CLatencyHandler;
typedef SPA::ClientSide::CSocketPool<CLatencyHandler, SPA::ClientSide::CClientSocket> CLatencyPool;

#endif
