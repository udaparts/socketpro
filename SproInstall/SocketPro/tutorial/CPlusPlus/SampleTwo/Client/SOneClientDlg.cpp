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
	: CDialog(CSOneClientDlg::IDD, pParent), 
	m_MySvsHandler(sidCTOne, 
	this, //attach a client socket
	this //set an interface to IAsyncResultsHandler
	)
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
	m_bUseSSL = TRUE;
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
	DDX_Check(pDX, IDC_USESSL_CHECK, m_bUseSSL);
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
	ON_BN_CLICKED(IDC_USESSL_CHECK, OnUsesslCheck)
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

void CSOneClientDlg::Process(CAsyncResult &AsyncResult)
{
	switch(AsyncResult.RequestId)
	{
	case idQueryCountCTOne:
		AsyncResult.UQueue >> m_nCount;
		UpdateData(FALSE);
		break;
	case idQueryGlobalCountCTOne:
		AsyncResult.UQueue >> m_nGlobalCount;
		UpdateData(FALSE);
		break;
	case idQueryGlobalFastCountCTOne:
		AsyncResult.UQueue >> m_nGlobalFastCount;
		UpdateData(FALSE);
		break;
	case idEchoCTOne:
		m_vtOut.Clear();
		AsyncResult.UQueue >> m_vtOut;
		break;
	case idSleepCTOne:
		GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(TRUE);
		break;
	default:
		break;
	}
}

void CSOneClientDlg::OnExceptionFromServer(CAsyncServiceHandler &AsyncServiceHandler, CSocketProServerException &Exception)
{
	USES_CONVERSION;
	switch(Exception.m_usRequestID)
	{
	case idSleepCTOne:
		GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(TRUE);
		break;
	default:
		break;
	}
	MessageBox(W2T(Exception.m_strMessage));
}

void CSOneClientDlg::OnBaseRequestProcessed(unsigned short usBaseRequestID)
{
	switch(usBaseRequestID)
	{
		case idEnter:
		case idXEnter:
		{
			CString strMsg;
			long nGroup = 0;
			long nPort = 0;
			long nSvsID = 0;
			TCHAR strPort[12] = {0};
			CComBSTR strUID;
			CComBSTR strIPAddr;
			GetIUChat()->GetInfo(0, &nGroup, &strUID, &nSvsID, &nPort, &strIPAddr);
			strMsg = strUID;
			strMsg += _T("@");
			strMsg += strIPAddr.m_str;
			strMsg += _T(":");
#if _MSC_VER >= 1400
			::_itoa_s(nPort, strPort, sizeof(strPort)/sizeof(TCHAR), 10);
#else
			::_itot(nPort, strPort, 10);
#endif
			strMsg += strPort;
			strMsg += _T(" has just joined the group");
			GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMsg);
		}
			break;
		case idExit:
		{
			CString strMsg;
			long nGroup = 0;
			long nPort = 0;
			long nSvsID = 0;
			TCHAR strPort[12] = {0};
			CComBSTR strUID;
			CComBSTR strIPAddr;
			GetIUChat()->GetInfo(0, &nGroup, &strUID, &nSvsID, &nPort, &strIPAddr);
			strMsg = strUID;
			strMsg += _T("@");
			strMsg += strIPAddr.m_str;
			strMsg += _T(":");
#if _MSC_VER >= 1400
			::_itoa_s(nPort, strPort, sizeof(strPort)/sizeof(TCHAR), 10);
#else
			::_itot(nPort, strPort, 10);
#endif
			strMsg += strPort;
			strMsg += _T(" has just exited from the group");
			GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMsg);
		}
			break;
		case idSendUserMessage:
		case idSendUserMessageEx:
		case idSpeak:
		case idXSpeak:
		{
			CString strMsg;
			CComVariant vtMsg;
			long nGroup = 0;
			long nPort = 0;
			long nSvsID = 0;
			TCHAR strPort[12] = {0};
			CComBSTR strUID;
			CComBSTR strIPAddr;
			GetIUChat()->GetInfo(0, &nGroup, &strUID, &nSvsID, &nPort, &strIPAddr);
			GetIUChat()->get_Message(&vtMsg);
			strMsg = vtMsg.bstrVal;
			strMsg += _T(" from ");
			strMsg += strUID.m_str;
			strMsg += _T("@");
			strMsg += strIPAddr.m_str;
			strMsg += _T(":");
#if _MSC_VER >= 1400
			::_itoa_s(nPort, strPort, sizeof(strPort)/sizeof(TCHAR), 10);
#else
			::_itot(nPort, strPort, 10);
#endif
			strMsg += strPort;
			GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMsg);
		}
			break;
		case idSpeakEx:
		case idXSpeakEx:
			break;
		default:
			break;
	}
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
		short sEM = 0;
		GetIUSocket()->get_EncryptionMethod(&sEM);
		if(sEM != NoEncryption && sEM != BlowFish)
		{
			long lErrorCode;
			CComVariant vtPeerCert;
			CComBSTR bstrSubject, bstrErrorInfo;
			
			HRESULT hr = GetIUSocket()->get_PeerCertificate(&vtPeerCert);
			CComQIPtr<IUCert> pIUCert = vtPeerCert.punkVal;
			
			hr = pIUCert->get_Subject(&bstrSubject);
			
			//check certificate subject here

			hr = pIUCert->Verify(&lErrorCode, &bstrErrorInfo);
			
			//verify certificate chain

			//authenticate a remote server before sending password by verifing a certificate
		}

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
	if(IsConnected())
	{
		GetIUSocket()->TurnOnZipAtSvr(m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
	}
}

