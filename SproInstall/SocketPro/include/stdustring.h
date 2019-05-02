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


#ifndef ___UDAPARTS_UQUEUE_STD_STRING_H__
#define ___UDAPARTS_UQUEUE_STD_STRING_H__

#include <string>
#include "uqueue.h"


namespace SocketProAdapter
{

	CUQueue& operator << (CUQueue &UQueue, const std::wstring &wstr )
	{
		ULONG ulSize;
		if(wstr.empty())
			ulSize = UQUEUE_SIZE_NULL;
		else
			ulSize = (ULONG)wstr.size() * sizeof(wchar_t);
		UQueue << ulSize;
		if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
		{
			UQueue.Push((BYTE*)wstr.data(), ulSize);
		}
		return UQueue;
	}
	
	CUQueue& operator >> (CUQueue& UQueue, std::wstring &wstr)
	{
		ULONG ulSize;
		ATLASSERT(UQueue.GetSize() >= sizeof(ulSize));
		UQueue >> ulSize;
		wstr.clear();
		if(ulSize == UQUEUE_SIZE_NULL)
		{
			
		}
		else if(ulSize > 0)
		{
			ATLASSERT((ulSize % sizeof(wchar_t)) == 0);
			ATLASSERT(UQueue.GetSize() >= ulSize);
			wchar_t *p = (wchar_t*)UQueue.GetBuffer();
			wstr.append(p, ulSize/sizeof(wchar_t));
			UQueue.Pop(ulSize);
		}
		return UQueue;
	}

	CUQueue& operator << (CUQueue &UQueue, const std::string &str )
	{
		ULONG ulSize;
		if(str.empty())
			ulSize = UQUEUE_SIZE_NULL;
		else
			ulSize = (ULONG)str.size();
		UQueue << ulSize;
		if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
		{
			UQueue.Push((BYTE*)str.data(), ulSize);
		}
		return UQueue;
	}
	
	CUQueue& operator >> (CUQueue& UQueue, std::string &str)
	{
		ULONG ulSize;
		ATLASSERT(UQueue.GetSize() >= sizeof(ulSize));
		UQueue >> ulSize;
		str.clear();
		if(ulSize == UQUEUE_SIZE_NULL)
		{
			
		}
		else if(ulSize > 0)
		{
			str.clear();
			ATLASSERT(UQueue.GetSize() >= ulSize);
			char *p = (char*)UQueue.GetBuffer();
			str.append(p, ulSize);
			UQueue.Pop(ulSize);
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
