#include "latencyimpl.h"

#ifndef ___SOCKETPRO_SERVICES_IMPL_LATENCY_SERVER_H__
#define ___SOCKETPRO_SERVICES_IMPL_LATENCY_SERVER_H__

class CLatencyServer : public SPA::ServerSide::CSocketProServer {
public:

    CLatencyServer(int param = 0) : SPA::ServerSide::CSocketProServer(param) {
    }

private:
    SPA::ServerSide::CSocketProService<CLatencyPeer> m_latency;

protected:

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        CSocketProServer::Config::SetAuthenticationMethod(SPA::ServerSide::amOwn);
        AddService();
        return true; //true -- ok; false -- no listening server
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        return true;
    }

private:

    void AddService() {
        bool ok = m_latency.AddMe(sidLatency, SPA::taNone);
        ok = m_latency.AddSlowRequest(idEchoInt2);
    }
};

#endif
