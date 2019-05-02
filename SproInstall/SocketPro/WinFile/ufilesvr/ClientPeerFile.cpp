// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
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

/*
	This source code is provided for your debug and study only.
	Don't distribute dll compiled from the source code.
	You must use the dll wfilesvr.dll within the package from UDAParts only.

*/

// ClientPeerFile.cpp: implementation of the CClientPeerFile class.
//
//////////////////////////////////////////////////////////////////////

#include "ClientPeerFile.h"
#include "ufile.h"
#include "ufilesvr.h"

DWORD GetErrorMsg(unsigned long ulErrorCode, TCHAR *strError, unsigned long ulChars);
extern unsigned long g_ulMaxMessageSize;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientPeerFile::CClientPeerFile()
{
	m_bReadOnly = false;
	m_ulSendingFileSizeHigh = 0;
	m_ulSendingFileSize = 0;
	m_hSendFile = NULL;
	m_hFindFile = NULL;
	m_hFile = NULL;
}

CClientPeerFile::~CClientPeerFile()
{
	ReleaseResource();	
}

void CClientPeerFile::OnSwitchFrom(unsigned long ulServiceID)
{
	CSwitchInfo SwitchInfo;
	g_SocketProLoader.GetServerInfo(GetSocket(), &SwitchInfo);
	SwitchInfo.m_ulParam5 &= (~TRANSFER_SERVER_EXCEPTION);
	g_SocketProLoader.SetServerInfo(GetSocket(), &SwitchInfo);
	SetMaxMessageSize(g_ulMaxMessageSize); //prevent DoS attacks
}

