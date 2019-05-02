// ClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"

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


// CClientDlg dialog




CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientDlg::IDD, pParent), m_ash(sidRemSum, this, this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DOSUM_BUTTON, &CClientDlg::OnBnClickedDosumButton)
	ON_BN_CLICKED(IDC_REDOSUM_BUTTON, &CClientDlg::OnBnClickedRedosumButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &CClientDlg::OnBnClickedPauseButton)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	Connect(_T("localhost"), 20901);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientDlg::Process(CAsyncResult &AsyncResult)
{
	switch(AsyncResult.RequestId)
	{
	case idDoSumRemSum:
	case idPauseRemSum:
	case idRedoSumRemSum:
		{
			int rtn;
			CString str;
			AsyncResult.UQueue >> rtn;
			str.Format(_T("%d"), rtn);
			GetDlgItem(IDC_SUM_EDIT)->SetWindowTextW(str);
		}
		break;
	case idReportProgress:
		{
			int nWhere, rtn;
			CString str;
			AsyncResult.UQueue >> nWhere >> rtn;
			str.Format(_T("Where = %d, Sum = %d"), nWhere, rtn);
			GetDlgItem(IDC_SUM_EDIT)->SetWindowTextW(str);
		}
		break;
	default:
		break;
	}
}

HRESULT CClientDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_DOSUM_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_REDOSUM_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CClientDlg::OnSocketConnected(long hSocket, long lError)
{
	if(lError == 0)
	{
		GetDlgItem(IDC_DOSUM_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_REDOSUM_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(TRUE);
		SwitchTo(sidRemSum);
	}
	else
	{
		CComBSTR bstrErrMsg = GetErrorMsg();
		MessageBox(bstrErrMsg);
	}
	return S_OK;
}


void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CClientDlg::OnBnClickedDosumButton()
{
	m_ash.SendRequest(idDoSumRemSum, 100, 400);
}

void CClientDlg::OnBnClickedRedosumButton()
{
	m_ash.SendRequest(idRedoSumRemSum);
}

void CClientDlg::OnBnClickedPauseButton()
{
	BeginBatching();
	//send a cancel request onto a remote server. Here is a big secret from SocketPro!
	Cancel();
	m_ash.SendRequest(idPauseRemSum);
	Commit(true); //send requests in one shot, and ask server return results in one short
}
