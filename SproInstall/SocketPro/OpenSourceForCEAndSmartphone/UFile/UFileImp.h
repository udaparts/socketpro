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


// UFileImp.h : Declaration of the CUFile

#ifndef __UFILE_H_
#define __UFILE_H_

#include "resource.h"       // main symbols
#include "UFileCP.h"
#include "usocketevent.h"

#define	DEFAULT_PACK_SIZE (20*1460)

/////////////////////////////////////////////////////////////////////////////
// CUFile
class ATL_NO_VTABLE CUFile : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUFile, &CLSID_UFile>,
	public IConnectionPointContainerImpl<CUFile>,
	public IDispatchImpl<IUFile, &IID_IUFile, &LIBID_UFILELib>,
	public CProxy_IURequestEvent< CUFile >
{
public:
	CUFile()
	{
		m_hr = uecOK;
		memset(&m_WinFindData, 0, sizeof(m_WinFindData));
		memset(m_aSpaceData, 0, sizeof(m_aSpaceData));
		m_ulFilePointer = 0;
		m_uiDriveType = dtUnknown;
		m_bGettingFile = false;
		m_hFileOpened = 0;
		m_bFirstPack = false;
		m_ulGetFileSize = 0;
		m_ulGetFileSizeHigh = 0;
		m_ulSendFileSize = 0;
		m_ulSendFileSizeHigh = 0;
		m_bCanceled = false;
		m_SocketEventSink.m_pContainer = this;
		m_bSwitched = false;
	}
	virtual ~CUFile()
	{
		if(m_bGettingFile)
		{
			CloseLocalFileGetFile(false);
		}
		if(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE)
		{
			CloseLocalFileSend();
		}
	}
	void FinalRelease()
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(m_bGettingFile)
		{
			CloseLocalFileGetFile(false);
		}
		if(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE)
		{
			CloseLocalFileSend();
		}
		if(m_pIUSocket.p)
		{
			m_hr = m_SocketEventSink.DispEventUnadvise(m_pIUSocket.p);
			m_pIUSocket.Release();
		}
		if(m_pIUFast.p)
			m_pIUFast.Release();
	}
DECLARE_REGISTRY_RESOURCEID(IDR_UFILE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CUFile)
	COM_INTERFACE_ENTRY(IUFile)
	COM_INTERFACE_ENTRY(IUObjBase)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CUFile)
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
//	WIN32_FIND_DATA				m_WinFindData;
	WIN32_FIND_DATAA			m_WinFindData; //Serevr sends the structure with ASCII chars
	CUQueue						m_tempQueue;
	CUQueue						m_rdQueue;
	CComBSTR					m_bstrFilePart;
	CComBSTR					m_bstrFileName;
	unsigned long				m_ulFilePointer;
	UINT						m_uiDriveType;
	CComBSTR					m_bstrRootDirectory;
	ULARGE_INTEGER				m_aSpaceData[3];
	CComAutoCriticalSection		m_cs;

private:
	bool						m_bGettingFile;
	HANDLE						m_hFileOpened;
	CComBSTR					m_bstrLocalFile;
	bool						m_bFirstPack;
	unsigned long				m_ulGetFileSize;
	unsigned long				m_ulGetFileSizeHigh;
	unsigned long				m_ulSendFileSize;
	unsigned long				m_ulSendFileSizeHigh;
	bool						m_bCanceled;
	long						m_hSocket;

	CClientSocketEvent<CUFile>	m_SocketEventSink;	

