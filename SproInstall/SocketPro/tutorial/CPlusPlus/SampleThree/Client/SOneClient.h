// SOneClient.h : main header file for the SONECLIENT application
//

#if !defined(AFX_SONECLIENT_H__6C026B9D_29D5_4519_8124_2AD76C19D448__INCLUDED_)
#define AFX_SONECLIENT_H__6C026B9D_29D5_4519_8124_2AD76C19D448__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSOneClientApp:
// See SOneClient.cpp for the implementation of this class
//

class CSOneClientApp : public CWinApp
{
public:
	CSOneClientApp();
	~CSOneClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSOneClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSOneClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONECLIENT_H__6C026B9D_29D5_4519_8124_2AD76C19D448__INCLUDED_)
