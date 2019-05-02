// SOneClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SOneClient.h"
#include "SOneClientDlg.h"

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
// CSOneClientDlg dialog

CSOneClientDlg::CSOneClientDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CSOneClientDlg::IDD, pParent), m_MySvsHandler(this)
{
	//{{AFX_DATA_INIT(CSOneClientDlg)
	m_strHost = _T("localhost");
	m_strPassword = _T("PassOne");
	m_nPort = 20901;
	m_nSleep = 5000;
	m_strUID = _T("SocketPro");
	m_bZip = FALSE;
	m_bFrozen = FALSE;
	m_nCount = 0;
	m_nGlobalCount = 0;
	m_nGlobalFastCount = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CSOneClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSOneClientDlg)
	DDX_Text(pDX, IDC_HOST_EDIT, m_strHost);
	DDX_Text(pDX, IDC_PASSWORD_EDIT, m_strPassword);
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDX_Text(pDX, IDC_SLEEP_EDIT, m_nSleep);
	DDX_Text(pDX, IDC_UID_EDIT, m_strUID);
	DDX_Check(pDX, IDC_ZIP_CHECK, m_bZip);
	DDX_Check(pDX, IDC_FROZEN_CHECK, m_bFrozen);
	DDX_Text(pDX, IDC_COUNT_EDIT, m_nCount);
	DDX_Text(pDX, IDC_GLOBALCOUNT_EDIT, m_nGlobalCount);
	DDX_Text(pDX, IDC_GLOBALFASTCOUNT_EDIT, m_nGlobalFastCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSOneClientDlg, CDialog)
	//{{AFX_MSG_MAP(CSOneClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ZIP_CHECK, OnZipCheck)
	ON_BN_CLICKED(IDC_FROZEN_CHECK, OnFrozenCheck)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnDisconnectButton)
	ON_BN_CLICKED(IDC_SLEEP_BUTTON, OnSleepButton)
	ON_BN_CLICKED(IDC_GETALLCOUNTS_BUTTON, OnGetallcountsButton)
	ON_BN_CLICKED(IDC_DUERYCOUNT_BUTTON, OnDuerycountButton)
	ON_BN_CLICKED(IDC_GLOBALCOUNT_BUTTON, OnGlobalcountButton)
	ON_BN_CLICKED(IDC_GLOBALFASTCOUNT_BUTTON, OnGlobalfastcountButton)
	ON_BN_CLICKED(IDC_ECHODATA_BUTTON, OnEchodataButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSOneClientDlg message handlers

BOOL CSOneClientDlg::OnInitDialog()
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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSOneClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSOneClientDlg::OnPaint() 
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
HCURSOR CSOneClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

HRESULT CSOneClientDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GETALLCOUNTS_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_DUERYCOUNT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GLOBALCOUNT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GLOBALFASTCOUNT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_ECHODATA_BUTTON)->EnableWindow(FALSE);
	if(lError != 0)
	{
		USES_CONVERSION;
		MessageBox(OLE2T(GetErrorMsg()));
	}
	return S_OK;
}

HRESULT CSOneClientDlg::OnSocketConnected(long hSocket, long lError)
{
	if(lError == S_OK)
	{
		UpdateData(TRUE);
		SetUID(m_strUID);
		SetPassword(m_strPassword);
		BeginBatching();
		SwitchTo(&m_MySvsHandler, true);
		GetIUSocket()->TurnOnZipAtSvr(m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
		Commit(false); //must be false for the very first switch

		GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_GETALLCOUNTS_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_DUERYCOUNT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_GLOBALCOUNT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_GLOBALFASTCOUNT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_ECHODATA_BUTTON)->EnableWindow(TRUE);
	}
	else
	{
		USES_CONVERSION;
		CComBSTR bstrErrorMsg = GetErrorMsg();
		MessageBox(OLE2T(bstrErrorMsg));
	}
	return S_OK;
}


void CSOneClientDlg::OnZipCheck() 
{
	UpdateData(TRUE);
	GetIUSocket()->put_ZipIsOn(m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
}

void CSOneClientDlg::OnFrozenCheck() 
{
	UpdateData(TRUE);
	DisableUI(m_bFrozen ? true : false);
}

void CSOneClientDlg::OnConnectButton() 
{
	UpdateData(TRUE);
	Connect(m_strHost, m_nPort);	
}

void CSOneClientDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CSOneClientDlg::OnSleepButton() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(FALSE);
	m_MySvsHandler.Sleep(m_nSleep);
	if(IsConnected())
		GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(TRUE);
}

void CSOneClientDlg::OnGetallcountsButton() 
{
	m_MySvsHandler.GetAllCounts(m_nCount, m_nGlobalCount, m_nGlobalFastCount);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnDuerycountButton() 
{
	m_nCount = m_MySvsHandler.QueryCount();
	UpdateData(FALSE);
}

void CSOneClientDlg::OnGlobalcountButton() 
{
	m_nGlobalCount = m_MySvsHandler.QueryGlobalCount();
	UpdateData(FALSE);
}

void CSOneClientDlg::OnGlobalfastcountButton() 
{
	m_nGlobalFastCount = m_MySvsHandler.QueryGlobalFastCount();
	UpdateData(FALSE);
}

void CSOneClientDlg::OnEchodataButton() 
{
	CComVariant vtData(L"This is a test string");
	CComVariant vtOut = m_MySvsHandler.Echo(vtData);
	vtData.Clear();
}