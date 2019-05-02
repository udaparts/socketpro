// UCommand.cpp : Implementation of CUCommand

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
#include "UCommand.h"

/////////////////////////////////////////////////////////////////////////////
// CUCommand

STDMETHODIMP CUCommand::AttachSocket(IUnknown *pIUnknownToUSocket)
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

STDMETHODIMP CUCommand::get_Rtn(long *plResult)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plResult)
		*plResult = m_hr;
	return S_OK;
}

STDMETHODIMP CUCommand::get_ErrorMsg(BSTR *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
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

HRESULT __stdcall CUCommand::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnSocketClosed(long hSocket, long lError)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_bSwitched = false;
	m_ulHandle = 0;
	m_ulParentHandle = 0;
	m_vtProperty.Clear();
	return S_OK;
}

HRESULT __stdcall CUCommand::OnSocketConnected(long hSocket, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnConnecting(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnSendingData(long hSocket, long lError, long lSent)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUCommand::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
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
	switch(nRequestID)
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
				m_ulHandle = 0;
				m_ulParentHandle = 0;
				m_vtProperty.Clear();
			}
		}
		break;
	case idCmndOpen:
	case idCmndClose:
	case idCmndOpenFromHandle:
		m_ulHandle = 0;
		m_ulParentHandle = 0;
	case idCmndExecuteSQL:
	case idCmndCancel:
	case idCmndGetProperty:
	case idCmndSetProperty:
	case idCmndPrepare:
	case idCmndUnprepare:
	case idCmndDoBatch:
	
	case idCmndGetOutputParams:
	case idCmndUseStorageObjectForBLOB:
	case idCmndReleaseCreatedObject:
//		ATLTRACE("nRequestID = %d\n", nRequestID);
		m_hr = S_OK;
		HandleRtn(nRequestID, lLen, lLenInBuffer);
		Fire_OnRequestProcessed(hSocket, nRequestID, 0, 0, rfCompleted);
		//version 4.3.0.2
//		m_hr = S_OK;
		break;
	default:
		break;
	}
	return S_OK;
}

STDMETHODIMP CUCommand::Close()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndClose, 0, NULL);
}

STDMETHODIMP CUCommand::Open(BSTR bstrData, long lHint)
{
	int n;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	CComVariant vtData;
	m_UQueue.Push(&m_ulParentHandle);
	m_UQueue.Push(&lHint);
	vtData = bstrData;
	m_UQueue.PushVT(vtData);
	int nCount = m_aParamInfo.GetSize();
	m_UQueue.Push(&nCount);
	for(n=0; n<nCount; n++)
	{
		m_UQueue.Push(&m_aParamInfo[n].m_bPrecision, 1);
		m_UQueue.Push(&m_aParamInfo[n].m_bScale, 1);
		vtData = m_aParamInfo[n].m_bstrPName;
		m_UQueue.PushVT(vtData);
		m_UQueue.Push(&m_aParamInfo[n].m_ulLen);
		m_UQueue.Push(&m_aParamInfo[n].m_ulDBParamIO);
		m_UQueue.Push(&m_aParamInfo[n].m_usDBType, 1);
	}
	return m_pIUFast->SendRequestEx(idCmndOpen, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUCommand::SetProperty(long lPropID, VARIANT vtValue, BSTR bstrPropSet)
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
	return m_pIUFast->SendRequestEx(idCmndSetProperty, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUCommand::GetProperty(long lPropID, BSTR bstrPropSet)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_vtProperty.Clear();
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lPropID);
	m_UQueue.Push(bstrPropSet);
	return m_pIUFast->SendRequestEx(idCmndGetProperty, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUCommand::Cancel()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_pIUSocket->Cancel();
	return m_pIUFast->SendRequestEx(idCmndCancel, 0, NULL);
}

STDMETHODIMP CUCommand::ExecuteSQL(BSTR bstrSQL, short sCreatedObject, short sCursorType, long lHint)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(sCreatedObject == coNothing || sCreatedObject == coRowset)
	{
	}
	else
	{
		m_hr = uecNotImplemented;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push((BYTE*)&sCreatedObject, sizeof(sCreatedObject));
	m_UQueue.Push(&sCursorType);
	m_UQueue.Push(&lHint);
	m_UQueue.Push(bstrSQL);
	return m_pIUFast->SendRequestEx(idCmndExecuteSQL, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUCommand::Prepare(long lExpectedRuns)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndPrepare, sizeof(lExpectedRuns), (BYTE*)&lExpectedRuns);
}

STDMETHODIMP CUCommand::Unprepare()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndUnprepare, 0, NULL);
}

