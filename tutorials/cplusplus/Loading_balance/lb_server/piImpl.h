#include "../../../../include/aserverw.h"

#ifndef ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__

using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../pi_i.h"

class CMySocketProServer : public CSocketProServer {
protected:
	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
		//amIntegrated and amMixed not supported yet
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddServices();
		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<SPA::ServerSide::CDummyPeer> m_Pi;
	CSocketProService<SPA::ServerSide::CDummyPeer> m_PiWorker;
	//One SocketPro server supports any number of services. You can list them here!

private:
	void AddServices() {
		bool ok = m_Pi.AddMe(sidPi);
		ok = m_PiWorker.AddMe(sidPiWorker);
	}
};

#endif