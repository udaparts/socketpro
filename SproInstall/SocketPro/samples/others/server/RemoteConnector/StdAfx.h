// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__96C91DBA_2566_4419_BD76_89D843E491C2__INCLUDED_)
#define AFX_STDAFX_H__96C91DBA_2566_4419_BD76_89D843E491C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0403
#endif
#define _ATL_FREE_THREADED

#include "rcdefaults.h"
#include <stdio.h>
#include <atlbase.h>
#include <ATLDBCLI.H>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module

class CServiceModule : public CComModule
{
public:
	CServiceModule();

public:
	HRESULT RegisterServer(BOOL bRegTypeLib, BOOL bService);
	HRESULT UnregisterServer();
	void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid = NULL);
    void Start();
	void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall();
	LONG Unlock();
	void LogEvent(WORD wType, DWORD dwEventID, LPCTSTR pFormat, ...);
	void LogEvent(DWORD dwSvsID,  const TCHAR *strUserID, const TCHAR *strIPAddr, const TCHAR *strMsg);
    void SetServiceStatus(DWORD dwState);
    void SetupAsLocalServer();
	bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID);
	void OnClose(unsigned int hSocket, int nErrorCode);
	void SetupRegistries();
	bool StartService();

public:
	BOOL m_bService;
	SERVICE_STATUS m_status;

// data members
private:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);
	bool DoAuthentication(LPWSTR strUID, LPWSTR strPassword, unsigned int hSocket, unsigned long ulSvsID);
    bool StartAuthDB();
	void SetupSSL();
	void SetupChatService();
	void LoadModules();

private:
	TCHAR m_szServiceName[_MAX_PATH];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
	DWORD dwThreadID;
	TCHAR m_szSSLCert[_MAX_PATH];
	TCHAR m_szSSLKey[_MAX_PATH];
	TCHAR m_szSubject[_MAX_PATH];
	TCHAR m_szPfxFile[_MAX_PATH];
	TCHAR m_szPassword[_MAX_PATH];

	unsigned short	m_nPort;
	TCHAR m_szFilePath[_MAX_PATH];
	TCHAR m_szAuthDB[_MAX_PATH];
	TCHAR m_szAuthDBExtended[_MAX_PATH];
	DWORD m_dwMaxConnections;
	CSimpleMap<CComBSTR, DWORD> m_mapPlugIns;
	CSimpleArray<unsigned long> m_aSvsIDs;
	CSession	m_Session;
	bool		m_bEnableWindowlogon;
	bool		m_bLocalAccountDatabaseOnly;
	bool		m_bEnableMessage;
	TCHAR		m_szFileName[_MAX_PATH];
	bool		m_bStop;
};

extern CServiceModule _Module;
#include <atlcom.h>

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

class CMyServer : public CSocketProServer
{
protected:
	virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	{
		return _Module.OnIsPermitted(hSocket, ulSvsID);
	}

	virtual void OnClose(unsigned int hSocket, int nErrorCode)
	{
		_Module.OnClose(hSocket, nErrorCode);
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__96C91DBA_2566_4419_BD76_89D843E491C2__INCLUDED)
