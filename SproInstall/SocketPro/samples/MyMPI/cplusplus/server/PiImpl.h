#ifndef ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "Pi_i.h"
#include <iostream>
using namespace std;

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

//server implementation for service CPPi
class CPPiPeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		cout << "Switch to the service CPPiPeer" << endl;
	}

	void Compute(double dStart, double dStep, int nNum, /*out*/double &ComputeRtn)
	{
		int n;
		int n100 = nNum/100;
		double dX = dStart;
		dX += dStep/2;
		double dd = dStep * 4.0;
		ComputeRtn = 0.0;
		for(n=0; n<nNum; n++)
		{
			dX += dStep;
			ComputeRtn += dd/(1 + dX*dX);
			if(n100 > 0 && ((n+1)%n100) == 0 && n>0)
			{
				if(IsCanceled() || IsClosing())
					break;
			}
		}
	}

	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch(usRequestID)
		{
		case idComputeCPPi:
			{
				double dStart;
				double dStep;
				int nNum;
				double ComputeRtn;
				m_UQueue >> dStart;
				m_UQueue >> dStep;
				m_UQueue >> nNum;
				Compute(dStart, dStep, nNum, ComputeRtn);
				SendResult(idComputeCPPi, ComputeRtn);
			}
			break;
		default:
			break;
		}
		return S_OK;
	}
};

class CMySocketProServer : public CSocketProServer
{
protected:
	virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	{
		//give permission to all
		return true;
	}

	virtual void OnAccept(unsigned int hSocket, int nError)
	{
		cout << "socket = " << hSocket << " connected\n";
	}

	virtual void OnClose(unsigned int hSocket, int nError)
	{
		cout << "socket = " << hSocket << " disconnected\n";
	}

	virtual bool OnSettingServer()
	{
		//try amIntegrated and amMixed
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();

		return true;
	}
private:
	CSocketProService<CPPiPeer> m_CPPi;

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CPPi.AddMe(sidCPPi, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CPPi.AddSlowRequest(idComputeCPPi);
	}
};


#endif