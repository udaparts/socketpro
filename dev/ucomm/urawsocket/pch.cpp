#include "pch.h"
#include "rawthread.h"

SPA::SessionPoolHandle WINAPI CreateASessionPool(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta) {
    if (!da) {
        return nullptr;
    }
    SPA::SessionPoolHandle rt = new SPA::CRawThread(da, sc, sessions, ta);
    return rt;
}
