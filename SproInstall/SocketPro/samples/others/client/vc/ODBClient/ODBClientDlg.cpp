// ODBClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ODBClient.h"
#include "ODBClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
LPOLESTR strProc=L"create Procedure OrderInfoEx \
@dtOrderDate datetime, @strCustomerID nchar(5), @strRegion nvarchar(15), @nSumEmployeeID int out, @strInfo nchar(255) out \
as \
select * from Orders where ShipRegion = @strRegion and OrderDate = @dtOrderDate and CustomerID<>@strCustomerID and EmployeeID<@nSumEmployeeID \
select @nSumEmployeeID=sum(EmployeeID) from Orders \
select @strInfo='This is a test from a procedure ' + @strCustomerID";

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
// CODBClientDlg dialog

CODBClientDlg::CODBClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CODBClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CODBClientDlg)
	m_strToDB = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
	m_strSQL = _T("");
	m_fPercent = 0.0f;
	m_bClientZip = FALSE;
	m_strHostAddr = _T("localhost");
	m_unPort = 17000;
	m_bServerZip = FALSE;
	m_bUseSSL = FALSE;
	m_strPassword = _T("PassOne");
	m_strUserID = _T("SocketPro");
	m_strLocalFile = _T("");
	m_strRemoteFile = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_DSEvent.m_pContainer = this;
	m_SessionEvent.m_pContainer = this;
	m_CmndEvent.m_pContainer = this;
	m_RowsetEvent.m_pContainer = this;
	m_FileEvent.m_pContainer = this;
	m_hSocketDB = 0xFFFFFFFF;
	m_hSocketFile = 0xFFFFFFFF;
	m_DBSocketEvent.m_pContainer = this;
	m_FileSocketEvent.m_pContainer = this;
}

void CODBClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CODBClientDlg)
	DDX_Text(pDX, IDC_TODB_EDIT, m_strToDB);
	DDX_Text(pDX, IDC_SQL_EDIT, m_strSQL);
	DDX_Text(pDX, IDC_PERCENT_EDIT, m_fPercent);
	DDX_Check(pDX, IDC_CLIENTZIP_CHECK, m_bClientZip);
	DDX_Text(pDX, IDC_HOSTADDRESS_EDIT, m_strHostAddr);
	DDX_Text(pDX, IDC_PORT_EDIT, m_unPort);
	DDX_Check(pDX, IDC_SERVERZIP_CHECK, m_bServerZip);
	DDX_Check(pDX, IDC_USESSL_CHECK, m_bUseSSL);
	DDX_Text(pDX, IDC_PASSWORD_EDIT, m_strPassword);
	DDX_Text(pDX, IDC_USERID_EDIT, m_strUserID);
	DDX_Text(pDX, IDC_LOCALFILE_EDIT, m_strLocalFile);
	DDX_Text(pDX, IDC_REMOTEFILE_EDIT, m_strRemoteFile);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CODBClientDlg, CDialog)
	//{{AFX_MSG_MAP(CODBClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_SHUTDOWN_BUTTON, OnShutdownButton)
	ON_BN_CLICKED(IDC_TODB_BUTTON, OnTodbButton)
	ON_BN_CLICKED(IDC_GETPROP_BUTTON, OnGetpropButton)
	ON_BN_CLICKED(IDC_EXECUTESQL_BUTTON, OnExecutesqlButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_BN_CLICKED(IDC_SEND_BUTTON, OnSendButton)
	ON_BN_CLICKED(IDC_GET_BUTTON, OnGetButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CODBClientDlg message handlers

BOOL CODBClientDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CODBClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CODBClientDlg::OnPaint() 
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
HCURSOR CODBClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

HRESULT __stdcall CODBClientDlg::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	if (nMsg == msgSSLEvent)
	{
		switch(wParam)
		{
		case ssleHandshakeStarted:
			break;
		case ssleHandshakeDone:
			break;
		case ssleHandshakeLoop:
			break;
		case ssleHandshakeExit:
			break;
		case ssleReadAlert:
			break;
		case ssleWriteAlert:
			break;
		default:
			break;
		}
	}
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnSocketClosed(long hSocket, long lError)
{
	if((unsigned int)hSocket == m_hSocketDB)
	{
		GetDlgItem(IDC_TODB_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_EXECUTESQL_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_GETPROP_BUTTON)->EnableWindow(FALSE);
		m_hSocketDB = 0xFFFFFFFF;
	}
	else if((unsigned int)hSocket == m_hSocketFile)
	{
		GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_GET_BUTTON)->EnableWindow(FALSE);
		m_hSocketFile = 0xFFFFFFFF;
	}
	else
	{
		ASSERT(FALSE);
	}
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnSocketConnected(long hSocket, long lError)
{
	if((unsigned int)hSocket == m_hSocketDB)
	{
		if(lError == S_OK)
		{
			HRESULT hr;
			hr = m_pIUSocketDB->SetSockOpt(soSndBuf, 116800);
			hr = m_pIUSocketDB->SetSockOpt(soRcvBuf, 116800);
			
 			hr = m_pIUSocketDB->StartBatching();

			CComBSTR	bstrUserID((LPCTSTR)m_strUserID);
			CComBSTR	bstrPassword((LPCTSTR)m_strPassword);
			hr = m_pIUSocketDB->put_UserID(bstrUserID);
			hr = m_pIUSocketDB->put_Password(bstrPassword);
			hr = m_pIUSocketDB->SwitchTo(sidOleDB);
			
			//clean password ASAP for the better security
			hr = m_pIUSocketDB->put_Password(CComBSTR());

			hr = m_pIUSocketDB->SetSockOptAtSvr(soSndBuf, 116800);
			hr = m_pIUSocketDB->SetSockOptAtSvr(soRcvBuf, 116800);
			hr = m_pIUSocketDB->TurnOnZipAtSvr(m_bServerZip ? VARIANT_TRUE : VARIANT_FALSE);
			hr = m_pIUSocketDB->CommitBatching();

			GetDlgItem(IDC_TODB_BUTTON)->EnableWindow(TRUE);
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(TRUE);
			GetDlgItem(IDC_EXECUTESQL_BUTTON)->EnableWindow(TRUE);
			GetDlgItem(IDC_GETPROP_BUTTON)->EnableWindow(TRUE);
		}
		else
		{
			USES_CONVERSION;
			CComBSTR	bstrErrorMsg;
			HRESULT hr = m_pIUSocketDB->get_ErrorMsg(&bstrErrorMsg);
			MessageBox(OLE2T(bstrErrorMsg));
		}
	}
	else if((unsigned int)hSocket == m_hSocketFile)
	{
		if(lError == S_OK)
		{
			HRESULT hr;
			hr = m_pIUSocketFile->SetSockOpt(soSndBuf, 116800);
			hr = m_pIUSocketFile->SetSockOpt(soRcvBuf, 116800);
 			
			hr = m_pIUSocketFile->StartBatching();

			CComBSTR	bstrUserID((LPCTSTR)m_strUserID);
			CComBSTR	bstrPassword((LPCTSTR)m_strPassword);
			hr = m_pIUSocketFile->put_UserID(bstrUserID);
			hr = m_pIUSocketFile->put_Password(bstrPassword);

			hr = m_pIUSocketFile->SwitchTo(sidWinFile);

			//clean password ASAP for the better security
			hr = m_pIUSocketFile->put_Password(CComBSTR());

			hr = m_pIUSocketFile->SetSockOptAtSvr(soSndBuf, 116800);
			hr = m_pIUSocketFile->SetSockOptAtSvr(soRcvBuf, 116800);
			hr = m_pIUSocketFile->TurnOnZipAtSvr(m_bServerZip ? VARIANT_TRUE : VARIANT_FALSE);
			hr = m_pIUSocketFile->CommitBatching();

			GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(TRUE);
			GetDlgItem(IDC_GET_BUTTON)->EnableWindow(TRUE);
		}
		else
		{
			USES_CONVERSION;
			CComBSTR	bstrErrorMsg;
			HRESULT hr = m_pIUSocketFile->get_ErrorMsg(&bstrErrorMsg);
			MessageBox(OLE2T(bstrErrorMsg));
		}
	}
	else
	{
		ASSERT(FALSE);
	}
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnConnecting(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnSendingData(long hSocket, long lError, long lSent)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnClosing(long hSocket, long hWnd)
{
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	if(sFlag != rfCompleted)
		return S_OK;
	long lRecv = 0;
	TCHAR	strBytes[33] = {0};
	if((unsigned int)hSocket == m_hSocketDB)
	{
		m_pIUSocketDB->GetBytesReceived(NULL, &lRecv);	
		DWORD dwRecv = (DWORD)lRecv;
		::_stprintf(strBytes, _T("%u"), dwRecv);
		GetDlgItem(IDC_BYTESDB_EDIT)->SetWindowText(strBytes);
	}
	else if((unsigned int)hSocket == m_hSocketFile)
	{
		m_pIUSocketFile->GetBytesReceived(NULL, &lRecv);
		DWORD dwRecv = (DWORD)lRecv;
		::_stprintf(strBytes, _T("%u"), dwRecv);
		GetDlgItem(IDC_BYTESFILE_EDIT)->SetWindowText(strBytes);
	}
	else
	{
		ASSERT(FALSE);
	}
	return S_OK;
}

HRESULT __stdcall CODBClientDlg::OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag)
{
	if((unsigned int)hSocket == m_hSocketDB)
	{

	}
	else if((unsigned int)hSocket == m_hSocketFile)
	{
		if(unRequestID == idSendFile || unRequestID == idGetFile)
		{
			switch(sFlag)
			{
			case rfComing:
				m_dwFileSize = ulLen;
				break;
			case rfReceiving:
				m_fPercent = 100.0*(m_dwFileSize-ulLen)/m_dwFileSize;
				m_fPercent = (DWORD)(m_fPercent * 100 + 0.5)/100.0;
				UpdateData(FALSE);
				break;
			case rfCompleted:
				GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(TRUE);
				GetDlgItem(IDC_GET_BUTTON)->EnableWindow(TRUE);
//				m_fPercent = 100;
//				UpdateData(FALSE);
				break;
			default:
				break;
			}
		}
	}
	else
	{
		ASSERT(FALSE);
	}
	return S_OK;
}

int CODBClientDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	HRESULT hr = m_pIUSocketDB.CoCreateInstance(__uuidof(USocket));
	if(FAILED(hr))
		return -1;
	hr = m_DBSocketEvent.DispEventAdvise(m_pIUSocketDB.p);
	if(FAILED(hr))
		return -1;

	hr = m_pIUSocketDB->put_RecvTimeout(4500000); //45 seconds

	hr = m_pIUDataSource.CoCreateInstance(__uuidof(UDataSource));
	if(FAILED(hr))
		return -1;
	hr = m_pIUDataSource->AttachSocket(m_pIUSocketDB.p);
	if(FAILED(hr))
		return -1;
	hr = m_DSEvent.DispEventAdvise(m_pIUDataSource.p);
	if(FAILED(hr))
		return -1;

	hr = m_pIUSession.CoCreateInstance(__uuidof(USession));
	if(FAILED(hr))
		return -1;
	hr = m_pIUSession->AttachSocket(m_pIUSocketDB.p);
	if(FAILED(hr))
		return -1;
	hr = m_SessionEvent.DispEventAdvise(m_pIUSession.p);
	if(FAILED(hr))
		return -1;
	
	hr = m_pIUCommand.CoCreateInstance(__uuidof(UCommand));
	if(FAILED(hr))
		return -1;
	hr = m_pIUCommand->AttachSocket(m_pIUSocketDB.p);
	if(FAILED(hr))
		return -1;
	hr = m_CmndEvent.DispEventAdvise(m_pIUCommand.p);
	if(FAILED(hr))
		return -1;

	hr = m_pIURowset.CoCreateInstance(__uuidof(URowset));
	if(FAILED(hr))
		return -1;
	hr = m_pIURowset->AttachSocket(m_pIUSocketDB.p);
	if(FAILED(hr))
		return -1;
	hr = m_RowsetEvent.DispEventAdvise(m_pIURowset.p);
	if(FAILED(hr))
		return -1;

	hr = m_pIUSocketFile.CoCreateInstance(__uuidof(USocket));
	if(FAILED(hr))
		return -1;
	hr = m_FileSocketEvent.DispEventAdvise(m_pIUSocketFile.p);
	if(FAILED(hr))
		return -1;
	hr = m_pIUSocketFile->put_RecvTimeout(4500000); //45 seconds
	
	hr = m_pIUFile.CoCreateInstance(__uuidof(UFile));
	if(FAILED(hr))
		return -1;
	hr = m_pIUFile->AttachSocket(m_pIUSocketFile.p);
	if(FAILED(hr))
		return -1;
	hr = m_FileEvent.DispEventAdvise(m_pIUFile.p);
	if(FAILED(hr))
		return -1;
	return 0;
}

void CODBClientDlg::OnDestroy() 
{
	m_pIUSocketFile->Disconnect();
	m_pIUSocketDB->Disconnect();

	CDialog::OnDestroy();
	// TODO: Add your message handler code here
}

void CODBClientDlg::OnConnectButton() 
{
	long hSocket = 0;
	USES_CONVERSION;
	HRESULT hr;
	UpdateData(TRUE);
	
	CComBSTR	bstrHost((LPCTSTR)m_strHostAddr);
	hr = m_pIUSocketDB->put_EncryptionMethod(m_bUseSSL ? MSSSL : NoEncryption);
	hr = m_pIUSocketDB->put_ZipIsOn(m_bClientZip ? VARIANT_TRUE : VARIANT_FALSE);
	hr = m_pIUSocketDB->Connect(bstrHost, m_unPort);
	hr = m_pIUSocketDB->get_Socket(&hSocket);
	m_hSocketDB = hSocket;
	hr = m_pIUSocketFile->put_EncryptionMethod(m_bUseSSL ? MSSSL : NoEncryption);
	hr = m_pIUSocketFile->put_ZipIsOn(m_bClientZip ? VARIANT_TRUE : VARIANT_FALSE);
	hr = m_pIUSocketFile->Connect(bstrHost, m_unPort);
	hSocket = 0;
	hr = m_pIUSocketFile->get_Socket(&hSocket);
	m_hSocketFile = hSocket;
}

void CODBClientDlg::OnShutdownButton() 
{
	m_pIUSocketDB->Shutdown();
	m_pIUSocketFile->Shutdown();
}

void CODBClientDlg::OnTodbButton() 
{
	UpdateData(TRUE);
	CComBSTR bstrConn((LPCTSTR)m_strToDB);
	HRESULT hr;
	hr = m_pIUSocketDB->StartBatching();
	hr = m_pIUDataSource->Open(bstrConn);
	hr = m_pIUSession->Open();
	hr = m_pIUSocketDB->CommitBatching(VARIANT_TRUE);
}

void CODBClientDlg::OnGetpropButton() 
{
	HRESULT hr = m_pIUSocketDB->StartBatching();
	hr = m_pIUDataSource->GetProperty(0x6D);
	hr = m_pIUSession->GetProperty();
	hr = m_pIUSocketDB->CommitBatching(VARIANT_TRUE);
}

void CODBClientDlg::OnExecutesqlButton() 
{
	USES_CONVERSION;
	UpdateData(TRUE);
	CComVariant vtData;
	HRESULT hr = m_pIUSocketDB->StartBatching();
	hr = m_pIUCommand->Open();
	hr = m_pIUCommand->ExecuteSQL(T2OLE(m_strSQL), coRowset, ctStatic);
	hr = m_pIURowset->Open();
	hr = m_pIURowset->AsynFetch();
	hr = m_pIURowset->Close();
	hr = m_pIUCommand->Close();
	hr = m_pIUSocketDB->CommitBatching(VARIANT_TRUE);
}

void CODBClientDlg::OnCancelButton() 
{
	m_pIUSocketDB->Cancel();
	m_pIUFile->Cancel();
}

void CODBClientDlg::OnSendButton() 
{
	HRESULT hr;
	USES_CONVERSION;
	long hSocket = 0;
	UpdateData(TRUE);
	GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GET_BUTTON)->EnableWindow(FALSE);
	VARIANT_BOOL bTimeOut = VARIANT_FALSE;
	m_fPercent = 0.0;
	UpdateData(FALSE);
	do
	{
		hr = m_pIUFile->SendFile(T2OLE(m_strLocalFile), T2OLE(m_strRemoteFile), VARIANT_FALSE);
		if(FAILED(hr))
			break;
//		hr = m_pIUSocketFile->WaitAll(0xFFFFFFFF, &bTimeOut);
	}while(false);
/*	m_pIUSocketFile->get_Socket(&hSocket);
	if(!bTimeOut && hSocket && hSocket != -1) //not timeout and socket not closed
	{
		GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_GET_BUTTON)->EnableWindow(TRUE);
	}*/
}

void CODBClientDlg::OnGetButton() 
{
	HRESULT hr;
	USES_CONVERSION;
	long hSocket = 0;
	UpdateData(TRUE);
	GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GET_BUTTON)->EnableWindow(FALSE);
	VARIANT_BOOL bTimeOut = VARIANT_FALSE;
	m_fPercent = 0.0;
	UpdateData(FALSE);
	do
	{
		hr = m_pIUFile->GetFile(T2OLE(m_strRemoteFile), T2OLE(m_strLocalFile), VARIANT_FALSE);
		if(FAILED(hr))
			break;
//		hr = m_pIUSocketFile->WaitAll(0xFFFFFFFF, &bTimeOut);
	}while(false);
/*	m_pIUSocketFile->get_Socket(&hSocket);
	if(!bTimeOut && hSocket && hSocket != -1) //not timeout and socket not closed
	{
		GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_GET_BUTTON)->EnableWindow(TRUE);
	}*/
}