STDMETHODIMP CUCommand::DoBatch(short sCreatedObject, short sCursorType, long lHint)
{
	int nCount;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_aParamInfo.GetSize() == 0)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_aParamData.GetSize() == 0)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if((m_aParamData.GetSize()%m_aParamInfo.GetSize()) != 0)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(sCreatedObject == coNothing || sCreatedObject == coRowset)
	{
		
	}
	else
	{
		m_hr = uecNotImplemented;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push((BYTE*)&sCreatedObject, sizeof(sCreatedObject));
	m_UQueue.Push(&sCursorType);
	m_UQueue.Push(&lHint);
	if(sCreatedObject == coRowset)
	{
		nCount = m_aParamInfo.GetSize();
	}
	else
	{
		nCount = m_aParamData.GetSize();
	}
	m_UQueue.Push(&nCount);
	PackAllParamData((sCreatedObject == coRowset));
	return m_pIUFast->SendRequestEx(idCmndDoBatch, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CUCommand::OpenFromHandle(long lHandle)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || lHandle == 0) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndOpenFromHandle, sizeof(lHandle), (BYTE*)&lHandle);
}

STDMETHODIMP CUCommand::GetOutputParams()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndGetOutputParams, 0, NULL);
}

STDMETHODIMP CUCommand::AddParamInfo(short sDBType, long lDBParamIO, long lLen, BSTR bstrPName, unsigned char bPrecision, unsigned char bScale)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(sDBType == sdVT_EMPTY || !IsDataTypeSupported(sDBType))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	CParamInfo	ParamInfo;
	ParamInfo.m_bPrecision = bPrecision;
	ParamInfo.m_bScale = bScale;
	ParamInfo.m_bstrPName = bstrPName;
	ParamInfo.m_ulDBParamIO = lDBParamIO;
	ParamInfo.m_usDBType = sDBType;
	ParamInfo.m_ulLen = lLen;
	if(lLen == 0)
	{
		if(sDBType == sdVT_STR || sDBType == sdVT_BYTES)
			ParamInfo.m_ulLen = 255;
		else if(sDBType == sdVT_STR)
			ParamInfo.m_ulLen = 255*sizeof(WCHAR);
	}
	m_aParamInfo.Add(ParamInfo);
	return uecOK;
}

STDMETHODIMP CUCommand::GetParamInfo(long nIndex, short *pnDBType, long *plDBParamIO, long *plLen, BSTR *pbstrPName, unsigned char *pnPrecision, unsigned char *pnScale)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if((unsigned long)nIndex >= m_aParamInfo.GetSize())
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(pnDBType)
		*pnDBType = m_aParamInfo[nIndex].m_usDBType;
	if(plDBParamIO)
		*plDBParamIO = m_aParamInfo[nIndex].m_ulDBParamIO;
	if(plLen)
		*plLen = m_aParamInfo[nIndex].m_ulLen;
	if(pbstrPName)
		*pbstrPName = m_aParamInfo[nIndex].m_bstrPName.Copy();
	if(pnPrecision)
		*pnPrecision = m_aParamInfo[nIndex].m_bPrecision;
	if(pnScale)
		*pnScale = m_aParamInfo[nIndex].m_bScale;

	return (m_hr = uecOK);
}

STDMETHODIMP CUCommand::GetCountParamInfos(long *plCount)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plCount)
		*plCount = m_aParamInfo.GetSize();
	return uecOK;
}

STDMETHODIMP CUCommand::GetCountParamData(long *plCount)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plCount)
		*plCount = m_aParamData.GetSize();
	return uecOK;
}

