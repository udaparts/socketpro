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

// UFileImp.cpp : Implementation of CUFile
#include "stdafx.h"
#include "UFile.h"

#include "UFileImp.h"



/////////////////////////////////////////////////////////////////////////////
// CUFile

STDMETHODIMP CUFile::Close()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_pIUFast->SendRequestEx(idClose, 0, NULL);
	return S_OK;
}

STDMETHODIMP CUFile::AttachSocket(IUnknown *pIUnknownToUSocket)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_bSwitched = false;
	m_hr = uecOK;
	if(m_bGettingFile)
	{
		CloseLocalFileGetFile(false);
	}
	if(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE)
	{
		CloseLocalFileSend();
	}
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
			if(lSvsID == sidWinFile)
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

STDMETHODIMP CUFile::get_Rtn(long *plResult)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plResult)
		*plResult = m_hr;
	return S_OK;
}

STDMETHODIMP CUFile::get_ErrorMsg(BSTR *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(*pVal)
			::SysFreeString(*pVal);
		if(m_bstrErrorMsg.Length() == 0)
		{
			switch (m_hr)
			{
			case uecOK:
				break;
			case uecSvrNotRegistered:
				break;
			case uecUnexpected: 
				m_bstrErrorMsg = ::SysAllocString(L"An unexpected error occurred.");
				break;
			case uecFail:
				m_bstrErrorMsg = ::SysAllocString(L"Unspecified error.");
				break;
			default:
				if(m_hr != uecOK)
				{
					USES_CONVERSION;
					TCHAR	strError[513] = {0};
					::FormatMessage(
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						m_hr,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						strError,
						sizeof(strError)/sizeof(TCHAR),
						NULL 
					);
					DWORD dwLen = _tcslen(strError);
					if(dwLen>3)
					{
						m_bstrErrorMsg = ::SysAllocStringLen(T2OLE(strError), dwLen-1);
					}
				}
				break;
			}
		}
		*pVal = m_bstrErrorMsg.Copy();
	}
	return S_OK;
}

HRESULT __stdcall CUFile::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CUFile::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	return S_OK;
}

HRESULT __stdcall CUFile::OnSocketClosed(long hSocket, long lError)
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
	m_bFirstPack = false;
	m_ulGetFileSize = 0;
	m_ulGetFileSizeHigh = 0;
	m_ulSendFileSize = 0;
	m_ulSendFileSizeHigh = 0;
	m_bGettingFile = false;
	m_tempQueue.SetSize(0);
	if(m_tempQueue.GetMaxSize()>1024)
		m_tempQueue.ReallocBuffer(1024);
	m_rdQueue.SetSize(0);
	if(m_rdQueue.GetMaxSize()>1024)
		m_rdQueue.ReallocBuffer(1024);
	m_hr = lError;
	m_bSwitched = false;
	return S_OK;
}

HRESULT __stdcall CUFile::OnSocketConnected(long hSocket, long lError)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hSocket = hSocket;
	m_bGettingFile = false;
	m_hr = lError;
	m_bFirstPack = false;
	m_ulGetFileSize = 0;
	m_ulGetFileSizeHigh = 0;
	return S_OK;
}

HRESULT __stdcall CUFile::OnConnecting(long hSocket, long hWnd)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hSocket = hSocket;
	return S_OK;
}

HRESULT __stdcall CUFile::OnSendingData(long hSocket, long lError, long lSent)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	m_hr = lError;
	if(lError != uecOK)
	{
		CloseLocalFileSend();
	}
	else
	{
		long lBytes;
		m_pIUSocket->get_BytesInSndMemory(&lBytes);
		while(lBytes<DEFAULT_PACK_SIZE && (m_ulSendFileSize || m_ulSendFileSizeHigh) && !m_bCanceled)
		{
			DWORD dwRead = 0;
			ATLASSERT(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE);
			if(m_tempQueue.GetMaxSize()<DEFAULT_PACK_SIZE)
				m_tempQueue.ReallocBuffer(DEFAULT_PACK_SIZE);
			::ReadFile(m_hFileOpened, (void*)m_tempQueue.GetBuffer(), m_tempQueue.GetMaxSize(), &dwRead, NULL);
			m_tempQueue.SetSize(dwRead);
			if(dwRead == 0)
			{
				ATLASSERT(FALSE);
				Fire_OnRequestProcessed(m_hSocket, idSendFile, 0, 0, rfCompleted); 
				m_pIUSocket->get_BytesInSndMemory(&lBytes);
				break;
			}
			ATLASSERT((m_ulSendFileSize >= dwRead) || m_ulSendFileSizeHigh);
			m_pIUFast->SendRequestEx(idSendBytesToServer, dwRead, (BYTE*)m_tempQueue.GetBuffer());
			if(dwRead>m_ulSendFileSize)
			{
				ATLASSERT(m_ulSendFileSizeHigh);
				m_ulSendFileSizeHigh--;
				m_ulSendFileSize += 1;
				m_ulSendFileSize += (0xFFFFFFFF - dwRead);
			}
			else
			{
				m_ulSendFileSize -= dwRead;
			}
			Fire_OnRequestProcessed(hSocket, idSendFile, m_ulSendFileSize, m_ulSendFileSizeHigh, rfReceiving); // a litte confused here
			if(m_ulSendFileSize == 0 && m_ulSendFileSizeHigh == 0)
			{
				Fire_OnRequestProcessed(m_hSocket, idSendFile, 0, 0, rfCompleted); 
			}
			m_pIUSocket->get_BytesInSndMemory(&lBytes);
		}
	}
	return S_OK;
}

