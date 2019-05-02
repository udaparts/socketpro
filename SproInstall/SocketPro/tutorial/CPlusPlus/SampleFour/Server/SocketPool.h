// SocketPool.h : main header file for the SOCKETPOOL application
//

#if !defined(AFX_SOCKETPOOL_H__33EC993C_9D0C_427C_A8BE_F9EEB1773BF3__INCLUDED_)
#define AFX_SOCKETPOOL_H__33EC993C_9D0C_427C_A8BE_F9EEB1773BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSocketPoolApp:
// See SocketPool.cpp for the implementation of this class
//

class CSocketPoolApp : public CWinApp
{
public:
	CSocketPoolApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketPoolApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // return app exit code
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSocketPoolApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETPOOL_H__33EC993C_9D0C_427C_A8BE_F9EEB1773BF3__INCLUDED_)
