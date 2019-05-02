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

// URowset.cpp : Implementation of CURowset
#include "stdafx.h"
#include "UDB.h"
#include "URowset.h"
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
// CURowset

STDMETHODIMP CURowset::AttachSocket(IUnknown *pIUnknownToUSocket)
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

STDMETHODIMP CURowset::get_Rtn(long *plResult)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plResult)
		*plResult = m_hr;
	return S_OK;
}

STDMETHODIMP CURowset::get_ErrorMsg(BSTR *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(*pVal)
			::SysFreeString(*pVal);
		switch(m_hr)
		{
		case DB_S_ENDOFROWSET:
			*pVal = ::SysAllocString(L"End of rowset reached.");
			break;
		case uecUnexpected: 
			*pVal = ::SysAllocString(L"An unexpected error occurred.");
			break;
		case uecNotImplemented:
			*pVal = ::SysAllocString(L"Not implemented yet.");
			break;
		case dbeFail:
			*pVal = ::SysAllocString(L"Failed in getting data.");
			break;
		case dbeDataTypeError:
			*pVal = ::SysAllocString(L"Wrong data type or unable to convert data implicitly.");
			break;
		case dbeLengthTooShort:
			*pVal = ::SysAllocString(L"Receiving string or byte buffer too small.");
			break;
		default:
			*pVal = m_bstrErrorMsg.Copy();
			break;
		}
	}
	return S_OK;
}

HRESULT __stdcall CURowset::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnSocketClosed(long hSocket, long lError)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_bSwitched = false;
	DestroyFields();
	m_aColName.RemoveAll();
	m_usBatchSize = 0;
	m_aColMeta.RemoveAll();
	m_Bookmark.SetSize(0);
	m_bReadOnly = false;
	m_bDelayUpdate = false;
	m_bNoDelay = false;
	m_usRecordsFetched = 0;
	m_UQueue.Empty();
	m_ulHandle = 0;
	m_ulParentHandle = 0;
	m_vtProperty.Clear();
	m_ulBLOBCount = 0;
	m_bOpen = false;
	return S_OK;
}

HRESULT __stdcall CURowset::OnSocketConnected(long hSocket, long lError)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnConnecting(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnSendingData(long hSocket, long lError, long lSent)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CURowset::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
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
				m_ulBLOBCount = 0;
				m_aColName.RemoveAll();
				m_ulHandle = 0;
				m_ulParentHandle = 0;
				m_Bookmark.SetSize(0);
				m_bReadOnly = false;
				m_bDelayUpdate = false;
				m_bNoDelay = false;
				m_vtProperty.Clear();
				m_bOpen = false;
			}
		}
		break;
	case idRowsetOpenFromHandle:
	case idRowsetGetSchemaRowset:
	case idRowsetGetProviders:
	case idRowsetOpen:
	case idRowsetClose:
		{
			CAutoLock AutoLock(&m_cs.m_sec);
			m_ulBLOBCount = 0;
			m_aColName.RemoveAll();
			m_ulHandle = 0;
			m_ulParentHandle = 0;
			m_Bookmark.SetSize(0);
			m_bReadOnly = false;
			m_bDelayUpdate = false;
			m_bNoDelay = false;
			m_vtProperty.Clear();
			m_bOpen = false;
		}
	case idRowsetMoveFirst:
	case idRowsetMoveLast:
	case idRowsetMovePrev:
	case idRowsetMoveNext:
	case idRowsetStartFetchingBatch:
		{
			CAutoLock AutoLock(&m_cs.m_sec);
			m_usRecordsFetched = 0;
			m_usWhatBatch = 0;
			m_ulBLOBIndex = 0;
		}
	case idRowsetGetBatchRecordsLast:
	case idRowsetGetBatchRecordsEx:
	case idRowsetAsynFetch:
	case idRowsetGetBatchRecords:
	case idRowsetSendSubBatch:
	case idRowsetUpdate:
	case idRowsetUpdateBatch:
	case idRowsetDelete:
	case idRowsetSetDataType:
	case idRowsetBookmark:
	case idRowsetUndo:
	case idRowsetGetRowsAt:
	case idRowsetGetProperty:
	case idRowsetUseStorageObjectForBLOB:
		{
			CAutoLock AutoLock(&m_cs.m_sec);
			m_hr = S_OK;
			HandleRtn(nRequestID, lLen, lLenInBuffer);
			Fire_OnRequestProcessed(hSocket, nRequestID, 0, 0, rfCompleted);
		//version 4.3.0.2
	//		m_hr = S_OK;
//			ATLTRACE("nRequestID = %d\n", nRequestID);
		}
		break;
	case idRowsetSendBLOB:
		{
			CAutoLock AutoLock(&m_cs.m_sec);
			m_hr = S_OK;
			HandleRtn(nRequestID, lLen, lLenInBuffer);
			m_ulBLOBIndex++;
			ATLTRACE("usRow = %d, BlobCol = %d\n", (m_ulBLOBIndex-1)/m_ulBLOBCount, m_ulBLOBCol);
			Fire_OnRequestProcessed(hSocket, nRequestID, (m_ulBLOBIndex-1)/m_ulBLOBCount, m_ulBLOBCol, rfCompleted);
		//version 4.3.0.2
	//		m_hr = S_OK;
		}
		break;
	case idRowsetAdd:
		{
			m_usRowAdded = (~0);
			CAutoLock AutoLock(&m_cs.m_sec);
			m_hr = S_OK;
			HandleRtn(nRequestID, lLen, lLenInBuffer);
			if(m_usRowAdded != 0xFFFF)
			{
				m_ulBLOBIndex = 0;
			}
			Fire_OnRequestProcessed(hSocket, nRequestID, m_usRowAdded, 0, rfCompleted);
			//version 4.3.0.2
//			m_hr = S_OK;
		}
		break;
	default:
		break;
	}
	return S_OK;
}

void CURowset::HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer)
{
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
			m_UQueue.SetSize(0);
		}
		return;
	}
	switch (usRequestID)
	{
	case idRowsetStartFetchingBatch:
		{
			m_ulBLOBIndex = 0;
			ATLASSERT(m_UQueue.GetSize() == sizeof(m_usWhatBatch));
			m_UQueue.Pop(&m_usWhatBatch);
		}
		break;
	case idRowsetMoveFirst:
	case idRowsetMoveLast:
	case idRowsetMovePrev:
	case idRowsetMoveNext:
		if(m_UQueue.GetSize())
		{
			RetrieveOneRecord();
		}
		break;
	case idRowsetSendSubBatch:
		while(m_UQueue.GetSize())
		{
			RetrieveOneRecord();
		}
		break;
	case idRowsetGetBatchRecordsLast:
	case idRowsetGetBatchRecordsEx:
	case idRowsetAsynFetch:
	case idRowsetGetRowsAt:
	case idRowsetGetBatchRecords:
		if(m_hr == DB_S_ENDOFROWSET)
		{
			m_usRecordsFetched = 0;
		}
		break;
	case idRowsetSendBLOB:
		{
			HandleBLOBIn();
		}
		break;
	case idRowsetUpdate:
		break;
	case idRowsetDelete:
		break;
	case idRowsetAdd:
		if(m_UQueue.GetSize())
		{
			ATLASSERT(m_UQueue.GetSize()>sizeof(m_usRowAdded));
			m_UQueue.Pop(&m_usRowAdded);
			ATLASSERT(m_usRowAdded<m_usBatchSize);
			UpdateOneRecordData(m_usRowAdded);
		}
		break;
	case idRowsetSetDataType:
		if(m_UQueue.GetSize())
		{
			unsigned short usRow;
			ULONG ulCol;
			ULONG ulCols;
			unsigned short usDBType;
			ULONG ulMaxLen;
			ATLASSERT(m_UQueue.GetSize() == sizeof(ulCol)+sizeof(usDBType)+sizeof(ulMaxLen));
			m_UQueue.Pop(&ulCol);
			m_UQueue.Pop(&usDBType);
			m_UQueue.Pop(&ulMaxLen);
			m_aColMeta[ulCol-1].m_usBoundDBType = usDBType;
			m_aColMeta[ulCol-1].m_ulBoundMaxLen = ulMaxLen;
			ATLASSERT(m_aColMeta[ulCol-1].m_ulOrdinal == ulCol);
			ULONG ulByte = StatusBytes();
			if(m_aField[0])
				delete (m_aField[0]);
			if(ulByte)
			{
				CField *pField = new CField;
				pField->m_pData = (BYTE*)::malloc(ulByte);
				memset(pField->m_pData, 0, ulByte);
				pField->m_ulLen = ulByte;
				m_aField[0] = pField;
			}
			else
			{
				m_aField[0] = NULL;
			}
			ulCols = m_aColName.GetSize();
			for(usRow=0; usRow<m_usBatchSize; usRow++)
			{
				m_aField[usRow*(ulCols+1)+ulCol]->m_pData = (BYTE*)::realloc(m_aField[usRow*(ulCols+1)+ulCol]->m_pData, ulMaxLen);
				m_aField[usRow*(ulCols+1)+ulCol]->m_ulLen = (~0);
				m_aField[usRow*(ulCols+1)+ulCol]->m_bModified = false;
				if(usDBType == sdVT_VARIANT)
					m_aField[usRow*(ulCols+1)+ulCol]->m_bVT = true;
				else
					m_aField[usRow*(ulCols+1)+ulCol]->m_bVT = false;
			}
		}
		break;
	case idRowsetBookmark:
		m_Bookmark.SetSize(0);
		if(m_UQueue.GetSize())
		{
			m_Bookmark.Push(m_UQueue.GetBuffer(), m_UQueue.GetSize());
			m_UQueue.SetSize(0);
		}
		break;
	case idRowsetOpenFromHandle:
	case idRowsetOpen:
	case idRowsetGetSchemaRowset:
	case idRowsetGetProviders:
		{
			CSimpleArray<CColMeta>		aColMeta;
			unsigned short usBatchSize = m_usBatchSize;
			aColMeta.m_aT = m_aColMeta.m_aT;
			aColMeta.m_nAllocSize = m_aColMeta.m_nAllocSize;
			aColMeta.m_nSize = m_aColMeta.m_nSize;
			m_aColMeta.m_aT = NULL;
			m_aColMeta.m_nAllocSize = 0;
			m_aColMeta.m_nSize = 0;
			bool bDestroy = true;
			m_UQueue.Pop(&m_ulHandle);
			m_UQueue.Pop(&m_ulParentHandle);
			m_bOpen = true;
			RetrieveRowsetMetaData();
			m_UQueue.Pop(&m_usBatchSize);
			m_UQueue.Pop(&m_bReadOnly);
			m_UQueue.Pop(&m_bDelayUpdate);
			ATLASSERT(m_UQueue.GetSize()==1);
			m_UQueue.Pop(&m_bNoDelay);
			StatusBytes();
			
			if(m_usBatchSize == usBatchSize && aColMeta.GetSize() == m_aColMeta.GetSize() && ::memcmp(aColMeta.m_aT, m_aColMeta.m_aT, aColMeta.GetSize()*sizeof(CColMeta)) == 0)
			{
				//if batch size and rowset meta data are not changed, simply reuse previous data fields	
			}
			else 
			{
				//if batch size and rowset meta data are changed, detroy previous fields and create new ones instead
				DestroyFields();
				SetupFields();
			}
		}
		break;
	case idRowsetGetProperty:
		{
			m_vtProperty.Clear();
			m_UQueue.PopVT(m_vtProperty);
		}
		break;
	case idRowsetUseStorageObjectForBLOB:
	case idRowsetClose:
	case idRowsetUndo:
	case idRowsetUpdateBatch:
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}
	ATLASSERT(m_UQueue.GetSize() == 0);
}

