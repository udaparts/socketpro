// MySecureClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MySecureClient.h"
#include "MySecureClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientApp

BEGIN_MESSAGE_MAP(CMySecureClientApp, CWinApp)
	//{{AFX_MSG_MAP(CMySecureClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientApp construction

CMySecureClientApp::CMySecureClientApp()
{
	::CoInitialize(UNULL_PTR);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMySecureClientApp object

CMySecureClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientApp initialization

BOOL CMySecureClientApp::InitInstance()
{
	AfxEnableControlContainer();

	{
		CMySecureClientDlg dlg;
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
	}

	CScopeUQueue::DestroyUQueuePool();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
