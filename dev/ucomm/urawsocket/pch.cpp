#include "pch.h"
#include "rawthread.h"

PIRawThread WINAPI CreateSessions(PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta) {
	PIRawThread rt = new CRawThread(sc, sessions, ta);
	return rt;
}
