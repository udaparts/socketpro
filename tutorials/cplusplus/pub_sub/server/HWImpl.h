#include "../../../../include/aserverw.h"
#include "../../uqueue_demo/mystruct.h"

#include <thread>
#include <chrono>

#ifndef ___SOCKETPRO_SERVICES_IMPL_HWIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_HWIMPL_H__

using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../HW_i.h"

//server implementation for service HelloWorld

class HelloWorldPeer : public CClientPeer {
private:

    void SayHello(const std::wstring &firstName, const std::wstring &lastName, /*out*/std::wstring &SayHelloRtn) {
        if (!firstName.size()) {
            throw CUException("First name are required and cannot be empty", __FILE__, __LINE__, "HelloWorldPeer::SayHello", 645321);
        }
        assert(CSocketProServer::IsMainThread());
        SayHelloRtn = L"Hello " + firstName + L" " + lastName;
        std::wstring msg = L"Say hello from " + firstName + L" " + lastName;
        UVariant vtMessage(msg.c_str());
        //notify a message to groups [2, 3] at server side
        unsigned int groups[] = {2, 3};
        GetPush().Publish(vtMessage, groups, 2);
    }

    void Sleep(int ms) {
        assert(!CSocketProServer::IsMainThread());
        if (ms < 0) {
            throw CUException("Parameter ms cannot be less than 0", __FILE__, __LINE__, "HelloWorldPeer::Sleep", 123456);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void Echo(const CMyStruct &ms, /*out*/CMyStruct &msOut) {
        msOut = ms;
    }

protected:

    virtual void OnSwitchFrom(unsigned int serviceId) {
        //subscribe for chat groups 1 and 3 at server side
        unsigned int chat_groups[] = {1, 3};
        GetPush().Subscribe(chat_groups, 2);
    }

    virtual void OnSubscribe(const unsigned int *pGroup, unsigned int count) {
        std::wcout << GetUID();
        std::cout << " subscribes for groups " << ToString(pGroup, (int) count) << std::endl;
    }

    virtual void OnUnsubscribe(const unsigned int *pGroup, unsigned int count) {
        std::wcout << GetUID();
        std::cout << " unsubscribes from groups " << ToString(pGroup, (int) count) << std::endl;
    }

    virtual void OnPublish(const UVariant& vtMessage, const unsigned int *pGroup, unsigned int count) {
        std::wcout << GetUID();
        std::cout << " publishes a message (vtMessage) to groups " << ToString(pGroup, (int) count) << std::endl;
    }

    virtual void OnSendUserMessage(const wchar_t* receiver, const UVariant& vtMessage) {
        std::wcout << GetUID();
        std::wcout << L" sends a message (vtMessage) to " << receiver << std::endl;
    }

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I2_R1(idSayHello, SayHello, std::wstring, std::wstring, std::wstring)
        M_I1_R1(idEcho, Echo, CMyStruct, CMyStruct)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R0(idSleep, Sleep, int)
        END_SWITCH
        return 0;
    }
};

#endif
