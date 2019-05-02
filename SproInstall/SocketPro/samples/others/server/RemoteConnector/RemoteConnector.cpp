// RemoteConnector.cpp : Implementation of WinMain


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f RemoteConnectorps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "RemoteConnector.h"

#include "RemoteConnector_i.c"


#include <stdio.h>

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

// Although some of these functions are big they are declared inline since they are only used once

inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

    // Remove any previous service since it may point to
    // the incorrect file
    Uninstall();

    // Add service entries
    UpdateRegistryFromResource(IDR_RemoteConnector, TRUE);

    // Adjust the AppID for Local Server or Service
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{E877C7C6-6921-4901-A5DC-FF59D8CCEDF4}"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;
    key.DeleteValue(_T("LocalService"));
    
    if (bService)
    {
        key.SetValue(_T("RemoteConnector"), _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));
        // Create service
        Install();
    }

    // Add object entries
    hr = CComModule::RegisterServer(bRegTypeLib);

    CoUninitialize();
    return hr;
}

inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

    // Remove service entries
    UpdateRegistryFromResource(IDR_RemoteConnector, FALSE);
    // Remove service
    Uninstall();
    // Remove object entries
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();
    return S_OK;
}

inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid)
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));

    // set up the initial service status 
    m_hServiceStatus = NULL;

	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_PAUSE_CONTINUE;

    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
	{
 //       PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
		CSocketProServer::PostQuit(dwThreadID);
	}
    return l;
}

BOOL CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

inline BOOL CServiceModule::Install()
{
    if (IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

    // Get the executable file path
    TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    SC_HANDLE hService = ::CreateService(
        hSCM, m_szServiceName, m_szServiceName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS/*|SERVICE_INTERACTIVE_PROCESS*/,
        SERVICE_AUTO_START/*SERVICE_DEMAND_START*/, SERVICE_ERROR_NORMAL,
        szFilePath, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't create service"), m_szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

inline BOOL CServiceModule::Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), m_szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    MessageBox(NULL, _T("Service could not be deleted"), m_szServiceName, MB_OK);
    return FALSE;
}

void CServiceModule::LogEvent(DWORD dwSvsID, const TCHAR *strUserID, const TCHAR *strIPAddr, const TCHAR *strMsg)
{
	FILE *pfStream = NULL;
	pfStream = ::fopen("login.txt", "a+");
	if(pfStream == NULL)
		return;
	::_ftprintf(pfStream, _T("Service ID = %d, User ID = %s, IP Address = %s, Message = %s\n"), dwSvsID, strUserID, strIPAddr, strMsg);
	::fclose(pfStream);
}

///////////////////////////////////////////////////////////////////////////////////////
// Logging functions
void CServiceModule::LogEvent(WORD wType, DWORD dwEventID, LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;

    if (m_bService)
    {
        /* Get a handle to use with ReportEvent(). */
        hEventSource = RegisterEventSource(NULL, m_szServiceName);
        if (hEventSource != NULL)
        {
            /* Write to event log. */
            ReportEvent(hEventSource, wType, 0, dwEventID, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
            DeregisterEventSource(hEventSource);
        }
    }
    else
    {
        // As we are not running as a service, just write the error to the console.
        _putts(chMsg);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Service startup and registration
inline void CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain },
        { NULL, NULL }
    };
    if (m_bService && !::StartServiceCtrlDispatcher(st))
    {
        m_bService = FALSE;
    }
    if (m_bService == FALSE)
        Run();
}

inline void CServiceModule::ServiceMain(DWORD /* dwArgc */, LPTSTR* /* lpszArgv */)
{
    // Register the control request handler
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        LogEvent(EVENTLOG_ERROR_TYPE, 2, _T("Handler not installed"));
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

    // When the Run function returns, the service has stopped.
    Run();

    SetServiceStatus(SERVICE_STOPPED);
    LogEvent(EVENTLOG_INFORMATION_TYPE, 0, _T("Service stopped"));
}

CServiceModule::CServiceModule()
{
	memset(m_szFileName, 0, sizeof(m_szFileName));
	memset (m_szSSLCert, 0, sizeof(m_szSSLCert));
	memset (m_szSSLKey, 0, sizeof(m_szSSLKey));
	memset (m_szServiceName, 0, sizeof(m_szServiceName));
	m_nPort = DEFAULT_PORT;
	memset (m_szFilePath, 0, sizeof(m_szFilePath));
	memset(m_szAuthDB, 0, sizeof(m_szAuthDB));
	memset(m_szAuthDBExtended, 0, sizeof(m_szAuthDBExtended));
	m_dwMaxConnections = DEFAULT_MAXCONNECTIONS_PER_CLIENT;
	m_bEnableWindowlogon = true;
	m_bLocalAccountDatabaseOnly = true;
	m_bEnableMessage = true;
	m_bStop = false;

	memset(m_szSubject, 0, sizeof(m_szSubject));
	memset(m_szPfxFile, 0, sizeof(m_szPfxFile));
	memset(m_szPassword, 0, sizeof(m_szPassword));
}

inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus(SERVICE_STOP_PENDING);
 //       ::PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
		CSocketProServer::PostQuit(dwThreadID);
        break;
    case SERVICE_CONTROL_PAUSE:
		m_bStop = true;
		SetServiceStatus(SERVICE_PAUSED);
        break;
    case SERVICE_CONTROL_CONTINUE:
		m_bStop = false;
		SetServiceStatus(SERVICE_RUNNING);
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
		SetServiceStatus(SERVICE_STOP_PENDING);
//        ::PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
		CSocketProServer::PostQuit(dwThreadID);
        break;
    default:
        LogEvent(EVENTLOG_ERROR_TYPE, 2, _T("Bad service request"));
    }
}