STDMETHODIMP CURowset::Close()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetClose, 0, NULL);
}

STDMETHODIMP CURowset::Open(BSTR bstrTableName, short sCursorType, long lHint, short nBatchSize, VARIANT_BOOL bNoDelay)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&m_ulParentHandle);
	m_UQueue.Push(&sCursorType);
	m_UQueue.Push(&lHint);
	m_UQueue.Push(&nBatchSize);
	m_UQueue.Push(&bNoDelay);
	m_UQueue.Push(bstrTableName);
	return m_pIUFast->SendRequestEx(idRowsetOpen, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::MoveFirst()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetMoveFirst, 0, NULL);
}

STDMETHODIMP CURowset::MoveLast()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetMoveLast, 0, NULL);
}

STDMETHODIMP CURowset::MovePrev()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetMovePrev, 0, NULL);
}

STDMETHODIMP CURowset::MoveNext(long lSkip)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetMoveNext, sizeof(lSkip), (BYTE*)&lSkip);
}

STDMETHODIMP CURowset::Delete(short nRow)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || !m_bOpen || m_usWhatBatch==idRowsetAsynFetch || m_aColName.GetSize()==0 || m_usRecordsFetched==0 || (unsigned short)nRow>=m_usBatchSize) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_bReadOnly)
	{
		m_hr = uecReadOnly;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetDelete, sizeof(nRow), (BYTE*)&nRow);
}

STDMETHODIMP CURowset::Add(VARIANT_BOOL bNeedNewRecord, short nRowIndex)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || !m_bOpen || m_aColName.GetSize()==0 || m_usWhatBatch==idRowsetAsynFetch || (unsigned short)nRowIndex>= m_usBatchSize) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_bReadOnly)
	{
		m_hr = uecReadOnly;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&bNeedNewRecord);
	m_UQueue.Push(&nRowIndex);
	PackOneRow(nRowIndex);
	return m_pIUFast->SendRequestEx(idRowsetAdd, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());	
}

STDMETHODIMP CURowset::Update(short nRow)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_bOpen || !m_pIUFast.p || m_usWhatBatch==idRowsetAsynFetch || m_aColName.GetSize()==0 || m_usRecordsFetched==0 || (unsigned short)nRow>=m_usRecordsFetched) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_bReadOnly)
	{
		m_hr = uecReadOnly;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	UpdateOneRow(nRow);
	m_UQueue.Insert(&nRow);
	return m_pIUFast->SendRequestEx(idRowsetUpdate, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());	
}

STDMETHODIMP CURowset::UpdateBatch()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_bOpen || !m_pIUFast.p || !m_bDelayUpdate || m_usWhatBatch==idRowsetAsynFetch || m_aColName.GetSize()==0) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_bReadOnly)
	{
		m_hr = uecReadOnly;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetUpdateBatch, 0, NULL);
}

STDMETHODIMP CURowset::OpenFromHandle(long lHandle, VARIANT_BOOL bKeepPrevRowset)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lHandle);
	m_UQueue.Push(&bKeepPrevRowset);
	return m_pIUFast->SendRequestEx(idRowsetOpenFromHandle, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());	
}

STDMETHODIMP CURowset::Bookmark(short sRow)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || (unsigned short)sRow>=m_usRecordsFetched) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetBookmark, sizeof(sRow), (BYTE*)&sRow);
}

STDMETHODIMP CURowset::AsynFetch(VARIANT_BOOL bFromBeginning, short sBatchSize, long lRows)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || lRows == 0) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&bFromBeginning);
	m_UQueue.Push(&sBatchSize);
	m_UQueue.Push(&lRows);
	return m_pIUFast->SendRequestEx(idRowsetAsynFetch, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::GetBatchRecords(short sSubBatchSize, VARIANT_BOOL bFirst)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&sSubBatchSize);
	m_UQueue.Push(&bFirst);
	return m_pIUFast->SendRequestEx(idRowsetGetBatchRecords, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::GetProviders(VARIANT_BOOL bKeepPrevRowset)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetGetProviders, sizeof(bKeepPrevRowset), (BYTE*)&bKeepPrevRowset);
}

STDMETHODIMP CURowset::Undo()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_bOpen || !m_pIUFast.p || !m_bDelayUpdate || m_usWhatBatch==idRowsetAsynFetch || m_aColName.GetSize()==0) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_bReadOnly)
	{
		m_hr = uecReadOnly;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetUndo, 0, NULL);
}

STDMETHODIMP CURowset::GetSchemaRowset(BSTR bstrSchemaGUID, VARIANT vtRestrictions, short nBatchSize, VARIANT_BOOL bKeepPrevRowset)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.PushVT(vtRestrictions);
	m_UQueue.Push(&nBatchSize);
	m_UQueue.Push(&bKeepPrevRowset);
	m_UQueue.Push(bstrSchemaGUID);
	return m_pIUFast->SendRequestEx(idRowsetGetSchemaRowset, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::GetProperty(long lPropID, BSTR bstrPropSet)
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
	return m_pIUFast->SendRequestEx(idRowsetGetProperty, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::SetDataType(long lCol, short nDBType, long lLen)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || !IsDataTypeSupported(nDBType)) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lCol);
	m_UQueue.Push(&nDBType);
	if(lLen == 0)
	{
		if(nDBType == sdVT_WSTR)
		{
			lLen = 512;
		}
		else if(nDBType == sdVT_STR)
		{
			lLen = 256;
		}
		else if(nDBType == sdVT_BYTES)
		{
			lLen = 255;
		}
		else
		{
			lLen = 0;
		}
	}
	m_UQueue.Push(&lLen);
	return m_pIUFast->SendRequestEx(idRowsetSetDataType, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::GetRowsAt(VARIANT vtBookmarkValue, long lRowsOffset, short sSubBatchSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || m_Bookmark.GetSize()==0 || !m_bOpen) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&lRowsOffset);
	m_UQueue.Push(&sSubBatchSize);
	if(vtBookmarkValue.vt == VT_NULL || vtBookmarkValue.vt == VT_EMPTY)
	{
		m_UQueue.Push(m_Bookmark.GetBuffer(), m_Bookmark.GetSize());
	}
	else if(vtBookmarkValue.vt == (VT_UI1|VT_ARRAY) || vtBookmarkValue.vt == (VT_I1|VT_ARRAY))
	{
		BYTE *pData;
		::SafeArrayAccessData(vtBookmarkValue.parray, (void**)&pData);
		m_UQueue.Push(pData, vtBookmarkValue.parray->rgsabound[0].cElements);
		::SafeArrayUnaccessData(vtBookmarkValue.parray);
	}
	else
	{
		m_UQueue.SetSize(0);
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetGetRowsAt, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::UseStorageObjectForBLOB(VARIANT_BOOL bUseStorageObject)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetUseStorageObjectForBLOB, sizeof(bUseStorageObject), (BYTE*)&bUseStorageObject);
}

