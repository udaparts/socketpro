#include "../../../../include/aclientw.h"
#include "../../uqueue_demo/mystruct.h"

#ifndef ___SOCKETPRO_CLIENT_HANDLER_HW_H__
#define ___SOCKETPRO_CLIENT_HANDLER_HW_H__

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../HW_i.h"

//client handler for service HelloWorld

class HelloWorld : public CAsyncServiceHandler {
public:

	HelloWorld(CClientSocket *pClientSocket)
		: CAsyncServiceHandler(sidHelloWorld, pClientSocket) {
	}

public:

	std::wstring SayHello(const wchar_t* firstName, const wchar_t* lastName) {
		std::wstring SayHelloRtn;
		bool bProcessRy = ProcessR1(idSayHelloHelloWorld, firstName, lastName, SayHelloRtn);
		return SayHelloRtn;
	}

	void Sleep(int ms) {
		bool bProcessRy = ProcessR0(idSleepHelloWorld, ms);
	}

	CMyStruct Echo(const CMyStruct& ms) {
		CMyStruct EchoRtn;
		bool bProcessRy = ProcessR1(idEchoHelloWorld, ms, EchoRtn);
		return EchoRtn;
	}
};
#endif
