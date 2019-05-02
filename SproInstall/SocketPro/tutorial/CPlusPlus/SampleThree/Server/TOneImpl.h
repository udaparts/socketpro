#ifndef ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */

#include <iostream>
using namespace std;

#include "TOne_i.h"
#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

#include "TThreeImpl.h"
#include "..\shared.h"

//server implementation for service CTOne
class CTOnePeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		m_uCount = 0; //initialize the data member to 0
		cout <<"Socket is switched for the service TOne"<<endl;

		unsigned long pGroup[5] = {1, 1, 32, 2, 22};
		bool ok = GetPush()->Enter(pGroup, 5);
	}

	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
			cout<< "Socket " << GetSocket() << " closed with error code = " << ulInfo << "." << endl;
		}
		else
		{
			cout << "Switched to the service ID = " << ulInfo << "." << endl;
		}

		//release all of your resources here as early as possible
	}

	virtual void OnDispatchingSlowRequest(unsigned short usRequestID);

	void QueryCount(/*out*/int &QueryCountRtn)
	{
		QueryCountRtn = m_uCount;
	}

	void QueryGlobalCount(/*out*/int &QueryGlobalCountRtn)
	{
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

		CComVariant vtMessage(L"Sleep called");

		unsigned long Groups[2] = {1, 2};

		//inform all of joined clients that idSleep is called
		GetPush()->Broadcast(vtMessage, Groups, 2);
	}

	void Echo(const CComVariant &objInput, /*out*/CComVariant &EchoRtn)
	{
		EchoRtn = objInput;
	}

	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		m_uGlobalFastCount++;
		m_uCount++;
		m_uGlobalCount++;
	
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
		BEGIN_SWITCH(usRequestID)
			//Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
			M_I1_R0(idSleepCTOne, Sleep, int)
		END_SWITCH
		return S_OK;
	}

private:
	//m_GlobalCount doesn't need to be synchronized 
	//because it is accessed only within main thread
	//although is is accessed from different socket clients
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
		USES_CONVERSION;
		WCHAR strUID[256] = {0};
		WCHAR strPassword[256] = {0};
		GetUserID(hSocket, strUID, 256);
		
		//password is available ONLY IF authentication method to either amOwn or amMixed
		GetPassword(hSocket, strPassword, 256);
		
		cout<<"For service = " << ulSvsID << ", User ID = " << OLE2A(strUID) << ", Password = " << OLE2A(strPassword)<<endl;
		
		tagAuthenticationMethod am = CSocketProServer::Config::GetAuthenticationMethod();

		if(am == amOwn || am == amMixed)
		{
			return true; //give permission to all

			//do my own authentication
//			return IsAllowed(strUID, strPassword);
		}

		return true;
	}

	virtual void OnAccept(unsigned int hSocket, int nError)
	{
		cout<<"Socket accepted, and its handle = " << hSocket << ", Clients = " << CSocketProServer::GetCountOfClients() + ", Error code = " << nError<<endl;
	}

	virtual void OnClose(unsigned int hSocket, int nError)
	{
		cout<<"Socket connection closed, and its handle = " << hSocket <<". Error code = "<< nError <<"." <<endl;
	}

	virtual bool OnSettingServer()
	{
		CSocketProServer::Config::SetSharedAM(true);

		//amMixed, try amOwn and amIntegrated
		CSocketProServer::Config::SetAuthenticationMethod(amMixed);
		
		//limit the max number of connections to 2 for a client machine
		CSocketProServer::Config::SetMaxConnectionsPerClient(2);

		ReuseLibraries();

		//add service(s) into SocketPro server
		AddService();

		SetBuiltinChatService();

		//use MSTLSv1 to secure all of data communication between a client and a SocketPro server
		//udacert.pfx contains both key and certificate, which is distributed in the ..\bin as sample certificate and key
		//UseSSL(L"C:\\Program Files\\UDAParts\\SocketPro\\bin\\udacert.pfx", L"mypassword", L"udaparts", MSTLSv1); 

		return true; //true -- ok; and false -- no listening socket
	}

private:
	CSocketProService<CTOnePeer>	m_CTOne;
	CTThreeSvs						m_CTThree;

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CTOne.AddMe(sidCTOne, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CTOne.AddSlowRequest(idSleepCTOne);

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CTThree.AddMe(sidCTThree, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CTThree.AddSlowRequest(idGetManyItemsCTThree);
		ok = m_CTThree.AddSlowRequest(idSendBatchItemsCTThree);
	}

	bool IsAllowed(LPCWSTR strUserID, LPCWSTR strPassword)
	{
		if(::wcscmp(strPassword, L"PassOne") != 0)
			return false;
		bool bAllowed = (::_wcsicmp(strUserID, L"socketpro") == 0);

		return bAllowed;
	}

	void ReuseLibraries()
	{
		//those libraries are distributed in the directory ..\bin

		HINSTANCE hInst = CBaseService::AddALibrary(L"uodbsvr.dll");
		if(hInst == UNULL_PTR)
		{
			cout<<"library "<<"uodbsvr.dll"<<" not available.\n";
		}

		hInst = CBaseService::AddALibrary(L"ufilesvr.dll");
		if(hInst == UNULL_PTR)
		{
			cout<<"library "<<"ufilesvr.dll"<<" not available.\n";
		}

		hInst = CBaseService::AddALibrary(L"udemo.dll");
		if(hInst == UNULL_PTR)
		{
			cout<<"library "<<"udemo.dll"<<" not available.\n";
		}
	}

	void SetBuiltinChatService()
	{
		bool ok = CSocketProServer::PushManager::AddAChatGroup(1, L"Group for SOne");
		ok = CSocketProServer::PushManager::AddAChatGroup(2, L"Group for Three");
		ok = CSocketProServer::PushManager::AddAChatGroup(5, L"Group for DB Management");
		ATLASSERT(ok);
	}

};


#endif