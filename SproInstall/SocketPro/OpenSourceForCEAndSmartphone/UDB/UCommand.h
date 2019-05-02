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

// UCommand.h : Declaration of the CUCommand

#ifndef __UCOMMAND_H_
#define __UCOMMAND_H_

#include "resource.h"       // main symbols
#include "UDBCP.h"

class CParamInfo
{
public:
	CParamInfo()
	{
		m_usDBType = 0;
		m_ulDBParamIO = sdParamInput;
		m_ulLen = 0;
		m_bPrecision = 0;
		m_bScale = 0;
	}
	
	CParamInfo(const CParamInfo &ParamInfo)
	{
		m_usDBType = ParamInfo.m_usDBType;
		m_ulDBParamIO = ParamInfo.m_ulDBParamIO;
		m_ulLen = ParamInfo.m_ulLen;
		m_bstrPName = ParamInfo.m_bstrPName;
		m_bPrecision = ParamInfo.m_bPrecision;
		m_bScale = ParamInfo.m_bScale;
	}
	
	CParamInfo& operator= (const CParamInfo &ParamInfo)
	{
		m_usDBType = ParamInfo.m_usDBType;
		m_ulDBParamIO = ParamInfo.m_ulDBParamIO;
		m_ulLen = ParamInfo.m_ulLen;
		m_bstrPName = ParamInfo.m_bstrPName;
		m_bPrecision = ParamInfo.m_bPrecision;
		m_bScale = ParamInfo.m_bScale;
		return *this;
	}
	
	bool operator== (const CParamInfo &ParamInfo)
	{
		if(m_usDBType != ParamInfo.m_usDBType)
			return false;
		if(m_ulDBParamIO != ParamInfo.m_ulDBParamIO)
			return false;
		if(m_ulLen != ParamInfo.m_ulLen)
			return false;
		if(m_bstrPName != ParamInfo.m_bstrPName)
			return false;
		if(m_bPrecision != ParamInfo.m_bPrecision)
			return false;
		if(m_bScale != ParamInfo.m_bScale)
			return false;
		return true;
	}

	bool operator!= (const CParamInfo &ParamInfo)
	{
		return (!(*this==ParamInfo));
	}

	unsigned short	m_usDBType;
	unsigned long	m_ulDBParamIO;
	unsigned long	m_ulLen;
	CComBSTR		m_bstrPName;
	unsigned char	m_bPrecision;
	unsigned char	m_bScale;
};

/////////////////////////////////////////////////////////////////////////////
// CUCommand
class ATL_NO_VTABLE CUCommand : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUCommand, &CLSID_UCommand>,
	public IConnectionPointContainerImpl<CUCommand>,
	public CProxy_IURequestEvent< CUCommand >,
	public IDispatchImpl<IUCommand, &IID_IUCommand, &LIBID_UDBLib>
{
public:
	CUCommand()
	{
		m_hr = uecOK;
		m_ulCreatedObject = 0;
		m_ulParentHandle = 0;
		m_ulAffectedRows = 0;
		m_ulHandle = 0;
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
DECLARE_REGISTRY_RESOURCEID(IDR_UCOMMAND)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CUCommand)
	COM_INTERFACE_ENTRY(IUCommand)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CUCommand)
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
	unsigned long				m_ulCreatedObject;
	unsigned long				m_ulParentHandle;
	unsigned long				m_ulAffectedRows;
	unsigned long				m_ulHandle;
	CSimpleArray<CParamInfo>	m_aParamInfo;
	CSimpleArray<CComVariant>	m_aParamData;
	CSimpleArray<CComVariant>	m_aOutput;
	
	CComAutoCriticalSection		m_cs;
	CClientSocketEvent<CUCommand>	m_SocketEventSink;
// IUCommand
public:
	STDMETHOD(ReleaseCreatedObject)();
	STDMETHOD(GetCountOutputData)(/*[out, retval]*/long *plCount);
	STDMETHOD(UseStorageObjectForBLOB)(/*[in, defaultvalue(-1)]*/VARIANT_BOOL bStorageObjectForBLOB);
	void HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer);
	STDMETHOD(get_CreatedObjectHandle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(ShrinkMemory)(/*[in]*/long lNewSize);
	STDMETHOD(get_ParentHandle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ParentHandle)(/*[in]*/ long newVal);
	STDMETHOD(get_AffectedRows)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Property)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(get_Handle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(SetParamData)(/*[in]*/long nIndex, /*[in]*/VARIANT vtData);
	STDMETHOD(CleanParamInfo)();
	STDMETHOD(GetOutputData)(/*[in]*/long nIndex, /*[out, retval]*/VARIANT *pvtData);
	STDMETHOD(CleanParamData)();
	STDMETHOD(AppendParamData)(/*[in]*/VARIANT vtParamData);
	STDMETHOD(GetCountParamData)(/*[out, retval]*/long *plCount);
	STDMETHOD(GetCountParamInfos)(/*[out, retval]*/long *plCount);
	STDMETHOD(GetParamInfo)(/*[in]*/long nIndex, /*[out, defaultvalue(0)]*/short *pnDBType, /*[out,defaultvalue(0)]*/long *plDBParamIO, /*[out,defaultvalue(0)]*/long *plLen, /*[out, defaultvalue(0)]*/BSTR *pbstrPName, /*[out, defaultvalue(0)]*/unsigned char *pnPrecision, /*[out,defaultvalue(0)]*/unsigned char *pnScale);
	STDMETHOD(AddParamInfo)(/*[in]*/short nDBType, /*[in, defaultvalue(1)]*/long nDBParamIO, /*[in, defaultvalue(0)]*/long nLen, /*[in, defaultvalue("")]*/BSTR bstrPName, /*[in, defaultvalue(0)]*/ unsigned char nPrecision, /*[in, defaultvalue(0)]*/ unsigned char nScale);
	STDMETHOD(GetOutputParams)();
	STDMETHOD(OpenFromHandle)(/*[in]*/long lHandle);
	STDMETHOD(DoBatch)(short sCreatedObject, short sCursorType, long lHint);
	STDMETHOD(Unprepare)();
	STDMETHOD(Prepare)(/*[in, defaultvalue(0)]*/long lExpectedRuns);
	STDMETHOD(ExecuteSQL)(/*[in]*/BSTR bstrSQL, /*[in]*/short sCreatedObject, short sCursorType, long lHint);
	STDMETHOD(Cancel)();
	STDMETHOD(GetProperty)(/*[in]*/long lPropID, /*[in, defaultvalue("{c8b522be-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(SetProperty)(/*[in]*/long lPropID, /*[in]*/VARIANT vtValue, /*[in, defaultvalue("{c8b522be-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(Open)(/*[in, defaultvalue("")]*/BSTR bstrData, /*[in, defaultvalue(0)]*/long lHint);
	STDMETHOD(Close)();
private:
	void PackAllParamData(bool bOneSetOnly = false);
	bool m_bSwitched;
};

#endif //__UCOMMAND_H_

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
