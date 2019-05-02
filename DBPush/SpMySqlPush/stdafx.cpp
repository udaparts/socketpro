// stdafx.cpp : source file that includes just the standard includes
// SpMySqlPush.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "MySocket.h"
#include "MyConnectionContext.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

CClientSocket					*g_pClientSocket = NULL;
HANDLE							g_hThread = NULL;
DWORD							g_dwThreadId = 0;
extern CMyConnectionContext		g_cc;
HANDLE							g_hEvent = NULL;
CSimpleMap<CComBSTR, CComBSTR>	g_dic;

CComAutoCriticalSection			g_csMySQLPush;


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	MSG msg;
	::CoInitializeEx(NULL,  COINIT_MULTITHREADED);
	{
		CMySocket	cs;
		if(cs.GetIUSocket() != NULL)
		{
			g_pClientSocket = &cs;
			::SetEvent(g_hEvent);
			while(::GetMessage(&msg, NULL, 0, 0))
			{
				switch(msg.message)
				{
				case WM_MYSQL_PUSH_CONNECT:
					{
						USES_CONVERSION;
						g_cc.m_VerifyCode = 0;
						cs.GetIUSocket()->put_ConnTimeout(60000);
						cs.GetIUSocket()->put_EncryptionMethod(g_cc.m_EncrytionMethod);
						cs.Connect(OLE2T(g_cc.m_strHost), g_cc.m_nPort);
            cs.GetIUSocket()->put_RecvTimeout(1000);
					}
					break;
				default:
          if(msg.message < 1721 && cs.IsConnected())
          {
            cs.GetIUSocket()->DoEcho();
          }
					break;
				}
				::DispatchMessage(&msg);
			}
		}
		else
			::SetEvent(g_hEvent);
		g_pClientSocket = NULL;
	}
	//::CoUninitialize(); //can't call this here for bad exception
	return 0;
}

bool bStop = true;

void StartPush()
{
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_hEvent == NULL)
		g_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if(g_hThread == NULL)
	{
		g_hThread = ::CreateThread(NULL, 0, ThreadProc, NULL, 0, &g_dwThreadId);
		::WaitForSingleObject(g_hEvent, 500);
	}
	else if(::WaitForSingleObject(g_hThread, 0) == WAIT_OBJECT_0)
	{
		::CloseHandle(g_hThread);
		g_hThread = ::CreateThread(NULL, 0, ThreadProc, NULL, 0, &g_dwThreadId);
		::WaitForSingleObject(g_hEvent, 500);
	}
	bStop = false;
}

void Stop()
{
	{
		CAutoLock al(&g_csMySQLPush.m_sec);
		if(g_pClientSocket != NULL)
			g_pClientSocket->Disconnect();
		if(bStop)
			return;
		bStop = true;
		g_pClientSocket = NULL;
	}
	if(g_hThread != NULL)
	{
		int nIndex = 0;
		while(!::PostThreadMessage(g_dwThreadId, WM_QUIT, 0, 0) && nIndex < 6)
		{
			::Sleep(1);
			nIndex++;
		}
		::WaitForSingleObject(g_hThread, 500);
		::CloseHandle(g_hThread);
		SocketProAdapter::CScopeUQueue::DestroyUQueuePool();
	}
	g_hThread = NULL;
	g_dwThreadId = 0;
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_hEvent != NULL)
	{
		::CloseHandle(g_hEvent);
		g_hEvent = NULL;
	}
	
}

