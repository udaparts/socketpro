#ifndef ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_PIIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include <iostream>
using namespace std;

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;
using namespace SocketProAdapter::ServerSide::PLG;

#define sidCPPi			(odUserServiceIDMin + 1212)

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
		return AddService();
	}

private:
	CPLGService<sidCPPi>	m_LoadingBalance;
	bool AddService()
	{
		int n;
		CComBSTR bstrLocal(L"localhost");
		CComBSTR bstrYYEXP(L"127.0.0.1");
		CComBSTR bstrDesk(L"localhost");
		CComBSTR bstrLaptop(L"localhost");
		CComBSTR bstrSomeOne(L"127.0.0.1");
		
		CComBSTR bstrUserId(L"SocketPro");
		CComBSTR bstrPassword(L"PassOne");

		//set connection contexts
		CConnectionContext pConnectionContext[5];
		pConnectionContext[0].m_strHost = bstrLocal.m_str;
		pConnectionContext[1].m_strHost = bstrYYEXP.m_str;
		pConnectionContext[2].m_strHost = bstrLaptop.m_str;
		pConnectionContext[3].m_strHost = bstrDesk.m_str;
		pConnectionContext[4].m_strHost = bstrSomeOne.m_str;
		for(n=0; n<5; n++)
		{
			pConnectionContext[n].m_nPort = 20901;
			pConnectionContext[n].m_strPassword = bstrPassword.m_str;
			pConnectionContext[n].m_strUID = bstrUserId.m_str;
			pConnectionContext[n].m_EncrytionMethod = NoEncryption;
			pConnectionContext[n].m_bZip = false;
		}

		//start socket pool
		if(!m_LoadingBalance.GetSocketPool().StartSocketPool(pConnectionContext, 5, 2, 3))
			return false;

		return m_LoadingBalance.AddMe(sidCPPi, 0, taFree) && m_LoadingBalance.AddSlowRequest(idEndJob);
	}
};


#endif