void CClientPeerFile::OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
{
	USES_CONVERSION;
	ATLASSERT(ulLen == m_UQueue.GetSize());
	HRESULT hr = S_OK;
	switch(usRequestID)
	{
	case idSendFile:  //A remote client is going to send us (SocketPro server) a file
		{
			DWORD	dwWritten = 0;
			VARIANT_BOOL bFailIfExists;
			unsigned short usLen;
			unsigned short usBackID = idSendBytesToServer;
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			SendClose();
			CComBSTR	bstrFile(m_strRoot);
			WCHAR strDesFile[_MAX_PATH+1] = {0};
			m_UQueue.Pop(&bFailIfExists);
			m_UQueue.Pop(&m_ulSendingFileSize);
			m_UQueue.Pop(&m_ulSendingFileSizeHigh);
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen && usLen<sizeof(strDesFile));
			ATLASSERT(m_ulSendingFileSize || m_ulSendingFileSizeHigh);
			m_UQueue.Pop((BYTE*)strDesFile, usLen);
			if(strDesFile[0] == L'\\')
				bstrFile += (strDesFile + 1);
			else
				bstrFile += strDesFile;
			m_hSendFile = ::CreateFile(OLE2T(bstrFile), (daGenericRead|daGenericWrite), smFileShareRead, 
				NULL, bFailIfExists ? cdCreateNew : cdCreateAlways, faNormal, NULL);
			if(m_hSendFile == INVALID_HANDLE_VALUE || m_hSendFile == 0)
			{
				m_UQueue.SetSize(0);
				hr = ::GetLastError();
				break;
			}
			ATLASSERT(m_ulSendingFileSize >= m_UQueue.GetSize() || m_ulSendingFileSizeHigh);
			m_strSendFile = bstrFile;
			if(!::WriteFile(m_hSendFile, m_UQueue.GetBuffer(), m_UQueue.GetSize(), &dwWritten, NULL))
			{
				hr = ::GetLastError();
				m_UQueue.SetSize(0);
				SendClose();
				break;
			}
			ATLASSERT(dwWritten == m_UQueue.GetSize());
			if(dwWritten > m_ulSendingFileSize)
			{
				ATLASSERT(m_ulSendingFileSizeHigh);
				m_ulSendingFileSize += 1;
				m_ulSendingFileSize += (0xFFFFFFFF - dwWritten);
				m_ulSendingFileSizeHigh--;
			}
			else
			{
				m_ulSendingFileSize -= dwWritten;
			}
			m_UQueue.SetSize(0);
			hr = uecOK;
			m_UQueue.Push(&hr);
			if(m_ulSendingFileSize == 0 && m_ulSendingFileSizeHigh == 0)
			{
				usBackID = idSendFile;
				::CloseHandle(m_hSendFile);
				m_hSendFile = NULL;
			}
			SendReturnData(usBackID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
			return;
		}
		break;
	case idCreateFile:
		{
			unsigned short usLen;
			unsigned long ulDesiredAccess;
			unsigned long ulShareMode;
			unsigned long ulCreationDisposition;
			unsigned long ulFlagsAndAttributes;
			CloseFile();
			CComBSTR	bstrFile(m_strRoot);
			WCHAR	strFile[_MAX_PATH+1] = {0};
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen && usLen<sizeof(strFile));
			m_UQueue.Pop((BYTE*)strFile, usLen);
			if(strFile[0]==L'\\')
			{
				bstrFile += (strFile+1);
			}
			else
			{
				bstrFile += strFile;
			}
			ATLASSERT(m_UQueue.GetSize()==sizeof(unsigned long)*4);
			m_UQueue.Pop(&ulDesiredAccess);
			m_UQueue.Pop(&ulShareMode);
			m_UQueue.Pop(&ulCreationDisposition);
			m_UQueue.Pop(&ulFlagsAndAttributes);
			if(m_bReadOnly)
			{
				ulDesiredAccess = (ulDesiredAccess &~ daGenericWrite);
				if(ulCreationDisposition != cdOpenExisting)
					ulCreationDisposition = cdOpenExisting;
			}
			m_hFile = ::CreateFile(OLE2T(bstrFile), ulDesiredAccess, ulShareMode, NULL,
				ulCreationDisposition, ulFlagsAndAttributes, NULL);
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == 0)
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idMoveFile:
		{
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			unsigned short usLen;
			CComBSTR	bstrExistingFile(m_strRoot);
			CComBSTR	bstrNewFile(m_strRoot);
			WCHAR	strFile[_MAX_PATH+1] = {0};
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen && usLen<sizeof(strFile));
			m_UQueue.Pop((BYTE*)strFile, usLen);
			if(strFile[0] == L'\\')
			{
				bstrExistingFile += (strFile+1);
			}
			else
			{
				bstrExistingFile += strFile;
			}
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen && usLen<sizeof(strFile));
			m_UQueue.Pop((BYTE*)strFile, usLen);
			if(strFile[0] == L'\\')
			{
				bstrNewFile += (strFile+1);
			}
			else
			{
				bstrNewFile += strFile;
			}
			if(!::MoveFile(OLE2T(bstrExistingFile), OLE2T(bstrNewFile)))
				hr = ::GetLastError();
		}
		break;
	case idSearchPath:
		{
			DWORD	dwFound = 0;
			unsigned short usLen;
			WCHAR *lpPath = NULL;  
			WCHAR lpFileName[_MAX_PATH + 1] = {0}; 
			WCHAR lpExtension[_MAX_PATH + 1] = {0}; 
			TCHAR *lpFilePart = NULL; 
			CComBSTR	bstrNewFile(m_strRoot);
			m_UQueue.Pop(&usLen);
			if(usLen)
			{
				WCHAR strPath[_MAX_PATH] = {0}; 
				ATLASSERT((usLen%sizeof(WCHAR))==0);
				m_UQueue.Pop((BYTE*)strPath, usLen);
				if(strPath[0]==L'\\')
					bstrNewFile += (strPath+1);
				else
					bstrNewFile += strPath;
			}
			if(bstrNewFile.Length())
				lpPath = bstrNewFile.m_str;
			m_UQueue.Pop(&usLen);
			if(usLen)
			{
				ATLASSERT((usLen%sizeof(WCHAR))==0);
				m_UQueue.Pop((BYTE*)lpFileName, usLen);
			}
			m_UQueue.Pop(&usLen);
			if(usLen)
			{
				ATLASSERT((usLen%sizeof(WCHAR))==0);
				m_UQueue.Pop((BYTE*)lpExtension, usLen);
			}

			ATLASSERT(m_UQueue.GetSize()==0);
			if(m_UQueue.GetMaxSize()<1025*sizeof(TCHAR))
				m_UQueue.ReallocBuffer(1024*sizeof(TCHAR) + sizeof(TCHAR));
			dwFound = ::SearchPath(OLE2T(lpPath), OLE2T(lpFileName), OLE2T(lpExtension), m_UQueue.GetMaxSize()/sizeof(TCHAR)-1, (TCHAR*)m_UQueue.GetBuffer(), &lpFilePart); 
			if(dwFound)
			{
				m_UQueue.SetSize(dwFound*sizeof(TCHAR));
				*((TCHAR*)m_UQueue.GetBuffer(dwFound*sizeof(TCHAR))) = 0; //get string null-terminated
				dwFound = (unsigned long)((BYTE*)lpFilePart - m_UQueue.GetBuffer());
				m_UQueue.Insert(&dwFound); 
			}
			else
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idClose:
		ATLASSERT(m_UQueue.GetSize()==0);
		CloseFile();
		break;
	case idFindClose:
		ATLASSERT(m_UQueue.GetSize()==0);
		FindClose();
		break;
	case idSetFileAttributes:
		{
			unsigned long ulFileAttributes;
			unsigned short usLen;
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			CComBSTR	bstrFile(m_strRoot);
			WCHAR	strFileName[_MAX_PATH+1] = {0};
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen && (usLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, usLen);
			ATLASSERT(m_UQueue.GetSize()==sizeof(ulFileAttributes));
			m_UQueue.Pop(&ulFileAttributes);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			if(!::SetFileAttributes(OLE2T(bstrFile.m_str), ulFileAttributes))
			{
				hr = ::GetLastError();
			}	
		}
		break;
	case idGetFileAttributes:
		{
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			DWORD dwAttributes = ::GetFileAttributes(OLE2T(bstrFile.m_str));
			ATLASSERT(m_UQueue.GetSize()==0);
			if(dwAttributes != 0xFFFFFFFF)
				m_UQueue.Push(&dwAttributes);
			else
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idDeleteFile:
		{
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			if(!::DeleteFile(OLE2T(bstrFile.m_str)))
			{
				hr = ::GetLastError();
			}	
		}
		break;
	case idCreateDirectory:
		{
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR)) == 0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			if(!::CreateDirectory(OLE2T(bstrFile.m_str), NULL))
			{
				hr = ::GetLastError();
			}	
		}
		break;
	case idRemoveDirectory:
		{
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR)) == 0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			if(!::RemoveDirectory(OLE2T(bstrFile.m_str)))
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idGetCurrentDirectory:
		ATLASSERT(m_UQueue.GetSize()==0);
		{
			TCHAR	strDirectory[_MAX_PATH+1]={0};
			if(::GetCurrentDirectory(_MAX_PATH+1, strDirectory))
			{
				m_UQueue.Push(strDirectory);
			}
			else
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idFindFile:
		{
			WIN32_FIND_DATA	Win32FindData;
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			memset(&Win32FindData, 0, sizeof(Win32FindData));
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			HANDLE hFileFound = ::FindFirstFile(OLE2T(bstrFile.m_str), &Win32FindData);
			if(hFileFound == INVALID_HANDLE_VALUE)
			{
				hr = ::GetLastError();
			}
			else
			{
				m_UQueue.Push(&Win32FindData);
				::FindClose(hFileFound);
			}
		}
		break;
	case idFindFirstFile:
		{
			HANDLE hFindFile = NULL;
			WIN32_FIND_DATA	Win32FindData;
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			memset(&Win32FindData, 0, sizeof(Win32FindData));
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			FindClose();
			m_hFindFile = ::FindFirstFile(OLE2T(bstrFile.m_str), &Win32FindData);
			if(m_hFindFile == INVALID_HANDLE_VALUE || m_hFindFile == NULL)
			{
				m_hFindFile = NULL;
				hr = ::GetLastError();
			}
			else
			{
				m_UQueue.Push(&Win32FindData);
			}
		}
		break;
	case idFindNextFile:
		ATLASSERT(m_UQueue.GetSize()==0);
		{
			if(m_hFindFile == INVALID_HANDLE_VALUE || m_hFindFile == NULL)
			{
				hr = uecUnexpected;
				break;
			}
			WIN32_FIND_DATA	Win32FindData;
			memset(&Win32FindData, 0, sizeof(Win32FindData));
			if(::FindNextFile(m_hFindFile, &Win32FindData))
			{
				m_UQueue.Push(&Win32FindData);
			}
			else
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idSetFilePointer:
		ATLASSERT(m_UQueue.GetSize()==2*sizeof(DWORD));
		{
			long lDistanceToMove;
			unsigned long ulMoveMethod;
			if(m_hFile == NULL || m_hFile == INVALID_HANDLE_VALUE)
			{
				m_UQueue.SetSize(0);
				hr = uecUnexpected;
				break;
			}
			m_UQueue.Pop(&lDistanceToMove);
			m_UQueue.Pop(&ulMoveMethod);
			ulMoveMethod = ::SetFilePointer(m_hFile, lDistanceToMove, NULL, ulMoveMethod);
			if(ulMoveMethod != 0xFFFFFFFF)
			{
				m_UQueue.Push(&ulMoveMethod); //new file pointer
			}
			else
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idSetCurrentDirectory:
		{
			if(m_bReadOnly)
			{
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			CComBSTR	bstrFile(m_strRoot);
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR)) == 0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			if(!::SetCurrentDirectory(OLE2T(bstrFile.m_str)))
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idFlushFileBuffers:
		ATLASSERT(m_UQueue.GetSize()==0);
		{
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL)
			{
				hr = uecUnexpected;
			}
			else
			{
				if(!::FlushFileBuffers(m_hFile))
				{
					hr = ::GetLastError();
				}
			}
		}
		break;
	case idGetDiskFreeSpaceEx:
		{
			ULARGE_INTEGER	uliData1;
			ULARGE_INTEGER	uliData2;
			ULARGE_INTEGER	uliData3;
			if(m_UQueue.GetSize()==0)
			{
				if(!::GetDiskFreeSpaceEx(NULL, &uliData1, &uliData2, &uliData3))
				{
					hr = ::GetLastError();
				}
				else
				{
					m_UQueue.Push(&uliData1);
					m_UQueue.Push(&uliData2);
					m_UQueue.Push(&uliData3);
				}
			}
			else
			{
				unsigned short wChar = 0;
				m_UQueue.Push((BYTE*)&wChar, sizeof(wChar));
				if(!::GetDiskFreeSpaceEx(OLE2T((WCHAR*)m_UQueue.GetBuffer()), &uliData1, &uliData2, &uliData3))
				{
					hr = ::GetLastError();
					m_UQueue.SetSize(0);
				}
				else
				{
					m_UQueue.SetSize(0);
					m_UQueue.Push(&uliData1);
					m_UQueue.Push(&uliData2);
					m_UQueue.Push(&uliData3);
				}
			}
		}
		break;
	case idGetDriveType:
		if(m_UQueue.GetSize()==0)
		{
			UINT uiType;
			CComBSTR	bstrFile(m_strRoot);
			if(bstrFile.Length())
			{
				uiType = ::GetDriveType(OLE2T(bstrFile.m_str));
			}
			else
			{
				uiType = ::GetDriveType(NULL);
			}
			m_UQueue.Push(&uiType);
		}
		else
		{
			CComBSTR	bstrFile(m_strRoot);
			WCHAR wChar = 0;
			m_UQueue.Push((BYTE*)&wChar, sizeof(wChar));
			WCHAR *pHead = (WCHAR*)m_UQueue.GetBuffer();
			if(*pHead == L'\\')
				pHead++;
			bstrFile += pHead;
			UINT uiType = ::GetDriveType(OLE2T(bstrFile.m_str));
			m_UQueue.SetSize(0);
			m_UQueue.Push(&uiType);
		}
		break;
	case idGetFileSize:
		ATLASSERT(m_UQueue.GetSize()==0);
		{
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL)
			{
				hr = uecUnexpected;
				break;
			}
			DWORD dwFileSizeHigh = 0;
			DWORD dwFileSizeLow = ::GetFileSize(m_hFile, &dwFileSizeHigh);
			if(dwFileSizeLow == 0xFFFFFFFF)
			{
				hr = ::GetLastError();
			}
			else
			{
				m_UQueue.Push(&dwFileSizeLow);
				m_UQueue.Push(&dwFileSizeHigh);
			}
		}
		break;
	case idLockFile:
		ATLASSERT(m_UQueue.GetSize()==4*sizeof(DWORD));
		{
			DWORD dwOne, dwTwo, dwThree, dwFour;
			m_UQueue.Pop(&dwOne);
			m_UQueue.Pop(&dwTwo);
			m_UQueue.Pop(&dwThree);
			m_UQueue.Pop(&dwFour);
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL)
			{
				hr = uecUnexpected;
			}
			else
			{
				if(!::LockFile(m_hFile, dwOne, dwTwo, dwThree, dwFour))
				{
					hr = ::GetLastError();
				}
			}
		}
		break;
	case idUnlockFile:
		ATLASSERT(m_UQueue.GetSize()==4*sizeof(DWORD));
		{
			DWORD dwOne, dwTwo, dwThree, dwFour;
			m_UQueue.Pop(&dwOne);
			m_UQueue.Pop(&dwTwo);
			m_UQueue.Pop(&dwThree);
			m_UQueue.Pop(&dwFour);
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL)
			{
				hr = uecUnexpected;
			}
			else
			{
				if(!::UnlockFile(m_hFile, dwOne, dwTwo, dwThree, dwFour))
				{
					hr = ::GetLastError();
				}
			}
		}
		break;
	case idSetEndOfFile:
		ATLASSERT(m_UQueue.GetSize()==0);
		{
			if(m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL)
			{
				hr = uecUnexpected;
			}
			else
			{
				if(!::SetEndOfFile(m_hFile))
				{
					hr = ::GetLastError();
				}
			}
		}
		break;
	case idSetRootDirectory:
		{
			WCHAR cEnd = 0;
			m_UQueue.Push(&cEnd, 1);
			m_strRoot = (LPCWSTR)m_UQueue.GetBuffer();
			m_UQueue.SetSize(0);
			ulLen = m_strRoot.Length();
			if(ulLen)
			{
				if(m_strRoot[ulLen-1] != L'\\')
				{
					m_strRoot[ulLen] += L'\\';
				}
			}
		}
		break;
	case idGetRootDirectory:
		{
			ATLASSERT(m_UQueue.GetSize()==0);
			m_UQueue.Push(LPCWSTR(m_strRoot));
		}
		break;
	default:
		break;
	}
	m_UQueue.Insert(&hr);
	if(hr != S_OK)
	{
		ATLASSERT(m_UQueue.GetSize() == sizeof(hr));
		if(hr == uecUnexpected)
		{
			m_UQueue.Push(_T("An unexpected error occurred."));
		}
		else if(hr == uecReadOnly)
		{
			m_UQueue.Push(_T("Access denied because of readonly."));  
		}
		else
		{
			TCHAR	strError[256] = {0};
			GetErrorMsg(hr, strError, sizeof(strError)/sizeof(TCHAR)-1);
			m_UQueue.Push(strError, ::_tcslen(strError)-1);  
		}
	}
	SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
}

