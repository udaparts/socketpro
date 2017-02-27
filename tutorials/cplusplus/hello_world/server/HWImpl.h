
#include "../../../../include/aserverw.h"
#include "../../uqueue_demo/mystruct.h"

#ifndef WIN32_64
#include <thread>
#include <chrono>
#endif

#ifndef ___SOCKETPRO_SERVICES_IMPL_HWIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_HWIMPL_H__

#include <iostream>

using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../HW_i.h"

//server implementation for service HelloWorld

class HelloWorldPeer : public CClientPeer {
private:

    void SayHello(const std::wstring &firstName, const std::wstring &lastName, /*out*/std::wstring &SayHelloRtn) {
        SayHelloRtn = L"Hello " + firstName + L" " + lastName;
        std::wcout << SayHelloRtn << std::endl;
    }

    void Sleep(int ms) {
#ifdef WIN32_64
        ::Sleep(ms);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
    }

    void Echo(const CMyStruct &ms, /*out*/CMyStruct &msOut) {
        msOut = ms;
    }

protected:

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I2_R1(idSayHelloHelloWorld, SayHello, std::wstring, std::wstring, std::wstring)
        M_I1_R1(idEchoHelloWorld, Echo, CMyStruct, CMyStruct)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R0(idSleepHelloWorld, Sleep, int)
        END_SWITCH
        return 0;
    }
};


#endif