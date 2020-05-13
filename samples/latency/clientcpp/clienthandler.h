#ifndef ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__
#define ___SOCKETPRO_CLIENT_HANDLER_LATENCY_H__

#include "../latencydef.h"
#include "../../../include/aclientw.h"

class CLatencyHandler : public SPA::ClientSide::CAsyncServiceHandler {
public:
	CLatencyHandler(SPA::ClientSide::CClientSocket *pClientSocket)
		: SPA::ClientSide::CAsyncServiceHandler(sidLatency, pClientSocket) {
	}
};
typedef SPA::ClientSide::CSocketPool<CLatencyHandler, SPA::ClientSide::CClientSocket> CLatencyPool;

#endif

