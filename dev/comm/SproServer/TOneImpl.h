#ifndef ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__

#include "../../include/aserverw.h"

using namespace SPA;
using namespace SPA::ServerSide;


/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TOne_i.h"

//server implementation for service CTOne

class CTOnePeer : public CClientPeer {
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

    void QueryCount(/*out*/int &QueryCountRtn) {
        QueryCountRtn = 1;
    }

    void QueryGlobalCount(/*out*/int &QueryGlobalCountRtn) {
        // TODO: Implement this method
    }

    void QueryGlobalFastCount(/*out*/int &QueryGlobalFastCountRtn) {
        // TODO: Implement this method
    }

    void Sleep(int nTime) {
        // TODO: Implement this method
    }

    void Echo(const UVariant &objInput, /*out*/UVariant &EchoRtn) {
        EchoRtn = objInput;
    }

    void EchoEx(const std::string &str, const std::wstring &wstr, const MyStruct &ms, /*out*/std::string &strOut, /*out*/std::wstring &wstrOut, /*out*/bool &EchoExRtn) {
        // TODO: Implement this method
    }

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I0_R1(idQueryCountCTOne, QueryCount, int)
        M_I0_R1(idQueryGlobalCountCTOne, QueryGlobalCount, int)
        M_I0_R1(idQueryGlobalFastCountCTOne, QueryGlobalFastCount, int)
        M_I1_R1(idEchoCTOne, Echo, UVariant, UVariant)
        M_I3_R3(idEchoExCTOne, EchoEx, std::string, std::wstring, MyStruct, std::string, std::wstring, bool)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R0(idSleepCTOne, Sleep, int)
        END_SWITCH
        return 0;
    }

};

#endif