HRESULT __stdcall CUFile::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUFile::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CUFile::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CUFile::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	if(sFlag != rfCompleted)
		return S_OK;
	//Handle returned data only if the client get all returned bytes of a request
	CAutoLock AutoLock(&m_cs.m_sec);
	if((unsigned short)nRequestID != idSwitchTo && !m_bSwitched)
		return S_OK;
	if(m_bstrErrorMsg.Length())
		m_bstrErrorMsg.Empty();
	switch (nRequestID)
	{
	case idSwitchTo:
		{
			long lSvsID = 0;
			ATLASSERT(m_pIUSocket != NULL);
			if(m_pIUSocket != NULL)
			{
				m_pIUSocket->get_CurrentSvsID(&lSvsID);
				if(lSvsID == sidWinFile)
				{
					m_bSwitched = true;
				}
				else
				{
					m_bSwitched = false;
				}
			}
		}
		break;
	case idClose:
	case idFindClose:
	case idSetFileAttributes:
	case idGetFileAttributes:
	case idDeleteFile:
	case idCreateDirectory:
	case idRemoveDirectory:
	case idGetCurrentDirectory:
	case idFindFile:
	case idFindFirstFile:
	case idFindNextFile:
	case idCreateFile:
	case idWriteFile:
	case idReadFile:
	case idMoveFile:
	case idCopyFile:
	case idSetFilePointer:
	case idSetCurrentDirectory:
	case idFlushFileBuffers:
	case idGetDiskFreeSpaceEx:
	case idGetDriveType:
	case idGetFile:
	case idSendFile:
	case idGetFileSize:
	case idLockFile:
	case idUnlockFile:
	case idSetEndOfFile:
	case idSearchPath:
	case idFindAll:
	case idSetRootDirectory:
	case idGetRootDirectory:
		m_hr = uecOK;
		HandleRtn(nRequestID, lLen, lLenInBuffer);
		Fire_OnRequestProcessed(hSocket, nRequestID, 0, 0, sFlag);
//		m_hr = S_OK;
		break;
	case idSendBytesToServer:
	case idSendBytesToClient:
		HandleRtn(nRequestID, lLen, lLenInBuffer);
		break;
	default:
		break;
	}
	return S_OK;
}

STDMETHODIMP CUFile::FindClose()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFindClose, 0, NULL);
}

STDMETHODIMP CUFile::SetFileAttributes(BSTR bstrFileName, long lFileAttributes)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	unsigned short usLen = ::wcslen(bstrFileName)*sizeof(WCHAR);
	if(!usLen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrFileName, usLen/sizeof(WCHAR));
	m_tempQueue.Push(&lFileAttributes);
	return m_pIUFast->SendRequestEx(idSetFileAttributes, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::DeleteFile(BSTR bstrFileName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName || !::wcslen(bstrFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idDeleteFile, ::wcslen(bstrFileName)*sizeof(WCHAR), (BYTE*)bstrFileName);
}

STDMETHODIMP CUFile::CreateDirectory(BSTR bstrDirectory)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrDirectory || !::wcslen(bstrDirectory))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idCreateDirectory, ::wcslen(bstrDirectory)*sizeof(WCHAR), (BYTE*)bstrDirectory);
}

STDMETHODIMP CUFile::RemoveDirectory(BSTR bstrDirectory)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{ 
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrDirectory || !::wcslen(bstrDirectory))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idRemoveDirectory, ::wcslen(bstrDirectory)*sizeof(WCHAR), (BYTE*)bstrDirectory);
}

STDMETHODIMP CUFile::GetCurrentDirectory()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idGetCurrentDirectory, 0, NULL);
}

STDMETHODIMP CUFile::FindFile(BSTR bstrFileName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName || !::wcslen(bstrFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFindFile, ::wcslen(bstrFileName)*sizeof(WCHAR), (BYTE*)bstrFileName);
}

STDMETHODIMP CUFile::FindFirstFile(BSTR bstrFileName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName || !::wcslen(bstrFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFindFirstFile, ::wcslen(bstrFileName)*sizeof(WCHAR), (BYTE*)bstrFileName);
}

STDMETHODIMP CUFile::FindNextFile()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFindNextFile, 0, NULL);
}

STDMETHODIMP CUFile::CreateFile(BSTR bstrFileName, long lDesiredAccess, long lShareMode, long lCreationDisposition, long lFlagsAndAttributes)
{
	unsigned short usLen;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	usLen = ::wcslen(bstrFileName)*sizeof(WCHAR);
	if(!usLen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrFileName, usLen/sizeof(WCHAR));
	m_tempQueue.Push(&lDesiredAccess);
	m_tempQueue.Push(&lShareMode);
	m_tempQueue.Push(&lCreationDisposition);
	m_tempQueue.Push(&lFlagsAndAttributes);
	return m_pIUFast->SendRequestEx(idCreateFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::WriteFile(VARIANT vtData)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p || !m_pIUSocket.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_pIUSocket->SendRequest(idWriteFile, vtData);
	return S_OK;
}

STDMETHODIMP CUFile::ReadFile(long lLen)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idReadFile, sizeof(lLen), (BYTE*)&lLen);
}

