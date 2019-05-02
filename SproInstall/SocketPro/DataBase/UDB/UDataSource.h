// This is a part of the SocketPro package.
// Copyright (C) 2000-2008 UDAParts 
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

// UDataSource.h : Declaration of the CUDataSource

#ifndef __UDATASOURCE_H_
#define __UDATASOURCE_H_

#include "resource.h"       // main symbols
#include "UDBCP.h"

/////////////////////////////////////////////////////////////////////////////
// CUDataSource
class ATL_NO_VTABLE CUDataSource : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUDataSource, &CLSID_UDataSource>,
	public IConnectionPointContainerImpl<CUDataSource>,
	public CProxy_IURequestEvent< CUDataSource >,
	public IDispatchImpl<IUDataSource, &IID_IUDataSource, &LIBID_UDBLib>,
	public IDispEventSimpleImpl<IDC_SRCUSOCKETEVENT, CUDataSource, &__uuidof(_IUSocketEvent)>
{
public:
	CUDataSource()
	{
		m_hr = uecOK;
		m_ulFlags = 0;
		m_ulHandle = 0;
		m_ulParentHandle = 0;
		m_bSwitched = false;
	}
	void FinalRelease()
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(m_pIUSocket.p)
		{
			m_hr = DispEventUnadvise(m_pIUSocket.p);
			m_pIUSocket.Release();
		}
		if(m_pIUFast.p)
			m_pIUFast.Release();
	}
DECLARE_REGISTRY_RESOURCEID(IDR_UDATASOURCE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CUDataSource)
	COM_INTERFACE_ENTRY(IUDataSource)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CUDataSource)
CONNECTION_POINT_ENTRY(DIID__IURequestEvent)
END_CONNECTION_POINT_MAP()

BEGIN_SINK_MAP(CUDataSource)
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
	
	STDMETHOD(AttachSocket)(IUnknown *pIUnknownToUSocket);
	STDMETHOD(get_Rtn)(long *plResult);
	STDMETHOD(get_ErrorMsg)(BSTR *pVal);

	virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError);
	virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam);
	virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError);
	virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError);
	virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent);
	virtual HRESULT __stdcall OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError);
	virtual HRESULT __stdcall OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError);
	virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);


	CComPtr<IUSocket>			m_pIUSocket;
	CComPtr<IUFast>				m_pIUFast;
	HRESULT						m_hr;
	CComBSTR					m_bstrErrorMsg;
	CUQueue						m_UQueue;
	unsigned long				m_ulFlags;
	CComVariant					m_vtProperty;
	unsigned long				m_ulHandle;
	unsigned long				m_ulParentHandle;

	CComAutoCriticalSection		m_cs;
//	CClientSocketEvent<CUDataSource>	m_SocketEventSink;
	bool m_bSwitched;
// IUDataSource
public:
	STDMETHOD(SetProperty)(/*[in]*/VARIANT vtValue, /*[in]*/long lPropID, /*[in]*/BSTR bstrPropSet);
	STDMETHOD(OpenFromHandle)(/*[in]*/long lHandle);
	void HandleDataSourceProcessedResult(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer);
	STDMETHOD(get_Property)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(get_Flags)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Handle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ParentHandle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ParentHandle)(/*[in]*/ long newVal);
	STDMETHOD(GetPropFlags)(/*[in]*/long lPropID, /*[in, defaultvalue("{c8b522be-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(GetProperty)(/*[in]*/long lPropID, /*[in, defaultvalue("{c8b522bb-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(Open)(/*[in]*/BSTR bstrConnection, /*[in, defaultvalue(0)]*/long lHint);
	STDMETHOD(Close)();
};

#endif //__UDATASOURCE_H_

// This is a part of the SocketPro package.
// Copyright (C) 2000-2008 UDAParts 
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
