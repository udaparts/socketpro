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
	: CDialog(CSOneClientDlg::IDD, pParent)//, m_ashEx(sidCTOne, this)
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
	m_nTimeRequired = 0;
	m_lBytesRecv = 0;
	m_lBytesSent = 0;
	m_lGetALot = 50000;
	m_lLatency = 0;
	m_lSendALot = 50000;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

CSOneClientDlg::~CSOneClientDlg()
{
	CleanStack();
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
	DDX_Text(pDX, IDC_TIMEREQUIRED_EDIT, m_nTimeRequired);
	DDX_Text(pDX, IDC_BYTESRECEIVED_EDIT, m_lBytesRecv);
	DDX_Text(pDX, IDC_BYTESSENT_EDIT, m_lBytesSent);
	DDX_Text(pDX, IDC_GETALOT_EDIT, m_lGetALot);
	DDV_MinMaxLong(pDX, m_lGetALot, 0, 10000000);
	DDX_Text(pDX, IDC_LATENCY_EDIT, m_lLatency);
	DDX_Text(pDX, IDC_SENDALOT_EDIT, m_lSendALot);
	DDV_MinMaxUInt(pDX, m_lSendALot, 0, 10000000);
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
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_USESSL_CHECK, OnUsesslCheck)
	ON_BN_CLICKED(IDC_GETONE_BUTTON, OnGetoneButton)
	ON_BN_CLICKED(IDC_GETALOT_BUTTON, OnGetalotButton)
	ON_BN_CLICKED(IDC_SENDONE_BUTTON, OnSendoneButton)
	ON_BN_CLICKED(IDC_SENDALOT_BUTTON, OnSendalotButton)
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
			::_itot(nPort, strPort, 10);
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
			::_itot(nPort, strPort, 10);
			strMsg += strPort;
			strMsg += _T(" has just exited from the group");
			GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMsg);
		}
			break;
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
			::_itot(nPort, strPort, 10);
			strMsg += strPort;
			GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMsg);
		}
			break;
		default:
			break;
	}
}

HRESULT CSOneClientDlg::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	switch(nRequestID)
	{
	case idQueryCountCTOne:
	case idQueryGlobalCountCTOne:
	case idQueryGlobalFastCountCTOne:
	case idSleepCTOne:
	case idEchoCTOne:
/*	case idGetOneItemCTThree:
	case idSendOneItemCTThree:
	case idGetManyItemsCTThree:
	case idSendManyItemsCTThree:
	case idGetBatchItemsCTThree:*/
	case idSendBatchItemsCTThree:
		UpdateBytes();
		break;
	default:
		break;
	}
	return S_OK;
}

HRESULT CSOneClientDlg::OnSocketClosed(long hSocket, long lError)
{
	EnableControls(FALSE);

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
		UpdateData(TRUE);
		m_lLatency = (UINT)m_PerfQuery.Diff(m_lPrev);
		SetUID(m_strUID);
		SetPassword(m_strPassword);

		BeginBatching();
		SwitchTo(&m_MySvsHandler);
		GetIUSocket()->TurnOnZipAtSvr(m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
		
		//Incerasing TCP sending and receiving buffer sizes will help performance 
		//when there is a lot of data transferred if your network bandwidth is over 10 mbps
		GetIUSocket()->SetSockOpt(soSndBuf, 116800);
		GetIUSocket()->SetSockOpt(soRcvBuf, 116800);

		//Incerasing TCP sending and receiving buffer sizes will help performance 
		//when there is a lot of data transferred if your network bandwidth is over 10 mbps
		GetIUSocket()->SetSockOptAtSvr(soSndBuf, 116800);
		GetIUSocket()->SetSockOptAtSvr(soRcvBuf, 116800);
		
		Commit(false); //must be false for the very first switch
		bool b = WaitAll();

		EnableControls(TRUE);

		m_nTimeRequired = m_PerfQuery.Diff(m_lPrev);
		UpdateData(FALSE);
		
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
		GetIUSocket()->put_EncryptionMethod(MSSSL); //MS SSPI/SSL3
	}
	else
	{
		GetIUSocket()->put_EncryptionMethod(NoEncryption);
	}
	GetIUSocket()->put_ZipIsOn(m_bZip ? VARIANT_TRUE : VARIANT_FALSE);

	m_lPrev = m_PerfQuery.Now();
	Connect(m_strHost, m_nPort);	
}

void CSOneClientDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CSOneClientDlg::OnSleepButton() 
{
	UpdateData(TRUE);

	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);

	GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(FALSE);

	m_lPrev = m_PerfQuery.Now();

	m_MySvsHandler.Sleep(m_nSleep);
		
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);

	if(IsConnected())
		GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(TRUE);

}