STDMETHODIMP CUCommand::AppendParamData(VARIANT vtParamData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(m_aParamInfo.GetSize() == 0)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	int nCol = (m_aParamData.GetSize()%m_aParamInfo.GetSize());
	unsigned short usDBType = m_aParamInfo[nCol].m_usDBType;
	if(vtParamData.vt == VT_EMPTY || vtParamData.vt == VT_NULL)
	{
		vtParamData.vt = VT_EMPTY;
		m_aParamData.Add(CComVariant(vtParamData));
	}
	else if(usDBType == VT_VARIANT)
	{
		if(vtParamData.vt == VT_NULL)
		{
			vtParamData.vt = VT_EMPTY;
		}
		m_aParamData.Add(CComVariant(vtParamData));
	}
	else if (usDBType == sdVT_BYTES) 
	{
		if(vtParamData.vt == (VT_ARRAY|VT_UI1))
		{
			CComVariant vtData(vtParamData);
			m_aParamData.Add(vtData);
			//silently data conversion
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
	}
	else if (usDBType == sdVT_STR || usDBType == sdVT_WSTR)	
	{
		if(vtParamData.vt == VT_BSTR)
		{
			CComVariant vtData(vtParamData);
			m_aParamData.Add(vtData);
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}	
	}
	else
	{
		if(usDBType == vtParamData.vt)
		{
			CComVariant vtData(vtParamData);
			m_aParamData.Add(vtData);
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
	}
	return uecOK;
}

STDMETHODIMP CUCommand::CleanParamData()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_aParamData.RemoveAll();
	return uecOK;
}

STDMETHODIMP CUCommand::GetOutputData(long nIndex, VARIANT *pvtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if((unsigned long)nIndex >= m_aOutput.GetSize())
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(pvtData)
	{
		::VariantClear(pvtData);
		::VariantCopy(pvtData, &(m_aOutput[nIndex]));
	}
	return (m_hr = uecOK);
}

STDMETHODIMP CUCommand::CleanParamInfo()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_aParamInfo.RemoveAll();
	return uecOK;
}

STDMETHODIMP CUCommand::SetParamData(long nIndex, VARIANT vtData)
{
	if(nIndex >= m_aParamData.GetSize())
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_aParamData[nIndex] = vtData;
	return (m_hr = uecOK);
}

STDMETHODIMP CUCommand::get_Handle(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulHandle;
	return uecOK;
}

STDMETHODIMP CUCommand::get_Property(VARIANT *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		::VariantClear(pVal);
		::VariantCopy(pVal, &m_vtProperty);
	}
	return uecOK;
}

STDMETHODIMP CUCommand::get_AffectedRows(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulAffectedRows;
	return uecOK;
}

STDMETHODIMP CUCommand::get_ParentHandle(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulParentHandle;
	return uecOK;
}

STDMETHODIMP CUCommand::put_ParentHandle(long newVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_ulParentHandle = newVal;
	return uecOK;
}

STDMETHODIMP CUCommand::ShrinkMemory(long lNewSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.ReallocBuffer(lNewSize);
	return uecOK;
}

STDMETHODIMP CUCommand::get_CreatedObjectHandle(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulCreatedObject;
	return uecOK;
}

void CUCommand::HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer)
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
			m_UQueue.Push(&c, 1); //null-terminate string
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
	switch(usRequestID)
	{
	case idCmndClose:
	case idCmndCancel:
	case idCmndSetProperty:
	case idCmndPrepare:
	case idCmndUnprepare:
	case idCmndUseStorageObjectForBLOB:
	case idCmndReleaseCreatedObject:
		break;
	case idCmndDoBatch:
		{
			m_UQueue.Pop(&m_ulCreatedObject);
			m_UQueue.Pop(&m_ulAffectedRows);
		}
		break;
	case idCmndExecuteSQL:
		{
			m_UQueue.Pop(&m_ulCreatedObject);
			m_UQueue.Pop(&m_ulAffectedRows);
		}
		break;
	case idCmndGetProperty:
		{
			m_vtProperty.Clear();
			m_UQueue.PopVT(m_vtProperty);
		}
		break;
	case idCmndOpen:
	case idCmndOpenFromHandle:
		{
			m_UQueue.Pop(&m_ulHandle);
			m_UQueue.Pop(&m_ulParentHandle);
		}
		break;
	case idCmndGetOutputParams:
		{
			CComVariant	vtOutput;
			m_aOutput.RemoveAll();
			while(m_UQueue.GetSize())
			{
				m_UQueue.PopVT(vtOutput);
				m_aOutput.Add(vtOutput);
				vtOutput.Clear();	
			}
		}
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}
	ATLASSERT(m_UQueue.GetSize() == 0);
}

