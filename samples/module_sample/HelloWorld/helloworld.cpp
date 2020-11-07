// HelloWorld.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../../../include/spa_module.h"
#include "../../../tutorials/cplusplus/hello_world/server/HWImpl.h"

std::shared_ptr<CSocketProService<HelloWorldPeer> > g_pHelloWorld;

bool WINAPI InitServerLibrary(int param) {
    g_pHelloWorld.reset(new CSocketProService<HelloWorldPeer>(sidHelloWorld, SPA::tagThreadApartment::taNone));
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pHelloWorld.reset();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return sidHelloWorld;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (serviceId == sidHelloWorld)
        sc = g_pHelloWorld->GetSvsContext();
    else
        memset(&sc, 0, sizeof (sc));
    return sc;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId) {
    if (serviceId == sidHelloWorld)
        return 1; //The service only has one slow request -- idSleepHelloWorld
    return 0;
}

unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index) {
    if (serviceId == sidHelloWorld && index == 0)
        return idSleep; //the service has one slow request -- idSleepHelloWorld
    return 0;
}