STDMETHODIMP CUFile::MoveFile(BSTR bstrExistingFileName, BSTR bstrNewFileName)
{
	unsigned short usLen;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrExistingFileName || !bstrNewFileName)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	usLen = ::wcslen(bstrExistingFileName)*sizeof(WCHAR);
	if(!usLen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrExistingFileName, usLen/sizeof(WCHAR));
	usLen = ::wcslen(bstrNewFileName)*sizeof(WCHAR);
	if(!usLen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrNewFileName, usLen/sizeof(WCHAR));
	return m_pIUFast->SendRequestEx(idMoveFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::CopyFile(BSTR bstrExistingFileName, BSTR bstrNewFileName, VARIANT_BOOL bFailIfExists)
{
	unsigned short usLen;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrExistingFileName || !bstrNewFileName)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!::wcslen(bstrExistingFileName) || !::wcslen(bstrNewFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	usLen = ::wcslen(bstrExistingFileName)*sizeof(WCHAR);
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrExistingFileName, usLen/sizeof(WCHAR));
	usLen = ::wcslen(bstrNewFileName)*sizeof(WCHAR);
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	m_tempQueue.Push(bstrNewFileName, usLen/sizeof(WCHAR));
	m_tempQueue.Push((const unsigned char*)&bFailIfExists, sizeof(bFailIfExists));
	return m_pIUFast->SendRequestEx(idCopyFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::SetFilePointer(long lDistanceToMove, long lMoveMethod)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	m_tempQueue.Push(&lDistanceToMove);
	m_tempQueue.Push(&lMoveMethod);
	return m_pIUFast->SendRequestEx(idSetFilePointer, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::SetCurrentDirectory(BSTR bstrCurrentDirectory)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrCurrentDirectory || !::wcslen(bstrCurrentDirectory))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idSetCurrentDirectory, ::wcslen(bstrCurrentDirectory)*sizeof(WCHAR), (BYTE*)bstrCurrentDirectory);
}

STDMETHODIMP CUFile::FlushFileBuffers()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFlushFileBuffers, 0, NULL);
}

STDMETHODIMP CUFile::GetDiskFreeSpaceEx(BSTR bstrDirectoryName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrDirectoryName || !::wcslen(bstrDirectoryName))
	{
		return m_pIUFast->SendRequestEx(idGetDiskFreeSpaceEx, 0, NULL);
	}
	return m_pIUFast->SendRequestEx(idGetDiskFreeSpaceEx, ::wcslen(bstrDirectoryName)*sizeof(WCHAR), (BYTE*)bstrDirectoryName);
}

STDMETHODIMP CUFile::GetDriveType(BSTR bstrRootPathName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrRootPathName || !::wcslen(bstrRootPathName))
	{
		return m_pIUFast->SendRequestEx(idGetDriveType, 0, NULL);
	}
	return m_pIUFast->SendRequestEx(idGetDriveType, ::wcslen(bstrRootPathName)*sizeof(WCHAR), (BYTE*)bstrRootPathName);
}

STDMETHODIMP CUFile::GetFile(BSTR bstrSrcFileName, BSTR bstrDesFileName, VARIANT_BOOL bFailIfExists)
{
	USES_CONVERSION;
	CAutoLock AutoLock(&m_cs.m_sec);
	//when sending a file to or getting a from a remote SocketPro server, shouldn't make this call
	if(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrSrcFileName || !bstrDesFileName || !::wcslen(bstrDesFileName) || !::wcslen(bstrSrcFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_bstrLocalFile = bstrDesFileName;
	ATLASSERT(m_hFileOpened==0 || m_hFileOpened == INVALID_HANDLE_VALUE);
	WIN32_FIND_DATA		WinFindData;
	memset(&WinFindData, 0, sizeof(WinFindData));
	HANDLE hFind = ::FindFirstFile(OLE2T(bstrDesFileName), &WinFindData);
	if(hFind != INVALID_HANDLE_VALUE && bFailIfExists)
	{
		TCHAR	strError[513] = {0};
		m_hr = ERROR_FILE_EXISTS;
		::FindClose(hFind);
		::FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				m_hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				strError,
				512,
				NULL 
		);
		m_bstrErrorMsg.Empty();
		m_bstrErrorMsg.m_str = ::SysAllocStringLen(T2OLE(strError), ::wcslen(T2OLE(strError))-1);
		return m_hr;
	}
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
	}
	m_hFileOpened = ::CreateFile(OLE2T(bstrDesFileName), daGenericRead + daGenericWrite, smFileShareRead, NULL,
		cdCreateAlways, faNormal, NULL);
	if(m_hFileOpened == INVALID_HANDLE_VALUE)
	{
		TCHAR	strError[513] = {0};
		m_hr = ::GetLastError();
		::FindClose(hFind);
		::FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				m_hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				strError,
				512,
				NULL 
		);
		m_bstrErrorMsg.Empty();
		m_bstrErrorMsg.m_str = ::SysAllocStringLen(T2OLE(strError), ::wcslen(T2OLE(strError))-1);
		return m_hr;
	}
	m_bGettingFile = true;
	m_bFirstPack = true;
	m_ulGetFileSize = 0;
	m_ulGetFileSizeHigh = 0;
	m_pIUFast->SendRequestEx(idGetFile, ::wcslen(bstrSrcFileName)*sizeof(WCHAR), (BYTE*)bstrSrcFileName);
	return S_OK;
}

