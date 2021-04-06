#include "pch.h"
#include "rawthread.h"

PIRawThread WINAPI CreateSessions(PDataArrive da, PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta) {
	if (!da) {
		return nullptr;
	}
	PIRawThread rt = new CRawThread(da, sc, sessions, ta);
	return rt;
}
