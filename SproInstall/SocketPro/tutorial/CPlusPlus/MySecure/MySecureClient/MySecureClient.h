// MySecureClient.h : main header file for the MYSECURECLIENT application
//

#if !defined(AFX_MYSECURECLIENT_H__BFF2C119_D2D2_4A0B_9A72_091F21C47ED1__INCLUDED_)
#define AFX_MYSECURECLIENT_H__BFF2C119_D2D2_4A0B_9A72_091F21C47ED1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientApp:
// See MySecureClient.cpp for the implementation of this class
//

class CMySecureClientApp : public CWinApp
{
public:
	CMySecureClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySecureClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMySecureClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSECURECLIENT_H__BFF2C119_D2D2_4A0B_9A72_091F21C47ED1__INCLUDED_)
