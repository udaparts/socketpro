// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#pragma warning(disable: 4996)	//'_vsnprintf' was declared deprecated
#pragma warning(disable: 4100)	//'UQueue' : unreferenced formal parameter
#pragma warning(disable: 4793)	//'vararg' : causes native code generation for function 'void ATL::AtlTrace(LPCWSTR,...)'
								//'__asm' : causes native code generation for function 'void ATL::CComStdCallThunkHelper(void)'
#pragma warning(disable: 4019)	//hile compiling class template member function 'HRESULT ATL::IDispEventSimpleImpl<nID,T,pdiid>::GetTypeInfoCount(UINT *)'
#pragma warning(disable: 4564)	//method 'GetAllListeners' of interface 'USOCKETLib::IUChat' defines unsupported default parameter 'lGroups'
#pragma warning(disable: 4035)	//'pEvent' : local variable is initialized but not referenced
#pragma warning(disable: 4189)	//'pT' : local variable is initialized but not referenced

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif


#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0410	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#define	TRANSFER_SERVER_EXCEPTION	(0x40000000)
#define RETURN_RESULT_RANDOM		((unsigned short)0x4000)

#define SOCKETPRO_MAX_BASE_REQUEST_ID		45

//turn on or off the following two features
//#define __DONT_NEED_REMOTE_DB_SERVICE__
#define __NO_OPEN_MP__ //no OpenMP

#include "sproadapter.h"
#include "uqueueinternal.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Web;
using namespace System::IO;
using namespace System::Net;
using namespace System::Web::UI;


#include "muqueue.h"
#include "perfquery.h"

#include "RequestAsynHandlerBase.h"
#include "clientSocket.h"
#include "socketpool.h"

#ifndef _WIN32_WCE
#include "internalsocketpro.h"
#include "SocketProServer.h"
#include "ClientPeer.h"
#include "BaseService.h"
#endif

namespace SocketProAdapter
{
namespace ClientSide
{
	
}
}
