// CeDBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CeDB.h"
#include "CeDBDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCeDBDlg dialog

CCeDBDlg::CCeDBDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CCeDBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCeDBDlg)
	m_strSQL = _T("Select * from Orders");
	m_nPort = 17001;
	m_strHost = _T("192.168.1.100");
	m_bZip = TRUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	//change these variables into yours
	m_strOLEDBConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
	m_strUserID = _T("RDBClient");
	m_strPassword = _T("PassTwo");

	m_DSEvent.m_pContainer = this;
	m_SessionEvent.m_pContainer = this;
	m_CmndEvent.m_pContainer = this;
	m_RowsetEvent.m_pContainer = this;
}

void CCeDBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCeDBDlg)
	DDX_Control(pDX, IDC_RECORDS_LIST, m_lstRecords);
	DDX_Text(pDX, IDC_SQL_EDIT, m_strSQL);
	DDX_Text(pDX, IDC_PORT_EDIT, m_nPort);
	DDX_Text(pDX, IDC_HOST_EDIT, m_strHost);
	DDX_Check(pDX, IDC_ZIP_CHECK, m_bZip);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCeDBDlg, CDialog)
	//{{AFX_MSG_MAP(CCeDBDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnConnectButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnDisconnectButton)
	ON_BN_CLICKED(IDC_GO_BUTTON, OnGoButton)
	ON_BN_CLICKED(IDC_FIRST_BUTTON, OnFirstButton)
	ON_BN_CLICKED(IDC_LAST_BUTTON, OnLastButton)
	ON_BN_CLICKED(IDC_PREV_BUTTON, OnPrevButton)
	ON_BN_CLICKED(IDC_NEXT_BUTTON, OnNextButton)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeDBDlg message handlers

BOOL CCeDBDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	HRESULT hr = GetIUSocket()->put_RecvTimeout(45000); //45 seconds

	hr = m_pIUDataSource.CoCreateInstance(__uuidof(UDataSource));
	if(FAILED(hr))
	{
		MessageBox(_T("Can't load library udb.dll"));
		return FALSE;
	}
	hr = m_pIUDataSource->AttachSocket(GetIUSocket());
	if(FAILED(hr))
		return FALSE;
	hr = m_DSEvent.DispEventAdvise(m_pIUDataSource);
	if(FAILED(hr))
		return FALSE;

	hr = m_pIUSession.CoCreateInstance(__uuidof(USession));
	if(FAILED(hr))
		return FALSE;
	hr = m_pIUSession->AttachSocket(GetIUSocket());
	if(FAILED(hr))
		return -1;
	hr = m_SessionEvent.DispEventAdvise(m_pIUSession);
	if(FAILED(hr))
		return FALSE;
	
	hr = m_pIUCommand.CoCreateInstance(__uuidof(UCommand));
	if(FAILED(hr))
		return FALSE;
	hr = m_pIUCommand->AttachSocket(GetIUSocket());
	if(FAILED(hr))
		return FALSE;
	hr = m_CmndEvent.DispEventAdvise(m_pIUCommand);
	if(FAILED(hr))
		return FALSE;

	hr = m_pIURowset.CoCreateInstance(__uuidof(URowset));
	if(FAILED(hr))
		return FALSE;
	hr = m_pIURowset->AttachSocket(GetIUSocket());
	if(FAILED(hr))
		return FALSE;
	hr = m_RowsetEvent.DispEventAdvise(m_pIURowset);
	if(FAILED(hr))
		return FALSE;

	m_lstRecords.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_TRACKSELECT);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

