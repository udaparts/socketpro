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
        Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);

        //add service(s) into SocketPro server
        AddServices();
        return true; //true -- ok; false -- no listening server
    }

private:
    CSocketProService<CDummyPeer> m_Pi;
    CSocketProService<CDummyPeer> m_PiWorker;

private:

    void AddServices() {
        bool ok = m_Pi.AddMe(sidPi);
        std::cout << "m_Pi.AddMe ok: " << ok << std::endl;
        ok = m_PiWorker.AddMe(sidPiWorker);
        std::cout << "m_PiWorker.AddMe ok: " << ok << std::endl;
        ok = Router::SetRouting(sidPi, sidPiWorker);
        std::cout << "Router::SetRouting ok: " << ok << std::endl;
    }
};

#endif