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


#ifndef ___SOCKETPRO_WRAPPER_C_PLUS_PLUS_DEVELOPMENT_H____
#define ___SOCKETPRO_WRAPPER_C_PLUS_PLUS_DEVELOPMENT_H____


namespace SocketProAdapter
{
	extern CComAutoCriticalSection	g_cs;
	[CLSCompliantAttribute(true)] 
	public ref class CUPerformanceQuery
	{
	public:
		CUPerformanceQuery();
	
	public:
		//return high frequency time at this time
		__int64 Now();

		__int64 Diff(__int64 lNow, __int64 lPrevCount);

		//return time difference from current time in micro-second
		__int64 Diff(__int64 lPrevCount);

	private:
		__int64 m_liFreq;
		__int64 m_liCount;
	};
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