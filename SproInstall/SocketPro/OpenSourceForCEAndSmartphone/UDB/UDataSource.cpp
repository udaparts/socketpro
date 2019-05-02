// UDataSource.cpp : Implementation of CUDataSource

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

#include "stdafx.h"
#include "UDB.h"
#include "UDataSource.h"
#include <oledb.h>

/////////////////////////////////////////////////////////////////////////////
// CUDataSource

STDMETHODIMP CUDataSource::AttachSocket(IUnknown *pIUnknownToUSocket)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_bSwitched = false;
	m_hr = uecOK;
	if(m_pIUFast.p)
		m_pIUFast.Release();
	if(m_pIUSocket.p)
	{
		m_hr = m_SocketEventSink.DispEventUnadvise(m_pIUSocket.p);
		m_pIUSocket.Release();
	}
	if(pIUnknownToUSocket)
	{
		m_hr = pIUnknownToUSocket->QueryInterface(__uuidof(IUSocket), (void**)&m_pIUSocket);
	}
	if(m_pIUSocket.p)
	{
		long hSocket = 0;
		m_hr = m_pIUSocket->get_Socket(&hSocket);
		if(hSocket > 0)
		{
			long lSvsID = 0;
			m_hr = m_pIUSocket->get_CurrentSvsID(&lSvsID);
			if(lSvsID == sidOleDB)
			{
				m_bSwitched = true;
			}
		}
		m_hr = m_SocketEventSink.DispEventAdvise(m_pIUSocket.p);
		if(!FAILED(m_hr))
		{
			m_hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&m_pIUFast);
			ATLASSERT(m_hr == uecOK);
		}
	}
	return m_hr;
}

STDMETHODIMP CUDataSource::get_Rtn(long *plResult)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plResult)
		*plResult = m_hr;
	return S_OK;
}

STDMETHODIMP CUDataSource::get_ErrorMsg(BSTR *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		if(*pVal)
			::SysFreeString(*pVal);
		switch(m_hr)
		{
		case uecUnexpected: 
			*pVal = ::SysAllocString(L"An unexpected error occurred.");
			break;
		case uecNotImplemented:
			*pVal = ::SysAllocString(L"Not implemented yet.");
			break;
		default:
			*pVal = m_bstrErrorMsg.Copy();
			break;
		}
	}
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnSocketClosed(long hSocket, long lError)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_bSwitched = false;
	m_ulFlags = 0;
	m_vtProperty.Clear();
	m_ulHandle = 0;
	m_ulParentHandle = 0;
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnSocketConnected(long hSocket, long lError)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	m_bstrErrorMsg.Empty();
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnConnecting(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnSendingData(long hSocket, long lError, long lSent)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUDataSource::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	switch(nRequestID)
	{
	case idCleanTrack:
		{
			CAutoLock AutoLock(&m_cs.m_sec);
			m_UQueue.CleanTrack();
		}
		break;
	default:
		break;
	}
	if(sFlag != rfCompleted)
		return S_OK;
	if((unsigned short)nRequestID != idSwitchTo && !m_bSwitched)
		return S_OK;
	switch (nRequestID)
	{
	case idSwitchTo:
		ATLASSERT(m_pIUSocket != NULL);
		if(m_pIUSocket != NULL)
		{
			long lSvsID;
			m_pIUSocket->get_CurrentSvsID(&lSvsID);
			if(lSvsID == sidOleDB)
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				m_bSwitched = true;
			}
			else
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				m_bSwitched = false;
				m_ulFlags = 0;
				m_vtProperty.Clear();
				m_ulHandle = 0;
				m_ulParentHandle = 0;
			}
		}
		break;
	case idDSOpenFromHandle:
	case idDSOpen:
	case idDSClose:
		m_ulHandle = 0;
		m_ulParentHandle = 0;
	case idDSGetProperty:
	case idDSGetPropFlags:
	case idDSSetProperty:
		m_hr = S_OK;
		HandleDataSourceProcessedResult(nRequestID, lLen, lLenInBuffer);
		Fire_OnRequestProcessed(hSocket, nRequestID, 0, 0, rfCompleted);
		//version 4.3.0.2