void CSOneClientDlg::OnGetallcountsButton() 
{
	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);
	m_lPrev = m_PerfQuery.Now();

	m_MySvsHandler.GetAllCounts(m_nCount, m_nGlobalCount, m_nGlobalFastCount);
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnDuerycountButton() 
{
	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);
	m_lPrev = m_PerfQuery.Now();

	m_nCount = m_MySvsHandler.QueryCount();
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnGlobalcountButton() 
{
	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);
	
	m_lPrev = m_PerfQuery.Now();

	m_nGlobalCount = m_MySvsHandler.QueryGlobalCount();
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnGlobalfastcountButton() 
{
	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);
	m_lPrev = m_PerfQuery.Now();

	m_nGlobalFastCount = m_MySvsHandler.QueryGlobalFastCount();
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnEchodataButton() 
{
	long lRtn = 0;
	CComVariant vtData(L"This is a test string");
	CComVariant vtOut;

	if(GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
		SwitchTo(&m_MySvsHandler);
	m_lPrev = m_PerfQuery.Now();

	vtOut = m_MySvsHandler.Echo(vtData);
	
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);

	//put debug break point here
	lRtn = 0;
}

int CSOneClientDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_MySvsHandler.Attach(this);
	m_S3Handler.Attach(this);
	
	return 0;
}

void CSOneClientDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	CleanStack();
	m_S3Handler.Detach();
	m_MySvsHandler.Detach();
}

void CSOneClientDlg::UpdateBytes()
{
	GetIUSocket()->GetBytesReceived(UNULL_PTR, &m_lBytesRecv);
	GetIUSocket()->GetBytesSent(UNULL_PTR, &m_lBytesSent);
	UpdateData(FALSE);
}

void CSOneClientDlg::OnUsesslCheck() 
{
	
}

void CSOneClientDlg::CleanStack()
{
	while(m_Stack.size() > 0)
	{
		CTestItem *pItem = m_Stack.top();
		m_Stack.pop();
		if(pItem != UNULL_PTR)
		{
			delete pItem;
		}
	}
}


void CSOneClientDlg::PrepareStack(int nSize)
{
	int n;
	CleanStack();
	for(n=0; n<nSize; n++)
	{
		SYSTEMTIME SysTime;
		::GetSystemTime(&SysTime);
		CTestItem *pItem = new CTestItem();

		pItem->m_lData = n;
		GetIUSocket()->get_UserID(&pItem->m_strUID);
		pItem->m_vtDT.vt = VT_DATE;
		::SystemTimeToVariantTime(&SysTime, &pItem->m_vtDT.date);

		m_Stack.push(pItem);
	}
}

void CSOneClientDlg::EnableControls(BOOL bEnable)
{
	GetDlgItem(IDC_SLEEP_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_GETALLCOUNTS_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_DUERYCOUNT_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_GLOBALCOUNT_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_GLOBALFASTCOUNT_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_ECHODATA_BUTTON)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_GETONE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_SENDONE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_GETALOT_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_SENDALOT_BUTTON)->EnableWindow(bEnable);
}

void CSOneClientDlg::OnGetoneButton() 
{
	if(GetCurrentServiceID() != m_S3Handler.GetSvsID())
		SwitchTo(&m_S3Handler);
	m_lPrev = m_PerfQuery.Now();
	const CTestItem &Item = m_S3Handler.GetOneItem();
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);	
}


void CSOneClientDlg::OnGetalotButton() 
{
	UpdateData(TRUE);
	if(GetCurrentServiceID() != m_S3Handler.GetSvsID())
		SwitchTo(&m_S3Handler);
	m_lPrev = m_PerfQuery.Now();
	stack<CTestItem*> &Item = m_S3Handler.GetManyItems(m_lGetALot);
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);	
}

void CSOneClientDlg::OnSendoneButton() 
{
	if(GetCurrentServiceID() != m_S3Handler.GetSvsID())
		SwitchTo(&m_S3Handler);
	PrepareStack(1);
	CTestItem *pItem = m_Stack.top();
	m_lPrev = m_PerfQuery.Now();
	m_S3Handler.SendOneItem(*pItem);
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	m_Stack.pop();
	delete pItem;
	UpdateData(FALSE);	
}

void CSOneClientDlg::OnSendalotButton() 
{
	UpdateData(TRUE);
	if(GetCurrentServiceID() != m_S3Handler.GetSvsID())
		SwitchTo(&m_S3Handler);
	PrepareStack(m_lSendALot);
	m_lPrev = m_PerfQuery.Now();
	m_S3Handler.SendManyItems(&m_Stack);
	m_nTimeRequired = (UINT)m_PerfQuery.Diff(m_lPrev);
	UpdateData(FALSE);
}
