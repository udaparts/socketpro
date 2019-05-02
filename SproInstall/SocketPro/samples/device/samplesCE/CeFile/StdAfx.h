// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B56A4A07_373E_436E_AB4F_7DDB4BBE86C6__INCLUDED_)
#define AFX_STDAFX_H__B56A4A07_373E_436E_AB4F_7DDB4BBE86C6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#if _MSC_VER < 1350
	#define _ATL_DLL
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#if defined(_WIN32_WCE) && (_WIN32_WCE >= 211) && (_AFXDLL)
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

#include "ufile.h"
#include "svsevent.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B56A4A07_373E_436E_AB4F_7DDB4BBE86C6__INCLUDED_)
