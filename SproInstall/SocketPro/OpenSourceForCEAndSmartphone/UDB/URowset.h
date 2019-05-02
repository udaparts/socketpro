// URowset.h : Declaration of the CURowset
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

#ifndef __UROWSET_H_
#define __UROWSET_H_

#include "resource.h"       // main symbols
#include "UDBCP.h"
#include "colmeta.h"
#include <oledb.h>

class CField
{
public:
	CField()
	{
		m_ulLen = (~0);
		m_pData = NULL;
		m_bModified = false;
		m_bVT = false;
	}
	~CField()
	{
		if(m_pData)
		{
			if(m_bVT)
			{
				::VariantClear((VARIANT*)m_pData);
			}
			::free(m_pData);
			m_pData = NULL;
		}
	}
public:
	ULONG	m_ulLen;
	BYTE	*m_pData;
	bool	m_bModified;
	bool	m_bVT;
};
/////////////////////////////////////////////////////////////////////////////
// CURowset
class ATL_NO_VTABLE CURowset : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CURowset, &CLSID_URowset>,
	public IUDataReader,
	public IConnectionPointContainerImpl<CURowset>,
	public CProxy_IURequestEvent< CURowset >,
	public IDispatchImpl<IURowset, &IID_IURowset, &LIBID_UDBLib>
{
public:
	CURowset()
	{
		m_hr = uecOK;
		m_usBatchSize = 0;
		m_usRecordsFetched = 0;
		m_bReadOnly = false;
		m_bDelayUpdate = false;
		m_bNoDelay = false;
		m_usWhatBatch = 0;
		m_ulBLOBCount = 0;
		m_bOpen = false;
		m_ulHandle = 0;
		m_ulParentHandle = 0;
		m_ulBLOBIndex = 0;
		m_usRowAdded = 0;
		m_SocketEventSink.m_pContainer = this;
		m_bSwitched = false;
	}
	virtual ~CURowset()
	{
		DestroyFields();
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
DECLARE_REGISTRY_RESOURCEID(IDR_UROWSET)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CURowset)
	COM_INTERFACE_ENTRY(IURowset)
	COM_INTERFACE_ENTRY(IUDataReader)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CURowset)
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

private:
	CComPtr<IUSocket>			m_pIUSocket;
	CComPtr<IUFast>				m_pIUFast;
	HRESULT						m_hr;
	CComBSTR					m_bstrErrorMsg;
	CUQueue						m_UQueue;
	CComVariant					m_vtProperty;
	unsigned long				m_ulHandle;
	unsigned long				m_ulParentHandle;
	CSimpleArray<CColMeta>		m_aColMeta;
	CSimpleArray<CField*>		m_aField;
	unsigned short				m_usBatchSize;
	unsigned short				m_usRecordsFetched;
	CSimpleArray<CComBSTR>		m_aColName;
	CUQueue						m_Bookmark;
	bool						m_bReadOnly;
	bool						m_bDelayUpdate;
	bool						m_bNoDelay;
	unsigned short				m_usWhatBatch;
	ULONG						m_ulBLOBIndex;
	ULONG						m_ulBLOBCount;
	CComAutoCriticalSection		m_cs;
	unsigned short				m_usRowAdded;
	bool						m_bOpen;
	ULONG						m_ulBLOBCol;
	CClientSocketEvent<CURowset>	m_SocketEventSink;
	bool m_bSwitched;
