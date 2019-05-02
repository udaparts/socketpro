// PiMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PiMFC.h"
#include "PiMFCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPiMFCDlg dialog
void CPiParallel::OnReturnedResultProcessed(CPPi *pHandler, IJobContext *pJobContext, unsigned short usRequestId)
{
	if(usRequestId == (unsigned short)idComputeCPPi)
	{
		//collecting data ......
		CAutoLock	AutoLock(&m_cs.m_sec);
		m_dPi += pHandler->m_ComputeRtn;
	}
}

void CPiParallel::OnJobDone(CPPi *pHandler, IJobContext *pJobContext)
{
	TCHAR str[1024] = {0};
	_stprintf_s(str, _T("JobDone, JobID = %d, Progress = %d\n"), (int)pJobContext->GetJobId(), GetProgress());
	ATLTRACE(str);
	::PostMessage(m_hWnd, WM_CUST_PI, 0, 0);
}

bool CPiParallel::OnFailover(CPPi *pHandler, IJobContext *pJobContext)
{
	TCHAR str[1024] = {0};
	_stprintf_s(str, _T("JobFail, JobID = %d, Progress = %d\n"), (int)pJobContext->GetJobId(), GetProgress());
	ATLTRACE(str);

	::PostMessage(m_hWnd, WM_CUST_PI, 0, 0);
	return true; //true or false. if true, do disaster recovery
}

CPiMFCDlg::CPiMFCDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CPiMFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPiMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPiMFCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SS, &CPiMFCDlg::OnBnClickedButtonSs)
	ON_BN_CLICKED(IDC_BUTTON_PR, &CPiMFCDlg::OnBnClickedButtonPr)
END_MESSAGE_MAP()


BOOL CPiMFCDlg::PreTranslateMessage(MSG *pMsg)
{
	switch(pMsg->message)
	{
	case WM_CUST_PI:
		UpdateControls();
		break;
	default:
		break;
	}
	return FALSE;
}

void CPiMFCDlg::UpdateControls()
{
	::Sleep(0);
	TCHAR str[2048] = {0};
	_stprintf_s(str, _T("AllLoaded: %d, Progress: %d, Parallels :%d, Fails: %d, Pause: %d, Working: %d, Pi: %f"), 
		m_PiMPI.IsAllLoaded() ? 1 : 0, 
		m_PiMPI.GetProgress(), m_PiMPI.GetSocketsInParallel(), 
		m_PiMPI.GetFails(), m_PiMPI.IsPaused() ? 1 : 0, m_PiMPI.IsWorking() ? 1 : 0, m_PiMPI.GetPi()
		);
	GetDlgItem(IDC_EDIT_PI)->SetWindowText(str);
	if(m_PiMPI.GetSocketsInParallel() == 0)
	{
		GetDlgItem(IDC_BUTTON_SS)->SetWindowText(_T("Start"));
		GetDlgItem(IDC_BUTTON_PR)->SetWindowText(_T("Pause"));
	}
}

// CPiMFCDlg message handlers
BOOL CPiMFCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != UNULL_PTR)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetDlgItem(IDC_BUTTON_SS)->EnableWindow(m_PiMPI.BuildConnections() ? TRUE : FALSE);

	m_PiMPI.m_hWnd = m_hWnd;

	m_PiMPI.SetRecvTimeout(5000);

	// TODO: Add extra initialization here
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPiMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPiMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPiMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPiMFCDlg::OnBnClickedButtonSs()
{
	CString str;
	GetDlgItem(IDC_BUTTON_SS)->GetWindowText(str);
	if(str == _T("Start"))
	{
		m_PiMPI.PrepareAndExecuteJobs();
		GetDlgItem(IDC_BUTTON_SS)->SetWindowText(_T("Stop"));
		GetDlgItem(IDC_BUTTON_PR)->EnableWindow(TRUE);
	}
	else
	{
		m_PiMPI.Stop();
		GetDlgItem(IDC_BUTTON_PR)->SetWindowText(_T("Pause"));
		GetDlgItem(IDC_BUTTON_SS)->SetWindowText(_T("Start"));
		GetDlgItem(IDC_BUTTON_PR)->EnableWindow(FALSE);
	}
}

void CPiMFCDlg::OnBnClickedButtonPr()
{
	CString str;
	GetDlgItem(IDC_BUTTON_PR)->GetWindowText(str);
	if(str == _T("Pause"))
	{
		m_PiMPI.Pause();
		GetDlgItem(IDC_BUTTON_PR)->SetWindowText(_T("Resume"));
	}
	else
	{
		m_PiMPI.Resume();
		GetDlgItem(IDC_BUTTON_PR)->SetWindowText(_T("Pause"));
	}
}
