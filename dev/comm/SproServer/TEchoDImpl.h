#ifndef ___SOCKETPRO_SERVICES_IMPL_TECHODIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TECHODIMPL_H__

#include "../../include/aserverw.h"

using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoD_i.h"

//server implementation for service CEchoSys

class CEchoSysPeer : public CClientPeer {
protected:

    virtual void OnSwitchFrom(unsigned int serviceId) {
        //initialize the object here
    }

    virtual void OnReleaseResource(bool closing, unsigned int info) {
        if (closing) {
            //closing the socket with error code = info
        } else {
            //switch to a new service with the service id = info
        }

        //release all of your resources here as early as possible
    }

    void EchoMyStruct(const MyStruct &my, /*out*/MyStruct &EchoMyStructRtn) {
        EchoMyStructRtn = my;
    }

    void EchoUQueue(const CUQueue &q, /*out*/CUQueue &EchoUQueueRtn) {
        EchoUQueueRtn << q;
    }

    void EchoComplex0(double d, const std::wstring &s, const UVariant &simpleObj, bool b, /*out*/std::wstring &sOut, /*out*/UVariant &EchoComplex0Rtn) {
        EchoComplex0Rtn = simpleObj;
        sOut = s;
    }

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R1(idEchoMyStructCEchoSys, EchoMyStruct, MyStruct, MyStruct)
        M_I1_R1(idEchoUQueueCEchoSys, EchoUQueue, CUQueue, CUQueue)
        M_I4_R2(idEchoComplex0CEchoSys, EchoComplex0, double, std::wstring, UVariant, bool, std::wstring, UVariant)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        END_SWITCH
        return 0;
    }

};

#endif