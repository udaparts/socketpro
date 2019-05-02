// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com

#include <atlbase.h>
#include "usocket.h"
#include "ufile.h"
#include "sockutil.h"
#include "WinFileSvs.h"
#include "ClientPeerFile.h"


unsigned short	g_SlowRequestIDs[6]; //used only with main thread. No need to synchronize g_SlowRequestIDs

CWinFileSvs	*g_pWinFileSvs = NULL;

void WINAPI UninitServerLibrary();

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)  
{
    switch(fdwReason) 
    { 
    case DLL_PROCESS_ATTACH:
		{
			ATLTRACE("DLL_PROCESS_ATTACH called, ufilesvr.dll\n");
		}
        break;
    case DLL_THREAD_ATTACH:
		{
			ATLTRACE("DLL_THREAD_ATTACH called, ufilesvr.dll\n");
		}
        break;
    case DLL_THREAD_DETACH:
		{
			ATLTRACE("DLL_THREAD_DETACH called, ufilesvr.dll\n");
		}
        break;
    case DLL_PROCESS_DETACH:
		{
			UninitServerLibrary();
			ATLTRACE("DLL_PROCESS_DETACH called, ufilesvr.dll\n");
		}
        break;
	default:
		ATLASSERT(FALSE);
		break;
    }
    return TRUE;  
}

bool WINAPI InitServerLibrary(int nParam)
{
	g_pWinFileSvs = new CWinFileSvs();
	g_SlowRequestIDs[0] = idGetFile;
	g_SlowRequestIDs[1] = idSendBytesToServer;
	g_SlowRequestIDs[2] = idCopyFile;
	g_SlowRequestIDs[3] = idFindAll;
	g_SlowRequestIDs[4] = idWriteFile;
	g_SlowRequestIDs[5] = idReadFile;

	//don't forget to set a service id
	g_pWinFileSvs->SetSvsID(sidWinFile);
	return true;
}

void WINAPI UninitServerLibrary()
{
	if(g_pWinFileSvs)
	{
		g_pWinFileSvs->RemoveMe();
		delete g_pWinFileSvs;
		g_pWinFileSvs = NULL;
	}
}

unsigned long g_ulMaxMessageSize = (~0);
void WINAPI SetMaxRequestSize(unsigned long ulMaxSize)
{
	if(ulMaxSize < 10240)
		ulMaxSize = 10240;
	g_ulMaxMessageSize = ulMaxSize;
}

unsigned short WINAPI GetNumOfServices()
{
	return 1;
}

unsigned long WINAPI GetAServiceID(unsigned short usIndex)
{
	if(usIndex >= GetNumOfServices())
		return 0;
	return sidWinFile;
}

CSvsContext WINAPI GetOneSvsContext(unsigned long ulSvsID)
{
	if(ulSvsID == sidWinFile)
		return g_pWinFileSvs->m_SvsContext;
	CSvsContext	SvsContext;
	memset(&SvsContext, 0, sizeof(SvsContext));
	return SvsContext;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned long ulSvsID)
{
	if(ulSvsID != sidWinFile)
		return 0;
	return sizeof(g_SlowRequestIDs)/sizeof(unsigned short);
}

unsigned short WINAPI GetOneSlowRequestID(unsigned long ulSvsID, unsigned long ulIndex)
{
	if(ulIndex >= GetNumOfSlowRequests(ulSvsID))
		return 0;
	return g_SlowRequestIDs[ulIndex];
}

void WINAPI SetWinFileReadOnly(unsigned int hSocket, bool bReadOnly)
{
	CAutoLock AutoLock(&g_cs.m_sec);
	CClientPeerFile *pClientPeerFile = (CClientPeerFile*)CBaseService::SeekClientPeerGlobally(hSocket);
	if(pClientPeerFile)
		pClientPeerFile->m_bReadOnly = bReadOnly;
}

bool WINAPI SetRootDirectory(unsigned int hSocket, const wchar_t *strRootDir)
{
	CAutoLock AutoLock(&g_cs.m_sec);
	CClientPeerFile *pClientPeerFile = (CClientPeerFile*)CBaseService::SeekClientPeerGlobally(hSocket);
	if(pClientPeerFile)
	{
		pClientPeerFile->m_strRoot = strRootDir;
		return true;
	}
	return false;
}

DWORD GetErrorMsg(unsigned long ulErrorCode, TCHAR *strError, unsigned long ulChars)
{
	return ::FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				ulErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				strError,
				ulChars,
				NULL 
			);
}

// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com