void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

//    HRESULT hr = CoInitialize(NULL);
//  If you are running on NT 4.0 or higher you can use the following call
//  instead to make the EXE free threaded.
//  This means that calls come in on a random RPC thread
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    _ASSERTE(SUCCEEDED(hr));

    // This provides a NULL DACL which will allow access to everyone.
    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();
    hr = CoInitializeSecurity(sd, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    _ASSERTE(SUCCEEDED(hr));

    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
    _ASSERTE(SUCCEEDED(hr));

    LogEvent(EVENTLOG_INFORMATION_TYPE, 0, _T("Service started"));
    if (m_bService)
        SetServiceStatus(SERVICE_RUNNING);
	
	bool ok;
	CMyServer	MyServer; //create an instance of socketpro server
	
	do
	{
		CSocketProServer::Config::SetSharedAM(true);

		if(!StartAuthDB() && !m_bEnableWindowlogon)
		{
			LogEvent(0, _T(""), _T(""), _T("Can't start the authentication database, and can't authenticate a client through window logon either."));
			break;
		}
		if(m_Session.m_spOpenRowset != NULL && m_bEnableWindowlogon)
		{
			//ABOUT amMixing

			/*
				Server will use my own way to validate a user first.
				if it is failed, server will use the following SSPI method to validate
				a client.
			*/
			//sspi authentication (NTLM or Kerberos)
			//see the following article

			//How To Validate User Credentials on Microsoft Operating Systems
			//Article ID	:	180548
			//Last Review	:	March 21, 2005
			//Revision	:	3.3

			CSocketProServer::Config::SetAuthenticationMethod(amMixed);
		}
		else if(m_Session.m_spOpenRowset == NULL && m_bEnableWindowlogon)
		{
			//ABOUT amIntegrated

			//sspi authentication (NTLM or Kerberos)
			//see the following article

			//How To Validate User Credentials on Microsoft Operating Systems
			//Article ID	:	180548
			//Last Review	:	March 21, 2005
			//Revision	:	3.3
			CSocketProServer::Config::SetAuthenticationMethod(amIntegrated); 
		}
		else if(m_Session.m_spOpenRowset != NULL && !m_bEnableWindowlogon)
		{
			//ABOUT amOwn

			//use my own way to authenticate user credentials.
//			MyServer.SetAuthenticationMethod(amOwn); //default to amOwn
		}
		else
		{
			ATLASSERT(FALSE); //
		}

		if(m_bEnableMessage)
		{
			SetupChatService();
		}
		
		LoadModules();
		SetupSSL();
		
		//used for authentication later
		MyServer.AskForEvents(eOnIsPermitted + eOnClose);
		
		CSocketProServer::Config::SetMaxConnectionsPerClient(m_dwMaxConnections);

		//start SocketPro server at the port 17001
		ok = MyServer.StartSocketProServer(m_nPort);
		if(!ok)
		{
			TCHAR strMsg[256] = {0};
			long lErrorCode = ::GetLastError();
			::_stprintf(strMsg, _T("Error code = 0x%X (%d), Check if the port %d is available, and check whether two OpenSSL libraries, key file and certification file are available if you use OpenSSL."), lErrorCode, lErrorCode, m_nPort);
			LogEvent(0, _T(""), _T(""), strMsg);
			break;
		}
		MyServer.StartMessagePump();
		MyServer.StopSocketProServer();
	}while(false);
	SetServiceStatus(SERVICE_STOP_PENDING);
	if(m_Session.m_spOpenRowset.p)
		m_Session.Close();
    _Module.RevokeClassObjects();
    CoUninitialize();
}

