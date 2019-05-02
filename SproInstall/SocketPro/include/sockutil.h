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


#ifndef	___SOCKPRO__UTIL_HEADER_FILE___H___
#define ___SOCKPRO__UTIL_HEADER_FILE___H___

namespace SocketProAdapter
{
	class CAutoLock
	{
	public:
		CAutoLock(CRITICAL_SECTION *pCriticalSection)
		{

			if(pCriticalSection)
			{
				::EnterCriticalSection(pCriticalSection);
			}
			m_pCriticalSection = pCriticalSection;
		}
		~CAutoLock()
		{
			if(m_pCriticalSection)
				::LeaveCriticalSection(m_pCriticalSection);
		}
	private:
		CRITICAL_SECTION	*m_pCriticalSection;
	};
	
	//use this class carefully!!!
	class CAutoReverseLock
	{
	public:
		CAutoReverseLock(CRITICAL_SECTION *pCriticalSection)
		{
			if(pCriticalSection)
			{
				//make sure that this is called within stack, and balanced with CAutoLock
#ifndef _WIN32_WCE
				ATLASSERT(pCriticalSection->RecursionCount > 0); 
#endif
				::LeaveCriticalSection(pCriticalSection);
			}
			m_pCriticalSection = pCriticalSection;
		}
		~CAutoReverseLock()
		{
			if(m_pCriticalSection)
				::EnterCriticalSection(m_pCriticalSection);
		}
	private:
		CRITICAL_SECTION	*m_pCriticalSection;
	};
};

#endif

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