void CUCommand::PackAllParamData(bool bOneSetOnly)
{
	int n;
	int nCol;
	unsigned short usDBType;
	int nCols = m_aParamInfo.GetSize();
	int nCount = m_aParamData.GetSize();
	for(n=0; n<nCount; n++)
	{
		nCol = (n%nCols);
		if(bOneSetOnly && nCol== 0 && n)
			break;
		usDBType = m_aParamInfo[nCol].m_usDBType;
		if(usDBType == sdVT_BYTES)
		{
			if(m_aParamInfo[nCol].m_ulLen>(0xFFFF-1)) 
			{
				unsigned long ulLen;
				if(m_aParamData[n].vt == (VT_ARRAY|VT_UI1))
				{
					BYTE *pData = NULL;
					ulLen = m_aParamData[n].parray->rgsabound[0].cElements;
					m_UQueue.Push(&ulLen);
					::SafeArrayAccessData(m_aParamData[n].parray, (void**)&pData);
					m_UQueue.Push(pData, ulLen);
					::SafeArrayUnaccessData(m_aParamData[n].parray);
				}
				else
				{
					ulLen = (~0);
					m_UQueue.Push(&ulLen);
				}
			}
			else if(m_aParamInfo[nCol].m_ulLen>(0xFF-1))
			{
				unsigned short usLen;
				if(m_aParamData[n].vt == (VT_ARRAY|VT_UI1))
				{
					BYTE *pData = NULL;
					usLen = (unsigned short)m_aParamData[n].parray->rgsabound[0].cElements;
					m_UQueue.Push(&usLen, 1);
					::SafeArrayAccessData(m_aParamData[n].parray, (void**)&pData);
					m_UQueue.Push(pData, usLen);
					::SafeArrayUnaccessData(m_aParamData[n].parray);
				}
				else
				{
					usLen = (~0);
					m_UQueue.Push(&usLen, 1);
				}
			}
			else
			{
				BYTE bLen;
				if(m_aParamData[n].vt == (VT_ARRAY|VT_UI1))
				{
					BYTE *pData = NULL;
					bLen = (BYTE)m_aParamData[n].parray->rgsabound[0].cElements;
					m_UQueue.Push(&bLen, 1);
					::SafeArrayAccessData(m_aParamData[n].parray, (void**)&pData);
					m_UQueue.Push(pData, bLen);
					::SafeArrayUnaccessData(m_aParamData[n].parray);
				}
				else
				{
					bLen = (~0);
					m_UQueue.Push(&bLen, 1);
				}
			}
		}
		else if(usDBType == sdVT_WSTR)
		{
			if(m_aParamInfo[nCol].m_ulLen>(0xFFFF-1)) 
			{
				unsigned long ulLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					ulLen = ::SysStringLen(m_aParamData[n].bstrVal)*sizeof(WCHAR);
					m_UQueue.Push(&ulLen);
					m_UQueue.Push((BYTE*)m_aParamData[n].bstrVal, ulLen);
				}
				else
				{
					ulLen = (~0);
					m_UQueue.Push(&ulLen);
				}
			}
			else if(m_aParamInfo[nCol].m_ulLen>(0xFF-1))
			{
				unsigned short usLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					usLen = ::SysStringLen(m_aParamData[n].bstrVal)*sizeof(WCHAR);
					m_UQueue.Push(&usLen, 1);
					m_UQueue.Push((BYTE*)m_aParamData[n].bstrVal, usLen);
				}
				else
				{
					usLen = (~0);
					m_UQueue.Push(&usLen, 1);
				}
			}
			else
			{
				BYTE bLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					if(::SysStringLen(m_aParamData[n].bstrVal)*sizeof(WCHAR) > 254)
					{
						bLen = 254; //truncated
					}
					else
					{
						bLen = ::SysStringLen(m_aParamData[n].bstrVal)*sizeof(WCHAR);
					}
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)m_aParamData[n].bstrVal, bLen);
				}
				else
				{
					bLen = (~0);
					m_UQueue.Push(&bLen, 1);
				}
			}
		}
		else if(usDBType == sdVT_STR)
		{
			if(m_aParamInfo[nCol].m_ulLen>(0xFFFF-1)) 
			{
				unsigned long ulLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					ulLen = ::SysStringLen(m_aParamData[n].bstrVal);
					m_UQueue.Push(&ulLen);
					char *strData = new char[ulLen + 1];
					strData[ulLen] = 0;
					::WideCharToMultiByte(CP_ACP, 0, m_aParamData[n].bstrVal, ulLen, strData, ulLen, NULL, NULL); 
					m_UQueue.Push((BYTE*)strData, ulLen);
					delete []strData;
				}
				else
				{
					ulLen = (~0);
					m_UQueue.Push(&ulLen);
				}
				
			}
			else if(m_aParamInfo[nCol].m_ulLen>(0xFF-1))
			{
				unsigned short usLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					usLen = ::SysStringLen(m_aParamData[n].bstrVal);
					m_UQueue.Push(&usLen, 1);
					char *strData = new char[usLen + 1];
					strData[usLen] = 0;
					::WideCharToMultiByte(CP_ACP, 0, m_aParamData[n].bstrVal, usLen, strData, usLen, NULL, NULL); 
					m_UQueue.Push((BYTE*)strData, usLen);
					delete []strData;
				}
				else
				{
					usLen = (~0);
					m_UQueue.Push(&usLen, 1);
				}
			}
			else
			{
				BYTE bLen;
				if(m_aParamData[n].vt == VT_BSTR)
				{
					USES_CONVERSION;
					bLen = ::SysStringLen(m_aParamData[n].bstrVal);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push(OLE2A(m_aParamData[n].bstrVal), bLen);
				}
				else
				{
					bLen = (~0);
					m_UQueue.Push(&bLen, 1);
				}
			}
		}
		else
		{
			BYTE bLen;
			if(m_aParamData[n].vt == VT_EMPTY)
			{
				bLen = (~0);
				m_UQueue.Push(&bLen, 1);
			}
			else
			{
				switch(usDBType)
				{
				case sdVT_I2:
					bLen = sizeof(short);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].iVal, bLen);
					break;
				case sdVT_I4:
					bLen = sizeof(long);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].lVal, bLen);
					break;
				case sdVT_R4:
					bLen = sizeof(float);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].fltVal, bLen);
					break;
				case sdVT_R8:
					bLen = sizeof(double);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].dblVal, bLen);
					break;
				case sdVT_CY:
					bLen = sizeof(CY);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].cyVal, bLen);
					break;
				case sdVT_DATE:
					bLen = sizeof(DATE);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].date, bLen);
					break;
				case sdVT_BOOL:
					bLen = sizeof(VARIANT_BOOL);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].boolVal, bLen);
					break;
				case sdVT_VARIANT:
					bLen = sizeof(VARIANT);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.PushVT(m_aParamData[n]);
					break;
				case sdVT_DECIMAL:
					bLen = sizeof(DECIMAL);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].decVal, bLen);
					break;
				case sdVT_I1:
					bLen = sizeof(char);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].cVal, bLen);
					break;
				case sdVT_UI1:
					bLen = sizeof(unsigned char);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].bVal, bLen);
					break;
				case sdVT_UI2:
					bLen = sizeof(unsigned short);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].uiVal, bLen);
					break;
				case sdVT_UI4:
					bLen = sizeof(unsigned long);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].ulVal, bLen);
					break;
#ifndef _WIN32_WCE
				case sdVT_I8:
					bLen = sizeof(LONGLONG);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].llVal, bLen);
					break;
				case sdVT_UI8:
					bLen = sizeof(ULONGLONG);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].ullVal, bLen);
					break;
#else
				case sdVT_I8:
					bLen = sizeof(LONGLONG);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].lVal, bLen);
					break;
				case sdVT_UI8:
					bLen = sizeof(ULONGLONG);
					m_UQueue.Push(&bLen, 1);
					m_UQueue.Push((BYTE*)&m_aParamData[n].ulVal, bLen);
					break;
#endif
				default:
					ATLASSERT(FALSE);
					break;
				}
			}
		}
	}
}

STDMETHODIMP CUCommand::UseStorageObjectForBLOB(VARIANT_BOOL bStorageObjectForBLOB)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndUseStorageObjectForBLOB, sizeof(bStorageObjectForBLOB), (BYTE*)&bStorageObjectForBLOB);
}

STDMETHODIMP CUCommand::GetCountOutputData(long *plCount)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plCount)
		*plCount = m_aOutput.GetSize();

	return S_OK;
}

STDMETHODIMP CUCommand::ReleaseCreatedObject()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCmndReleaseCreatedObject, 0, NULL);
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
