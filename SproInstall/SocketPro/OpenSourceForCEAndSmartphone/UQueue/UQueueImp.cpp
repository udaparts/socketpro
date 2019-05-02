// UQueueImp.cpp : Implementation of CUQueue
#include "stdafx.h"
#include "UQueue.h"
#include "UQueueImp.h"

/////////////////////////////////////////////////////////////////////////////
// CUQueue


STDMETHODIMP CUQueueImp::AttachSocket(IUnknown *pIUnknownToUSocket)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(m_pIUFast.p)
		m_pIUFast.Release();
	if(m_pIUSocket.p)
	{
		m_SocketEventSink.DispEventUnadvise(m_pIUSocket.p);
		m_pIUSocket.Release();
	}
	if(pIUnknownToUSocket)
	{
		m_hr = pIUnknownToUSocket->QueryInterface(__uuidof(IUSocket), (void**)&m_pIUSocket);
	}
	if(m_pIUSocket.p)
	{
		m_hr = m_SocketEventSink.DispEventAdvise(m_pIUSocket.p);
		if(!FAILED(m_hr))
		{
			m_hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&m_pIUFast);
			ATLASSERT(m_hr == uecOK);
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::SpeakEx(long lGroups)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(m_pIUFast.p == NULL)
	{
		m_hr = ueSocketNotAttached;
	}
	else
	{
		m_hr = m_pIUFast->SpeakEx(m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer(), lGroups);
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::SpeakToEx(BSTR bstrIPAddr, long lPort)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(m_pIUFast.p == NULL)
	{
		m_hr = ueSocketNotAttached;
	}
	else
	{
		m_hr = m_pIUFast->SpeakToEx(bstrIPAddr, lPort, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
	}
	return m_hr;
}

HRESULT __stdcall CUQueueImp::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnSocketClosed(long hSocket, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnSocketConnected(long hSocket, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnConnecting(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnSendingData(long hSocket, long lError, long lSent)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUQueueImp::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
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
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if((ULONG)lLen > m_UQueue.GetMaxSize())
	{
		m_UQueue.ReallocBuffer((ULONG)lLen);
		if(m_UQueue.GetBuffer() == NULL)
		{
			m_hr = ueAllocatingMemoryFailed;
			return m_hr;
		}
	}
	m_UQueue.SetSize(0);
	if((ULONG)lLen > 0)
	{
		unsigned long ulGet;
		m_hr = m_pIUFast->GetRtnBufferEx((ULONG)lLen, (BYTE*)m_UQueue.GetBuffer(), &ulGet);
		ATLASSERT(ulGet == (ULONG)lLen);
		m_UQueue.SetSize((ULONG)lLen);
	}
	return Fire_OnRequestProcessed(hSocket, nRequestID, 0, 0, sFlag);
}

STDMETHODIMP CUQueueImp::get_Rtn(long *plResult)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plResult)
		*plResult = m_hr;
	return S_OK;
}

STDMETHODIMP CUQueueImp::get_ErrorMsg(BSTR *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		if(*pVal)
		{
			::SysFreeString(*pVal);
			*pVal = NULL;
		}
		switch (m_hr)
		{
		case uecOK:
			break;
		case ueUnknown:
			*pVal = ::SysAllocString(L"Unknown error.");
			break;
		case ueSocketNotAttached:
			*pVal = ::SysAllocString(L"Not attached with a socket yet.");
			break;
		case ueNewSizeLargerThanBufferSize:
			*pVal = ::SysAllocString(L"Can't set a size larger than the size of uqueue buffer.");
			break;
		case ueAllocatingMemoryFailed:
			*pVal = ::SysAllocString(L"Allocating a buffer failed.");
			break;
		case ueDataSizeWrong:
			*pVal = ::SysAllocString(L"Data size wrong.");
			break;
		case ueNotBytes:
			*pVal = ::SysAllocString(L"An array of bytes expected.");
			break;
		default:
			if(m_hr != uecOK && m_pIUSocket.p != NULL)
			{
				m_pIUSocket->get_ErrorMsg(pVal);
			}
			break;
		}
	}
	return S_OK;
}

STDMETHODIMP CUQueueImp::PushInteger(short sData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&sData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushLong(long lData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&lData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushString(BSTR bstrData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(bstrData != NULL)
	{
		ULONG ulLen = ::SysStringLen(bstrData)*sizeof(WCHAR);
		m_UQueue.Push((BYTE*)bstrData, ulLen);
		if(m_UQueue.GetBuffer() == NULL)
		{
			m_hr = ueAllocatingMemoryFailed;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushFloat(float fData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&fData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushDouble(double dData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&dData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushBoolean(VARIANT_BOOL bData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&bData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushDate(DATE dtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&dtData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}


STDMETHODIMP CUQueueImp::get_Size(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_UQueue.GetSize();
	return S_OK;
}

STDMETHODIMP CUQueueImp::put_Size(long newVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if((ULONG)newVal <= m_UQueue.GetMaxSize())
	{
		m_UQueue.SetSize(newVal);
	}
	else
	{
		m_hr = ueNewSizeLargerThanBufferSize;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::ReallocBuffer(long lNewSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.ReallocBuffer(lNewSize);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::get_BufferSize(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_UQueue.GetMaxSize();
	return S_OK;
}

STDMETHODIMP CUQueueImp::PopInteger(short *psData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(psData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(psData);
		if(ulGet < sizeof(short))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopLong(long *plData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(plData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(plData);
		if(ulGet < sizeof(long))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopBoolean(VARIANT_BOOL *pbData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pbData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pbData);
		if(ulGet < sizeof(VARIANT_BOOL))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopString(long lLenInByte, BSTR *pbstrData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pbstrData)
	{
		ULONG ulLen = (ULONG)lLenInByte - ((ULONG)lLenInByte%2);
		if(ulLen > m_UQueue.GetSize())
		{
			ulLen = m_UQueue.GetSize() - (m_UQueue.GetSize()%2);
		}
		if(ulLen == 0)
			return m_hr;
		if(*pbstrData != NULL)
		{
			::SysFreeString(*pbstrData);
			*pbstrData = NULL;
		}
		*pbstrData = ::SysAllocStringLen((LPOLESTR)m_UQueue.GetBuffer(), ulLen/sizeof(WCHAR));
		m_UQueue.Pop(ulLen);
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopDate(DATE *pdtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pdtData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pdtData);
		if(ulGet < sizeof(DATE))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopDouble(double *pdData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pdData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pdData);
		if(ulGet < sizeof(double))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopFloat(float *pfData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pfData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pfData);
		if(ulGet < sizeof(float))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::Push(VARIANT vtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.PushVT(vtData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::Pop(VARIANT *pvtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pvtData != NULL)
	{
		::VariantClear(pvtData);
		ULONG ulGet = m_UQueue.PopVT(*pvtData);
		if(ulGet < sizeof(pvtData->vt))
		{
			m_hr = ueDataSizeWrong;
		}
		else if(((pvtData->vt & VT_BSTR) & VT_ARRAY) > 0 && ulGet < (pvtData->parray->rgsabound[0].cElements*sizeof(long) + sizeof(pvtData->vt) + sizeof(long)))
		{
			m_hr = ueDataSizeWrong;
		}
		else if((pvtData->vt & VT_ARRAY) > 0)
		{
			switch(pvtData->vt - VT_ARRAY)
			{
			case VT_VARIANT:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*2 + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_I1:
				pvtData->vt = (VT_UI1|VT_ARRAY);
			case VT_UI1:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_UI2:
				pvtData->vt = (VT_I2|VT_ARRAY);
			case VT_BOOL:
			case VT_I2:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*2 + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_UINT:
			case VT_UI4:
			case VT_INT:
				pvtData->vt = (VT_I4|VT_ARRAY);
			case VT_I4:
			case VT_BSTR:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*4 + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_R4:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*4 + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_UI8:
				pvtData->vt = (VT_I8|VT_ARRAY);
			case VT_CY:
			case VT_I8:
			case VT_DATE:
			case VT_FILETIME:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*8 + 6))
					m_hr = ueDataSizeWrong;
				break;
			case VT_DECIMAL:
			case VT_CLSID:
				if(ulGet < (pvtData->parray->rgsabound[0].cElements*16 + 6))
					m_hr = ueDataSizeWrong;
				break;
			default:
				m_hr = ueDataSizeWrong;
				break;
			}
		}
		else 
		{
			switch(pvtData->vt)
			{
			case VT_VARIANT:
				if(ulGet < 2)
					m_hr = ueDataSizeWrong;
				break;
			case VT_I1:
				pvtData->vt = VT_UI1; 
			case VT_UI1:
				if(ulGet < 1)
					m_hr = ueDataSizeWrong;
				break;
			case VT_UI2:
				pvtData->vt = VT_I2;
			case VT_BOOL:
			case VT_I2:
				if(ulGet < 2)
					m_hr = ueDataSizeWrong;
				break;
			case VT_UINT:
			case VT_UI4:
			case VT_INT:
				pvtData->vt = VT_I4;
			case VT_I4:
			case VT_BSTR:
				if(ulGet < 4)
					m_hr = ueDataSizeWrong;
				break;
			case VT_R4:
				if(ulGet < 4)
					m_hr = ueDataSizeWrong;
				break;
			case VT_UI8:
				pvtData->vt = VT_I8;
			case VT_CY:
			case VT_I8:
			case VT_DATE:
			case VT_FILETIME:
				if(ulGet < 8)
					m_hr = ueDataSizeWrong;
				break;
			case VT_DECIMAL:
			case VT_CLSID:
				if(ulGet < 16)
					m_hr = ueDataSizeWrong;
				break;
			default:
				m_hr = ueDataSizeWrong;
				break;
			}
		}
	}
	if(m_hr != S_OK)
	{
		m_UQueue.SetSize(0);
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushCurrency(CURRENCY cyData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&cyData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopCurrency(CURRENCY *pcyData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pcyData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pcyData);
		if(ulGet < sizeof(CURRENCY))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushDecimal(DECIMAL dcData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&dcData);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopDecimal(DECIMAL *pdcData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pdcData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pdcData);
		if(ulGet < sizeof(DECIMAL))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::SendRequest(short sRequestID)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(m_pIUFast.p == NULL)
	{
		m_hr = ueSocketNotAttached;
	}
	else
	{
		m_hr = m_pIUFast->SendRequestEx(sRequestID, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
		
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushByte(BYTE bData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.Push(&bData, 1);
	if(m_UQueue.GetBuffer() == NULL)
	{
		m_hr = ueAllocatingMemoryFailed;
	}
	else
	{
		m_hr = S_OK;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopByte(BYTE *pbData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(pbData != NULL)
	{
		ULONG ulGet = m_UQueue.Pop(pbData);
		if(ulGet < sizeof(BYTE))
		{
			m_hr = ueDataSizeWrong;
		}
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PushBytes(VARIANT vtBytes)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = S_OK;
	if(vtBytes.vt == VT_EMPTY)
		return S_OK;
	if(vtBytes.vt == (VT_ARRAY|VT_UI1) || vtBytes.vt == (VT_ARRAY|VT_I1))
	{
		BYTE *pByte;
		ULONG ulLen = vtBytes.parray->rgsabound[0].cElements;
		::SafeArrayAccessData(vtBytes.parray, (void**)&pByte);
		m_UQueue.Push(pByte, ulLen);
		if(m_UQueue.GetBuffer() == NULL)
		{
			m_hr = ueAllocatingMemoryFailed;
		}
		::SafeArrayUnaccessData(vtBytes.parray);
	}
	else
	{
		m_hr = ueNotBytes;
	}
	return m_hr;
}

STDMETHODIMP CUQueueImp::PopBytes(long lLen, VARIANT *pvtBytes)
{
	ULONG ulLen = (ULONG)lLen;
	if(pvtBytes)
	{
		::VariantClear(pvtBytes);
	}
	CAutoLock AutoLock(&m_cs.m_sec);
	if(ulLen > m_UQueue.GetSize())
		ulLen = m_UQueue.GetSize();
	m_hr = S_OK;
	if(pvtBytes != NULL && ulLen > 0)
	{
		BYTE *pByte;
		SAFEARRAYBOUND sab[1] = {ulLen, 0};
		pvtBytes->vt = (VT_ARRAY|VT_UI1);
		pvtBytes->parray = ::SafeArrayCreate(VT_UI1, 1, sab);
		::SafeArrayAccessData(pvtBytes->parray, (void**)&pByte);
		m_UQueue.Pop(pByte, ulLen);
		::SafeArrayUnaccessData(pvtBytes->parray);
	}
	return m_hr;
}
