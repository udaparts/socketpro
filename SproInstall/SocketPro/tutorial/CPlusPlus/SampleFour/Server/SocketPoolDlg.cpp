// SocketPoolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SocketPool.h"
#include "SocketPoolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSocketPoolDlg dialog

CSocketPoolDlg::CSocketPoolDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CSocketPoolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSocketPoolDlg)
	m_strLBStatus = _T("");
	m_nPort = 20910;
	m_lConnectedSockets = 0;
	m_lSocketConnections = 0;
	m_bSocketsPerThread = 2;
	m_bThreadCount = 3;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pPoolSvr = UNULL_PTR;
}

void CSocketPoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSocketPoolDlg)
	DDX_Control(pDX, IDC_INFO_LIST, m_lstInfo);
	DDX_Text(pDX, IDC_CONNSTRING_EDIT, m_strLBStatus);
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDX_Text(pDX, IDC_CONNECTEDSOCKETS_EDIT, m_lConnectedSockets);
	DDX_Text(pDX, IDC_SOCKETCONNECTIONS_EDIT, m_lSocketConnections);
	DDX_Text(pDX, IDC_SOCKETSPERTHREAD_EDIT, m_bSocketsPerThread);
	DDX_Text(pDX, IDC_THREADCOUNT_EDIT, m_bThreadCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSocketPoolDlg, CDialog)
	//{{AFX_MSG_MAP(CSocketPoolDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_BUTTON, OnStartButton)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStopButton)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ADDREALSERVER_BUTTON, &CSocketPoolDlg::OnBnClickedAddrealserverButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSocketPoolDlg message handlers

BOOL CSocketPoolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	GetDlgItem(IDC_START_BUTTON)->EnableWindow(TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSocketPoolDlg::OnPaint() 
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
HCURSOR CSocketPoolDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSocketPoolDlg::OnStartButton() 
{
	USES_CONVERSION;
	
	UpdateData(TRUE);

	if(m_pPoolSvr != UNULL_PTR)
	{
		delete m_pPoolSvr;
		m_pPoolSvr = UNULL_PTR;
	}

	m_pPoolSvr = new CPoolSvr();
	m_pPoolSvr->m_hDlg = m_hWnd;
	m_pPoolSvr->m_RadoPoolSvs.m_hDlg = m_hWnd;

	if(m_pPoolSvr->Run(m_nPort, m_bThreadCount, m_bSocketsPerThread))
	{
		GetDlgItem(IDC_START_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_STOP_BUTTON)->EnableWindow(TRUE);

		m_pPoolSvr->m_RadoPoolSvs.GetSocketPool().GetUSocketPool()->get_ConnectedSocketsEx(&m_lConnectedSockets);

		UpdateData(FALSE);
	}
}

void CSocketPoolDlg::OnStopButton() 
{
	m_pPoolSvr->Stop();
	GetDlgItem(IDC_START_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP_BUTTON)->EnableWindow(FALSE);
	PostMessage(WM_UPDATE_LB_STATUS);
}

void CSocketPoolDlg::OnAccept(unsigned int hSocket, int nError)
{
	m_lSocketConnections = m_pPoolSvr->GetCountOfClients();
	UpdateData(FALSE);
}

void CSocketPoolDlg::OnClose(unsigned int hSocket, int nError)
{
	::Sleep(0);
	m_lSocketConnections = m_pPoolSvr->GetCountOfClients();
	if(nError != 0)
	{
		WCHAR	strUID[256] = {0};
		WCHAR	strPassword[256] = {0};
		WCHAR	strAddr[32] = {0};
		TCHAR	strPort[16] = {0};
		TCHAR	strError[16] = {0};

		unsigned int nPort = 0;

		CString strMsg;

		CSocketProServer::GetUserID(hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));

		//password is available ONLY IF authentication method to either amOwn or amMixed
		CSocketProServer::GetPassword(hSocket, strPassword, sizeof(strPassword)/sizeof(WCHAR));

		g_SocketProLoader.GetPeerName(hSocket, &nPort, strAddr, sizeof(strAddr)/sizeof(WCHAR));
		
		strMsg = strUID;
		strMsg += _T(" from ");
		strMsg += strAddr;
		strMsg += _T("@");
		::_itoa_s(nPort, strPort, 10);
		strMsg += strPort;

		strMsg += _T(" closed with error code = ");

		::_itoa_s(nError, strError, 10);
		strMsg += strError;
		
		m_lstInfo.AddString(strMsg);
	}
	if(m_hWnd != UNULL_PTR)
		UpdateData(FALSE);
}

