// UQueueImp.h : Declaration of the CUQueueImp

#ifndef __UQUEUE_H_
#define __UQUEUE_H_

#include "resource.h"       // main symbols
#include "UQueueCP.h"

/////////////////////////////////////////////////////////////////////////////
// CUQueueImp
class ATL_NO_VTABLE CUQueueImp : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUQueueImp, &CLSID_UQueue>,
	public IDispatchImpl<IUQueue, &IID_IUQueue, &LIBID_UQUEUELib>,
	public CProxy_IURequestEvent< CUQueueImp >,
	public IConnectionPointContainerImpl<CUQueueImp>
{
public:
	CUQueueImp()
	{
		m_hr = S_OK;
		m_SocketEventSink.m_pContainer = this;
	}

	virtual ~CUQueueImp()
	{
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

DECLARE_REGISTRY_RESOURCEID(IDR_UQUEUE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CUQueueImp)
	COM_INTERFACE_ENTRY(IUQueue)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CUQueueImp)
CONNECTION_POINT_ENTRY(DIID__IURequestEvent)
END_CONNECTION_POINT_MAP()

// IUQueue
public:
	STDMETHOD(PopBytes)(long lLen, /*[out, retval]*/VARIANT *pvtBytes);
	STDMETHOD(PushBytes)(/*[in]*/VARIANT vtBytes);
	STDMETHOD(SpeakEx)(/*[in, defaultvalue(-1)]*/long lGroups);
	STDMETHOD(SpeakToEx)(/*[in]*/BSTR bstrIPAddr, /*[in]*/long lPort);
	STDMETHOD(PopByte)(/*[out, retval]*/BYTE *pbData);
	STDMETHOD(PushByte)(/*[in]*/BYTE bData);
	STDMETHOD(SendRequest)(/*[in]*/short sRequestID);
	STDMETHOD(PopDecimal)(/*[out, retval]*/DECIMAL *pdcData);
	STDMETHOD(PushDecimal)(/*[in]*/DECIMAL dcData);
	STDMETHOD(PopCurrency)(/*[out, retval]*/CURRENCY *pcyData);
	STDMETHOD(PushCurrency)(/*[in]*/CURRENCY cyData);
	STDMETHOD(Pop)(/*[out, retval]*/VARIANT *pvtData);
	STDMETHOD(Push)(/*[in]*/VARIANT vtData);
	STDMETHOD(PopFloat)(/*[out, retval]*/float *pfData);
	STDMETHOD(PopDouble)(/*[out, retval]*/double *pdData);
	STDMETHOD(PopDate)(/*[out, retval]*/DATE *pdtData);
	STDMETHOD(PopString)(long lLenInByte, /*[out, retval]*/BSTR *pbstrData);
	STDMETHOD(PopBoolean)(/*[out, retval]*/VARIANT_BOOL *pbData);
	STDMETHOD(PopLong)(/*[out, retval]*/long *plData);
	STDMETHOD(PopInteger)(/*[out, retval]*/short *psData);
	STDMETHOD(get_BufferSize)(/*[out, retval]*/ long *pVal);
	STDMETHOD(ReallocBuffer)(/*[in]*/long lNewSize);
	STDMETHOD(get_Size)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Size)(/*[in]*/ long newVal);
	STDMETHOD(PushDate)(/*[in]*/DATE dtData);
	STDMETHOD(PushBoolean)(/*[in]*/VARIANT_BOOL bData);
	STDMETHOD(PushDouble)(/*[in]*/double ddata);
	STDMETHOD(PushFloat)(/*[in]*/float fData);
	STDMETHOD(PushString)(/*[in]*/BSTR bstrData);
	STDMETHOD(PushLong)(/*[in]*/long lData);
	STDMETHOD(PushInteger)(/*[in]*/short sData);
	STDMETHOD(AttachSocket)(/*[in]*/IUnknown *pIUnknownToUSocket);
	STDMETHOD(get_Rtn)(long *plResult);
	STDMETHOD(get_ErrorMsg)(BSTR *pVal);

public:
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

private:
	CComAutoCriticalSection		m_cs;
	CComPtr<IUFast>				m_pIUFast;
	CComPtr<IUSocket>			m_pIUSocket;
	HRESULT						m_hr;
	CUQueue						m_UQueue;
	CUQueue						m_qTemp;
	CClientSocketEvent<CUQueueImp>	m_SocketEventSink;	
};

#endif //__UQUEUE_H_