//		m_hr = S_OK;
		break;
	default:
		break;
	}
	return S_OK;
}

STDMETHODIMP CUDataSource::Close()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idDSClose, 0, NULL);
}

STDMETHODIMP CUDataSource::Open(BSTR bstrConnection, long lHint)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	lHint = 0;
	m_ulFlags = 0;
	m_vtProperty.Clear();
	m_ulHandle = 0;
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&m_ulParentHandle);
	m_UQueue.Push(&lHint);
	m_UQueue.Push(bstrConnection);
	return m_pIUFast->SendRequestEx(idDSOpen, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUDataSource::GetProperty(long lPropID, BSTR bstrPropSet)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lPropID);
	m_UQueue.Push(bstrPropSet);
	return m_pIUFast->SendRequestEx(idDSGetProperty, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUDataSource::GetPropFlags(long lPropID, BSTR bstrPropSet)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lPropID);
	m_UQueue.Push(bstrPropSet);
	return m_pIUFast->SendRequestEx(idDSGetPropFlags, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUDataSource::get_ParentHandle(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulParentHandle;
	return S_OK;
}

STDMETHODIMP CUDataSource::put_ParentHandle(long newVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = uecNotImplemented;

	return m_hr;
}

STDMETHODIMP CUDataSource::get_Handle(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulHandle;
	return S_OK;
}

STDMETHODIMP CUDataSource::get_Flags(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulFlags;
	return S_OK;
}

STDMETHODIMP CUDataSource::get_Property(VARIANT *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		::VariantClear(pVal);
		::VariantCopy(pVal, &m_vtProperty);
	}
	return S_OK;
}

void CUDataSource::HandleDataSourceProcessedResult(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	ATLASSERT(ulLen == ulLenInBuffer);
	m_UQueue.SetSize(0);
	m_bstrErrorMsg.Empty();
	if(ulLen>m_UQueue.GetMaxSize())
	{
		m_UQueue.ReallocBuffer(ulLen);
	}
	if(ulLen)
	{
		m_pIUFast->GetRtnBufferEx(ulLen, (BYTE*)m_UQueue.GetBuffer(), &ulLenInBuffer);
		ATLASSERT(ulLen == ulLenInBuffer);
		m_UQueue.SetSize(ulLen);
	}
	m_UQueue.Pop(&m_hr);
	if(FAILED(m_hr))
	{
		if(m_UQueue.GetSize())
		{
			WCHAR c = 0;
			m_UQueue.Push(&c, 1); //null-terminating string
			ATLASSERT((m_UQueue.GetSize()%sizeof(WCHAR)) == 0);
			m_bstrErrorMsg = (LPCWSTR)m_UQueue.GetBuffer();
#ifdef _DEBUG
			{
				USES_CONVERSION;
				ATLTRACE("%s\n", OLE2A(m_bstrErrorMsg));
			}
#endif
			m_UQueue.SetSize(0);
		}
		return;
	}
	switch (usRequestID)
	{
	case idDSOpenFromHandle:
	case idDSOpen:
		m_UQueue.Pop(&m_ulHandle);
		m_UQueue.Pop(&m_ulParentHandle);
		break;
	case idDSClose:
	case idDSSetProperty:
		break;
	case idDSGetProperty:
		m_vtProperty.Clear();
		m_UQueue.PopVT(m_vtProperty);
		break;
	case idDSGetPropFlags:
		m_UQueue.Pop(&m_ulFlags);
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}
	ATLASSERT(m_UQueue.GetSize() == 0);
}

STDMETHODIMP CUDataSource::OpenFromHandle(long lHandle)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_ulFlags = 0;
	m_vtProperty.Clear();
	m_ulHandle = 0;
	if(!m_pIUFast.p || lHandle == 0) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idDSOpenFromHandle, sizeof(lHandle), (BYTE*)&lHandle);
}



STDMETHODIMP CUDataSource::SetProperty(VARIANT vtValue, long lPropID, BSTR bstrPropSet)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.PushVT(vtValue);
	m_UQueue.Push(&lPropID);
	m_UQueue.Push(bstrPropSet);
	return m_pIUFast->SendRequestEx(idDSSetProperty, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

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