// ComputePiByLoadingBalanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ComputePiByLoadingBalance.h"
#include "ComputePiByLoadingBalanceDlg.h"

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


// CComputePiByLoadingBalanceDlg dialog




CComputePiByLoadingBalanceDlg::CComputePiByLoadingBalanceDlg(CWnd* pParent /*=UNULL_PTR*/)
	: CDialog(CComputePiByLoadingBalanceDlg::IDD, pParent), m_dPi(0.0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CComputePiByLoadingBalanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CComputePiByLoadingBalanceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, &CComputePiByLoadingBalanceDlg::OnBnClickedCancelButton)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CComputePiByLoadingBalanceDlg::OnBnClickedConnectButton)
	ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CComputePiByLoadingBalanceDlg::OnBnClickedCloseButton)
	ON_BN_CLICKED(IDC_COMPUTE_BUTTON, &CComputePiByLoadingBalanceDlg::OnBnClickedComputeButton)
END_MESSAGE_MAP()


// CComputePiByLoadingBalanceDlg message handlers

BOOL CComputePiByLoadingBalanceDlg::OnInitDialog()
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

	GetDlgItem(IDC_HOST_EDIT)->SetWindowText(_T("localhost"));
	GetDlgItem(IDC_PORT_EDIT)->SetWindowText(_T("20910"));

	//attach the handler to a CClientSocket
	m_PiHandler.Attach(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

HRESULT CComputePiByLoadingBalanceDlg::OnSocketClosed(long hSocket, long lError)
{
	GetDlgItem(IDC_COMPUTE_BUTTON)->EnableWindow(FALSE);
	return S_OK;
}

HRESULT CComputePiByLoadingBalanceDlg::OnSocketConnected(long hSocket, long lError)
{
	if(lError == S_OK)
	{
		SetUID(_T("SocketPro"));
		SetPassword(_T("PassOne"));
		SwitchTo(&m_PiHandler);
		GetDlgItem(IDC_COMPUTE_BUTTON)->EnableWindow(TRUE);
	}

	return S_OK;
}

HRESULT CComputePiByLoadingBalanceDlg::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	HRESULT hr = CClientSocket::OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
	if(nRequestID == idComputeCPPi && sFlag == rfCompleted)
	{
		m_dPi += m_PiHandler.m_ComputeRtn;
		m_nDivision++;
		TCHAR strStatus[1024] = {0};
		::_stprintf_s(strStatus, _T("Pi = %.15f with completion percentage = %d"), m_dPi, m_nDivision);
		GetDlgItem(IDC_STATUS_EDIT)->SetWindowText(strStatus);
	}
	return hr;
}

void CComputePiByLoadingBalanceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CComputePiByLoadingBalanceDlg::OnPaint()
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
HCURSOR CComputePiByLoadingBalanceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CComputePiByLoadingBalanceDlg::OnBnClickedCancelButton()
{
	Cancel();
}

void CComputePiByLoadingBalanceDlg::OnBnClickedConnectButton()
{
	CString strHost;
	CString strPort;

	GetDlgItem(IDC_HOST_EDIT)->GetWindowText(strHost);
	GetDlgItem(IDC_PORT_EDIT)->GetWindowText(strPort);
	int nPort = _ttoi(strPort);

	Connect(strHost, nPort);
}

void CComputePiByLoadingBalanceDlg::OnBnClickedCloseButton()
{
	Shutdown();
}

void CComputePiByLoadingBalanceDlg::OnBnClickedComputeButton()
{
	int		n;
	double	dStart;
	
	if(GetCountOfRequestsInQueue() > 0)
	{
		//if it is still in processing, we cancel first, and wait until all of requests are processed
		Cancel();
		WaitAll();
	}

	int	nNum = 10000000;
	m_nDivision = 0;
	double	dStep = 1.0/nNum/100;
	m_dPi = 0.0;
	
	BeginBatching();
	//divide a large task into nDivision sub-tasks
	for(n=0; n<100; n++)
	{
		dStart = (double)n/100;
		m_PiHandler.ComputeAsyn(dStart, dStep, nNum);
	}
	Commit();
}
