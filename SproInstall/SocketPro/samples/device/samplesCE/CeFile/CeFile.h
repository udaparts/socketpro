// CeFile.h : main header file for the CEFILE application
//

#if !defined(AFX_CEFILE_H__E564CB94_2FEC_44BF_A8C3_4ADB4F99765F__INCLUDED_)
#define AFX_CEFILE_H__E564CB94_2FEC_44BF_A8C3_4ADB4F99765F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCeFileApp:
// See CeFile.cpp for the implementation of this class
//

class CCeFileApp : public CWinApp
{
public:
	CCeFileApp();
	virtual ~CCeFileApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeFileApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCeFileApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEFILE_H__E564CB94_2FEC_44BF_A8C3_4ADB4F99765F__INCLUDED_)