void CSocketPoolDlg::OnUpdateLBStatus()
{
	TCHAR str[2048] = {0};
	if(m_pPoolSvr != UNULL_PTR)
	{
		::Sleep(0); 
		CMyPLGService::PLGSocketPool &sp = m_pPoolSvr->m_RadoPoolSvs.GetSocketPool();
		_stprintf_s(str, _T("AllLoaded: %s, Parallels :%d, Fails: %d, Pause: %s, Working: %s"), 
			sp.IsAllLoaded() ? "True" : "False", 
			sp.GetSocketsInParallel(), 
			sp.GetFails(), 
			sp.IsPaused() ? "True" : "False", 
			sp.IsWorking() ? "True" : "False"
			);
		sp.GetUSocketPool()->get_ConnectedSocketsEx(&m_lConnectedSockets);
	}
	m_strLBStatus = str;
	UpdateData(FALSE);
}

bool CSocketPoolDlg::OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
{
	WCHAR	strUID[256] = {0};
	WCHAR	strAddr[32] = {0};
	TCHAR	strPort[16] = {0};

	unsigned int nPort = 0;

	CString strMsg;

	CSocketProServer::GetUserID(hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));

	g_SocketProLoader.GetPeerName(hSocket, &nPort, strAddr, sizeof(strAddr)/sizeof(WCHAR));
	
	strMsg = strUID;
	strMsg += _T(" from ");
	strMsg += strAddr;
	strMsg += _T("@");
	::_itoa_s(nPort, strPort, 10);
	strMsg += strPort;

	m_lstInfo.AddString(strMsg);
	
	return true; 

}

BOOL CSocketPoolDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message)
	{
	case WM_MY_EVENT_ONACCEPT:
		OnAccept(pMsg->wParam, pMsg->lParam);
		break;
	case WM_MY_EVENT_ONCLOSE:
		OnClose(pMsg->wParam, pMsg->lParam);
		break;
	case WM_MY_EVENT_ONISPERMITTED:
		OnIsPermitted(pMsg->wParam, pMsg->lParam);
		break;
	case WM_UPDATE_LB_STATUS:
		OnUpdateLBStatus();
		break;
	default:
		break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CSocketPoolDlg::OnDestroy() 
{
	if(m_pPoolSvr != UNULL_PTR)
	{
		m_pPoolSvr->Stop();
		delete m_pPoolSvr;
		m_pPoolSvr = UNULL_PTR;
	}
	CDialog::OnDestroy();
}

void CSocketPoolDlg::OnBnClickedAddrealserverButton()
{
	if(m_pPoolSvr == UNULL_PTR)
		return;
	CComBSTR bstrUserId(L"SocketPro");
	CComBSTR bstrPassword(L"PassOne");
	CComBSTR bstrHost(L"127.0.0.1");
	CConnectionContext cc;
	cc.m_bZip = false;
	cc.m_EncrytionMethod = NoEncryption;
	cc.m_nPort = 20901;
	cc.m_strHost = bstrHost.m_str;
	cc.m_strPassword = bstrPassword.m_str;
	cc.m_strUID = bstrUserId.m_str;

	if(m_pPoolSvr->m_RadoPoolSvs.GetSocketPool().MakeConnection(cc))
	{
		PostMessage(WM_UPDATE_LB_STATUS);
	}
}
