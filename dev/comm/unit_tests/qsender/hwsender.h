#ifndef ___SOCKETPRO_CLIENT_HANDLER_QHW_H__
#define ___SOCKETPRO_CLIENT_HANDLER_QHW_H__

#include "aclientw.h"
using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../qhw_i.h"

//client handler for service HWSender
class HWSender : public CAsyncServiceHandler
{
public:
	HWSender(CClientSocket *pClientSocket)
	: CAsyncServiceHandler(sidHWSender, pClientSocket)
	{
	}

public:
	unsigned int SayHelloWord(const wchar_t* fullName, unsigned int count) {
		unsigned int SayHelloWordRtn;
		bool bProcessRy = ProcessR1(idSayHelloWordHWSender, fullName, count, SayHelloWordRtn);
		return SayHelloWordRtn;
	}
};

#endif