HRESULT __stdcall CCeDBDlg::OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag)
{
	if(sFlag != rfCompleted)
		return S_OK;
	HRESULT hr;
	long lErrorCode = S_OK;
	switch(unRequestID)
	{
	case idDSOpen:
		hr = m_pIUDataSource->get_Rtn(&lErrorCode);
		if(lErrorCode != S_OK)
		{
			USES_CONVERSION;
			CString strMsg;
			CComBSTR bstrError;
			hr = m_pIUDataSource->get_ErrorMsg(&bstrError);
			if(bstrError.Length() == 0)
				bstrError += L"Unknown error";
			strMsg.Format(_T("Error code = %d; Error Message = %s"), lErrorCode, OLE2T(bstrError));
			MessageBox(strMsg);
		}
		break;
	case idCmndOpen:
		hr = m_pIUCommand->get_Rtn(&lErrorCode);
		if(lErrorCode == S_OK)
		{
			GetDlgItem(IDC_GO_BUTTON)->EnableWindow(TRUE);
		}
		break;
	case idRowsetOpen:
		{
			USES_CONVERSION;
			long l;
			CComBSTR bstrColName;
			m_lstRecords.DeleteAllItems();
			while(m_lstRecords.GetHeaderCtrl()->GetItemCount() > 0)
			{
				m_lstRecords.DeleteColumn(m_lstRecords.GetHeaderCtrl()->GetItemCount()-1);
			}
			long lCols = 0;
			hr = m_pIURowset->GetCols(&lCols);
			for(l=0; l<lCols; l++)
			{
				hr = m_pIURowset->GetColName(l+1, &bstrColName);
				m_lstRecords.InsertColumn(l, OLE2T(bstrColName), LVCFMT_LEFT, 50, l);				
				bstrColName.Empty();
			}
			if(lCols > 0)
			{
				GetDlgItem(IDC_FIRST_BUTTON)->EnableWindow(TRUE);
				GetDlgItem(IDC_NEXT_BUTTON)->EnableWindow(TRUE);
			}
			else
			{
				GetDlgItem(IDC_FIRST_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_NEXT_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_LAST_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_PREV_BUTTON)->EnableWindow(FALSE);
			}
		}
		break;
	case idRowsetGetProperty:
		{
			CComVariant vtScrollable;
			hr = m_pIURowset->get_Property(&vtScrollable);
			if(((vtScrollable.vt == VT_BOOL) && vtScrollable.boolVal) )
			{
				GetDlgItem(IDC_LAST_BUTTON)->EnableWindow(TRUE);
				GetDlgItem(IDC_PREV_BUTTON)->EnableWindow(TRUE);
			}
			else
			{
				GetDlgItem(IDC_LAST_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_PREV_BUTTON)->EnableWindow(FALSE);
			}
		}
		break;
	case idRowsetAsynFetch:
	case idRowsetGetBatchRecords:
	case idRowsetMoveFirst:
	case idRowsetMoveLast:
	case idRowsetMovePrev:	
	case idRowsetMoveNext:
	case idRowsetGetBatchRecordsEx:
	case idRowsetGetBatchRecordsLast:
		{
			long l;
			long lCols = 0;
			short sObtained = 0;
			short s;
			int nItem;
			int nItems = m_lstRecords.GetItemCount();

			TCHAR strData[1025] = {0};
			ULONG ulGet;
			long lSize;
			hr = m_pIURowset->GetCols(&lCols);
			hr = m_pIURowset->GetRowsFetched(&sObtained);
			CComQIPtr<IUDataReader> pIUDataReader(m_pIURowset);

			m_lstRecords.SendMessage(WM_SETREDRAW, FALSE, 0); //
			while(nItems > sObtained)
			{
				m_lstRecords.DeleteItem(--nItems);
			}
			for(s=0; s<sObtained; s++)
			{
				for(l=0; l<lCols; l++)
				{
					memset(strData, 0, sizeof(strData));
#ifdef _UNICODE
					hr = pIUDataReader->GetStringW(s, l+1, 1025, strData, &ulGet); 
#else
					hr = pIUDataReader->GetStringA(s, l+1, 1025, strData, &ulGet); 
#endif
					if(ulGet == 0xFFFFFFFF)
					{
						::_tcscpy(strData, _T("(null)"));
					}
					else if(hr == dbeDataTypeError)
					{
						m_pIURowset->GetDataSize(s, l+1, &lSize);
						::_stprintf(strData, _T("Binary data with size of %d in byte"), lSize);
					}
					else if(hr == dbeLengthTooShort )
					{
						::_tcscpy(strData, _T("Data truncated because buffer size is too small."));
					}
					else
					{
						
					}
					
					if(l == 0 && s >= nItems)
					{
						USES_CONVERSION;
						nItem = m_lstRecords.InsertItem(s, strData, -1);
					}
					else
					{
						USES_CONVERSION;
						nItem = s;
						m_lstRecords.SetItemText(nItem, l, strData);
					}

				}
			}
			m_lstRecords.SendMessage(WM_SETREDRAW, TRUE, 0);
		}
		break;
	default:
		break;
	}
	return S_OK;
}
	

