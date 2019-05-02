// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2A1E3A8D_340C_4344_B4CB_CB84F3F6B486__INCLUDED_)
#define AFX_STDAFX_H__2A1E3A8D_340C_4344_B4CB_CB84F3F6B486__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#define _ATL_FREE_THREADED
#include <objbase.h>

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include "sprowrap.h"

using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;
using namespace SocketProAdapter::ServerSide;
using namespace SocketProAdapter::ServerSide::PLG;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2A1E3A8D_340C_4344_B4CB_CB84F3F6B486__INCLUDED_)
