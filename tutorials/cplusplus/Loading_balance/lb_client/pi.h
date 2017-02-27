#ifndef ___SOCKETPRO_CLIENT_HANDLER_PI_H__
#define ___SOCKETPRO_CLIENT_HANDLER_PI_H__

#include "../../../../include/aclientw.h"

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../pi_i.h"

//client handler for service Pi

class Pi : public CAsyncServiceHandler {
public:
	Pi(CClientSocket *pClientSocket)
		: CAsyncServiceHandler(sidPi, pClientSocket) {
	}
};
#endif
