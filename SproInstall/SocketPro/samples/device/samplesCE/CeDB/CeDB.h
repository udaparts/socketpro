// CeDB.h : main header file for the CEDB application
//

#if !defined(AFX_CEDB_H__8A62D1C1_CCC6_4A87_B962_915C4E8F47BF__INCLUDED_)
#define AFX_CEDB_H__8A62D1C1_CCC6_4A87_B962_915C4E8F47BF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCeDBApp:
// See CeDB.cpp for the implementation of this class
//

class CCeDBApp : public CWinApp
{
public:
	CCeDBApp();
	virtual ~CCeDBApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDBApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCeDBApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEDB_H__8A62D1C1_CCC6_4A87_B962_915C4E8F47BF__INCLUDED_)
