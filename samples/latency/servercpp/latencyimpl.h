#ifndef ___SOCKETPRO_SERVICES_IMPL_LATENCYIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_LATENCYIMPL_H__

#include "../../../include/aserverw.h"
#include "../latencydef.h"

class CLatencyPeer : public SPA::ServerSide::CClientPeer {
private:

    void Echo1(unsigned int ms, /*out*/unsigned int &msOut) {
        msOut = ms;
    }

    void Echo2(unsigned int ms, /*out*/unsigned int &msOut) {
        msOut = ms;
    }


protected:

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R1(idEchoInt1, Echo1, unsigned int, unsigned int);
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R1(idEchoInt2, Echo2, unsigned int, unsigned int);
        END_SWITCH
        return 0;
    }
};

#endif

