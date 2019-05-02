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


// ClientPeerFile.h: interface for the CClientPeerFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTPEERFILE_H__BE532C66_F6A1_430F_BA2E_6A95144902E0__INCLUDED_)
#define AFX_CLIENTPEERFILE_H__BE532C66_F6A1_430F_BA2E_6A95144902E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

class CClientPeerFile : public CClientPeer  
{
public:
	CClientPeerFile();
	virtual ~CClientPeerFile();

protected:
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen);
	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen);
	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo);
	virtual void OnSwitchFrom(unsigned long ulServiceID);

private:
	void SendClose();
	void FindClose();
	void CloseFile();
	void ReleaseResource();

public:
	bool				m_bReadOnly;
	//m_strRoot must be ended with '\' when it is set
	CComBSTR			m_strRoot;

private:
	unsigned long		m_ulSendingFileSizeHigh;
	unsigned long		m_ulSendingFileSize;
	HANDLE				m_hFindFile;
	CComBSTR			m_strSendFile;
	HANDLE				m_hSendFile;
	HANDLE				m_hFile;
};

#endif // !defined(AFX_CLIENTPEERFILE_H__BE532C66_F6A1_430F_BA2E_6A95144902E0__INCLUDED_)

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
