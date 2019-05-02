// CeDB.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CeDB.h"
#include "CeDBDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCeDBApp

BEGIN_MESSAGE_MAP(CCeDBApp, CWinApp)
	//{{AFX_MSG_MAP(CCeDBApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeDBApp construction

CCeDBApp::CCeDBApp()
	: CWinApp()
{
	HRESULT hr = ::CoInitializeEx(0, COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/);
	hr = S_OK;
}

CCeDBApp::~CCeDBApp()
{
	::CoUninitialize();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCeDBApp object

CCeDBApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCeDBApp initialization

BOOL CCeDBApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CCeDBDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
