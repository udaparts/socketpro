#ifndef ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */

#include <iostream>
using namespace std;

#include "podefines.h"


//server implementation for service CTOne
class CTOnePeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		m_uCount = 0; //initialize the data member to 0
		cout <<"Socket is switched for the service TOne"<<endl;
	}

	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
			cout <<"Socket is going to be closed with error code = "<< ulInfo << endl;
		}
		else
		{
			//switch to a new service with the service id = ulInfo
		}

		//release all of your resources here as early as possible
	}

	void QueryCount(/*out*/int &QueryCountRtn)
	{
		QueryCountRtn = m_uCount;
	}

	void QueryGlobalCount(/*out*/int &QueryGlobalCountRtn)
	{
		CAutoLock Lock(&m_cs.m_sec);
		QueryGlobalCountRtn = m_uGlobalCount;
	}

	void QueryGlobalFastCount(/*out*/int &QueryGlobalFastCountRtn)
	{
		QueryGlobalFastCountRtn = m_uGlobalFastCount;
	}

	void Sleep(int nTime)
	{
		if(nTime < 200 && TransferServerException())
		{
			throw CSocketProServerException(E_UNEXPECTED, L"Sleep time is too short!");
		}
		::Sleep(nTime);
	}

	void Echo(const CComVariant &objInput, /*out*/CComVariant &EchoRtn)
	{
		EchoRtn = objInput;
	}

	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		m_uGlobalFastCount++;
		m_uCount++;
		{
			CAutoLock Lock(&m_cs.m_sec);
			m_uGlobalCount++;
		}
		BEGIN_SWITCH(usRequestID)
			//Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
			M_I0_R1(idQueryCountCTOne, QueryCount, int) 
			
			M_I0_R1(idQueryGlobalCountCTOne, QueryGlobalCount, int)
			M_I0_R1(idQueryGlobalFastCountCTOne, QueryGlobalFastCount, int)
			M_I1_R1(idEchoCTOne, Echo, CComVariant, CComVariant)
		END_SWITCH
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		m_uCount++;
		{
			CAutoLock Lock(&m_cs.m_sec);
			m_uGlobalCount++;
		}
		BEGIN_SWITCH(usRequestID)
			//Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
			M_I1_R0(idSleepCTOne, Sleep, int)
		END_SWITCH
		return S_OK;
	}

private:
	//m_GlobalCount must be synchronized 
	//because it is accessed within different threads from different socket clients
	static CComAutoCriticalSection	m_cs;
	static unsigned long m_uGlobalCount;

	//m_uGlobalFastCount doesn't need to be synchronized 
	//because it is accessed within main thread, 
	//although is is accessed from different socket clients
	static unsigned long m_uGlobalFastCount;

	//m_uCount doesn't need to be synchronized 
	//because it is always accessed from one socket client only, 
	//even though it may be accessed within different threads.
	unsigned long m_uCount;
};

class CMySocketProServer : public CSocketProServer
{
protected:
	virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	{
		cout<<"A socket connection is permitted for srvice id = "<<ulSvsID << endl;
		return true;
	}

	virtual void OnAccept(unsigned int hSocket, int nError)
	{
		cout<<"A socket connection is initially established" << endl;
	}

	virtual void OnClose(unsigned int hSocket, int nError)
	{
		cout<<"A socket connection is closed with error code = " << nError << endl;
	}

	virtual bool OnSettingServer()
	{
		//try amIntegrated and amMixed
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();
		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<CTOnePeer> m_CTOne;

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CTOne.AddMe(sidCTOne, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CTOne.AddSlowRequest(idSleepCTOne);
	}
};


#endif