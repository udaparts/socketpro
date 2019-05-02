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


#ifndef ___UDAPARTS_UQUEUE_COM_UTIL_H__
#define ___UDAPARTS_UQUEUE_COM_UTIL_H__

#include "uqueue.h"
#include <comutil.h>

#pragma comment(lib, "comsupp.lib")

namespace SocketProAdapter
{
	CUQueue& operator << (CUQueue &UQueue, const _variant_t &vtData )
	{
		UQueue.PushVT(vtData);
		return UQueue;
	}
	
	CUQueue& operator >> (CUQueue &UQueue, _variant_t &vtData )
	{
		UQueue.PopVT(vtData);
		return UQueue;
	}

	CUQueue& operator << (CUQueue &UQueue, const _bstr_t &bstr )
	{
		ULONG ulSize;
		if(LPCWSTR(bstr) == NULL)
			ulSize = UQUEUE_SIZE_NULL;
		else
			ulSize = bstr.length() * sizeof(WCHAR);
		UQueue << ulSize;
		if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
		{
			UQueue.Push((BYTE*)LPCWSTR(bstr), ulSize);
		}
		return UQueue;
	}

	CUQueue& operator >> (CUQueue& UQueue, _bstr_t &bstr)
	{
		ULONG ulSize;
		ATLASSERT(UQueue.GetSize() >= sizeof(ulSize));
		UQueue >> ulSize;
		if(ulSize == UQUEUE_SIZE_NULL)
		{
			bstr = (LPCWSTR)NULL;
		}
		else if (ulSize == 0)
		{
			bstr = L"";
		}
		else
		{
			ATLASSERT((ulSize % sizeof(WCHAR)) == 0);
			ATLASSERT(UQueue.GetSize() >= ulSize);
			BSTR bstrTemp = ::SysAllocStringLen((LPCWSTR)UQueue.GetBuffer(), ulSize/sizeof(WCHAR));
#ifdef VC_PLUSPLUS_70_OR_LATER || (_MSC_VER >= 1300)
			//faster because there is no need to create a new BSTR
			bstr.Attach(bstrTemp);
#else
			bstr.Assign(bstrTemp);
			::SysFreeString(bstrTemp);
#endif
			UQueue.Pop(ulSize);
		}
		return UQueue;
	}
};

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
