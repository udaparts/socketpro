// CeFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CeFile.h"
#include "CeFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCeFileDlg dialog

CCeFileDlg::CCeFileDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CCeFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCeFileDlg)
	m_strHost = _T("192.168.1.100");
	m_strLocal = _T("\\oledbpro.msi");
	m_nPort = 17001;
	m_strRemote = _T("c:\\oledbpro.msi");
	m_dPercent = 0.0;
	m_lRecv = 0;
	m_strDir = _T("c:\\windows\\system32\\*.*");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strUserID = _T("SocketPro");
	m_strPassword = _T("PassOne");
	m_FileEvent.m_pContainer = this;
	m_ulFileSize = 0;


}

void CCeFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCeFileDlg)
	DDX_Control(pDX, IDC_FILE_LIST, m_lstFile);
	DDX_Text(pDX, IDC_HOST_EDIT, m_strHost);
	DDX_Text(pDX, IDC_LOCAL_EDIT, m_strLocal);
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDX_Text(pDX, IDC_REMOTE_EDIT, m_strRemote);
	DDX_Text(pDX, IDC_PERCENT_EDIT, m_dPercent);
	DDX_Text(pDX, IDC_RECV_EDIT, m_lRecv);
	DDX_Text(pDX, IDC_DIR_EDIT, m_strDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCeFileDlg, CDialog)
	//{{AFX_MSG_MAP(CCeFileDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnDisconnectButton)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_DOWNLOAD_BUTTON, OnDownloadButton)
	ON_BN_CLICKED(IDC_UPLOAD_BUTTON, OnUploadButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_BN_CLICKED(IDC_FIRST_BUTTON, OnFirstButton)
	ON_BN_CLICKED(IDC_NEXT_BUTTON, OnNextButton)
	ON_BN_CLICKED(IDC_ALL_BUTTON, OnAllButton)
	ON_EN_CHANGE(IDC_DIR_EDIT, OnChangeDirEdit)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeFileDlg message handlers

BOOL CCeFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	HRESULT hr = m_pIUFile.CoCreateInstance(__uuidof(UFile));
	if(FAILED(hr))
	{
		MessageBox(_T("Can't load library ufile.dll"));
		return FALSE;
	}
	hr = m_pIUFile->AttachSocket(GetIUSocket());
	if(FAILED(hr))
		return FALSE;
	hr = m_FileEvent.DispEventAdvise(m_pIUFile);
	if(FAILED(hr))
		return FALSE;
	m_lstFile.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_TRACKSELECT);
	m_lstFile.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 200, 0);
	m_lstFile.InsertColumn(1, _T("Size"), LVCFMT_LEFT, 60, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

HRESULT __stdcall CCeFileDlg::OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag)
{
	switch(unRequestID)
	{
	case idFindFirstFile:
	case idFindNextFile:
	case idFindAll:
		{
			USES_CONVERSION;
			long lSize;
			CComBSTR bstrFile;
			CString str;
			m_pIUFile->get_FileNameOrDirectory(&bstrFile);
			int nItem = m_lstFile.InsertItem(m_lstFile.GetItemCount(), OLE2T(bstrFile));
			m_pIUFile->FileSize(UNULL_PTR, &lSize);
			str.Format(_T("%d"), lSize);
			m_lstFile.SetItemText(nItem, 1, str);
		}
		break;
	case idSendFile:
	case idGetFile:
		switch(sFlag)
		{
		case rfComing:
			m_ulFileSize = ulLen;
			break;
		case rfReceiving:
			m_dPercent = 100.0*(m_ulFileSize-ulLen)/m_ulFileSize;
			m_dPercent = (DWORD)(m_dPercent * 100 + 0.5)/100.0; //round to 0.01
			UpdateData(FALSE);
			break;
		case rfCompleted:
			GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(TRUE);
			GetDlgItem(IDC_UPLOAD_BUTTON)->EnableWindow(TRUE);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return S_OK;
}
	

HRESULT CCeFileDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_UPLOAD_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_FIRST_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_NEXT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALL_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CCeFileDlg::OnSocketConnected(long hSocket, long lError)
{
	HRESULT hr;
	if(lError == 0) //no error
	{
		CComPtr<IUChat>	pIUChat;
		hr = GetIUSocket()->put_UserID(CComBSTR(m_strUserID));
		hr = GetIUSocket()->put_Password(CComBSTR(m_strPassword));
		
		SwitchTo(sidWinFile); //switch for remote window file service
		
		BeginBatching();
		if(IsDlgButtonChecked(IDC_ZIP_CHECK))
		{
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_TRUE);
		}
		else
		{
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_FALSE);
		}
		hr = GetIUSocket()->QueryInterface(__uuidof(IUChat), (void**)&pIUChat);
		hr = pIUChat->Enter(2); //join the chat group 2
		Commit(true);

		GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_UPLOAD_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_FIRST_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEXT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_ALL_BUTTON)->EnableWindow(TRUE);
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


HRESULT CCeFileDlg::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	if(sFlag != rfCompleted)
		return S_OK;
	GetIUSocket()->GetBytesReceived(UNULL_PTR, &m_lRecv);
	UpdateData(FALSE);
	return S_OK;
}

void CCeFileDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	Disconnect();
}

void CCeFileDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CCeFileDlg::OnConnectButton() 
{
	USES_CONVERSION;
	HRESULT hr;
	UpdateData();
	if(IsDlgButtonChecked(IDC_SSL_CHECK))
	{
		hr = GetIUSocket()->put_EncryptionMethod(MSSSL); //MS SSPI/SSL3
	}
	else
	{
		hr = GetIUSocket()->put_EncryptionMethod(NoEncryption);
	}
	if(IsDlgButtonChecked(IDC_ZIP_CHECK))
	{
		hr = GetIUSocket()->put_ZipIsOn(VARIANT_TRUE);
	}
	else
	{
		hr = GetIUSocket()->put_ZipIsOn(VARIANT_FALSE);
	}
	Connect(m_strHost, m_nPort);
}

void CCeFileDlg::OnDownloadButton() 
{
	HRESULT hr;
	USES_CONVERSION;
	UpdateData(TRUE);
	m_dPercent = 0.0;
	UpdateData(FALSE);
	hr = m_pIUFile->GetFile(T2OLE((LPTSTR)LPCTSTR(m_strRemote)), T2OLE((LPTSTR)LPCTSTR(m_strLocal)), VARIANT_FALSE);
	if(hr == S_OK)
	{
		GetDlgItem(IDC_UPLOAD_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);
	}
}

void CCeFileDlg::OnUploadButton() 
{
	HRESULT hr;
	USES_CONVERSION;
	UpdateData(TRUE);
	m_dPercent = 0.0;
	UpdateData(FALSE);
	hr = m_pIUFile->SendFile(T2OLE((LPTSTR)LPCTSTR(m_strLocal)), T2OLE((LPTSTR)LPCTSTR(m_strRemote)), VARIANT_FALSE);
	if(hr == S_OK)
	{
		GetDlgItem(IDC_UPLOAD_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);
	}
}

void CCeFileDlg::OnCancelButton() 
{
	GetIUSocket()->Cancel();
	m_pIUFile->Cancel();
}

void CCeFileDlg::OnFirstButton() 
{
	m_lstFile.DeleteAllItems();
	m_pIUFile->FindFirstFile(CComBSTR(m_strDir));
}

void CCeFileDlg::OnNextButton() 
{
	m_lstFile.DeleteAllItems();
	m_pIUFile->FindNextFile();
}

void CCeFileDlg::OnAllButton() 
{
	m_lstFile.DeleteAllItems();
	m_pIUFile->FindAll(CComBSTR(m_strDir));
}

void CCeFileDlg::OnChangeDirEdit() 
{
	UpdateData(TRUE);
	long lSocket = 0;
	GetIUSocket()->get_Socket(&lSocket);
	if(lSocket > 0)
	{
		m_pIUFile->SetCurrentDirectory(CComBSTR(m_strDir));
	}
	m_lstFile.DeleteAllItems();
}

int CCeFileDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