void CServiceModule::SetupRegistries()
{
	DWORD	dwData = m_nPort;
	{
		TCHAR	szFilePath[_MAX_PATH + 1] = {0};
		TCHAR	szFileName[_MAX_PATH] = {0};
		::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
		TCHAR	*pstrName = ::_tcsrchr(szFilePath, _T('\\'));
		TCHAR	*pstrDot = ::_tcsrchr(pstrName, _T('.'));
		_tcsncpy(m_szFilePath, szFilePath, _tcslen(szFilePath) + 1 - _tcslen(pstrName));
		BOOL ok = ::SetCurrentDirectory(m_szFilePath);
		if(pstrDot != NULL)
		{
			::_tcsncpy(m_szFileName, pstrName+1, _tcslen(pstrName+1) - _tcslen(pstrDot));
			::_tcsncpy(m_szServiceName, pstrName+1, _tcslen(pstrName+1) - _tcslen(pstrDot));
		}
		else
		{
			_tcscpy(m_szFileName, pstrName);
		}
	}

	CRegKey	RegKey;
	if(RegKey.Open(HKEY_LOCAL_MACHINE, _T("Software")) != S_OK)
	{
		if(RegKey.Open(HKEY_CURRENT_USER, _T("Software")) != S_OK)
		{
			ATLASSERT(FALSE);
			return;
		}
	}

	if(RegKey.Open(RegKey.m_hKey, _T("UDAParts")) != S_OK)
		RegKey.Create(RegKey.m_hKey, _T("UDAParts"));

	if(RegKey.Open(RegKey.m_hKey, _T("SocketPro")) != S_OK)
		RegKey.Create(RegKey.m_hKey, _T("SocketPro"));
	
	if(RegKey.Open(RegKey.m_hKey, m_szServiceName) != S_OK)
		RegKey.Create(RegKey.m_hKey, m_szServiceName);

	if(RegKey.QueryValue(dwData, _T("UseSystemlogon")) == S_OK)
	{
		m_bEnableWindowlogon = (dwData > 0) ? true : false;
	}
	else
	{
		RegKey.SetValue(m_bEnableWindowlogon, _T("UseSystemlogon"));
	}

	if(RegKey.QueryValue(dwData, _T("UseLocalAccountDatabaseOnly")) == S_OK)
	{
		m_bLocalAccountDatabaseOnly = (dwData > 0) ? true : false;
	}
	else
	{
		RegKey.SetValue(m_bLocalAccountDatabaseOnly, _T("UseLocalAccountDatabaseOnly"));
	}

	if(RegKey.QueryValue(dwData, _T("EnableMessage")) == S_OK)
	{
		m_bEnableMessage = (dwData > 0) ? true : false;
	}
	else
	{
		RegKey.SetValue(m_bEnableMessage, _T("EnableMessage"));
	}

	if(RegKey.QueryValue(dwData, _T("Port")) == S_OK)
	{
		m_nPort = dwData;
	}
	else
	{
		RegKey.SetValue((DWORD)DEFAULT_PORT, _T("Port"));
		m_nPort = DEFAULT_PORT;
	}
	
	if(RegKey.QueryValue(dwData, _T("MaxConnections")) == S_OK)
	{
		if(dwData == 0)
			dwData = DEFAULT_MAXCONNECTIONS_PER_CLIENT;
		m_dwMaxConnections = dwData;
	}
	else
	{
		RegKey.SetValue((DWORD)DEFAULT_MAXCONNECTIONS_PER_CLIENT, _T("MaxConnections"));
		m_bService = DEFAULT_MAXCONNECTIONS_PER_CLIENT;
	}
	
	dwData = sizeof(m_szAuthDB)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szAuthDB, _T("AuthDB"), &dwData) == S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T("AuthDB.MDB"), _T("AuthDB"));
		_tcscpy(m_szAuthDB, _T("AuthDB.MDB"));
	}
	
	dwData = sizeof(m_szSSLCert)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szSSLCert, _T("SSLCert"), &dwData)==S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T(""), _T("SSLCert"));
	}
	dwData = sizeof(m_szSSLKey)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szSSLKey, _T("SSLKey"), &dwData)==S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T(""), _T("SSLKey"));
	}

	dwData = sizeof(m_szPfxFile)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szPfxFile, _T("PfxFile"), &dwData)==S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T(""), _T("PfxFile"));
	}

	dwData = sizeof(m_szPassword)/sizeof(TCHAR);
	RegKey.QueryValue(m_szPassword, _T("Password"), &dwData);
	
	RegKey.SetValue(_T(""), _T("Password"));
	

	dwData = sizeof(m_szSubject)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szSubject, _T("Subject"), &dwData)==S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T(""), _T("Subject"));
	}

	dwData = sizeof(m_szAuthDBExtended)/sizeof(TCHAR);
	if(RegKey.QueryValue(m_szAuthDBExtended, _T("AuthDBExtended"), &dwData)==S_OK)
	{
	}
	else
	{
		RegKey.SetValue(_T(""), _T("AuthDBExtended"));
	}

	if(RegKey.Open(RegKey.m_hKey, _T("PlugIns")) != S_OK)
	{
		RegKey.Create(RegKey.m_hKey, _T("PlugIns"));
		RegKey.Create(RegKey.m_hKey, _T("Module1"));
		RegKey.SetValue(_T("uodbsvr.dll"), _T("Location"));
		RegKey.SetValue((DWORD)0, _T("Param"));
	}
	else
	{
		CRegKey	rkPlug;
		ULONG ulIndex = 1; 
		CComBSTR	bstrKey;
		
		TCHAR szFilePath[_MAX_PATH + 1] = {0};
		DWORD		dwVal = 0;
		
		TCHAR szModule[64] = {0};
		::_stprintf(szModule, _T("Module%d"), ulIndex);
		while(rkPlug.Open(RegKey.m_hKey,szModule) == S_OK)
		{
			dwData = sizeof(szFilePath)/sizeof(TCHAR);
			if(rkPlug.QueryValue(dwVal, _T("Param")) == S_OK && rkPlug.QueryValue(szFilePath, _T("Location"), &dwData)==S_OK)
			{
				bstrKey = szFilePath;
				m_mapPlugIns.Add(bstrKey, dwVal);
				memset(szFilePath, 0, sizeof(szFilePath));
			}
			ulIndex++;
			::_stprintf(szModule, _T("Module%d"), ulIndex);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT
    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, &LIBID_REMOTECONNECTORLib);
    _Module.m_bService = TRUE;
	
	_Module.SetupRegistries();	

	TCHAR szTokens[] = _T("-/");
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0 || lstrcmpi(lpszToken, _T("Uninstall"))==0)
            return _Module.UnregisterServer();

        // Register as Local Server
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
            return _Module.RegisterServer(TRUE, FALSE);
        
        // Register as Service
        if (lstrcmpi(lpszToken, _T("Service"))==0 || lstrcmpi(lpszToken, _T("Install"))==0 || lstrcmpi(lpszToken, _T("Run"))==0)
		{
            _Module.RegisterServer(TRUE, TRUE);
			return _Module.StartService() ? 0 : 1;
		}
        
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    // Are we Service or Local Server
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{E877C7C6-6921-4901-A5DC-FF59D8CCEDF4}"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
        _Module.m_bService = TRUE;

    _Module.Start();

    // When we get here, the service has been stopped
    return _Module.m_status.dwWin32ExitCode;
}

bool CServiceModule::StartService()
{
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		TCHAR pTemp[128] = {0};
		_stprintf(pTemp, _T("OpenSCManager failed, error code = %d\n"), nError);
		LogEvent(0, NULL, NULL, pTemp);
	}
	else
	{
		SC_HANDLE schService = OpenService(schSCManager, m_szServiceName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			long nError = GetLastError();
			TCHAR pTemp[128] = {0};
			_stprintf(pTemp, _T("OpenService failed, error code = %d\n"), nError);
			LogEvent(0, NULL, NULL, pTemp);
		}
		else
		{
			if(::StartService(schService, 0, NULL))
			{
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager); 
				return true;
			}
			else
			{
				long nError = GetLastError();
				TCHAR pTemp[128];
				_stprintf(pTemp, _T("StartService failed, error code = %d\n"), nError);
				LogEvent(0, NULL, NULL, pTemp);
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return false;
}