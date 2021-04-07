#include "pch.h"
#include "rawthread.h"

SPA::PIRawThread WINAPI CreateSessions(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta) {
	if (!da) {
		return nullptr;
	}
	SPA::PIRawThread rt = new SPA::CRawThread(da, sc, sessions, ta);
	return rt;
}