HRESULT CCeDBDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_FIRST_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_NEXT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_LAST_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_PREV_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_GO_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CCeDBDlg::OnSocketConnected(long hSocket, long lError)
{
	HRESULT hr;
	if(lError == 0) //no error
	{
		short sEM = 0;
		GetIUSocket()->get_EncryptionMethod(&sEM);
		if(sEM == MSSSL || sEM == MSTLSv1)
		{
			CComBSTR bstrSubject;
			CComBSTR bstrIssuer;
			CComBSTR bstrSigAlg;
			VARIANT_BOOL bValid = VARIANT_FALSE;
			CComVariant vtPeerCert;
			GetIUSocket()->get_PeerCertificate(&vtPeerCert);
			CComQIPtr<IUCert> pIUCert = vtPeerCert.punkVal;
			pIUCert->get_SigAlg(&bstrSigAlg);
			pIUCert->get_Validity(&bValid);
			pIUCert->get_Subject(&bstrSubject);
			pIUCert->get_Issuer(&bstrIssuer);
			if(bValid == VARIANT_FALSE)
			{
				Shutdown();	//can't call IUSocket::Disconnect() here
											//Doing so will leads a crash inside OpenSSL dlls.
				return S_OK;
			}
		}

		SetUID(m_strUserID);
		SetPassword(m_strPassword);
		
		SwitchTo(sidOleDB); //switch for OLEDB service

		BeginBatching();
		if(IsDlgButtonChecked(IDC_ZIP_CHECK))
		{
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_TRUE);
		}
		else
		{
			hr = GetIUSocket()->TurnOnZipAtSvr(VARIANT_FALSE);
		}
		hr = m_pIUDataSource->Open(CComBSTR(m_strOLEDBConnection)); //open a datasource
		hr = m_pIUSession->Open(); //open a DB session
		hr = m_pIUCommand->Open(); //open a command
		Commit(true);

		WaitAll();
	}
	else
	{
		USES_CONVERSION;
		CString strMsg;
		CComBSTR bstrError= GetErrorMsg();
		strMsg.Format(_T("Error code = %d; Error Message = %s"), lError, OLE2T(bstrError));
		MessageBox(strMsg);
	}
	return S_OK;
}

void CCeDBDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	Disconnect();
}

void CCeDBDlg::OnConnectButton() 
{
	USES_CONVERSION;
	HRESULT hr;
	UpdateData();
	if(IsDlgButtonChecked(IDC_SSL_CHECK))
	{
		hr = GetIUSocket()->put_EncryptionMethod(MSTLSv1); //MS SSPI
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

void CCeDBDlg::OnDisconnectButton() 
{
	Disconnect();
}

void CCeDBDlg::OnGoButton() 
{
	HRESULT hr;
	USES_CONVERSION;
	UpdateData(TRUE);

	BeginBatching();
	hr = m_pIUCommand->ExecuteSQL(T2OLE((LPTSTR)LPCTSTR(m_strSQL)), coRowset, ctStatic, rhScrollable);
	hr = m_pIURowset->Open();
//	DBPROP_CANSCROLLBACKWARDS	= 0x15L,
	hr = m_pIURowset->GetProperty(0x15);
	hr = m_pIURowset->GetBatchRecords(0, VARIANT_TRUE);
	Commit(true);
}

void CCeDBDlg::OnFirstButton() 
{
	HRESULT hr;
	hr = m_pIURowset->GetBatchRecords(0, VARIANT_TRUE);
}

void CCeDBDlg::OnLastButton() 
{
	HRESULT hr;
	hr = m_pIURowset->GetBatchRecordsLast();
}

void CCeDBDlg::OnPrevButton() 
{
	HRESULT hr;
	hr = m_pIURowset->GetBatchRecordsEx(0, -2);
}

void CCeDBDlg::OnNextButton() 
{
	HRESULT hr;
	hr = m_pIURowset->GetBatchRecordsEx();
}

int CCeDBDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
