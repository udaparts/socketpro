// This is a part of the SocketPro package.
// Copyright (C) 2000-2005 UDAParts 
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


#ifndef ___UDAPARTS_UQUEUE_MFC__UTIL_H__
#define ___UDAPARTS_UQUEUE_MFC__UTIL_H__

#include "uqueue.h"

namespace SocketProAdapter
{
	/*
		Serialize/deserialize COleVariant and CString in MFC/ATL CString
	*/
	CUQueue& operator << (CUQueue &UQueue, const COleVariant &vtData )
	{
		UQueue.PushVT(vtData);
		return UQueue;
	}
	
	CUQueue& operator >> (CUQueue &UQueue, COleVariant &vtData )
	{
		UQueue.PopVT(vtData);
		return UQueue;
	}

	CUQueue& operator << (CUQueue &UQueue, const CString &str )
	{
		ULONG ulSize;
		if(LPCTSTR(str) == NULL)
			ulSize = UQUEUE_SIZE_NULL;
		else
			ulSize = str.GetLength() * sizeof(TCHAR);
		UQueue << ulSize;
		if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
		{
			UQueue.Push((BYTE*)LPCTSTR(str), ulSize);
		}
		return UQueue;
	}

	CUQueue& operator >> (CUQueue& UQueue, CString &str)
	{
		ULONG ulSize;
		ATLASSERT(UQueue.GetSize() >= sizeof(ulSize));
		UQueue >> ulSize;
		if(ulSize == UQUEUE_SIZE_NULL)
		{
			str.Empty();
		}
		else if (ulSize == 0)
		{
			str = _T("");
		}
		else
		{
			ATLASSERT((ulSize % sizeof(TCHAR)) == 0);
			ATLASSERT(UQueue.GetSize() >= ulSize);
			TCHAR *p = str.GetBuffer(ulSize/sizeof(TCHAR) + 1);
			UQueue.Pop((BYTE*)p, ulSize);
			p[ulSize/sizeof(TCHAR)] = 0;
			str.ReleaseBuffer();
		}
		return UQueue;
	}
}

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2005 UDAParts 
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
