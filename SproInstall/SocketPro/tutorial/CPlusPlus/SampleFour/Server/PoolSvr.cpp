// PoolSvr.cpp: implementation of the CPoolSvr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketPool.h"
#include "PoolSvr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPoolSvr::CPoolSvr() 
	: m_bRunning(false)
{
	m_nPort = 0;
	m_bThreadCount = 0;
    m_bSocketsPerThread = 0;
	m_hDlg = UNULL_PTR;
}

CPoolSvr::~CPoolSvr()
{
	Stop();
}

void CPoolSvr::OnAccept(unsigned int hSocket, int nError)
{
	if(m_hDlg != UNULL_PTR)
	{
		::PostMessage(m_hDlg, WM_MY_EVENT_ONACCEPT, hSocket, nError);
	}
}

void CPoolSvr::OnClose(unsigned int hSocket, int nError)
{
	if(m_hDlg != UNULL_PTR)
	{
		::PostMessage(m_hDlg, WM_MY_EVENT_ONCLOSE, hSocket, nError);
	}
}

bool CPoolSvr::IsAllowed(LPCWSTR strUserID, LPCWSTR strPassword)
{
	ATLASSERT(strUserID && strPassword);
	if(::wcscmp(strPassword, L"PassOne") != 0)
		return false;
	return (::_wcsicmp(strUserID, L"socketpro") == 0);
}

bool CPoolSvr::OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
{
	if(m_hDlg != UNULL_PTR)
	{
		::PostMessage(m_hDlg, WM_MY_EVENT_ONISPERMITTED, hSocket, ulSvsID);
	}
	
	WCHAR	strUID[256] = {0};
	WCHAR	strPassword[256] = {0};

	GetUserID(hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));

	//password is available ONLY IF authentication method to either amOwn or amMixed
	GetPassword(hSocket, strPassword, sizeof(strPassword)/sizeof(WCHAR));

	return IsAllowed(strUID, strPassword);
}

bool CPoolSvr::OnSettingServer()
{
	//turn off online compressing at the server side so that
	//SocketPro server will not compress return results unless a client turns on it 
	Config::SetDefaultZip(false);

	//amMixed
	Config::SetAuthenticationMethod(amMixed);

	//set a chat group
	PushManager::AddAChatGroup(1, L"Loading Balance Job Queue Size");

	return AddService();
}


bool CPoolSvr::Run(int nPort, BYTE bThreadCount, BYTE bSocketsPerThread)
{
	Stop();
	m_nPort = nPort;
	m_bThreadCount = bThreadCount;
	m_bSocketsPerThread = bSocketsPerThread;
	m_bRunning = CSocketProServer::Run(nPort);
	return m_bRunning;
}

void CPoolSvr::Stop()
{
	if(m_bRunning)
	{
		CSocketProServer::StopSocketProServer();
		m_bRunning = false;
	}
}

bool CPoolSvr::AddService()
{
	int n;
	CComBSTR bstrLocal(L"localhost");
	CComBSTR bstrDesk(L"localhost");
	CComBSTR bstrLaptop(L"localhost");
	CComBSTR bstrYYEXP(L"charliedev");
	CComBSTR bstrSomeOne(L"charliedev");
	
	CComBSTR bstrUserId(L"SocketPro");
	CComBSTR bstrPassword(L"PassOne");

	//set connection contexts
	CConnectionContext pConnectionContext[5];
	pConnectionContext[0].m_strHost = bstrLocal.m_str;
	pConnectionContext[1].m_strHost = bstrDesk.m_str;
	pConnectionContext[2].m_strHost = bstrLaptop.m_str;
	pConnectionContext[3].m_strHost = bstrYYEXP.m_str;
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
	if(!m_RadoPoolSvs.GetSocketPool().StartSocketPool(pConnectionContext, 5, m_bSocketsPerThread, m_bThreadCount))
		return false;

	if(!m_RadoPoolSvs.AddMe(sidCRAdo, 0, taNone))
		return false;
	return m_RadoPoolSvs.AddSlowRequest(idEndJob); 
}