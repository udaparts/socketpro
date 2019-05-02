#ifndef ___SOCKETPRO_SERVICES_IMPL_PERFIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_PERFIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "..\Perf_i.h"

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

//server implementation for service CPerf
class CPerfPeer : public CClientPeer
{
protected:
	//virtual void OnSwitchFrom(unsigned long ulServiceID)
	//{
	//	//initialize the object here
	//}

	//virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	//{
	//	if(bClosing)
	//	{
	//		//closing the socket with error code = ulInfo
	//	}
	//	else
	//	{
	//		//switch to a new service with the service id = ulInfo
	//	}

	//	//release all of your resources here as early as possible
	//}

	void MyEcho(const CComBSTR &strInput, /*out*/CComBSTR &MyEchoRtn)
	{
		MyEchoRtn = strInput;
	}

	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch(usRequestID)
		{
		case idMyEchoCPerf:
		{
			CComBSTR strInput;
			CComBSTR MyEchoRtn;
			m_UQueue >> strInput;
			MyEcho(strInput, MyEchoRtn);
			SendResult(usRequestID, MyEchoRtn);
		}
			break;
		default:
			break;
		}
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		return S_OK;
	}

};

class CMySocketProServer : public CSocketProServer
{
protected:
	//virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	//{
	//	//give permission to all
	//	return true;
	//}

	//virtual void OnAccept(unsigned int hSocket, int nError)
	//{
	//	//when a socket is initially established
	//}

	//virtual void OnClose(unsigned int hSocket, int nError)
	//{
	//	//when a socket is closed
	//}

	virtual bool OnSettingServer()
	{
		//add service(s) into SocketPro server
		AddService();
		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<CPerfPeer> m_CPerf;
	//One SocketPro server supports any number of services. You can list them here!

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CPerf.AddMe(sidCPerf, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!


		//Add all of other services into SocketPro server here!
	}
};


#endif