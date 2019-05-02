// This is a part of the SocketPro package.
// Copyright (C) 2000-2004 UDAParts 
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

#ifndef __U_SOCKET_EVENT_FILE_H__
#define __U_SOCKET_EVENT_FILE_H__

#define	IDC_SRCUSOCKETEVENT		1

static _ATL_FUNC_INFO OnSockEventFuncInfo = {CC_STDCALL, VT_I4, 2, {VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSockEventWinMsgInfo = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnDataAvailableInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSendingDataInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
static _ATL_FUNC_INFO OnSockGetHostByAddr = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_BSTR, VT_BSTR, VT_I4}};
static _ATL_FUNC_INFO OnSockGetHostByName = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_BSTR, VT_BSTR, VT_BSTR, VT_I4}};
static _ATL_FUNC_INFO OnRequestProcessedInfo = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_I2, VT_I4, VT_I4, VT_I2}};
/*
template <class TContainer>
class CClientSocketEvent : public IDispEventSimpleImpl<IDC_SRCUSOCKETEVENT, CClientSocketEvent, &__uuidof(_IUSocketEvent)>							
{
public:
	TContainer	*m_pContainer;

BEGIN_SINK_MAP(CClientSocketEvent)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 1, OnDataAvailable, &OnDataAvailableInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 2, OnOtherMessage, &OnSockEventWinMsgInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 3, OnSocketClosed, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 4, OnSocketConnected, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 5, OnConnecting, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 6, OnSendingData, &OnSendingDataInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 7, OnGetHostByAddr, &OnSockGetHostByAddr)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 8, OnGetHostByName, &OnSockGetHostByName)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 9, OnClosing, &OnSockEventFuncInfo)
	SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 10, OnRequestProcessed, &OnRequestProcessedInfo)
END_SINK_MAP()
	virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError)
	{
		return m_pContainer->OnDataAvailable(hSocket, lBytes, lError);
	}
	virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
	{
		return m_pContainer->OnOtherMessage(hSocket, nMsg, wParam, lParam);
	}
	virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError)
	{
		return m_pContainer->OnSocketClosed(hSocket, lError);
	}
	virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError)
	{
		return m_pContainer->OnSocketConnected(hSocket, lError);
	}
	virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd)
	{
		return m_pContainer->OnConnecting(hSocket, hWnd);
	}
	virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent)
	{
		return m_pContainer->OnSendingData(hSocket, lError, lSent);
	}
	virtual HRESULT __stdcall OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError)
	{
		return m_pContainer->OnGetHostByAddr(lHandle, bstrHostName, bstrHostAlias, lError);
	}
	virtual HRESULT __stdcall OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError)
	{
		return m_pContainer->OnGetHostByName(lHandle, bstrHostName, bstrAlias, bstrIPAddr, lError);
	}
	virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd)
	{
		return m_pContainer->OnClosing(hSocket, hWnd);
	}
	virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
	{
		return m_pContainer->OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
	}
};*/

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2004 UDAParts 
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