STDMETHODIMP CUFile::SendFile(BSTR bstrSrcFileName, BSTR bstrDesFileName, VARIANT_BOOL bFailIfExists)
{
	unsigned short usLen;
	DWORD dwSize;
	DWORD dwRead;
	USES_CONVERSION;
	CAutoLock AutoLock(&m_cs.m_sec);
	//when sending a file to or getting a from a remote SocketPro server, shouldn't make this call
	if(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrSrcFileName || !bstrDesFileName || !::wcslen(bstrDesFileName) || !::wcslen(bstrSrcFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_bCanceled = false;
	m_bstrLocalFile = bstrSrcFileName;
	ATLASSERT(m_hFileOpened==0 || m_hFileOpened == INVALID_HANDLE_VALUE);
	m_hFileOpened = ::CreateFile(OLE2T(bstrSrcFileName), daGenericRead, smFileShareRead, NULL,
		cdOpenExisting, faNormal, NULL);
	if(m_hFileOpened == INVALID_HANDLE_VALUE)
	{
		TCHAR	strError[513] = {0};
		m_hr = ::GetLastError();
		::FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				m_hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				strError,
				512,
				NULL 
		);
		m_bstrErrorMsg.Empty();
		m_bstrErrorMsg.m_str = ::SysAllocStringLen(T2OLE(strError), ::wcslen(T2OLE(strError))-1);
		return m_hr;
	}
	m_ulSendFileSize = dwSize = ::GetFileSize(m_hFileOpened, &m_ulSendFileSizeHigh);
	m_tempQueue.SetSize(0);
	m_tempQueue.Push((BYTE*)&bFailIfExists, sizeof(bFailIfExists));
	m_tempQueue.Push(&m_ulSendFileSize);
	m_tempQueue.Push(&m_ulSendFileSizeHigh);
	usLen = ::wcslen(bstrDesFileName)*sizeof(WCHAR);
	m_tempQueue.Push((BYTE*)&usLen, sizeof(usLen));
	m_tempQueue.Push((BYTE*)bstrDesFileName, usLen);
	if(dwSize > m_tempQueue.GetMaxSize() - m_tempQueue.GetSize())
	{
		m_tempQueue.ReallocBuffer(DEFAULT_PACK_SIZE);
	}
	dwSize = m_tempQueue.GetMaxSize() - m_tempQueue.GetSize();
	if(!::ReadFile(m_hFileOpened, (void*)(m_tempQueue.GetBuffer(m_tempQueue.GetSize())), dwSize, &dwRead, NULL))
	{
		::CloseHandle(m_hFileOpened);
		m_hFileOpened = 0;
		TCHAR	strError[513] = {0};
		m_hr = ::GetLastError();
		::FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				m_hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				strError,
				512,
				NULL 
		);
		m_bstrErrorMsg.Empty();
		m_bstrErrorMsg.m_str = ::SysAllocStringLen(T2OLE(strError), ::wcslen(T2OLE(strError))-1);
		return m_hr;
	}
	m_tempQueue.SetSize(m_tempQueue.GetSize()+dwRead);
	Fire_OnRequestProcessed(m_hSocket, idSendFile, m_ulSendFileSize, m_ulSendFileSizeHigh, rfComing); // a litte confused here
	m_pIUFast->SendRequestEx(idSendFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
	if(dwRead>m_ulSendFileSize)
	{
		ATLASSERT(m_ulSendFileSizeHigh);
		m_ulSendFileSizeHigh--;
		m_ulSendFileSize += 1;
		m_ulSendFileSize += (0xFFFFFFFF - dwRead);
	}
	else
	{
		m_ulSendFileSize -= dwRead;
	}
	Fire_OnRequestProcessed(m_hSocket, idSendFile, m_ulSendFileSize, m_ulSendFileSizeHigh, rfReceiving); // a litte confused here
	if(m_ulSendFileSize == 0 && m_ulSendFileSizeHigh == 0)
	{
		Fire_OnRequestProcessed(m_hSocket, idSendFile, 0, 0, rfCompleted); 
	}
	return S_OK;
}

STDMETHODIMP CUFile::GetFileSize()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idGetFileSize, 0, NULL);
}

STDMETHODIMP CUFile::LockFile(long lFileOffsetLow, long lFileOffsetHigh, long lNumberOfBytesToLockLow, long lNumberOfBytesToLockHigh)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	m_tempQueue.Push(&lFileOffsetLow);
	m_tempQueue.Push(&lFileOffsetHigh);
	m_tempQueue.Push(&lNumberOfBytesToLockLow);
	m_tempQueue.Push(&lNumberOfBytesToLockHigh);
	return m_pIUFast->SendRequestEx(idLockFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::UnlockFile(long lFileOffsetLow, long lFileOffsetHigh, long lNumberOfBytesToLockLow, long lNumberOfBytesToLockHigh)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	m_tempQueue.Push(&lFileOffsetLow);
	m_tempQueue.Push(&lFileOffsetHigh);
	m_tempQueue.Push(&lNumberOfBytesToLockLow);
	m_tempQueue.Push(&lNumberOfBytesToLockHigh);
	return m_pIUFast->SendRequestEx(idUnlockFile, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::SetEndOfFile()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idSetEndOfFile, 0, NULL);
}

