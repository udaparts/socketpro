// DevTest.h : main header file for the DEVTEST application
//

#if !defined(AFX_DEVTEST_H__6FED2981_6301_4514_9541_A877AFD51FDF__INCLUDED_)
#define AFX_DEVTEST_H__6FED2981_6301_4514_9541_A877AFD51FDF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDevTestApp:
// See DevTest.cpp for the implementation of this class
//

class CDevTestApp : public CWinApp
{
public:
	CDevTestApp();
	~CDevTestApp();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDevTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVTEST_H__6FED2981_6301_4514_9541_A877AFD51FDF__INCLUDED_)
