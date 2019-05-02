// DevTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DevTest.h"
#include "DevTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDevTestDlg dialog

CDevTestDlg::CDevTestDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CDevTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDevTestDlg)
	m_strHost = _T("192.168.1.100");
	m_nPort = 17001;
	m_strMsg = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDevTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDevTestDlg)
	DDX_Text(pDX, IDC_HOST_EDIT, m_strHost);
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDV_MinMaxUInt(pDX, m_nPort, 1, 65536);
	DDX_Text(pDX, IDC_MESSAGE_EDIT, m_strMsg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDevTestDlg, CDialog)
	//{{AFX_MSG_MAP(CDevTestDlg)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnDisconnectButton)
	ON_BN_CLICKED(IDC_SEND_BUTTON, OnSendButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDevTestDlg message handlers

BOOL CDevTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HRESULT hr;

	hr = GetIUSocket()->put_ZipIsOn(VARIANT_TRUE);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


HRESULT CDevTestDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CDevTestDlg::OnSocketConnected(long hSocket, long lError)
{
	HRESULT hr;
	if(lError == S_OK)
	{
		SetUID(_T("SocketPro"));
		SetPassword(_T("PassOne"));

		BeginBatching();
		SwitchTo(sidChat);
		if(IsDlgButtonChecked(IDC_ZIP_CHECK))
		{
			hr = GetIUSocket()->put_ZipIsOn(VARIANT_TRUE);
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_TRUE);
		}
		else
		{
			hr = GetIUSocket()->put_ZipIsOn(VARIANT_FALSE);
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_FALSE);
		}
		unsigned long pGroup[] = {1};
		GetPush()->Enter(pGroup, 1); //enter chat group 1
		Commit(false);
		GetDlgItem(IDC_SEND_BUTTON)->EnableWindow(TRUE);
		WaitAll();
	}
	else
	{
		USES_CONVERSION;
		CString strMsg;
		CComBSTR bstrError = GetErrorMsg();
		strMsg.Format(_T("Error code = %d; Error Message = %s"), lError, OLE2T(bstrError));
		MessageBox(strMsg);
	}
	return S_OK;
}

void CDevTestDlg::OnBaseRequestProcessed(unsigned short nRequestID)
{
	
	HRESULT hr;
	CString strMethod;
	CComBSTR bstrUID;
	CComBSTR bstrAddr;
	CString strMsg;
	CComVariant vtMsg;
	CString strText;
	switch(nRequestID)
	{
	case idGetAllGroups:
		strMethod = _T("GetAllGroups");
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMethod);
		break;
	case idGetAllListeners:
		strMethod = _T("GetAllListeners");
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMethod);
		break;
	case idGetAllClients:
		strMethod = _T("GetAllListeners");
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strMethod);
		break;
	case idSpeak:
	case idXSpeak:
		hr = GetIUChat()->get_Message(&vtMsg);
		strMsg = vtMsg.bstrVal;
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("Speak");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T(" ");
		strText += strMsg;
		strText += _T(" ");
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	case idSpeakTo:
		hr = GetIUChat()->get_Message(&vtMsg);
		strMsg = vtMsg.bstrVal;
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("SpeakTo");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T(" ");
		strText += strMsg;
		strText += _T(" ");
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	case idEnter:
	case idXEnter:
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("Enter");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	case idExit:
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("Exit");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	case idSpeakEx:
	case idXSpeakEx:
		hr = GetIUChat()->get_Message(&vtMsg);
		strMsg = vtMsg.bstrVal;
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("SpeakEx");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T(" ");
		strText += strMsg;
		strText += _T(" ");
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	case idSpeakToEx:
		hr = GetIUChat()->get_Message(&vtMsg);
		strMsg = vtMsg.bstrVal;
		hr = GetIUChat()->GetInfo(0, UNULL_PTR, &bstrUID, UNULL_PTR, UNULL_PTR, &bstrAddr);
		strMethod = _T("SpeakToEx");
		strText = LPCWSTR(bstrUID);
		strText += _T(" ");
		strText += strMethod;
		strText += _T(" ");
		strText += strMsg;
		strText += _T(" ");
		strText += _T("@");
		strText += LPCWSTR(bstrAddr);
		GetDlgItem(IDC_MESSAGE_EDIT)->SetWindowText(strText);
		break;
	default:
		break;
	}
}



int CDevTestDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if(GetIUSocket().p == UNULL_PTR)
	{
		::MessageBox(UNULL_PTR, _T("Can't start USocket object. Check COM registration!"), AfxGetAppName(), MB_ICONSTOP);
		return -1;
	}
	
	return 0;
}

void CDevTestDlg::OnDestroy() 
{
	Disconnect();
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here	
}

void CDevTestDlg::OnConnectButton() 
{
	HRESULT hr;
	UpdateData();
	if(IsDlgButtonChecked(IDC_USESSL_CHECK))
	{
		hr = GetIUSocket()->put_EncryptionMethod(MSTLSv1); //MS SSPI/TLSv1
	}
	else
	{
		hr = GetIUSocket()->put_EncryptionMethod(NoEncryption);
	}
	Connect(m_strHost, m_nPort);
}

void CDevTestDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CDevTestDlg::OnSendButton() 
{
	UpdateData();

	unsigned long pGroup[] = {1};

	// TODO: Add your control notification handler code here
	CComVariant vtMsg = m_strMsg;
	GetPush()->Broadcast(vtMsg, pGroup, 1);
}
