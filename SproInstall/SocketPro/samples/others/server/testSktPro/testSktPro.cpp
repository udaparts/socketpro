// Wrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;

#include "sprowrap.h"

using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

#include "mysamplesvs.h"

int main(int argc, char* argv[])
{
	bool bUseSSL = false;
	bool ok;

	CSocketProServer		SocketProServer;
	CNotificationService	NotificationService;
	CMySampleService		MySampleService;
	
	ATLASSERT(g_pSocketProServer != NULL);	
	
	CSocketProServer::Config::SetAuthenticationMethod(amMixed);

	//register an independent chat service 
	//so that a client can use a dedicated socket connection to send messages
	ok = NotificationService.AddMe(sidChat);
	
	//register a service
	ok = MySampleService.AddMe(sidMySampleService);
	//setup slow requests
	ok = MySampleService.AddSlowRequest(idSleep);
	ok = MySampleService.AddSlowRequest(idDoSomeWorkWithMSSQLServer);
	
	
	//reuse high performance libraries written from C/C++
	HINSTANCE hInst = CBaseService::AddALibrary(L"ufilesvr.dll");
	hInst = CBaseService::AddALibrary(L"uodbsvr.dll");
	
	//set up discussion groups....
	ok = CSocketProServer::PushManager::AddAChatGroup(1, L"File Group");
	ok = CSocketProServer::PushManager::AddAChatGroup(2, L"DB Group");
	ok = CSocketProServer::PushManager::AddAChatGroup(4, L"My Group");
	
	if(bUseSSL)
	{
		//secure the SocketPro Server
		SocketProServer.UseSSL(MSTLSv1, L"udaparts", false, true);
	}

	//listen socket at 17000
	ok = SocketProServer.StartSocketProServer(17000);
	if(ok)
	{
		cout<<"SocketPro server started at port = 17000"<<endl;

		//start message pump if an application doesn't have one
		SocketProServer.StartMessagePump();
		
		//close all of socket connections and kill listening socket
		SocketProServer.StopSocketProServer();
	}
	else
	{
		cout<<"Can't start SocketPro server!"<<endl;
		cout<<"As usual, the problem could be one of the following three causes:"<<endl;
		cout<<"1 -- Two OpenSSL libraries can't been loaded if SSL are used."<<endl;
		cout<<"2 -- Certificate and priverte key files are not available or have problems if SSL are used."<<endl;
		cout<<"3 -- Port not available."<<endl;
		cout<<"4 -- A private key/certificate is not found in a certificate store if MS SSPI is used."<<endl;
	}

	return 0;
}
