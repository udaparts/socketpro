// SProAdapter.h

#pragma once

#ifndef __SOCKETPRO_CLIENT_EVENT_WRAPPER_H__
#define __SOCKETPRO_CLIENT_EVENT_WRAPPER_H__

#include <atlbase.h>

#ifndef __ATLCOM_H__
	extern CComModule _Module;
	#include <atlcom.h>
#endif

#include "sockutil.h"

#pragma warning(disable: 4100)
#include "usocket.h"

#include <vcclr.h>
using namespace System;

namespace SocketProAdapter
{
namespace ClientSide
{

#define	IDC_SRCUSOCKETEVENT		2
static _ATL_FUNC_INFO OnSockEventFuncInfo = {CC_STDCALL, VT_I4, 2, {VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSockEventWinMsgInfo = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnDataAvailableInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSendingDataInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSockGetHostByAddr = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_BSTR, VT_BSTR, VT_I4}};
static _ATL_FUNC_INFO OnSockGetHostByName = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_BSTR, VT_BSTR, VT_BSTR, VT_I4}};
static _ATL_FUNC_INFO OnRequestProcessedInfo = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_I2, VT_I4, VT_I4, VT_I2}};

ref class CClientSocket;

class CCSEvent : public IDispEventSimpleImpl<IDC_SRCUSOCKETEVENT, CCSEvent, &__uuidof(_IUSocketEvent)>							
{
public:
BEGIN_SINK_MAP(CCSEvent)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 1, &CCSEvent::OnDataAvailable, &OnDataAvailableInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 2, &CCSEvent::OnOtherMessage, &OnSockEventWinMsgInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 3, &CCSEvent::OnSocketClosed, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 3, &CCSEvent::OnSocketClosed, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 4, &CCSEvent::OnSocketConnected, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 5, &CCSEvent::OnConnecting, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 6, &CCSEvent::OnSendingData, &OnSendingDataInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 7, &CCSEvent::OnGetHostByAddr, &OnSockGetHostByAddr)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 8, &CCSEvent::OnGetHostByName, &OnSockGetHostByName)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 9, &CCSEvent::OnClosing, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 10, &CCSEvent::OnRequestProcessed, &OnRequestProcessedInfo)
END_SINK_MAP()

protected:
	virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError);
	virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam);
	virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError);
	virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError);
	virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent);
	virtual HRESULT __stdcall OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError);
	virtual HRESULT __stdcall OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError);
	virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);

public:
	gcroot<CClientSocket ^>	m_pClientSocket;

}; //CCSEvent

};

};

#endif