STDMETHODIMP CUFile::SearchPath(BSTR bstrFileName, BSTR bstrPath, BSTR bstrExtension)
{
	unsigned short usLen;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	usLen = ::wcslen(bstrFileName);
	if(!usLen)
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	m_tempQueue.SetSize(0);
	usLen = 0;
	if(bstrPath)
	{
		usLen = ::wcslen(bstrPath)*sizeof(WCHAR);
	}
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	if(usLen)
	{
		m_tempQueue.Push(bstrPath, usLen/sizeof(WCHAR));
	}
	
	usLen = ::wcslen(bstrFileName)*sizeof(WCHAR);
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	if(usLen)
	{
		m_tempQueue.Push(bstrFileName, usLen/sizeof(WCHAR));
	}
	
	usLen = 0;
	if(bstrExtension)
	{
		usLen = ::wcslen(bstrExtension)*sizeof(WCHAR);
	}
	m_tempQueue.Push((const unsigned char*)&usLen, sizeof(usLen));
	if(usLen)
	{
		m_tempQueue.Push(bstrExtension, usLen/sizeof(WCHAR));
	}
	return m_pIUFast->SendRequestEx(idSearchPath, m_tempQueue.GetSize(), (BYTE*)m_tempQueue.GetBuffer());
}

STDMETHODIMP CUFile::get_FileAttributes(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_WinFindData.dwFileAttributes;
	return S_OK;
}

STDMETHODIMP CUFile::FileSize(long *plFileSizeHigh, long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_WinFindData.nFileSizeLow;
	if(plFileSizeHigh)
		*plFileSizeHigh = m_WinFindData.nFileSizeHigh;
	return S_OK;
}

STDMETHODIMP CUFile::get_FileNameOrDirectory(BSTR *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
	{
		USES_CONVERSION;
		if(*pVal)
			::SysFreeString(*pVal);
		*pVal = ::SysAllocString(A2OLE(m_WinFindData.cFileName)); //ASCII char
	}
	return S_OK;
}

STDMETHODIMP CUFile::get_DriveType(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_uiDriveType;
	return S_OK;
}

STDMETHODIMP CUFile::get_FilePointer(long *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pVal)
		*pVal = m_ulFilePointer;
	return S_OK;
}

STDMETHODIMP CUFile::GetFilePart(BSTR *pbstrFullFile, BSTR *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(pbstrFullFile)
	{
		if(*pbstrFullFile)
		{
			::SysFreeString(*pbstrFullFile);
			*pbstrFullFile = NULL;
		}
		if(m_bstrFileName.Length())
		{
			*pbstrFullFile = ::SysAllocString(m_bstrFileName.m_str);
		}
	}
	if(pVal)
	{
		if(*pVal)
		{
			::SysFreeString(*pVal);
			*pVal = NULL;
		}
		if(m_bstrFilePart.Length())
		{
			*pVal = ::SysAllocString(m_bstrFilePart.m_str);
		}
	}
	return S_OK;
}

STDMETHODIMP CUFile::GetSpaceData(long *plFreeBytesAvailableToCallerLowPart, long *plTotalNumberOfBytesLowPart, long *plTotalNumberOfFreeBytesLowPart, long *plFreeBytesAvailableToCallerHighPart, long *plTotalNumberOfBytesHighPart, long *plTotalNumberOfFreeBytesHighPart)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(plFreeBytesAvailableToCallerLowPart)
		*plFreeBytesAvailableToCallerLowPart=m_aSpaceData[0].LowPart;
	if(plFreeBytesAvailableToCallerHighPart)
		*plFreeBytesAvailableToCallerHighPart=m_aSpaceData[0].HighPart;
	if(plTotalNumberOfBytesLowPart)
		*plTotalNumberOfBytesLowPart=m_aSpaceData[1].LowPart;
	if(plTotalNumberOfBytesHighPart)
		*plTotalNumberOfBytesHighPart=m_aSpaceData[1].HighPart;
	if(plTotalNumberOfFreeBytesLowPart)
		*plTotalNumberOfFreeBytesLowPart=m_aSpaceData[2].LowPart;
	if(plTotalNumberOfFreeBytesHighPart)
		*plTotalNumberOfFreeBytesHighPart=m_aSpaceData[2].HighPart;
	return S_OK;
}

STDMETHODIMP CUFile::GetLastWriteTime(short *pnYear, short *pnMonth, short *pnDayOfWeek, short *pnDay, short *pnHour, short *pnMinute, short *pnSecond, short *pnMilliseconds)
{
	SYSTEMTIME	SysTime;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!::FileTimeToSystemTime(&(m_WinFindData.ftLastWriteTime), &SysTime))
	{
		m_hr = ::GetLastError();
		return m_hr;
	}
	if(pnYear)
		*pnYear=SysTime.wYear;
	if(pnMonth)
		*pnMonth=SysTime.wMonth;
	if(pnDayOfWeek)
		*pnDayOfWeek=SysTime.wDayOfWeek;
	if(pnDay)
		*pnDay=SysTime.wDay;
	if(pnHour)
		*pnHour=SysTime.wHour;
	if(pnMinute)
		*pnMinute=SysTime.wMinute;
	if(pnSecond)
		*pnSecond=SysTime.wSecond;
	if(pnMilliseconds)
		*pnMilliseconds=SysTime.wMilliseconds;
	return S_OK;
}