long CClientPeerFile::OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
{
	ATLASSERT(m_UQueue.GetSize() == ulLen);
	HRESULT hr = uecOK;
	switch (usRequestID)
	{
	case idSendBytesToServer:
		{
			DWORD dwWritten;
			if(m_bReadOnly)
			{
				usRequestID = idSendFile;
				hr = uecReadOnly;
				m_UQueue.SetSize(0);
				break;
			}
			if(m_hSendFile == 0 || m_hSendFile == INVALID_HANDLE_VALUE)
			{
				usRequestID = idSendFile;
				m_UQueue.SetSize(0);
				hr = uecUnexpected;
				break;
			}
			ATLASSERT(m_strSendFile.Length() > 0);
			if(m_UQueue.GetSize()==0) //canceled
			{
				usRequestID = idSendFile;
				hr = uecRequestCanceled;
				SendClose();
				break;
			}
			if(!::WriteFile(m_hSendFile, m_UQueue.GetBuffer(), m_UQueue.GetSize(), &dwWritten, NULL))
			{
				usRequestID = idSendFile;
				m_UQueue.SetSize(0);
				hr = ::GetLastError();
				SendClose();
				break;
			}
			else
			{
				ATLASSERT(dwWritten == m_UQueue.GetSize());
				ATLASSERT(m_ulSendingFileSize >= dwWritten || m_ulSendingFileSizeHigh);
				if(dwWritten <= m_ulSendingFileSize)
				{
					m_ulSendingFileSize -= dwWritten;
				}
				else
				{
					ATLASSERT(m_ulSendingFileSizeHigh);
					m_ulSendingFileSize += 1;
					m_ulSendingFileSize += (0xFFFFFFFF - dwWritten);
					m_ulSendingFileSizeHigh--;
				}
			}
			m_UQueue.SetSize(0);
			if(m_ulSendingFileSize == 0 && m_ulSendingFileSizeHigh == 0)
			{
				hr = uecOK;
				SendReturnData(usRequestID, (BYTE*)&hr, sizeof(hr));
				usRequestID = idSendFile;
				::CloseHandle(m_hSendFile);
				m_hSendFile = NULL;
			}
		}
		break;
	case idReadFile:
		{
			DWORD	dwRead;
			DWORD	dwGet;
			if(m_hFile == 0 || m_hFile == INVALID_HANDLE_VALUE)
			{
				m_UQueue.SetSize(0);
				hr = uecUnexpected;
				break;
			}
			m_UQueue.Pop(&dwRead);
			ATLASSERT(m_UQueue.GetSize() == 0);
			if((dwRead+sizeof(hr)) > m_UQueue.GetMaxSize())
			{
				m_UQueue.ReallocBuffer(dwRead+sizeof(hr));
			}
			if(!::ReadFile(m_hFile, (BYTE*)m_UQueue.GetBuffer(), dwRead, &dwGet, NULL))
			{
				m_UQueue.SetSize(0);
				hr = ::GetLastError();
				break; 
			}
			else
			{
				m_UQueue.SetSize(dwGet);
			}
		}
		break;
	case idWriteFile:
		{
			DWORD dwWritten = 0;
			if(m_bReadOnly)
			{
				m_UQueue.SetSize(0);
				hr = uecReadOnly;
				break;
			}
			if(m_hFile == 0 || m_hFile == INVALID_HANDLE_VALUE)
			{
				m_UQueue.SetSize(0);
				hr = uecUnexpected;
				break;
			}
			if(!::WriteFile(m_hFile, m_UQueue.GetBuffer(), m_UQueue.GetSize(), &dwWritten, NULL))
			{
				hr = ::GetLastError();
			}
			else
			{
				ATLASSERT(dwWritten == m_UQueue.GetSize());
			}
			m_UQueue.SetSize(0);
		}
		break;
	case idFindAll:
		{
			USES_CONVERSION;
			DWORD		dwRtn;
			HANDLE		hFindFile;
			WIN32_FIND_DATA	Win32FindData;
			unsigned short usCount = 0;
			bool bIsBatching = IsBatching();
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			memset(&Win32FindData, 0, sizeof(Win32FindData));
			ATLASSERT(ulLen && (ulLen%sizeof(WCHAR))==0);
			m_UQueue.Pop((BYTE*)strFileName, ulLen);
			CComBSTR bstrFile(m_strRoot);
			if(strFileName[0]==L'\\')
				bstrFile += (strFileName+1);
			else
				bstrFile += strFileName;
			ATLASSERT(m_UQueue.GetSize()==0);
			hFindFile = ::FindFirstFile(OLE2T(bstrFile.m_str), &Win32FindData);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{
				hr = ::GetLastError();
				break;
			}
			else
			{
				if(!bIsBatching)
					StartBatching();
				m_UQueue.Push(&hr);
				m_UQueue.Push(&Win32FindData);
				dwRtn = SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				if(dwRtn == SOCKET_NOT_FOUND || dwRtn == REQUEST_CANCELED)
				{
					CommitBatching();
					::FindClose(hFindFile);
					return uecOK;
				}
			}
			while(::FindNextFile(hFindFile, &Win32FindData))
			{
				m_UQueue.SetSize(0);
				if(usCount == 50 || GetBytesBatched() > GET_FILE_BATCH_SIZE)  //send return results in batch to improve network efficiency, which is specially good to modems, low bandwith networks with long latency
				{
					CommitBatching();
					StartBatching();
					usCount = 0;
				}
				m_UQueue.Push(&hr);
				m_UQueue.Push(&Win32FindData);
				dwRtn = SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				if(dwRtn == SOCKET_NOT_FOUND || dwRtn == REQUEST_CANCELED)
					break;
				usCount++;
			}
			::FindClose(hFindFile);
			if(!(dwRtn == SOCKET_NOT_FOUND || dwRtn == REQUEST_CANCELED))
			{
				TCHAR	strError[256] = {0};
				m_UQueue.SetSize(0);
				hr = ::GetLastError();
				m_UQueue.Push(&hr);
				GetErrorMsg(hr, strError, sizeof(strError)/sizeof(TCHAR)-1);
				m_UQueue.Push(strError, ::_tcslen(strError)-1);  
				SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
			}
			if(!bIsBatching && dwRtn != SOCKET_NOT_FOUND)
				CommitBatching();
			return uecOK;
		}
		break;
	case idCopyFile:
		{
			USES_CONVERSION;
			unsigned short usLen;
			VARIANT_BOOL	bFailIfExists;
			WCHAR	strFileName[_MAX_PATH + 1] = {0};
			if(m_bReadOnly)
			{
				m_UQueue.SetSize(0);
				hr = uecReadOnly;
				break;
			}
			CComBSTR bstrExistingFileName(m_strRoot);
			CComBSTR bstrNewFileName(m_strRoot);
			m_UQueue.Pop(&usLen);
			ATLASSERT(usLen);
			m_UQueue.Pop((BYTE*)strFileName, usLen);
			if(strFileName[0]==L'\\')
				bstrExistingFileName += (strFileName+1);
			else
				bstrExistingFileName += strFileName;
			m_UQueue.Pop(&usLen);
			strFileName[usLen/sizeof(WCHAR)] = 0;
			m_UQueue.Pop((BYTE*)strFileName, usLen);
			if(strFileName[0]==L'\\')
				bstrNewFileName += (strFileName+1);
			else
				bstrNewFileName += strFileName;
			ATLASSERT(m_UQueue.GetSize() == sizeof(bFailIfExists));
			m_UQueue.Pop(&bFailIfExists);
			if(!::CopyFile(OLE2T(bstrExistingFileName), OLE2T(bstrNewFileName), bFailIfExists))
			{
				hr = ::GetLastError();
			}
		}
		break;
	case idGetFile:
		{
			USES_CONVERSION;
			BOOL	ok = FALSE;
			ULONG   dwFileSize = 0;
			ULONG	dwFileSizeHigh = 0;
			bool	bFirst = true;
			DWORD	dwRead = 0;
			DWORD	dwSent = 0;
			WCHAR	strFile[_MAX_PATH+1] = {0};
			CComBSTR bstrFile(m_strRoot);
			m_UQueue.Pop((BYTE*)strFile, sizeof(strFile));
			if(strFile[0]==L'\\')
				bstrFile += (strFile+1);
			else
				bstrFile += strFile;
			HANDLE hFile = ::CreateFile(OLE2T(bstrFile.m_str), daGenericRead, smFileShareRead, NULL,
				cdOpenExisting, faNormal, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				m_UQueue.SetSize(0);
				hr = ::GetLastError();
				break;
			}
			dwFileSize = ::GetFileSize(hFile, &dwFileSizeHigh);
			m_UQueue.ReallocBuffer(GET_FILE_BATCH_SIZE + 3*sizeof(hr));
			m_UQueue.Push(&hr);
			while((ok=::ReadFile(hFile, (BYTE*)m_UQueue.GetBuffer(sizeof(hr)), GET_FILE_BATCH_SIZE, &dwRead, NULL)))
			{
				if(dwRead == 0) //file end found
					break;
				m_UQueue.SetSize(dwRead+sizeof(hr));
				if(bFirst)
				{
					m_UQueue.Insert(&dwFileSizeHigh, sizeof(hr));
					m_UQueue.Insert(&dwFileSize, sizeof(hr));
					bFirst = false;
				}
				dwSent = SendReturnData(idSendBytesToClient, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				if(dwRead<GET_FILE_BATCH_SIZE)  //file end found
					break;
				if(dwSent == SOCKET_NOT_FOUND || dwSent == REQUEST_CANCELED)
					break;
				dwRead = 0;
			}
			m_UQueue.SetSize(0);
			if(dwSent==SOCKET_NOT_FOUND)
			{
				::CloseHandle(hFile);
				return uecOK;
			}
			else if(dwSent==REQUEST_CANCELED)
			{
				hr = uecRequestCanceled;
			}
			else
			{
				if(ok)
				{
					ATLASSERT(hr == uecOK);
				}
			}
			::CloseHandle(hFile);
		}
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}
	m_UQueue.Insert(&hr);
	if(hr != uecOK)
	{
		ATLASSERT(m_UQueue.GetSize() == sizeof(hr));
		if(hr == uecUnexpected)
		{
			m_UQueue.Push(_T("An unexpected error occurred."));
		}
		else if(hr == uecReadOnly)
		{
			m_UQueue.Push(_T("Access denied because of readonly."));  
		}
		else if(hr == uecRequestCanceled)
		{
			m_UQueue.Push(_T("Request is canceled."));
		}
		else
		{
			TCHAR	strError[256] = {0};
			GetErrorMsg(hr, strError, sizeof(strError)/sizeof(TCHAR)-1);
			m_UQueue.Push(strError, ::_tcslen(strError)-1);  
		}
	}
	SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
	return 0;
}


void CClientPeerFile::OnReleaseResource(bool bClosing, unsigned long ulInfo)
{
	ReleaseResource();
}

void CClientPeerFile::ReleaseResource()
{
	SendClose();
	CloseFile();
	FindClose();
	m_strRoot.Empty();
	m_bReadOnly = false;
}

void CClientPeerFile::CloseFile()
{
	if(m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

void CClientPeerFile::FindClose()
{
	if(m_hFindFile != NULL && m_hFindFile != INVALID_HANDLE_VALUE)
	{
		::FindClose(m_hFindFile);
		m_hFindFile = NULL;
	}	
}

void CClientPeerFile::SendClose()
{
	if(m_hSendFile != NULL && m_hSendFile != INVALID_HANDLE_VALUE)
	{
		USES_CONVERSION;
		::CloseHandle(m_hSendFile);
		m_hSendFile = NULL;
		ATLASSERT(m_strSendFile.Length());
		::DeleteFile(OLE2T(m_strSendFile));
	}
	m_ulSendingFileSizeHigh = 0;
	m_ulSendingFileSize = 0;
	m_strSendFile.Empty();
}

// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
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

/*
	This source code is provided for your debug and study only.
	Don't distribute dll compiled from the source code.
	You must use the dll wfilesvr.dll within the package from UDAParts only.

*/
