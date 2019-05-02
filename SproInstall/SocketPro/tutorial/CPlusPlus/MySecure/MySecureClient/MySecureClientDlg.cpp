// MySecureClientDlg.cpp : implementation file
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
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientDlg dialog

CMySecureClientDlg::CMySecureClientDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CMySecureClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMySecureClientDlg)
	m_nPort = 20901;
	m_strHost = _T("localhost");
	m_strPassword = _T("PassOne");
	m_strSQL = _T("Delete from Shippers Where ShipperID > 3");
	m_strUserID = _T("SocketPro");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMySecureClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMySecureClientDlg)
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDX_Text(pDX, IDC_HOST_EDIT, m_strHost);
	DDX_Text(pDX, IDC_PASSWORD_EDIT, m_strPassword);
	DDX_Text(pDX, IDC_SQL_EDIT, m_strSQL);
	DDX_Text(pDX, IDC_USERID_EDIT, m_strUserID);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMySecureClientDlg, CDialog)
	//{{AFX_MSG_MAP(CMySecureClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnDisconnectButton)
	ON_BN_CLICKED(IDC_EXECUTESQL_BUTTON, OnExecutesqlButton)
	ON_BN_CLICKED(IDC_TODB_BUTTON, OnTodbButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientDlg message handlers

BOOL CMySecureClientDlg::OnInitDialog()
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

	m_MySecure.Attach(this);
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMySecureClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMySecureClientDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMySecureClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

HRESULT CMySecureClientDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_TODB_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_EXECUTESQL_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CMySecureClientDlg::OnSocketConnected(long hSocket, long lError)
{
	if(lError == 0)
	{
		GetDlgItem(IDC_TODB_BUTTON)->EnableWindow(TRUE);
		
		GetIUSocket()->put_EncryptionMethod(BlowFish);
		
		CString str;

		GetDlgItem(IDC_USERID_EDIT)->GetWindowText(str);
		SetUID(str);

		GetDlgItem(IDC_PASSWORD_EDIT)->GetWindowText(str);
		SetPassword(str);

		SwitchTo(&m_MySecure);
	}
	return S_OK;
}

void CMySecureClientDlg::OnConnectButton() 
{
	Connect(m_strHost, m_nPort);	
}

void CMySecureClientDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CMySecureClientDlg::OnExecutesqlButton() 
{
	UpdateData(TRUE);

	//No encryption is required
	GetIUSocket()->put_EncryptionMethod(NoEncryption);
	BeginBatching();
	m_MySecure.BeginTransAsyn();
	m_MySecure.ExecuteNoQueryAsyn(CComBSTR(m_strSQL));
	m_MySecure.CommitAsyn(true);	
	Commit(true);
	WaitAll();
}

void CMySecureClientDlg::OnTodbButton() 
{
	//Encryption is required for securely sending password to a remote server
	GetIUSocket()->put_EncryptionMethod(BlowFish);
	const CComBSTR &bstr = m_MySecure.Open(CComBSTR(m_strUserID), CComBSTR(m_strPassword));
	if(bstr.Length() > 0)
	{
		GetDlgItem(IDC_EXECUTESQL_BUTTON)->EnableWindow(TRUE);
	}
	else
	{
		MessageBox(CString(m_MySecure.m_bstrErrorMessage));	
	}
}
