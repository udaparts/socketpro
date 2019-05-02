// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#include "services.h"
#include "users.h"

void CServiceModule::LoadModules()
{
	ULONG ulIndex;
	HINSTANCE	hInstance;
	ULONG ulSize = m_mapPlugIns.GetSize();
	for(ulIndex=0; ulIndex<ulSize; ulIndex++)
	{
		hInstance = AddADll(m_mapPlugIns.GetKeyAt(ulIndex), m_mapPlugIns.GetValueAt(ulIndex));
	}
}

void CServiceModule::SetupChatService()
{
	DWORD dwGroupID;
	ULONG ul;
	CSvsContext	SvsContext;
	memset(&SvsContext, 0, sizeof(SvsContext));
	SvsContext.m_enumTA = taNone;
	bool ok = AddSvsContext(sidChat, SvsContext); //add chat service
	
	WCHAR	str[64] = {0};
	for(ul=0; ul<32; ul++)
	{
		dwGroupID = 1;
		dwGroupID = (dwGroupID<<ul);
		::swprintf(str, L"Group %d", ul+1);
		ok = AddAChatGroup(dwGroupID, str);
	}

	SetGroupsNotifiedWhenEntering(0xFFFFFFFF); //all groups
	SetGroupsNotifiedWhenExiting(0xFFFFFFFF);	//all groups
}

void CServiceModule::SetupSSL()
{
	if(::_tcslen(m_szPfxFile) > 0)
	{
		USES_CONVERSION;
		if(::_tcslen(m_szSubject) > 0)
		{
			UseMSCert(false, true, T2OLE(m_szSubject));
			SetDefaultEncryptionMethod(MSTLSv1);
		}
		else
		{
			SetDefaultEncryptionMethod(TLSv1);
		}
		SetPfxFile(T2OLE(m_szPfxFile), T2OLE(m_szPassword));
		memset(m_szPassword, 0, sizeof(m_szPassword));
	}
	else if(::_tcslen(m_szSubject) > 0)
	{
		USES_CONVERSION;
		UseMSCert(true, true, T2OLE(m_szSubject));
		SetDefaultEncryptionMethod(MSTLSv1);
	}
	else if(::_tcslen(m_szSSLCert) && ::_tcslen(m_szSSLKey)) //if both cert and key available
	{
		USES_CONVERSION;
		SetDefaultEncryptionMethod(TLSv1);
		SetCertFile(T2OLE(m_szSSLCert));	
		SetPrivateKeyFile(T2OLE(m_szSSLKey));
	}
}

bool CServiceModule::DoAuthentication(LPWSTR strUID, LPWSTR strPassword, unsigned int hSocket, unsigned long ulSvsID)
{
	
	if(strUID == NULL || strPassword == NULL || ::wcslen(strUID) == 0 || ::wcslen(strPassword) == 0)
		return false;

	//we may implement winnt integration authentication in the future
	USES_CONVERSION;
	CComBSTR	bstrSQL;
	CCommand<CAccessor<CUsersAccessor> >	Rowset;
	TCHAR	strSQL[512] = {0};
	::_stprintf(strSQL, _T("Select Password from Users, Granted Where Granted.UserID = Users.UserID and SvsID = %d and Users.UserID = "), ulSvsID);
	bstrSQL = strSQL;
	bstrSQL += L"'";
	bstrSQL += strUID;
	bstrSQL += L"'";

	HRESULT hr = Rowset.Open(m_Session, OLE2T(bstrSQL));
	if(hr != S_OK)
		return false;
	Rowset.ClearRecord();
	hr = Rowset.MoveFirst();
	if(::wcscmp(strPassword, Rowset.m_Password) == 0)
	{
		return true;
	}
	return false;
}

bool CServiceModule::StartAuthDB()
{
	USES_CONVERSION;
	HRESULT hr;
	m_aSvsIDs.RemoveAll();
	CDataSource db;
	m_Session.Close();
	CDBPropSet	dbinit(DBPROPSET_DBINIT);
	dbinit.AddProperty(DBPROP_INIT_DATASOURCE, T2OLE(m_szAuthDB));
	if(::_tcslen(m_szAuthDBExtended))
	{
		dbinit.AddProperty(DBPROP_INIT_PROVIDERSTRING, T2OLE(m_szAuthDBExtended));
	}
	hr = db.Open(_T("Microsoft.Jet.OLEDB.4.0"), &dbinit);
	if (FAILED(hr))
		return false;
	hr = m_Session.Open(db);
	if (FAILED(hr))
		return false;
	return true;
}

void CServiceModule::OnClose(unsigned int hSocket, int nErrorCode)
{
	if(nErrorCode != 0)
	{
		USES_CONVERSION;
		TCHAR strMsg[255] = {0};
		WCHAR strUID[256] = {0};
		WCHAR strIPAddr[33] = {0};
		ULONG ulSvsID = GetSvsID(hSocket);
		SYSTEMTIME	SysTime;
		::GetLocalTime(&SysTime);
		unsigned long ulGet = GetUID(hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));
		GetPeerName(hSocket, NULL, strIPAddr, 32);
		::_stprintf(strMsg, _T("Socket closed with error code = %d at %.4d:%.2d:%.2d %.2d:%.2d:%.2d"), nErrorCode, SysTime.wYear,
			SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
		_Module.LogEvent(ulSvsID, OLE2T(strUID), OLE2T(strIPAddr), strMsg);
	}
}

bool CServiceModule::OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
{
	tagAuthenticationMethod am = CSocketProServer::Config::GetAuthenticationMethod();
	if(am != amIntegrated)
	{
		USES_CONVERSION;
		if(ulSvsID == sidStartup)
			return false;
		TCHAR strMsg[255] = {0};
		WCHAR strUID[256] = {0};
		WCHAR strPassword[256] = {0};
		WCHAR strIPAddr[33] = {0};
		SYSTEMTIME	SysTime;
		::GetLocalTime(&SysTime);
		unsigned long ulGet = GetUID(hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));
		ulGet = GetPassword(hSocket, strPassword, sizeof(strPassword)/sizeof(WCHAR));
		GetPeerName(hSocket, NULL, strIPAddr, 32);
		if(!DoAuthentication(strUID, strPassword, hSocket, ulSvsID))
		{
			::_stprintf(strMsg, _T("login denied from my own authentication at %.4d:%.2d:%.2d %.2d:%.2d:%.2d"), SysTime.wYear,
				SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			LogEvent(ulSvsID, OLE2T(strUID), OLE2T(strIPAddr), strMsg);
			return false;
		}
		else
		{
			::_stprintf(strMsg, _T("login allowed at %.4d:%.2d:%.2d %.2d:%.2d:%.2d"), SysTime.wYear,
				SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			LogEvent(ulSvsID, OLE2T(strUID), OLE2T(strIPAddr), strMsg);
		}
	}
	return true;
}

