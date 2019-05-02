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

#include "stdafx.h"
#include "perfquery.h"
#include <atlbase.h>

namespace SocketProAdapter
{
	CComAutoCriticalSection	g_cs;
	CUPerformanceQuery::CUPerformanceQuery()
	{
		pin_ptr<__int64> pFreq = &m_liFreq;
		::QueryPerformanceFrequency((LARGE_INTEGER*)pFreq);
		m_liCount = 0;
	}

	__int64 CUPerformanceQuery::Now()
	{
		pin_ptr<__int64> pCount = &m_liCount;
		::QueryPerformanceCounter((LARGE_INTEGER*)pCount);
		return m_liCount;
	}
	
	__int64 CUPerformanceQuery::Diff(__int64 liNow, __int64 liOld)
	{
		return (liNow - liOld) * 1000000 / m_liFreq;
	}

	__int64 CUPerformanceQuery::Diff(__int64 liOldCount)
	{
		Now();
		return Diff(m_liCount, liOldCount);
	}
}

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