// IUFile
public:
	STDMETHOD(Cancel)();
	STDMETHOD(SetRootDirectory)(/*[in]*/BSTR bstrRootDirectory);
	STDMETHOD(get_Data)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(GetFileAttributes)(/*[in]*/BSTR bstrFileName);
	STDMETHOD(get_RootDirectory)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(GetRootDirectory)();
	STDMETHOD(FindAll)(BSTR bstrFileName);
	STDMETHOD(GetCreationTime)(/*[out]*/short *pnYear, /*[out]*/short *pnMonth, /*[out]*/short *pnDayOfWeek, /*[out]*/short *pnDay, /*[out]*/short *pnHour, /*[out]*/short *pnMinute, /*[out]*/short *pnSecond, /*[out]*/short *pnMilliseconds);
	STDMETHOD(GetLastAccessTime)(/*[out]*/short *pnYear, /*[out]*/short *pnMonth, /*[out]*/short *pnDayOfWeek, /*[out]*/short *pnDay, /*[out]*/short *pnHour, /*[out]*/short *pnMinute, /*[out]*/short *pnSecond, /*[out]*/short *pnMilliseconds);
	STDMETHOD(GetLastWriteTime)(/*[out]*/short *pnYear, /*[out]*/short *pnMonth, /*[out]*/short *pnDayOfWeek, /*[out]*/short *pnDay, /*[out]*/short *pnHour, /*[out]*/short *pnMinute, /*[out]*/short *pnSecond, /*[out]*/short *pnMilliseconds);
	STDMETHOD(GetSpaceData)(/*[out]*/long *plFreeBytesAvailableToCallerLowPart, /*[out, defaultvalue(0)]*/long *plTotalNumberOfBytesLowPart, /*[out, defaultvalue(0)]*/long *plTotalNumberOfFreeBytesLowPart, /*[out, defaultvalue(0)]*/long *plFreeBytesAvailableToCallerHighPart, /*[out, defaultvalue(0)]*/long *plTotalNumberOfBytesHighPart, /*[out, defaultvalue(0)]*/long *plTotalNumberOfFreeBytesHighPart);
	STDMETHOD(GetFilePart)(/*[out]*/BSTR *pbstrFullFile, /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_FilePointer)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_DriveType)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_FileNameOrDirectory)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(FileSize)(/*[out, defaultvalue(0)]*/long *plFileSizeHigh, /*[out, retval]*/ long *pVal);
	STDMETHOD(get_FileAttributes)(/*[out, retval]*/ long *pVal);
	STDMETHOD(SearchPath)(/*[in]*/BSTR bstrFileName, /*[in]*/BSTR bstrPath, /*[in]*/BSTR bstrExtension);
	STDMETHOD(SetEndOfFile)();
	STDMETHOD(UnlockFile)(/*[in]*/long lFileOffsetLow, /*[in]*/long lFileOffsetHigh, /*[in]*/long lNumberOfBytesToLockLow, /*[in]*/long lNumberOfBytesToLockHigh);
	STDMETHOD(LockFile)(/*[in]*/long lFileOffsetLow, /*[in]*/long lFileOffsetHigh, /*[in]*/long lNumberOfBytesToLockLow, /*[in]*/long lNumberOfBytesToLockHigh);
	STDMETHOD(GetFileSize)();
	STDMETHOD(SendFile)(/*[in]*/BSTR bstrSrcFileName, /*[in]*/BSTR bstrDesFileName, /*[in, defaultvalue(-1)]*/VARIANT_BOOL bFailIfExists);
	STDMETHOD(GetFile)(/*[in]*/BSTR bstrSrcFileName, /*[in]*/BSTR bstrDesFileName, /*[in, defaultvalue(-1)]*/VARIANT_BOOL bFailIfExists);
	STDMETHOD(GetDriveType)(/*[in]*/BSTR bstrRootPathName);
	STDMETHOD(GetDiskFreeSpaceEx)(/*[in]*/BSTR bstrDirectoryName);
	STDMETHOD(FlushFileBuffers)();
	STDMETHOD(SetCurrentDirectory)(/*[in]*/BSTR bstrCurrentDirectory);
	STDMETHOD(SetFilePointer)(/*[in]*/long lDistanceToMove, /*[in]*/long lMoveMethod);
	STDMETHOD(CopyFile)(/*[in]*/BSTR bstrExistingFileName, /*[in]*/BSTR bstrNewFileName, /*[in, defaultvalue(-1)]*/VARIANT_BOOL bFailIfExists);
	STDMETHOD(MoveFile)(/*[in]*/BSTR bstrExistingFileName, /*[in]*/BSTR bstrNewFileName);
	STDMETHOD(ReadFile)(/*[in]*/long lLen);
	STDMETHOD(WriteFile)(/*[in]*/ VARIANT vtData);
	STDMETHOD(CreateFile)(/*[in]*/BSTR bstrFileName, /*[in]*/long lDesiredAccess, /*[in]*/long lShareMode, /*[in]*/long lCreationDisposition, /*[in]*/long lFlagsAndAttributes);
	STDMETHOD(FindNextFile)();
	STDMETHOD(FindFirstFile)(/*[in]*/BSTR bstrFileName);
	STDMETHOD(FindFile)(/*[in]*/BSTR bstrFileName);
	STDMETHOD(GetCurrentDirectory)();
	STDMETHOD(RemoveDirectory)(/*[in]*/BSTR bstrDirectory);
	STDMETHOD(CreateDirectory)(/*[in]*/BSTR bstrDirectory);
	STDMETHOD(DeleteFile)(/*[in]*/BSTR bstrFileName);
	STDMETHOD(SetFileAttributes)(/*[in]*/ BSTR bstrFileName, /*[in]*/long lFileAttributes);
	STDMETHOD(FindClose)();
	STDMETHOD(Close)();

private:
	bool m_bSwitched;
	void CloseLocalFileSend();
	void CloseLocalFileGetFile(bool bOk);
	void HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer);
};

#endif //__UFILE_H_

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