STDMETHODIMP CUFile::GetLastAccessTime(short *pnYear, short *pnMonth, short *pnDayOfWeek, short *pnDay, short *pnHour, short *pnMinute, short *pnSecond, short *pnMilliseconds)
{
	SYSTEMTIME	SysTime;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!::FileTimeToSystemTime(&(m_WinFindData.ftLastAccessTime), &SysTime))
	{
		m_hr = ::GetLastError();
		return m_hr;
	}
	if(pnYear)
		*pnYear=SysTime.wYear;
	if(pnMonth)
		*pnMonth=SysTime.wMonth;
	if(pnDayOfWeek)
		*pnDayOfWeek=SysTime.wDayOfWeek;
	if(pnDay)
		*pnDay=SysTime.wDay;
	if(pnHour)
		*pnHour=SysTime.wHour;
	if(pnMinute)
		*pnMinute=SysTime.wMinute;
	if(pnSecond)
		*pnSecond=SysTime.wSecond;
	if(pnMilliseconds)
		*pnMilliseconds=SysTime.wMilliseconds;
	return S_OK;
}

STDMETHODIMP CUFile::GetCreationTime(short *pnYear, short *pnMonth, short *pnDayOfWeek, short *pnDay, short *pnHour, short *pnMinute, short *pnSecond, short *pnMilliseconds)
{
	SYSTEMTIME	SysTime;
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!::FileTimeToSystemTime(&(m_WinFindData.ftCreationTime), &SysTime))
	{
		m_hr = ::GetLastError();
		return m_hr;
	}
	if(pnYear)
		*pnYear=SysTime.wYear;
	if(pnMonth)
		*pnMonth=SysTime.wMonth;
	if(pnDayOfWeek)
		*pnDayOfWeek=SysTime.wDayOfWeek;
	if(pnDay)
		*pnDay=SysTime.wDay;
	if(pnHour)
		*pnHour=SysTime.wHour;
	if(pnMinute)
		*pnMinute=SysTime.wMinute;
	if(pnSecond)
		*pnSecond=SysTime.wSecond;
	if(pnMilliseconds)
		*pnMilliseconds=SysTime.wMilliseconds;
	return S_OK;
}

STDMETHODIMP CUFile::FindAll(BSTR bstrFileName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName || !::wcslen(bstrFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idFindAll, ::wcslen(bstrFileName)*sizeof(WCHAR), (BYTE*)bstrFileName);
}

STDMETHODIMP CUFile::GetRootDirectory()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idGetRootDirectory, 0, NULL);
}

STDMETHODIMP CUFile::get_RootDirectory(BSTR *pVal)
{
	if(pVal)
	{
		CAutoLock AutoLock(&m_cs.m_sec);
		if(*pVal)
		{
			::SysFreeString(*pVal);
			*pVal = NULL;
		}
		if(m_bstrRootDirectory.Length())
			*pVal = ::SysAllocString(m_bstrRootDirectory.m_str);
	}
	return S_OK;
}

