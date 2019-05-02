#ifndef ___SOCKETPRO_CLIENT_HANDLER_PI_H__
#define ___SOCKETPRO_CLIENT_HANDLER_PI_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "Pi_i.h"

//client handler for service CPPi
class CPPi : public CAsyncServiceHandler
{
public:
	CPPi(CClientSocket *pClientSocket = UNULL_PTR, IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR)
		: CAsyncServiceHandler(sidCPPi, pClientSocket, pDefaultAsyncResultsHandler)
	{
	}

protected:
	//When a result comes from a remote SocketPro server, the below virtual function will be called.
	//We always process returning results inside the function.
	virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
	{
		switch(usRequestID)
		{
		case idComputeCPPi:
			UQueue >> m_ComputeRtn;
			break;
		default:
			break;
		}
	}
	
public:
	double	m_ComputeRtn;
	double Compute(double dStart, double dStep, int nNum)
	{
		ComputeAsyn(dStart, dStep, nNum);
		GetAttachedClientSocket()->WaitAll();
		return m_ComputeRtn;
	}
	void ComputeAsyn(double dStart, double dStep, int nNum)
	{
		SendRequest((unsigned short)idComputeCPPi, dStart, dStep, nNum);
	}
};
#endif