void CSOneClientDlg::OnFrozenCheck() 
{
	UpdateData(TRUE);
	DisableUI(m_bFrozen ? true : false);
}

void CSOneClientDlg::OnConnectButton() 
{
	UpdateData(TRUE);
	if(IsDlgButtonChecked(IDC_USESSL_CHECK))
	{
		GetIUSocket()->put_EncryptionMethod(MSTLSv1);
	}
	else
	{
		GetIUSocket()->put_EncryptionMethod(NoEncryption);
	}
	Connect(m_strHost, m_nPort);	
}

void CSOneClientDlg::OnDisconnectButton() 
{
	Shutdown();
}

void CSOneClientDlg::OnSleepButton() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(FALSE);
	m_MySvsHandler.SendRequest(idSleepCTOne, m_nSleep);
}

void CSOneClientDlg::OnGetallcountsButton() 
{
	BeginBatching();
	//Use IAsyncResultHandler::Process for processing returning result
	m_MySvsHandler(idQueryCountCTOne)(idQueryGlobalCountCTOne)(idQueryGlobalFastCountCTOne); 
	
	Commit(true); //requests batch, results batch
}

void CSOneClientDlg::OnDuerycountButton() 
{
#if _MSC_VER >= 1600
	//use Lambda expression
	m_MySvsHandler.SendRequest(TA_R(ar){
		Process(ar);
	}, idQueryCountCTOne);
#else
	//you can bind it to a global callback for async result
	m_MySvsHandler.SendRequest(idQueryCountCTOne, MyProcess);
#endif
}

void CSOneClientDlg::MyProcess(CAsyncResult &AsyncResult)
{
	CSOneClientDlg *p = (CSOneClientDlg*)AsyncResult.AsyncServiceHandler->GetAsyncResultsHandler();
	p->Process(AsyncResult);
}

void CSOneClientDlg::OnGlobalcountButton() 
{
#if _MSC_VER >= 1600
	//use Lambda expression
	m_MySvsHandler.SendRequest(TA_R(ar){
		Process(ar);
	}, idQueryGlobalCountCTOne);
#else
	//you can bind it to a global callback for async result
	m_MySvsHandler.SendRequest(idQueryGlobalCountCTOne, MyProcess);
#endif
}

void CSOneClientDlg::OnGlobalfastcountButton() 
{
#if _MSC_VER >= 1600
	//use Lambda expression
	m_MySvsHandler.SendRequest(TA_R(ar){
		Process(ar);
	}, idQueryGlobalFastCountCTOne);
#else
	//you can bind it to a global callback for async result
	m_MySvsHandler.SendRequest(idQueryGlobalFastCountCTOne, MyProcess);
#endif
}

void CSOneClientDlg::OnEchodataButton() 
{
	CComVariant vtData(L"This is a test string");
	CComVariant vtMsg(L"Test message from method Echo ");
	unsigned long pGroups[] = {1, 2, 3, 4, 5};
	BeginBatching();
	m_MySvsHandler.SendRequest(idEchoCTOne, vtData);
	GetPush()->Broadcast(vtMsg, pGroups, 5);
	Commit(true);
}

void CSOneClientDlg::OnUsesslCheck() 
{
	// TODO: Add your control notification handler code here
	
}