STDMETHODIMP CURowset::GetData(short sRow, long lCol, VARIANT *pvtData)
{
	unsigned short usRow = (unsigned short)sRow;
	ULONG ulCol = (ULONG)lCol;
	if(pvtData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		if(pvtData->vt != VT_EMPTY)
		{
			try
			{
				::VariantClear(pvtData);
			}
			catch(...)
			{
			}
		}
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I2:
				{
					pvtData->vt = VT_I2;
					pvtData->iVal = *((short*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_I4:
				{
					pvtData->vt = VT_I4;
					pvtData->lVal = *((long*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_R4:
				{
					pvtData->vt = VT_R4;
					pvtData->fltVal = *((float*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_R8:
				{
					pvtData->vt = VT_R8;
					pvtData->dblVal = *((double*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_CY:
				{
					pvtData->vt = VT_CY;
					pvtData->cyVal = *((CY*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_DATE:
				{
					pvtData->vt = VT_DATE;
					pvtData->date = *((DATE*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_BOOL:
				{
					pvtData->vt = VT_BOOL;
					pvtData->boolVal = *((VARIANT_BOOL*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_VARIANT:
				::VariantCopy(pvtData, (VARIANT*)(m_aField[ulPos]->m_pData));
				break;
			case sdVT_DECIMAL:
				{
					pvtData->vt = VT_DECIMAL;
					pvtData->decVal = *((DECIMAL*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_I1:
				{
					pvtData->vt = VT_I1;
					pvtData->cVal = *((char*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_UI1:
				{
					pvtData->vt = VT_UI1;
					pvtData->cVal = *((BYTE*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_UI2:
				{
					pvtData->vt = VT_UI2;
					pvtData->uiVal = *((unsigned short*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_UI4:
				{
					pvtData->vt = VT_UI4;
					pvtData->ulVal = *((unsigned long*)m_aField[ulPos]->m_pData);
				}
				break;
#ifndef _WIN32_WCE
			case sdVT_I8:
				{
					pvtData->vt = VT_I8;
					pvtData->llVal = *((LONGLONG*)m_aField[ulPos]->m_pData);
				}
				break;
			case sdVT_UI8:
				{
					pvtData->vt = VT_UI8;
					pvtData->ullVal = *((ULONGLONG*)m_aField[ulPos]->m_pData);
				}
				break;
#else
			case sdVT_I8:
				{
					pvtData->vt = VT_I8;
					memcpy(&(pvtData->lVal),  m_aField[ulPos]->m_pData, sizeof(LONGLONG));
				}
				break;
			case sdVT_UI8:
				{
					pvtData->vt = VT_UI8;
					memcpy(&(pvtData->ulVal),  m_aField[ulPos]->m_pData, sizeof(ULONGLONG));
				}
				break;
#endif
			case sdVT_BYTES:
				{
					if(!m_bNoDelay && IsBLOB(lCol-1) && (sRow*m_ulBLOBCount >= m_ulBLOBIndex) )
					{
						m_hr = dbeFail;
						return m_hr;
					}
					void *pData;
					SAFEARRAYBOUND sab[1] = {m_aField[ulPos]->m_ulLen, 0};
					pvtData->vt = (VT_ARRAY|VT_UI1);
					pvtData->parray = ::SafeArrayCreate(VT_UI1, 1, sab);
					::SafeArrayAccessData(pvtData->parray, &pData);
					memcpy(pData, m_aField[ulPos]->m_pData, sab[0].cElements);
					::SafeArrayUnaccessData(pvtData->parray);
				}
				break;
			case sdVT_STR:
				{
					if(!m_bNoDelay && IsBLOB(lCol-1) && (sRow*m_ulBLOBCount >= m_ulBLOBIndex) )
					{
						m_hr = dbeFail;
						return m_hr;
					}
					pvtData->vt = VT_BSTR;
					pvtData->bstrVal = ::SysAllocStringLen(NULL, m_aField[ulPos]->m_ulLen);
					MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(m_aField[ulPos]->m_pData),
						m_aField[ulPos]->m_ulLen, pvtData->bstrVal, m_aField[ulPos]->m_ulLen);
				}
				break;
			case sdVT_WSTR:
				{
					if(!m_bNoDelay && IsBLOB(lCol-1) && (sRow*m_ulBLOBCount >= m_ulBLOBIndex) )
					{
						m_hr = dbeFail;
						return m_hr;
					}
					pvtData->vt = VT_BSTR;
					pvtData->bstrVal = ::SysAllocStringLen((LPOLESTR)(m_aField[ulPos]->m_pData), m_aField[ulPos]->m_ulLen/sizeof(WCHAR));
				}
				break;
			default:
				ATLASSERT(FALSE);
				break;	
			}
		}
	}
	return S_OK;	
}

STDMETHODIMP CURowset::IsBLOB(long lCol, VARIANT_BOOL *pbBLOB)
{
	if(pbBLOB)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(lCol == 0 || (unsigned long)lCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*pbBLOB = IsBLOB(lCol-1) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetColName(long lCol, BSTR *pbstrColName)
{
	if(pbstrColName)
	{
		ULONG ulCol = (ULONG)lCol;
		CAutoLock AutoLock(&m_cs.m_sec);
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		if(*pbstrColName)
		{
			try
			{
				::SysFreeString(*pbstrColName);
			}
			catch(...)
			{
			}
		}
		*pbstrColName = m_aColName[ulCol-1].Copy();
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetDataType(long lCol, short *psDBType)
{
	if(psDBType)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulCol = (ULONG)lCol;
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*psDBType = m_aColMeta[ulCol-1].m_usBoundDBType;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetStatus(short sRow, long lCol, long *plStatus)
{
	if(plStatus)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if((unsigned short)sRow>=m_usRecordsFetched || lCol==0 || (unsigned long)lCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		if(m_aField[sRow*(m_aColName.GetSize()+1)+lCol]->m_ulLen != 0xFFFFFFFF)
		{
			*plStatus = DBSTATUS_S_OK; //DBSTATUS_S_OK
		}
		else
		{
			*plStatus = DBSTATUS_S_ISNULL; //DBSTATUS_S_ISNULL
		}
	}
	return S_OK;
}

STDMETHODIMP CURowset::FindColOrdinal(BSTR bstrColName, VARIANT_BOOL bCaseSensitive, long *plCol)
{
	if(plCol)
	{
		*plCol = -1;
		ULONG ul;
		CAutoLock AutoLock(&m_cs.m_sec);
		for(ul=0; ul<m_aColName.GetSize(); ul++)
		{
			if(bCaseSensitive)
			{
				if(m_aColName[ul] == bstrColName)
				{
					*plCol = ul+1;
					break;
				}
			}
			else
			{
				if(bstrColName && _wcsicmp(m_aColName[ul], bstrColName)==0)
				{
					*plCol = ul+1;
					break;
				}
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CURowset::SetData(short sRow, long lCol, VARIANT vtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_bOpen || (unsigned short)sRow>=m_usBatchSize || lCol==0 || (unsigned long)lCol>m_aColName.GetSize())
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	ULONG ulSize = m_aColName.GetSize()+1;
	m_aField[sRow*ulSize+lCol]->m_bModified = true;
	if(vtData.vt == VT_NULL || vtData.vt == VT_EMPTY)
	{
		if(!IsNullable(lCol-1))
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_aField[sRow*ulSize+lCol]->m_ulLen = (~0);
		return uecOK;
	}
	if(m_aColMeta[lCol-1].m_usBoundDBType == sdVT_VARIANT)
	{
		m_aField[sRow*ulSize+lCol]->m_ulLen = sizeof(VARIANT);
		::VariantClear((VARIANT*)(m_aField[sRow*ulSize+lCol]->m_pData));
		::VariantCopy((VARIANT*)(m_aField[sRow*ulSize+lCol]->m_pData), &vtData);
		m_hr = uecOK;
		return uecOK;
	}
	if(vtData.vt == VT_BSTR)
	{
		if(m_aColMeta[lCol-1].m_usBoundDBType == sdVT_WSTR)
		{
			ULONG ulLen = ::SysStringLen(vtData.bstrVal)*sizeof(WCHAR);
			if(IsBLOB(lCol-1))
			{
				m_aField[sRow*ulSize+lCol]->m_ulLen = ulLen;
				m_aField[sRow*ulSize+lCol]->m_pData = (BYTE*)::realloc(m_aField[sRow*ulSize+lCol]->m_pData, ulLen);
				memcpy(m_aField[sRow*ulSize+lCol]->m_pData, vtData.bstrVal, ulLen);
			}
			else
			{
				m_aField[sRow*ulSize+lCol]->m_ulLen = ulLen = (ulLen < m_aColMeta[lCol-1].m_ulBoundMaxLen) ? ulLen : (m_aColMeta[lCol-1].m_ulBoundMaxLen-sizeof(WCHAR));
				if(ulLen)
					memcpy(m_aField[sRow*ulSize+lCol]->m_pData, vtData.bstrVal, ulLen);
			}
		}
		else if(m_aColMeta[lCol-1].m_usBoundDBType == sdVT_STR)
		{
			ULONG ulLen = ::SysStringLen(vtData.bstrVal);
			if(IsBLOB(lCol-1))
			{
				m_aField[sRow*ulSize+lCol]->m_ulLen = ulLen;
				m_aField[sRow*ulSize+lCol]->m_pData = (BYTE*)::realloc(m_aField[sRow*ulSize+lCol]->m_pData, ulLen);
				WideCharToMultiByte(CP_ACP, 0, vtData.bstrVal, ulLen, (LPSTR)(m_aField[sRow*ulSize+lCol]->m_pData), ulLen, NULL, NULL);
			}
			else
			{
				m_aField[sRow*ulSize+lCol]->m_ulLen = ulLen = (ulLen < m_aColMeta[lCol-1].m_ulBoundMaxLen) ? ulLen : (m_aColMeta[lCol-1].m_ulBoundMaxLen-sizeof(char));
				if(ulLen)
					WideCharToMultiByte(CP_ACP, 0, vtData.bstrVal, ulLen, (LPSTR)(m_aField[sRow*ulSize+lCol]->m_pData), ulLen, NULL, NULL);
			}
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
	}
	else if(vtData.vt == (VT_ARRAY|VT_UI1) || vtData.vt == (VT_ARRAY|VT_I1))
	{
		if(m_aColMeta[lCol-1].m_usBoundDBType == sdVT_BYTES)
		{
			void *pData;
			ULONG ulLen = vtData.parray->rgsabound[0].cElements;
			if(IsBLOB(lCol-1))
			{
				m_aField[sRow*ulSize+lCol]->m_pData = (BYTE*)::realloc(m_aField[sRow*ulSize+lCol]->m_pData, ulLen);
			}
			else
			{
				ulLen = (ulLen < m_aColMeta[lCol-1].m_ulBoundMaxLen) ? ulLen : m_aColMeta[lCol-1].m_ulBoundMaxLen;
			}
			::SafeArrayAccessData(vtData.parray, &pData);
			memcpy(m_aField[sRow*ulSize+lCol]->m_pData, pData, ulLen);
			::SafeArrayUnaccessData(vtData.parray);
			m_aField[sRow*ulSize+lCol]->m_ulLen = ulLen;
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
	}
	else
	{
		DBTYPE dbType = m_aColMeta[lCol-1].m_usBoundDBType;
		if(vtData.vt == dbType)
		{
			BYTE bLen = (BYTE)m_aColMeta[lCol-1].m_ulBoundMaxLen;
			memcpy(m_aField[sRow*ulSize+lCol]->m_pData, &(vtData.bVal), bLen);
			m_aField[sRow*ulSize+lCol]->m_ulLen = bLen;
		}
		else
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
	}
	return S_OK;
}

STDMETHODIMP CURowset::IsEOF(VARIANT_BOOL *pbIsEOF)
{
	if(pbIsEOF)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pbIsEOF = m_usRecordsFetched ? VARIANT_FALSE : VARIANT_TRUE;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetRowsFetched(short *psCount)
{
	if(psCount)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*psCount = m_usRecordsFetched;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetCols(long *plCols)
{
	if(plCols)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*plCols = m_aColMeta.GetSize();
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetMaxLen(long lCol, long *plMaxLen)
{
	if(plMaxLen)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulCol = (ULONG)lCol;
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*plMaxLen = m_aColMeta[ulCol-1].m_ulBoundMaxLen;
	}
	return S_OK;
}

STDMETHODIMP CURowset::IsWritable(long lCol, VARIANT_BOOL *pbWritable)
{
	if(pbWritable)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(!m_bOpen || lCol==0 || (unsigned long)lCol>m_aColName.GetSize())
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*pbWritable = (m_aColMeta[lCol-1].m_ulFlags & (DBCOLUMNFLAGS_WRITE|DBCOLUMNFLAGS_WRITEUNKNOWN)) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetRawDataType(long lCol, short *psDataType)
{
	if(psDataType)
	{
		ULONG ulCol = (ULONG)lCol;
		CAutoLock AutoLock(&m_cs.m_sec);
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*psDataType = m_aColMeta[ulCol-1].m_usRawDBType;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetDataSize(short sRow, long lCol, long *plSize)
{
	if(plSize)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if((unsigned short)sRow>=m_usRecordsFetched || lCol==0 || (unsigned long)lCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*plSize = m_aField[sRow*(m_aColName.GetSize()+1)+lCol]->m_ulLen;
		if(*plSize != -1)
		{
			if(IsFixed(lCol-1))
			{
				*plSize = m_aColMeta[lCol-1].m_ulBoundMaxLen;
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetBatchSize(short *psBatchSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(psBatchSize)
		*psBatchSize = m_usBatchSize;
	return S_OK;
}

STDMETHODIMP CURowset::GetColFlags(long lCol, long *plFlags)
{
	if(plFlags)
	{
		ULONG ulCol = (ULONG)lCol;
		CAutoLock AutoLock(&m_cs.m_sec);
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*plFlags = m_aColMeta[ulCol-1].m_ulFlags;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetRawMaxColLen(long lCol, long *plLen)
{
	if(plLen)
	{
		ULONG ulCol = (ULONG)lCol;
		CAutoLock AutoLock(&m_cs.m_sec);
		if(ulCol == 0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*plLen = m_aColMeta[ulCol-1].m_ulRawMaxLen;
	}
	return S_OK;
}

STDMETHODIMP CURowset::ShrinkMemory(long lNewSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_UQueue.ReallocBuffer(lNewSize);
	return S_OK;
}

void CURowset::DestroyFields()
{
	int n;
	int nCount = m_aField.GetSize();
	for(n=0; n<nCount; n++)
	{
		CField *pField = m_aField[n];
		if(pField)
			delete pField;
	}
	m_aField.RemoveAll();
}

bool CURowset::IsFixed(ULONG ulCol)
{
	switch(m_aColMeta[ulCol].m_usBoundDBType)
	{
	case sdVT_BYTES:
	case sdVT_STR:
	case sdVT_WSTR:
		return false;
	default:
		return true;
	}
	return true;
}

bool CURowset::IsBLOB(ULONG ulCol)
{
	if((m_aColMeta[ulCol].m_ulFlags & DBCOLUMNFLAGS_ISLONG)==DBCOLUMNFLAGS_ISLONG || (m_aColMeta[ulCol].m_ulRawMaxLen+1)==0)
		return true;
	return false;
}

bool CURowset::IsNullable(ULONG ulCol)
{
	if((m_aColMeta[ulCol].m_ulFlags & DBCOLUMNFLAGS_ISNULLABLE) == DBCOLUMNFLAGS_ISNULLABLE);
		return true;
	return false;
}

unsigned long CURowset::StatusBytes()
{
	ULONG ulBits = 0;
	ULONG ul;
	ULONG ulSize = m_aColMeta.GetSize();
	for(ul=0; ul<ulSize; ul++)
	{
		CColMeta &ColMeta = m_aColMeta[ul];
		ColMeta.m_ulBytePos = (ulBits/8);
		ColMeta.m_bBitPos = (ulBits%8);
		ulBits++;
	}
	return ((ulBits+7)/8);
}

void CURowset::SetupFields()
{
	ULONG ulCols = m_aColMeta.GetSize();
	if(ulCols)
	{
		ULONG ulRow;
		ULONG ulByte = StatusBytes();
		for(ulRow=0; ulRow<m_usBatchSize; ulRow++)
		{
			ULONG ul;
			CField *pField = NULL;
			if(ulByte)
			{
				pField = new CField;
				pField->m_pData = (BYTE*)::malloc(ulByte);
				memset(pField->m_pData, 0, ulByte);
				pField->m_ulLen = ulByte;
			}
			m_aField.Add(pField);
			for(ul=0; ul<ulCols; ul++)
			{
				pField = new CField;
				CColMeta &ColMeta = m_aColMeta[ul];
				if(!IsBLOB(ul))
				{
					ATLASSERT(ColMeta.m_ulBoundMaxLen<0xFFFF);
					pField->m_pData = (BYTE*)::malloc(ColMeta.m_ulBoundMaxLen+2);
					memset(pField->m_pData, 0, ColMeta.m_ulBoundMaxLen+2);
					if(ColMeta.m_usBoundDBType == sdVT_VARIANT)
						pField->m_bVT = true;
				}
				m_aField.Add(pField);
			}
		}
	}
}

void CURowset::RetrieveRowsetMetaData()
{
	ULONG ul;
	unsigned short usLen;
	ULONG ulCols;
	CCellMeta	CellMeta;
	CColMeta	ColMeta;
	memset(&ColMeta, 0, sizeof(ColMeta));
	ColMeta.m_ulBytePos = (~0);
	ColMeta.m_bBitPos = (~0);
	m_UQueue.Pop(&ulCols);
	for(ul=0; ul<ulCols; ul++)
	{
		m_UQueue.Pop(&CellMeta);
		ColMeta.m_ulBoundMaxLen = CellMeta.m_ulBoundMaxLen;
		ColMeta.m_ulFlags = CellMeta.m_ulFlags;
		ColMeta.m_ulOrdinal = CellMeta.m_ulOrdinal;
		ColMeta.m_ulRawMaxLen = CellMeta.m_ulRawMaxLen;
		ColMeta.m_usBoundDBType = CellMeta.m_usBoundDBType;
		ColMeta.m_usRawDBType = CellMeta.m_usRawDBType;
		m_aColMeta.Add(ColMeta);
		if(IsBLOB(ul))
		{
			m_ulBLOBCount++;
		}
	}
	for(ul=0; ul<ulCols; ul++)
	{
		m_UQueue.Pop(&usLen);
		if(usLen)
		{
			CComBSTR bstrCol;
			bstrCol.m_str = ::SysAllocStringLen(NULL, usLen/sizeof(WCHAR));
			m_UQueue.Pop((BYTE*)bstrCol.m_str, usLen);
			m_aColName.Add(bstrCol);
		}
		else
		{
			m_aColName.Add(CComBSTR(L""));
		}
	}
}

void CURowset::RetrieveOneRecord()
{
	ULONG ul;
	ULONG ulPos;
	bool bStatus = false;
	ULONG ulCols = m_aColName.GetSize();
	ULONG ulHead = m_usRecordsFetched*(ulCols+1);
	BYTE bData = 0;
	CField	*pField;
	if(m_aField[ulHead] != NULL)
	{
		m_UQueue.Pop(m_aField[ulHead]->m_pData, m_aField[ulHead]->m_ulLen);
		bStatus = true;
	}
	for(ul=0; ul<ulCols; ul++)
	{
		ulPos = ulHead+ul+1;
//		ulPos = m_usRecordsFetched*(ulCols+1)+ul+1;
		pField = m_aField[ulPos];
		pField->m_bModified = false;
		if(bStatus)
		{
			bData = 1;
//			bData = (bData << m_aColMeta[ul].m_bBitPos);
//			bData = (bData & m_aField[ulHead]->m_pData[m_aColMeta[ul].m_ulBytePos]);
			bData <<= m_aColMeta[ul].m_bBitPos;
			bData &= m_aField[ulHead]->m_pData[m_aColMeta[ul].m_ulBytePos];
		}
		if(bData) //null
		{
			pField->m_ulLen = (~0);
			continue;
		}
		if(IsFixed(ul))
		{
			ATLASSERT(m_UQueue.GetSize()>=m_aColMeta[ul].m_ulBoundMaxLen);
			pField->m_ulLen = m_aColMeta[ul].m_ulBoundMaxLen;
			if(m_aColMeta[ul].m_usBoundDBType == sdVT_VARIANT)
			{
				ATLASSERT(m_aField[ulPos]->m_bVT);
				m_UQueue.PopVT(*((VARIANT*)pField->m_pData));
			}
			else
			{
				m_UQueue.Pop(pField->m_pData, m_aColMeta[ul].m_ulBoundMaxLen);
			}
		}
		else
		{
			if(m_aColMeta[ul].m_ulBoundMaxLen > (0xFFFF-1))
			{
				ULONG ulLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(ulLen));
				m_UQueue.Pop(&ulLen);
				pField->m_ulLen = ulLen;
				ATLASSERT(ulLen != 0xFFFFFFFF);
				ATLASSERT(m_UQueue.GetSize()>=ulLen);
				m_UQueue.Pop(pField->m_pData, ulLen);
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen > (0xFF-1))
			{
				unsigned short usLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(usLen));
				m_UQueue.Pop(&usLen);
				ATLASSERT(usLen != (0xFFFF));
				pField->m_ulLen = usLen;
				ATLASSERT(m_UQueue.GetSize()>=usLen);
				m_UQueue.Pop(pField->m_pData, usLen);	
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen)
			{
				unsigned char bLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(bLen));
				m_UQueue.Pop(&bLen, 1);
				ATLASSERT(bLen != 0xFF);
				ATLASSERT(m_UQueue.GetSize()>=bLen);
				pField->m_ulLen = bLen;
				m_UQueue.Pop(pField->m_pData, bLen);
			}
			else //BLOB == 0
			{
				ULONG ulLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(ulLen));
				m_UQueue.Pop(&ulLen);
				pField->m_ulLen = ulLen;
				ATLASSERT(ulLen != 0xFFFFFFFF);
				if(m_bNoDelay)
				{
					ATLASSERT(m_UQueue.GetSize()>=ulLen);
					pField->m_pData = (BYTE*)::realloc(pField->m_pData, ulLen);
					if(ulLen)
					{
						m_UQueue.Pop(pField->m_pData, ulLen);
					}
				}
			}
		}
	}
	m_usRecordsFetched++;
}

void CURowset::HandleBLOBIn()
{
	ULONG ulCol;
	ULONG ulCols;
	ATLASSERT(!m_bNoDelay);
	unsigned short usRow = m_ulBLOBIndex/m_ulBLOBCount;
	ULONG ulBlobCol = (m_ulBLOBIndex%m_ulBLOBCount);
	ulCols = m_aColName.GetSize();
	for(ulCol=0; ulCol<ulCols; ulCol++)
	{
		if(IsBLOB(ulCol))
		{
			if(ulBlobCol == 0)
			{
				m_ulBLOBCol = ulCol + 1;
				if(m_aField[usRow*(ulCols+1)+ulCol+1]->m_ulLen != 0xFFFFFFFF)
				{
					m_aField[usRow*(ulCols+1)+ulCol+1]->m_pData = (BYTE*)::realloc(m_aField[usRow*(ulCols+1)+ulCol+1]->m_pData, m_UQueue.GetSize());
					m_aField[usRow*(ulCols+1)+ulCol+1]->m_bModified = false;

/*
#ifdef _DEBUG
					if(m_UQueue.GetSize())
						ATLASSERT(m_aField[usRow*(ulCols+1)+ulCol+1]->m_ulLen == m_UQueue.GetSize());
#endif
*/
					m_aField[usRow*(ulCols+1)+ulCol+1]->m_ulLen = m_UQueue.GetSize();
					if(m_UQueue.GetSize())
					{
						m_UQueue.Pop(m_aField[usRow*(ulCols+1)+ulCol+1]->m_pData, m_UQueue.GetSize());
					}
				}
				ATLASSERT(m_UQueue.GetSize() == 0);
				break;
			}
			ulBlobCol--;
		}
	}
}

STDMETHODIMP CURowset::IsNullable(long lCol, VARIANT_BOOL *pbIsNullable)
{
	if(pbIsNullable)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(lCol == 0 || (unsigned long)lCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*pbIsNullable = IsNullable(lCol-1) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return uecOK;
}

STDMETHODIMP CURowset::get_Property(VARIANT *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		::VariantClear(pVal);
		::VariantCopy(pVal, &m_vtProperty);
	}
	return S_OK;
}

STDMETHODIMP CURowset::get_ParentHandle(long *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pVal = m_ulParentHandle;
	}
	return S_OK;
}

STDMETHODIMP CURowset::put_ParentHandle(long newVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_ulParentHandle = newVal;
	return S_OK;
}

STDMETHODIMP CURowset::get_Handle(long *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pVal = m_ulHandle;
	}
	return S_OK;
}

void CURowset::PackOneRow(unsigned short usRow)
{
	ULONG ul;
	ULONG ulCols = m_aColName.GetSize();
	ULONG ulPos;
	for(ul=0; ul<ulCols; ul++)
	{
		ulPos = usRow*(ulCols+1)+ul+1;
		if(IsFixed(ul))
		{
			BYTE bLen;
			if(m_aField[ulPos]->m_ulLen==0xFFFFFFFF)
			{
				BYTE bLen = 0xFF;
				m_UQueue.Push(&bLen, 1);
			}
			else
			{
				bLen = m_aColMeta[ul].m_ulBoundMaxLen;
				m_UQueue.Push(&bLen, 1);
				if(m_aColMeta[ul].m_usBoundDBType == sdVT_VARIANT)
				{
					ATLASSERT(bLen == sizeof(VARIANT));
					m_UQueue.PushVT(*((VARIANT*)m_aField[ulPos]->m_pData));
				}
				else
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, bLen);
				}
			}
		}
		else
		{
			if(IsBLOB(ul) || m_aColMeta[ul].m_ulBoundMaxLen>=0xFFFF)
			{
				ULONG ulLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push(&ulLen);
				if(ulLen != 0xFFFFFFFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, ulLen);
				}
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen>=0xFF)
			{
				unsigned short usLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push((BYTE*)&usLen, sizeof(usLen));
				if(usLen != 0xFFFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, usLen);
				}
			}
			else
			{
				BYTE bLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push(&bLen, 1);
				if(bLen != 0xFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, bLen);
				}
			}
		}
	}
}

STDMETHODIMP CURowset::IsModified(short sRow, long lCol, VARIANT_BOOL *pbModified)
{
	if(pbModified)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if((unsigned short)sRow>=m_usRecordsFetched || lCol==0 || (unsigned long)lCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		*pbModified = m_aField[sRow*(m_aColName.GetSize()+1)+lCol]->m_bModified ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return S_OK;
}

void CURowset::UpdateOneRow(unsigned short usRow)
{
	ULONG ul;
	ULONG ulCol;
	ULONG ulModfied = 0;
	ULONG ulCols = m_aColName.GetSize();
	ULONG ulPos;
	for(ul=0; ul<ulCols; ul++)
	{
		ulPos = usRow*(ulCols+1)+ul+1;
		if(!m_aField[ulPos]->m_bModified || !Writable(ul))
			continue;
		ulCol = ul + 1;
		m_UQueue.Push(&ulCol);
		if(IsFixed(ul))
		{
			BYTE bLen;
			if(m_aField[ulPos]->m_ulLen==0xFFFFFFFF)
			{
				BYTE bLen = 0xFF;
				m_UQueue.Push(&bLen, 1);
			}
			else
			{
				bLen = m_aColMeta[ul].m_ulBoundMaxLen;
				m_UQueue.Push(&bLen, 1);
				if(m_aColMeta[ul].m_usBoundDBType == sdVT_VARIANT)
				{
					ATLASSERT(bLen == sizeof(VARIANT));
					m_UQueue.PushVT(*((VARIANT*)m_aField[ulPos]->m_pData));
				}
				else
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, bLen);
				}
			}
		}
		else
		{
			if(IsBLOB(ul) || m_aColMeta[ul].m_ulBoundMaxLen>=0xFFFF)
			{
				ULONG ulLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push(&ulLen);
				if(ulLen != 0xFFFFFFFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, ulLen);
				}
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen>=0xFF)
			{
				unsigned short usLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push((BYTE*)&usLen, sizeof(usLen));
				if(usLen != 0xFFFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, usLen);
				}
			}
			else
			{
				BYTE bLen = m_aField[ulPos]->m_ulLen;
				m_UQueue.Push(&bLen, 1);
				if(bLen != 0xFF)
				{
					m_UQueue.Push(m_aField[ulPos]->m_pData, bLen);
				}
			}
		}
		ulModfied++;
	}
	m_UQueue.Insert(&ulModfied);
}

bool CURowset::Writable(ULONG ulCol)
{
	return ((m_aColMeta[ulCol].m_ulFlags & (DBCOLUMNFLAGS_WRITE|DBCOLUMNFLAGS_WRITEUNKNOWN)) > 0);
}

STDMETHODIMP CURowset::BLOBDelayed(VARIANT_BOOL *pbDelayed)
{
	if(pbDelayed)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pbDelayed = m_bNoDelay ? VARIANT_FALSE : VARIANT_TRUE;
	}

	return S_OK;
}

STDMETHODIMP CURowset::IsDelayedUpdate(VARIANT_BOOL *pbDelayedUpdate)
{
	if(pbDelayedUpdate)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pbDelayedUpdate = m_bDelayUpdate ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CURowset::IsReadOnly(VARIANT_BOOL *pbIsReadOnly)
{
	if(pbIsReadOnly)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*pbIsReadOnly = m_bReadOnly ? VARIANT_TRUE : VARIANT_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CURowset::GetCountOfBLOBsPerRow(long *plCountBLOBs)
{
	if(plCountBLOBs)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		*plCountBLOBs = m_ulBLOBCount;
	}
	return S_OK;
}

STDMETHODIMP CURowset::get_BookmarkValue(VARIANT *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		::VariantClear(pVal);
		if(m_Bookmark.GetSize())
		{
			void *pData;
			SAFEARRAYBOUND sab[1] = {m_Bookmark.GetSize(), 0};
			pVal->vt = (VT_ARRAY|VT_UI1);
			pVal->parray = ::SafeArrayCreate(VT_UI1, 1, sab);
			::SafeArrayAccessData(pVal->parray, &pData);
			memcpy(pData, m_Bookmark.GetBuffer(), m_Bookmark.GetSize());
			::SafeArrayUnaccessData(pVal->parray);
		}
	}
	return S_OK;
}

void CURowset::UpdateOneRecordData(unsigned short usRow)
{
	ULONG ul;
	ULONG ulPos;
	bool bStatus = false;
	ULONG ulCols = m_aColName.GetSize();
	ULONG ulHead = usRow*(ulCols+1);
	BYTE bData = 0;
	if(m_aField[ulHead] != NULL)
	{
		m_UQueue.Pop(m_aField[ulHead]->m_pData, m_aField[ulHead]->m_ulLen);
		bStatus = true;
	}
	for(ul=0; ul<ulCols; ul++)
	{
		ulPos = m_usRecordsFetched*(ulCols+1)+ul+1;
		m_aField[ulPos]->m_bModified = false;
		if(bStatus)
		{
			bData = 1;
			bData = (bData << m_aColMeta[ul].m_bBitPos);
			bData = (bData & m_aField[ulHead]->m_pData[m_aColMeta[ul].m_ulBytePos]);
		}
		if(bData) //null
		{
			m_aField[ulPos]->m_ulLen = (~0);
			continue;
		}
		if(IsFixed(ul))
		{
			ATLASSERT(m_UQueue.GetSize()>=m_aColMeta[ul].m_ulBoundMaxLen);
			m_aField[ulPos]->m_ulLen = m_aColMeta[ul].m_ulBoundMaxLen;
			if(m_aColMeta[ul].m_usBoundDBType == sdVT_VARIANT)
			{
				ATLASSERT(m_aField[ulPos]->m_bVT);
				m_UQueue.PopVT(*((VARIANT*)m_aField[ulPos]->m_pData));
			}
			else
			{
				m_UQueue.Pop(m_aField[ulPos]->m_pData, m_aColMeta[ul].m_ulBoundMaxLen);
			}
		}
		else
		{
			if(m_aColMeta[ul].m_ulBoundMaxLen > (0xFFFF-1))
			{
				ULONG ulLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(ulLen));
				m_UQueue.Pop(&ulLen);
				m_aField[ulPos]->m_ulLen = ulLen;
				ATLASSERT(ulLen != 0xFFFFFFFF);
				ATLASSERT(m_UQueue.GetSize()>=ulLen);
				m_UQueue.Pop(m_aField[ulPos]->m_pData, ulLen);
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen > (0xFF-1))
			{
				unsigned short usLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(usLen));
				m_UQueue.Pop(&usLen);
				ATLASSERT(usLen != (0xFFFF));
				m_aField[ulPos]->m_ulLen = usLen;
				ATLASSERT(m_UQueue.GetSize()>=usLen);
				m_UQueue.Pop(m_aField[ulPos]->m_pData, usLen);	
			}
			else if(m_aColMeta[ul].m_ulBoundMaxLen)
			{
				unsigned char bLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(bLen));
				m_UQueue.Pop(&bLen, 1);
				ATLASSERT(bLen != 0xFF);
				ATLASSERT(m_UQueue.GetSize()>=bLen);
				m_aField[ulPos]->m_ulLen = bLen;
				m_UQueue.Pop(m_aField[ulPos]->m_pData, bLen);
			}
			else //BLOB == 0
			{
				ULONG ulLen;
				ATLASSERT(m_UQueue.GetSize()>=sizeof(ulLen));
				m_UQueue.Pop(&ulLen);
				m_aField[ulPos]->m_ulLen = ulLen;
				ATLASSERT(ulLen != 0xFFFFFFFF);
				if(m_bNoDelay)
				{
					ATLASSERT(m_UQueue.GetSize()>=ulLen);
					m_aField[ulPos]->m_pData = (BYTE*)::realloc(m_aField[ulPos]->m_pData, ulLen);
					if(ulLen)
					{
						m_UQueue.Pop(m_aField[ulPos]->m_pData, ulLen);
					}
				}
			}
		}
	}
	if(usRow >= m_usRecordsFetched)
		m_usRecordsFetched++;
	ATLASSERT(m_usRecordsFetched <= m_usBatchSize);
}

bool CURowset::IsOrigFixed(unsigned long ulCol)
{
	if(m_aColMeta[ulCol].m_usRawDBType == sdVT_BYTES || m_aColMeta[ulCol].m_usRawDBType == sdVT_STR || m_aColMeta[ulCol].m_usRawDBType == sdVT_WSTR || m_aColMeta[ulCol].m_usRawDBType == DBTYPE_VARNUMERIC)
		return false;
	return true;
}

STDMETHODIMP CURowset::GetUInt8(short nRow, long lCol, VARIANT_BOOL *pbIsNull, BYTE *pData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_UI1:
				{
					*pData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetInt8(short nRow, long lCol, VARIANT_BOOL *pbIsNull, signed char *pData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I1:
				{
					*pData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetUInt16(short nRow, long lCol, VARIANT_BOOL *pbIsNull, unsigned short *pusData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pusData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_UI1:
				{
					*pusData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
			case sdVT_UI2:
				{
					*pusData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pusData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetInt16(short nRow, long lCol, VARIANT_BOOL *pbIsNull, short *psData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(psData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I1:
				{
					*psData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI1:
				{
					*psData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I2:
				{
					*psData = *((short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*psData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetUInt32(short nRow, long lCol, VARIANT_BOOL *pbIsNull, unsigned long *punData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(punData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_UI1:
				{
					*punData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
			case sdVT_UI2:
				{
					*punData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI4:
				{
					*punData = *((unsigned long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*punData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetInt32(short nRow, long lCol, VARIANT_BOOL *pbIsNull, long *pnData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pnData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_UI1:
				{
					*pnData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI2:
				{
					*pnData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I1:
				{
					*pnData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I2:
				{
					*pnData = *((short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I4:
				{
					*pnData = *((long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pnData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetFloat(short nRow, long lCol, VARIANT_BOOL *pbIsNull, float *pfData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pfData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I1:
				{
					*pfData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I2:
				{
					*pfData = *((short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI1:
				{
					*pfData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI2:
				{
					*pfData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI4:
				{
					*pfData = *((unsigned long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I4:
				{
					*pfData = *((long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_R4:
				{
					*pfData = *((float*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pfData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetDouble(short nRow, long lCol, VARIANT_BOOL *pbIsNull, double *pdData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pdData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I1:
				{
					*pdData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I2:
				{
					*pdData = *((short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI1:
				{
					*pdData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI2:
				{
					*pdData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI4:
				{
					*pdData = *((unsigned long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I4:
				{
					*pdData = *((long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI8:
				{
					*pdData = *((__int64*)m_aField[ulPos]->m_pData); //unsigned __int64 not supported
					m_hr = uecOK;
				}
				break;
			case sdVT_I8:
				{
					*pdData = *((__int64*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_R4:
				{
					*pdData = *((float*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_R8:
				{
					*pdData = *((double*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pdData = 0.0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetBool(short nRow, long lCol, VARIANT_BOOL *pbIsNull, VARIANT_BOOL *pbData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pbData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_BOOL:
				{
					*pbData = (*((VARIANT_BOOL*)m_aField[ulPos]->m_pData) != VARIANT_FALSE) ? VARIANT_TRUE : VARIANT_FALSE;
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pbData = VARIANT_FALSE;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetVariant(short nRow, long lCol, VARIANT *pvtData)
{
	return GetData(nRow, lCol, pvtData);
}

STDMETHODIMP CURowset::GetUInt64(short nRow, long lCol, VARIANT_BOOL *pbIsNull, unsigned __int64 *pulData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pulData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_UI1:
				{
					*pulData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI2:
				{
					*pulData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI4:
				{
					*pulData = *((unsigned long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI8:
				{
					*pulData = *((unsigned __int64*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pulData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetInt64(short nRow, long lCol, VARIANT_BOOL *pbIsNull, __int64 *plData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(plData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_I1:
				{
					*plData = *((char*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I2:
				{
					*plData = *((short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I4:
				{
					*plData = *((long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI1:
				{
					*plData = *((BYTE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI2:
				{
					*plData = *((unsigned short*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI4:
				{
					*plData = *((unsigned long*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_I8:
				{
					*plData = *((__int64*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*plData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetDecimal(short nRow, long lCol, VARIANT_BOOL *pbIsNull, DECIMAL *pdecData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pdecData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		memset(pdecData, 0, sizeof(DECIMAL));
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_DECIMAL:
				{
					*pdecData = *((DECIMAL*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			case sdVT_UI1:
			case sdVT_UI2:
			case sdVT_UI4:
			case sdVT_UI8:
				unsigned __int64 ullData;
				GetUInt64(nRow, lCol, pbIsNull, &ullData);
				pdecData->Lo64 = (ULONGLONG)ullData;
				break;
			case sdVT_I1:
			case sdVT_I2:
			case sdVT_I4:
			case sdVT_I8:
				__int64 llData;
				GetInt64(nRow, lCol, pbIsNull, &llData);
				pdecData->Lo64 = (ULONGLONG)llData;
				if(llData < 0)
				{
					pdecData->sign = 0x80;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetCurrency(short nRow, long lCol, VARIANT_BOOL *pbIsNull, CY *pcyData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pcyData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_CY:
				{
					*pcyData = *((CY*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			memset(pcyData, 0, sizeof(CY));
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetDateTime(short nRow, long lCol, VARIANT_BOOL *pbIsNull, DATE *pdData)
{
	if(pbIsNull)
		*pbIsNull = VARIANT_FALSE;
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;
	if(pdData)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
		if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
		{
			m_hr = uecUnexpected;
			return m_hr;
		}
		m_hr = uecOK;
		if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
		{
			switch (m_aColMeta[ulCol-1].m_usBoundDBType)
			{
			case sdVT_DATE:
				{
					*pdData = *((DATE*)m_aField[ulPos]->m_pData);
					m_hr = uecOK;
				}
				break;
			default:
				m_hr = dbeDataTypeError;
				break;	
			}
		}
		else
		{
			*pdData = 0;
			m_hr = uecOK;
			if(pbIsNull)
				*pbIsNull = VARIANT_TRUE;
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::GetBytes(short nRow, long lCol, unsigned long ulLen, BYTE *pBuffer, unsigned long *plRtnSize)
{
	if(pBuffer == NULL || ulLen == 0)
	{
		if(plRtnSize)
		{
			*plRtnSize = 0;
		}
		return S_OK;
	}
	
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;

	CAutoLock AutoLock(&m_cs.m_sec);
	ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
	if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_hr = uecOK;
	if(m_aField[ulPos]->m_ulLen != 0xFFFFFFFF)
	{
		switch (m_aColMeta[ulCol-1].m_usBoundDBType)
		{
		case sdVT_UI2:
		case sdVT_BOOL:
		case sdVT_I2:
			{
				if(ulLen >= sizeof(short))
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, sizeof(short));
					if(plRtnSize)
					{
						*plRtnSize = sizeof(short);
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI4:
		case sdVT_I4:
		case sdVT_R4:
			{
				if(ulLen >= sizeof(float))
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, sizeof(float));
					if(plRtnSize)
					{
						*plRtnSize = sizeof(float);
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I8:
		case sdVT_UI8:
		case sdVT_R8:
		case sdVT_CY:
		case sdVT_DATE:
			{
				if(ulLen >= sizeof(double))
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, sizeof(double));
					if(plRtnSize)
					{
						*plRtnSize = sizeof(double);
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_VARIANT:
			m_hr = dbeDataTypeError;
			break;
		case sdVT_DECIMAL:
			{
				if(ulLen >= sizeof(DECIMAL))
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, sizeof(DECIMAL));
					if(plRtnSize)
					{
						*plRtnSize = sizeof(DECIMAL);
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I1:
		case sdVT_UI1:
			{
				if(ulLen >= sizeof(BYTE))
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, sizeof(BYTE));
					if(plRtnSize)
					{
						*plRtnSize = sizeof(BYTE);
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_WSTR:
		case sdVT_STR:
		case sdVT_BYTES:
			{
				if(!m_bNoDelay && IsBLOB(lCol-1) && (nRow*m_ulBLOBCount >= m_ulBLOBIndex) )
				{
					m_hr = dbeFail;
					return m_hr;
				}
				if(ulLen >= m_aField[ulPos]->m_ulLen)
				{
					memcpy(pBuffer, m_aField[ulPos]->m_pData, m_aField[ulPos]->m_ulLen);
					if(plRtnSize)
					{
						*plRtnSize = m_aField[ulPos]->m_ulLen;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		default:
			m_hr = uecUnexpected;
			break;	
		}
	}
	else
	{
		if(plRtnSize)
		{
			//if data is DB null
			*plRtnSize = 0xFFFFFFFF;
		}
	}
	
	return m_hr;
}

STDMETHODIMP CURowset::GetStringA(short nRow, long lCol, unsigned long ulLen, signed char *strA, unsigned long *plRtnSize)
{
	if(strA == NULL || ulLen == 0)
	{
		if(plRtnSize)
		{
			*plRtnSize = 0;
		}
		return S_OK;
	}
	
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;

	CAutoLock AutoLock(&m_cs.m_sec);
	ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
	if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_hr = uecOK;
	if(m_aField[ulPos]->m_ulLen == 0xFFFFFFFF) 
	{
		if(plRtnSize)
		{
			//if data is DB null
			*plRtnSize = 0xFFFFFFFF;
		}
	}
	else
	{
		switch (m_aColMeta[ulCol-1].m_usBoundDBType)
		{
		case sdVT_I2:
			{
				char strData[64] = {0};
				::_itoa(*((short*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I4:
			{
				char strData[64] = {0};
				::_itoa(*((long*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_R4:
			{
				char strData[128] = {0};
				::sprintf(strData, "%f", *((float*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_R8:
			{
				char strData[256] = {0};
				::sprintf(strData, "%f", *((double*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_CY:
			{
				USES_CONVERSION;
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_CY;
				vtData.cyVal = *((CY*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				char *strData = OLE2A(vtString.bstrVal);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_DATE:
			{
				USES_CONVERSION;
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_DATE;
				vtData.date = *((DATE*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				char *strData = OLE2A(vtString.bstrVal);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_BOOL:
			{
				if(*((VARIANT_BOOL*)m_aField[ulPos]->m_pData) == VARIANT_FALSE)
				{
					char *str = "False";
					if(ulLen > 5)
					{
						::strcpy((char*)strA, str);
						if(plRtnSize)
						{
							*plRtnSize = 5;
						}
					}
					else
					{
						m_hr = dbeLengthTooShort;
					}
				}
				else
				{
					char *str = "True";
					if(ulLen > 4)
					{
						::strcpy((char*)strA, str);
						if(plRtnSize)
						{
							*plRtnSize = 4;
						}
					}
					else
					{
						m_hr = dbeLengthTooShort;
					}
				}
			}
			break;
		case sdVT_VARIANT:
			{
				USES_CONVERSION;
				CComVariant vtString;
				VARIANT *p = (VARIANT*)m_aField[ulPos]->m_pData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				char *strData = OLE2A(vtString.bstrVal);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_DECIMAL:
			{
				USES_CONVERSION;
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_DECIMAL;
				vtData.decVal = *((DECIMAL*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				char *strData = OLE2A(vtString.bstrVal);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I1:
			{
				char strData[64] = {0};
				::_itoa(*((char*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI1:
			{
				char strData[64] = {0};
				::_itoa(*((BYTE*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI2:
			{
				char strData[64] = {0};
				::_itoa(*((unsigned short*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI4:
			{
				char strData[128] = {0};
				::_ultoa(*((unsigned long*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I8:
			{
				char strData[128] = {0};
				::sprintf(strData, "%I64d", *((__int64*)m_aField[ulPos]->m_pData));
//				::_i64toa(*((__int64*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI8:
			{
				char strData[128] = {0};
				::sprintf(strData, "%I64ud", *((unsigned __int64*)m_aField[ulPos]->m_pData));
//				::_ui64toa(*((unsigned __int64*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::strlen(strData);
				if(ulLen > ul)
				{
					::strcpy((char*)strA, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_BYTES:
			{
				m_hr = dbeFail;
			}
			break;
		case sdVT_STR:
			{
				if(!m_bNoDelay && IsBLOB(lCol-1) && (nRow*m_ulBLOBCount >= m_ulBLOBIndex) )
				{
					m_hr = dbeFail;
					return m_hr;
				}
				if(ulLen > m_aField[ulPos]->m_ulLen)
				{
					::strncpy((char*)strA, (LPCSTR)(m_aField[ulPos]->m_pData), m_aField[ulPos]->m_ulLen);
					if(plRtnSize)
					{
						*plRtnSize = m_aField[ulPos]->m_ulLen;
					}
					strA[m_aField[ulPos]->m_ulLen] = 0;
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_WSTR:
			{
				if(!m_bNoDelay && IsBLOB(lCol-1) && (nRow*m_ulBLOBCount >= m_ulBLOBIndex) )
				{
					m_hr = dbeFail;
					return m_hr;
				}
				unsigned long ul = m_aField[ulPos]->m_ulLen/sizeof(WCHAR);
				if(ulLen > ul)
				{
					WideCharToMultiByte(CP_ACP, 0, (LPOLESTR)m_aField[ulPos]->m_pData, 
						ul, (char*)strA, ulLen, 0, 0);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		default:
			m_hr = uecUnexpected;
			break;	
		}
	}

	return m_hr;
}

STDMETHODIMP CURowset::GetStringW(short nRow, long lCol, unsigned long ulLen, WCHAR *strW, unsigned long *plRtnSize)
{
	if(strW == NULL || ulLen == 0)
	{
		if(plRtnSize)
		{
			*plRtnSize = 0;
		}
		return S_OK;
	}
	
	unsigned short usRow = (unsigned short)nRow;
	ULONG ulCol = (ULONG)lCol;

	CAutoLock AutoLock(&m_cs.m_sec);
	ULONG ulPos = usRow*(m_aColName.GetSize()+1)+ulCol;
	if(usRow >= m_usBatchSize || ulCol==0 || ulCol>m_aColName.GetSize() || !m_bOpen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_hr = uecOK;
	if(m_aField[ulPos]->m_ulLen == 0xFFFFFFFF)
	{
		if(plRtnSize)
		{
			//if data is DB null
			*plRtnSize = 0xFFFFFFFF;
		}
	}
	else
	{
		switch (m_aColMeta[ulCol-1].m_usBoundDBType)
		{
		case sdVT_I2:
			{
				WCHAR strData[64] = {0};
				::_itow(*((short*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I4:
			{
				WCHAR strData[64] = {0};
				::_itow(*((long*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_R4:
			{
				WCHAR strData[128] = {0};
				::swprintf(strData, L"%f", *((float*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_R8:
			{
				WCHAR strData[256] = {0};
				::swprintf(strData, L"%f", *((double*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_CY:
			{
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_CY;
				vtData.cyVal = *((CY*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				WCHAR *strData = vtString.bstrVal;
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_DATE:
			{
				USES_CONVERSION;
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_DATE;
				vtData.date = *((DATE*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				WCHAR *strData = vtString.bstrVal;
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_BOOL:
			{
				if(*((VARIANT_BOOL*)m_aField[ulPos]->m_pData) == VARIANT_FALSE)
				{
					WCHAR *str = L"False";
					if(ulLen > 5)
					{
						::wcscpy((WCHAR*)strW, str);
						if(plRtnSize)
						{
							*plRtnSize = 5;
						}
					}
					else
					{
						m_hr = dbeLengthTooShort;
					}
				}
				else
				{
					WCHAR *str = L"True";
					if(ulLen > 4)
					{
						::wcscpy((WCHAR*)strW, str);
						if(plRtnSize)
						{
							*plRtnSize = 4;
						}
					}
					else
					{
						m_hr = dbeLengthTooShort;
					}
				}
			}
			break;
		case sdVT_VARIANT:
			{
				USES_CONVERSION;
				CComVariant vtString;
				VARIANT *p = (VARIANT*)m_aField[ulPos]->m_pData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				WCHAR *strData = vtString.bstrVal;
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_DECIMAL:
			{
				USES_CONVERSION;
				CComVariant vtString;
				CComVariant vtData;
				vtData.vt = VT_DECIMAL;
				vtData.decVal = *((DECIMAL*)m_aField[ulPos]->m_pData);
				VARIANT *p = &vtData;
				::VariantChangeType(&vtString, p, VARIANT_ALPHABOOL, VT_BSTR);
				WCHAR *strData = vtString.bstrVal;
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I1:
			{
				WCHAR strData[64] = {0};
				::_itow(*((WCHAR*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI1:
			{
				WCHAR strData[64] = {0};
				::_itow(*((BYTE*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI2:
			{
				WCHAR strData[64] = {0};
				::_itow(*((unsigned short*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI4:
			{
				WCHAR strData[128] = {0};
				::_ultow(*((unsigned long*)m_aField[ulPos]->m_pData), strData, 10);
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_I8:
			{
				WCHAR strData[128] = {0};
				::swprintf(strData, L"%I64d", *((__int64*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_UI8:
			{
				WCHAR strData[128] = {0};
				::swprintf(strData, L"%I64ud", *((unsigned __int64*)m_aField[ulPos]->m_pData));
				unsigned long ul = ::wcslen(strData);
				if(ulLen > ul)
				{
					::wcscpy((WCHAR*)strW, strData);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_BYTES:
			{
				m_hr = dbeFail;
			}
			break;
		case sdVT_STR:
			{
				if(!m_bNoDelay && IsBLOB(lCol-1) && (nRow*m_ulBLOBCount >= m_ulBLOBIndex) )
				{
					m_hr = dbeFail;
					return m_hr;
				}
				if(ulLen > m_aField[ulPos]->m_ulLen)
				{
					MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(m_aField[ulPos]->m_pData),
						m_aField[ulPos]->m_ulLen, strW, m_aField[ulPos]->m_ulLen);
					if(plRtnSize)
					{
						*plRtnSize = m_aField[ulPos]->m_ulLen;
					}
					strW[m_aField[ulPos]->m_ulLen] = 0;
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		case sdVT_WSTR:
			{
				if(!m_bNoDelay && IsBLOB(lCol-1) && (nRow*m_ulBLOBCount >= m_ulBLOBIndex) )
				{
					m_hr = dbeFail;
					return m_hr;
				}
				unsigned long ul = m_aField[ulPos]->m_ulLen/sizeof(WCHAR);
				if(ulLen > ul)
				{
					::wcsncpy((WCHAR*)strW, (LPCWSTR)(m_aField[ulPos]->m_pData), ul);
					if(plRtnSize)
					{
						*plRtnSize = ul;
					}
				}
				else
				{
					m_hr = dbeLengthTooShort;
				}
			}
			break;
		default:
			m_hr = uecUnexpected;
			break;	
		}
	}
	return m_hr;
}

STDMETHODIMP CURowset::get_MatrixData(VARIANT *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	ULONG ulCols = m_aColMeta.GetSize();
	if(!m_bOpen || ulCols == 0)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(pVal)
	{
		try
		{
			::VariantClear(pVal);
		}
		catch(...)
		{
		}
		if(m_usRecordsFetched > 0)
		{
			unsigned short us;
			unsigned long ul;
			VARIANT *p;
			VARIANT *pPos;
			SAFEARRAYBOUND sab[1] = {ulCols*m_usRecordsFetched, 0};
			pVal->vt = (VT_ARRAY|VT_VARIANT);
			pVal->parray = ::SafeArrayCreate(VT_VARIANT, 1, sab);
			::SafeArrayAccessData(pVal->parray, (void**)&p);

			memset(p, 0, sizeof(VARIANT)*sab->cElements);

			for(us=0; us<m_usRecordsFetched; us++)
			{
				for(ul=1; ul<=ulCols; ul++)
				{
					pPos = p + us*ulCols + (ul - 1);
					GetData(us, ul, pPos);
				}
			}

			::SafeArrayUnaccessData(pVal->parray);
		}
		
	}
	m_hr = uecOK;
	return m_hr;
}




STDMETHODIMP CURowset::GetBatchRecordsEx(short sSubBatchSize, long lSkip)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_UQueue.SetSize(0);
	m_UQueue.Push(&sSubBatchSize);
	m_UQueue.Push(&lSkip);
	return m_pIUFast->SendRequestEx(idRowsetGetBatchRecordsEx, m_UQueue.GetSize(), (BYTE*)m_UQueue.GetBuffer());
}

STDMETHODIMP CURowset::GetBatchRecordsLast(short sSubBatchSize)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRowsetGetBatchRecordsLast, sizeof(sSubBatchSize), (BYTE*)&sSubBatchSize);
}

// This is a part of the SocketPro package.
// Copyright (C) 2000-2007 UDAParts 
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

STDMETHODIMP CURowset::get_InternalDataPointer(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		if(m_aField.GetSize() > 0)
			*pVal = (long)(m_aField.GetData());
		else
			*pVal = NULL;
	}
	return S_OK;
}
