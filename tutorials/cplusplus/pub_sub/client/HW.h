#include "../../../../include/aclientw.h"

#ifndef ___SOCKETPRO_CLIENT_HANDLER_HW_H__
#define ___SOCKETPRO_CLIENT_HANDLER_HW_H__

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../HW_i.h"

//client handler for service HelloWorld

class HelloWorld : public CAsyncServiceHandler {
public:

	HelloWorld(CClientSocket *pClientSocket)
		: CAsyncServiceHandler(sidHelloWorld, pClientSocket) {
	}
};
#endif
