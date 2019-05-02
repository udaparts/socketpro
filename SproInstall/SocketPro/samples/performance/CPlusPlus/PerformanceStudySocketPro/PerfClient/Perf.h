#ifndef ___SOCKETPRO_CLIENT_HANDLER_PERF_H__
#define ___SOCKETPRO_CLIENT_HANDLER_PERF_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../Perf_i.h"

//client handler for service CPerf
class CPerf : public CAsyncServiceHandler
{
public:
	CPerf(CClientSocket *pClientSocket = NULL, IAsyncResultsHandler *pDefaultAsyncResultsHandler = NULL)
	: CAsyncServiceHandler(sidCPerf, pClientSocket, pDefaultAsyncResultsHandler)
	{
	}

protected:
	CComBSTR m_MyEchoRtn;
	
	//We can process returning results inside the function.
	virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
	{
		switch(usRequestID)
		{
		case idMyEchoCPerf:
			UQueue >> m_MyEchoRtn;
			break;
		default:
			break;
		}
	}

public:
	void MyEchoAsyn(LPCWSTR strInput)
	{
		//make sure that the handler is attached to a client socket before calling the below statement
		SendRequest(idMyEchoCPerf, strInput);
	}
	const CComBSTR& MyEcho(LPCWSTR strInput)
	{
		MyEchoAsyn(strInput);
		GetAttachedClientSocket()->WaitAll();
		return m_MyEchoRtn;
	}
};
#endif