void CUFile::HandleRtn(unsigned short usRequestID, unsigned long ulLen, unsigned long ulLenInBuffer)
{
	USES_CONVERSION;
	memset(&m_WinFindData, 0, sizeof(m_WinFindData));
	memset(m_aSpaceData, 0, sizeof(m_aSpaceData));
	m_ulFilePointer = 0;
	m_uiDriveType = dtUnknown;
	if(!ulLen)
	{
		return;
	}
	unsigned long ulGet = 0;
	if(ulLen>m_tempQueue.GetMaxSize())
		m_tempQueue.ReallocBuffer(ulLen + 1024);
	m_hr = m_pIUFast->GetRtnBufferEx(ulLen, (BYTE*)m_tempQueue.GetBuffer(), &ulGet);
	ATLASSERT(ulGet == ulLen);
	m_tempQueue.SetSize(ulGet);
	m_tempQueue.Pop(&m_hr);
	if(m_hr != uecOK)
	{
		if(m_tempQueue.GetSize())
		{
			USES_CONVERSION;
			ATLASSERT(m_bstrErrorMsg.Length() == 0);
//			ATLASSERT((m_tempQueue.GetSize()%sizeof(TCHAR)) == 0); 
			//Server sends error messaqe in ASCII char
			m_bstrErrorMsg.m_str = ::SysAllocStringLen(A2OLE((LPSTR)m_tempQueue.GetBuffer()), m_tempQueue.GetSize()/sizeof(CHAR));
			m_tempQueue.SetSize(0);
		}
		if(usRequestID == idGetFile)
		{
			ATLASSERT(m_bGettingFile);
			CloseLocalFileGetFile(false);	
		}
		if(usRequestID == idSendFile)
		{
			m_bCanceled = false;
			CloseLocalFileSend();
		}
		return;
	}
	switch (usRequestID)
	{
	case idWriteFile:
	case idFindClose:
	case idClose:
	case idDeleteFile:
	case idSetFileAttributes:
	case idCreateDirectory:
	case idRemoveDirectory:
	case idMoveFile:
	case idCopyFile:
	case idSetCurrentDirectory:
	case idFlushFileBuffers:
	case idLockFile:
	case idUnlockFile:
	case idSetEndOfFile:
	case idCreateFile:
		break;
	case idFindAll:
	case idFindFile:
	case idFindFirstFile:
	case idFindNextFile:
		ATLASSERT(m_tempQueue.GetSize()==sizeof(m_WinFindData));
		m_tempQueue.Pop(&m_WinFindData);
		break;
	case idReadFile:
		m_rdQueue.Push(m_tempQueue.GetBuffer(), m_tempQueue.GetSize());
		m_tempQueue.SetSize(0);
		break;
	case idGetFileAttributes:
		ATLASSERT(m_tempQueue.GetSize()==sizeof(unsigned long));
		m_tempQueue.Pop(&m_WinFindData.dwFileAttributes);
		break;
	case idSetFilePointer:
		ATLASSERT(m_tempQueue.GetSize()==sizeof(unsigned long));
		m_tempQueue.Pop(&m_ulFilePointer);
		break;
	case idGetDiskFreeSpaceEx:
		ATLASSERT(m_tempQueue.GetSize()==sizeof(m_aSpaceData));
		m_tempQueue.Pop((BYTE*)m_aSpaceData, m_tempQueue.GetSize());
		break;
	case idGetDriveType:
		ATLASSERT(m_tempQueue.GetSize()==sizeof(UINT));
		m_tempQueue.Pop(&m_uiDriveType);
		break;
	case idGetFile:
		CloseLocalFileGetFile(true);
		break;
	case idSendFile:
		CloseLocalFileSend();
		m_bCanceled = false;
		m_pIUSocket->IgnorePendingRequests();
		break;
	case idGetFileSize:
		ATLASSERT(m_tempQueue.GetSize()==2*sizeof(unsigned long));
		m_tempQueue.Pop(&m_WinFindData.nFileSizeLow);
		m_tempQueue.Pop(&m_WinFindData.nFileSizeHigh);
		break;
	case idSearchPath:
		{
			unsigned long ulLen = 0;
			m_tempQueue.Pop(&ulLen);
			m_bstrFileName.Empty();
			if(ulLen)
			{
//				ATLASSERT((ulLen%sizeof(TCHAR))==0);
				//Server sends file name in ASCII char
				m_bstrFileName.m_str = ::SysAllocStringLen(A2OLE((LPSTR)m_tempQueue.GetBuffer()), ulLen/sizeof(CHAR));
				m_tempQueue.Pop(ulLen);
			}
			ulLen = m_tempQueue.GetSize();
			m_bstrFilePart.Empty();
			if(ulLen)
			{
//				ATLASSERT((ulLen%sizeof(TCHAR))==0);
				//Server sends file part in ASCII char
				m_bstrFilePart.m_str = ::SysAllocStringLen(A2OLE((LPSTR)m_tempQueue.GetBuffer()), ulLen/sizeof(CHAR));
				m_tempQueue.Pop(ulLen);
			}
			ATLASSERT(m_tempQueue.GetSize()==0);
		}
		break;
	case idGetCurrentDirectory:
		ATLASSERT(m_tempQueue.GetSize()<sizeof(m_WinFindData.cFileName));
		m_tempQueue.Pop((BYTE*)m_WinFindData.cFileName, sizeof(m_WinFindData.cFileName));
		break;
	case idSetRootDirectory:
		break;
	case idGetRootDirectory:
		m_bstrRootDirectory.Empty();
		ATLASSERT((m_tempQueue.GetSize()%sizeof(WCHAR))==0);
		m_bstrRootDirectory.m_str = ::SysAllocStringLen((LPOLESTR)m_tempQueue.GetBuffer(), m_tempQueue.GetSize()/sizeof(WCHAR));
		m_tempQueue.SetSize(0);
		break;
	case idSendBytesToServer:
		{
			long lBytes;
			m_pIUSocket->get_BytesInSndMemory(&lBytes);
			while(lBytes<DEFAULT_PACK_SIZE && (m_ulSendFileSize || m_ulSendFileSizeHigh) && !m_bCanceled)
			{
				DWORD dwRead = 0;
				ATLASSERT(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE);
				if(m_tempQueue.GetMaxSize()<DEFAULT_PACK_SIZE)
					m_tempQueue.ReallocBuffer(DEFAULT_PACK_SIZE);
				::ReadFile(m_hFileOpened, (void*)m_tempQueue.GetBuffer(), m_tempQueue.GetMaxSize(), &dwRead, NULL);
				m_tempQueue.SetSize(dwRead);
				if(dwRead == 0)
				{
					ATLASSERT(FALSE);
					Fire_OnRequestProcessed(m_hSocket, idSendFile, 0, 0, rfCompleted); 
					m_pIUSocket->get_BytesInSndMemory(&lBytes);
					break;
				}
				ATLASSERT((m_ulSendFileSize >= dwRead) || m_ulSendFileSizeHigh);
				m_pIUFast->SendRequestEx(idSendBytesToServer, dwRead, (BYTE*)m_tempQueue.GetBuffer());
				if(dwRead>m_ulSendFileSize)
				{
					ATLASSERT(m_ulSendFileSizeHigh);
					m_ulSendFileSizeHigh--;
					m_ulSendFileSize += 1;
					m_ulSendFileSize += (0xFFFFFFFF - dwRead);
				}
				else
				{
					m_ulSendFileSize -= dwRead;
				}
				Fire_OnRequestProcessed(m_hSocket, idSendFile, m_ulSendFileSize, m_ulSendFileSizeHigh, rfReceiving); // a litte confused here
				if(m_ulSendFileSize == 0 && m_ulSendFileSizeHigh == 0)
				{
					Fire_OnRequestProcessed(m_hSocket, idSendFile, 0, 0, rfCompleted); 
				}
				m_pIUSocket->get_BytesInSndMemory(&lBytes);
			}
		}
		break;
	case idSendBytesToClient:
		{
			DWORD dwWritten;
			if(m_bFirstPack)
			{
				m_tempQueue.Pop(&m_ulGetFileSize);
				m_tempQueue.Pop(&m_ulGetFileSizeHigh);
				Fire_OnRequestProcessed(m_hSocket, idGetFile, m_ulGetFileSize, m_ulGetFileSizeHigh, rfComing);
				m_bFirstPack = false;
			}
			if(m_tempQueue.GetSize())
			{
				ATLASSERT(m_ulGetFileSize >= m_tempQueue.GetSize() || m_ulGetFileSizeHigh);
				::WriteFile(m_hFileOpened, m_tempQueue.GetBuffer(), m_tempQueue.GetSize(), &dwWritten, NULL);
				ATLASSERT(dwWritten == m_tempQueue.GetSize());
				if(dwWritten <= m_ulGetFileSize)
				{
					m_ulGetFileSize -= dwWritten;
				}
				else
				{
					ATLASSERT(m_ulGetFileSizeHigh);
					m_ulGetFileSize += 1;
					m_ulGetFileSize += (0xFFFFFFFF - dwWritten);
					m_ulGetFileSizeHigh--;
				}
				Fire_OnRequestProcessed(m_hSocket, idGetFile, m_ulGetFileSize, m_ulGetFileSizeHigh, rfReceiving);
				if(m_ulGetFileSize == 0 && m_ulGetFileSizeHigh == 0)
				{
					Fire_OnRequestProcessed(m_hSocket, idGetFile, m_ulGetFileSize, m_ulGetFileSizeHigh, rfCompleted);
				}
			}
			m_tempQueue.SetSize(0);
		}
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}
	ATLASSERT(m_tempQueue.GetSize()==0);
}

