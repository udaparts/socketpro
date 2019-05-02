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

// USession.h : Declaration of the CUSession

#ifndef __USESSION_H_
#define __USESSION_H_

#include "resource.h"       // main symbols
#include "UDBCP.h"

/////////////////////////////////////////////////////////////////////////////
// CUSession
class ATL_NO_VTABLE CUSession : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUSession, &CLSID_USession>,
	public IConnectionPointContainerImpl<CUSession>,
	public CProxy_IURequestEvent< CUSession >,
	public IDispatchImpl<IUSession, &IID_IUSession, &LIBID_UDBLib>
{
public:
	CUSession()
	{
		m_hr = uecOK;
		m_ulHandle = 0;
		m_ulParentHandle = 0;
		m_SocketEventSink.m_pContainer = this;
		m_bSwitched = false;
	}
	
	void FinalRelease()
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(m_pIUSocket.p)
		{
			m_hr = m_SocketEventSink.DispEventUnadvise(m_pIUSocket.p);
			m_pIUSocket.Release();
		}
		if(m_pIUFast.p)
			m_pIUFast.Release();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_USESSION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CUSession)
	COM_INTERFACE_ENTRY(IUSession)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CUSession)
CONNECTION_POINT_ENTRY(DIID__IURequestEvent)
END_CONNECTION_POINT_MAP()
	
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
	CComVariant					m_vtProperty;
	unsigned long				m_ulHandle;
	unsigned long				m_ulParentHandle;
	CComAutoCriticalSection		m_cs;
	CClientSocketEvent<CUSession>	m_SocketEventSink;
	bool m_bSwitched;
// IUSession
public:
	void HandleSessionProcessedResult(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer);
	STDMETHOD(OpenFromHandle)(/*[in]*/long lHandle);
	STDMETHOD(get_Property)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(get_Handle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ParentHandle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ParentHandle)(/*[in]*/ long newVal);
	STDMETHOD(GetProperty)(/*[in, defaultvalue(0xbeL)]*/long lPropID, /*[in, defaultvalue("{c8b522c6-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(SetProperty)(/*[in]*/VARIANT vtValue, /*[in, defaultvalue(0xbeL)]*/long lPropID, /*[in, defaultvalue("{c8b522c6-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(BeginTrans)(/*[in, defaultvalue(0x1000)]*/long lISolationLevel);
	STDMETHOD(Rollback)();
	STDMETHOD(Commit)();
	STDMETHOD(Open)(/*[in, defaultvalue("")]*/BSTR bstrData, /*[in, defaultvalue(0)]*/long lHint);
	STDMETHOD(Close)();
};

#endif //__USESSION_H_

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