// IURowset
public:
	STDMETHOD(get_InternalDataPointer)(/*[out, retval]*/ long *pVal);
	STDMETHOD(GetBatchRecordsLast)(/*[in, defaultvalue(0)]*/short sSubBatchSize);
	STDMETHOD(GetBatchRecordsEx)(/*[in, defaultvalue(0)]*/short sSubBatchSize, /*[in, defaultvalue(0)]*/long lSkip);
	STDMETHOD(get_MatrixData)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(GetStringW)(/*[in]*/short nRow, /*[in]*/long lCol, /*[in]*/unsigned long cwchar, /*[in, size_is(cwchar)]*/WCHAR *strW, unsigned long *plRtnSize);
	STDMETHOD(GetStringA)(/*[in]*/short nRow, /*[in]*/long lCol, /*[in]*/unsigned long ulLen, /*[in, size_is(ulLen)]*/signed char *strA, unsigned long *plRtnSize);
	STDMETHOD(GetBytes)(/*[in]*/short nRow, /*[in]*/long lCol, /*[in]*/unsigned long ulLen, /*[in, size_is(ulLen)]*/BYTE *pBuffer, /*[out, retval]*/unsigned long *plRtnSize);
	STDMETHOD(GetDateTime)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/DATE *pdData);
	STDMETHOD(GetCurrency)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/CY *pcyData);
	STDMETHOD(GetDecimal)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/DECIMAL *pdecData);
	STDMETHOD(GetInt64)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/__int64 *plData);
	STDMETHOD(GetUInt64)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/unsigned __int64 *pulData);
	STDMETHOD(GetVariant)(/*[in]*/short nRow, /*[in]*/long lCol, /*[out, retval]*/VARIANT *pvtData);
	STDMETHOD(GetBool)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/VARIANT_BOOL *pbData);
	STDMETHOD(GetDouble)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/double *pdData);
	STDMETHOD(GetFloat)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/float *pfData);
	STDMETHOD(GetInt32)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/long *pnData);
	STDMETHOD(GetUInt32)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/unsigned long *punData);
	STDMETHOD(GetInt16)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/short *psData);
	STDMETHOD(GetUInt16)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/unsigned short *pusData);
	STDMETHOD(GetInt8)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/signed char *pData);
	STDMETHOD(GetUInt8)(/*[in]*/short nRow, /*[in]*/long lCol, VARIANT_BOOL *pbIsNull, /*[out, retval]*/BYTE *pData);
	bool IsOrigFixed(unsigned long lCol);
	STDMETHOD(get_BookmarkValue)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(GetCountOfBLOBsPerRow)(/*[out, retval]*/long *plCountBLOBs);
	STDMETHOD(IsReadOnly)(/*[out, retval]*/VARIANT_BOOL *pbIsReadOnly);
	STDMETHOD(IsDelayedUpdate)(/*[out, retval]*/VARIANT_BOOL *pbDelayedUpdate);
	STDMETHOD(BLOBDelayed)(/*[out, retval]*/VARIANT_BOOL *pbDelayed);
	STDMETHOD(IsModified)(/*[in]*/short sRow, /*[in]*/long lCol, VARIANT_BOOL *pbModified);
	STDMETHOD(get_Handle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ParentHandle)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ParentHandle)(/*[in]*/ long newVal);
	STDMETHOD(get_Property)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(IsNullable)(/*[in]*/long lCol, /*[out, retval]*/VARIANT_BOOL *pbIsNullable);
	STDMETHOD(ShrinkMemory)(long lNewSize);
	STDMETHOD(GetRawMaxColLen)(/*[in]*/long lCol, /*[out, retval]*/long *plLen);
	STDMETHOD(GetColFlags)(/*[in]*/long lCol, /*[out, retval]*/long *plFlags);
	STDMETHOD(GetBatchSize)(/*[out, retval]*/short *psBatchSize);
	STDMETHOD(GetDataSize)(short sRow, long lCol, /*[out, retval]*/long *plSize);
	STDMETHOD(GetRawDataType)(/*[in]*/long lCol, /*[out, retval]*/short *psDataType);
	STDMETHOD(IsWritable)(/*[in]*/long lCol, /*[out, retval]*/VARIANT_BOOL *pbWritable);
	STDMETHOD(GetMaxLen)(/*[in]*/long lCol, /*[out, retval]*/long *plMaxLen);
	STDMETHOD(GetCols)(/*[out, retval]*/long *plCols);
	STDMETHOD(GetRowsFetched)(/*[out, retval]*/short *psCount);
	STDMETHOD(IsEOF)(/*[out, retval]*/VARIANT_BOOL *pbIsEOF);
	STDMETHOD(SetData)(short sRow, long lCol, /*[in]*/VARIANT vtData);
	STDMETHOD(FindColOrdinal)(/*[in]*/BSTR bstrColName, /*[in, defaultvalue(0)]*/VARIANT_BOOL bCaseSensitive, /*[out, retval]*/long *plCol);
	STDMETHOD(GetStatus)(short sRow, long lCol, /*[out, retval]*/long *plStatus);
	STDMETHOD(GetDataType)(/*[in]*/long lCol, /*[out, retval]*/short *psDBType);
	STDMETHOD(GetColName)(/*[in]*/long lCol, /*[out, retval]*/BSTR *pbstrColName);
	STDMETHOD(IsBLOB)(/*[in]*/long lCol, /*[out, retval]*/VARIANT_BOOL *pbBLOB);
	STDMETHOD(GetData)(short sRow, long lCol, /*[out, retval]*/VARIANT *pvtData);
	STDMETHOD(UseStorageObjectForBLOB)(/*[in, defaultvalue(-1)]*/VARIANT_BOOL bUseStorageObject);
	STDMETHOD(GetRowsAt)(/*[in]*/VARIANT vtBookmarkValue, /*[in, defaultvalue(0)]*/long lRowsOffset, /*[in, defaultvalue(0)]*/short sSubBatchSize);
	STDMETHOD(SetDataType)(/*[in]*/long lCol, /*[in]*/short nDBType, /*[in, defaultvalue(0)]*/long lLen);
	STDMETHOD(GetProperty)(/*[in]*/long lPropID, /*[in, defaultvalue("{c8b522be-5cf3-11ce-ade5-00aa0044773d}")]*/BSTR bstrPropSet);
	STDMETHOD(GetSchemaRowset)(/*[in]*/BSTR bstrSchemaGUID, /*[in]*/VARIANT vtRestrictions, /*[in, defaultvalue(0)]*/short nBatchSize, /*[in, defaultvalue(0)]*/VARIANT_BOOL bKeepPrevRowset);
	STDMETHOD(Undo)();
	STDMETHOD(GetProviders)(/*[in, defaultvalue(0)]*/VARIANT_BOOL bKeepPrevRowset);
	STDMETHOD(GetBatchRecords)(/*[in, defaultvalue(-1)]*/short sSubBatchSize, VARIANT_BOOL bFirst);
	STDMETHOD(AsynFetch)(/*[in, defaultvalue(-1)]*/VARIANT_BOOL bFromBeginning, /*[in, defaultvalue(-1)]*/short sSubBatchSize, /*[in, defaultvalue(-1)]*/long lRows);
	STDMETHOD(Bookmark)(short sRow);
	STDMETHOD(OpenFromHandle)(/*[in]*/long lHandle, /*[in, defaultvalue(0)]*/VARIANT_BOOL bKeepPrevRowset);
	STDMETHOD(UpdateBatch)();
	STDMETHOD(Update)(/*[in, defaultvalue(0)]*/short nRow);
	STDMETHOD(Add)(/*[in, defaultvalue(0)]*/VARIANT_BOOL bNeedNewRecord, /*[in, defaultvalue(0)]*/short nRowIndex);
	STDMETHOD(Delete)(/*[in, defaultvalue(0)]*/short nRow);
	STDMETHOD(MoveNext)(/*[in, defaultvalue(0)]*/long lSkip);
	STDMETHOD(MovePrev)();
	STDMETHOD(MoveLast)();
	STDMETHOD(MoveFirst)();
	STDMETHOD(Open)(/*[in, defaultvalue("")]*/BSTR bstrTableName, /*[in, defaultvalue(ctStatic)]*/short sCursorType, /*[in, defaultvalue(0)]*/long lHint, /*[in, defaultvalue(0)]*/short nBatchSize, VARIANT_BOOL bNoDelay);
	STDMETHOD(Close)();

	void HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer);
private:
	void UpdateOneRecordData(unsigned short usRow);
	bool Writable(ULONG ulCol);
	void UpdateOneRow(unsigned short usRow);
	void PackOneRow(unsigned short usRow);
	void HandleBLOBIn();
	inline void RetrieveOneRecord();
	void SetupFields();
	unsigned long StatusBytes();
	bool IsNullable(ULONG ulCol);
	inline bool IsBLOB(ULONG ulCol);
	inline bool IsFixed(ULONG ulCol);
	void DestroyFields();
	void RetrieveRowsetMetaData();
};

#endif //__UROWSET_H_

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