STDMETHODIMP CUFile::GetFileAttributes(BSTR bstrFileName)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrFileName || !::wcslen(bstrFileName))
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	return m_pIUFast->SendRequestEx(idGetFileAttributes, ::wcslen(bstrFileName)*sizeof(WCHAR), (BYTE*)bstrFileName);
}

STDMETHODIMP CUFile::get_Data(VARIANT *pVal)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	unsigned long ulSize = m_rdQueue.GetSize();
	if(pVal)
	{
		::VariantClear(pVal);
	}
	if(ulSize)
	{
		BYTE	*pBuffer;
		SAFEARRAYBOUND sab[1] = {ulSize, 0};
		pVal->parray = ::SafeArrayCreate(VT_UI1, 1, sab);
		::SafeArrayAccessData(pVal->parray, (void**)&pBuffer);
		memcpy(pBuffer, m_rdQueue.GetBuffer(), ulSize);
		::SafeArrayUnaccessData(pVal->parray);
		pVal->vt = (VT_ARRAY|VT_UI1);
		m_rdQueue.SetSize(0);
	}
	return uecOK;
}

STDMETHODIMP CUFile::SetRootDirectory(BSTR bstrRootDirectory)
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(!bstrRootDirectory || !::wcslen(bstrRootDirectory))
	{
		return (m_hr = m_pIUFast->SendRequestEx(idSetRootDirectory, 0, NULL));
	}
	return m_pIUFast->SendRequestEx(idSetRootDirectory, ::wcslen(bstrRootDirectory)*sizeof(WCHAR), (BYTE*)bstrRootDirectory);
}

void CUFile::CloseLocalFileGetFile(bool bOk)
{
	USES_CONVERSION;
	ATLASSERT(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE);
	::CloseHandle(m_hFileOpened);
	if(!bOk)
		::DeleteFile(OLE2T(m_bstrLocalFile));
	m_bstrLocalFile.Empty();
	m_hFileOpened = 0;
	m_bGettingFile = false;
}

void CUFile::CloseLocalFileSend()
{
	ATLASSERT(m_bGettingFile == false);
	ATLASSERT(m_hFileOpened != 0 && m_hFileOpened != INVALID_HANDLE_VALUE);
	::CloseHandle(m_hFileOpened);
	m_hFileOpened = 0;
	m_ulSendFileSize = 0;
	m_ulSendFileSizeHigh = 0;
}

STDMETHODIMP CUFile::Cancel()
{
	CAutoLock AutoLock(&m_cs.m_sec);
	if(!m_pIUFast.p) //make sure the object is attached to a socket
	{
		m_hr = uecUnexpected;
		return m_hr;
	}
	if(m_ulSendFileSize)
	{
		m_pIUFast->SendRequestEx(idSendBytesToServer, 0, NULL);	
		m_bCanceled = true;
		m_pIUSocket->IgnorePendingRequests();
	}
	else
	{
		m_pIUSocket->Cancel();
	}
	return S_OK;
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