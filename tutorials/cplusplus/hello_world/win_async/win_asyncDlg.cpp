#include "stdafx.h"
#include "win_async.h"
#include "win_asyncDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data

    enum {
        IDD = IDD_ABOUTBOX
    };

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cwin_asyncDlg dialog
Cwin_asyncDlg::Cwin_asyncDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(Cwin_asyncDlg::IDD, pParent), m_spHw(false) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cwin_asyncDlg::DoDataExchange(CDataExchange* pDX) {

}

BEGIN_MESSAGE_MAP(Cwin_asyncDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_TEST_BUTTON, &Cwin_asyncDlg::OnBnClickedTestButton)
END_MESSAGE_MAP()


// Cwin_asyncDlg message handlers
BOOL Cwin_asyncDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE); // Set big icon
    SetIcon(m_hIcon, FALSE); // Set small icon

    // TODO: Add extra initialization here

    CConnectionContext cc("localhost", 20901, L"MyUserId", L"MyPassword");

    if (!m_spHw.StartSocketPool(cc, 1)) {
        USES_CONVERSION;
        GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
        std::string err = m_spHw.GetSockets()[0]->GetErrorMsg();
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(CA2CT(err.c_str()));
    }
    return TRUE; // return TRUE  unless you set the focus to a control
}

void Cwin_asyncDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cwin_asyncDlg::OnPaint() {
    if (IsIconic()) {
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
    } else {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.

HCURSOR Cwin_asyncDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

CAwTask Cwin_asyncDlg::ExecuteTask() {
    HWND hWnd = GetDlgItem(IDC_RESULT_EDIT)->m_hWnd;
    auto hw = m_spHw.GetAsyncHandlers()[0];
    try{
        CStringW s = co_await hw->wait_send<CStringW>(idSayHello, L"Jack", L"Smith");
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowTextW(s);
    }

    catch(CServerError & err) {
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(err.ToString().c_str());
        GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
    }

    catch(CSocketError & err) {
        if (::IsWindow(hWnd)) { //dialog destroyed?
            GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(err.ToString().c_str());
            GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
        }
    }

    catch(std::exception & err) {
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(CString(err.what()));
        GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
    }
}

CAwTask Cwin_asyncDlg::ExecuteTasksInBatch() {
    CMyStruct res;
    HWND hWnd = GetDlgItem(IDC_RESULT_EDIT)->m_hWnd;
    auto hw = m_spHw.GetAsyncHandlers()[0];
    CMyStruct ms;
    SetMyStruct(ms);
    try{
        auto fms = hw->send<CMyStruct, CMyStruct>(idEcho, ms);
        auto f0 = hw->sendRequest(idSleep, (unsigned int) 5000);
        CStringW s = co_await hw->wait_send<CStringW>(idSayHello, L"Hillary", L"Clinton");
        //fms definitely contains an instance of returned CMyStruct by this time
        res = fms.get();
        assert(res == ms);
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowTextW(s);
    }

    catch(CServerError & err) {
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(err.ToString().c_str());
        GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
    }

    catch(CSocketError & err) {
        if (::IsWindow(hWnd)) { //dialog destroyed?
            GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(err.ToString().c_str());
            GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
        }
    }

    catch(std::exception & err) {
        GetDlgItem(IDC_RESULT_EDIT)->SetWindowText(CString(err.what()));
        GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
    }
}

void Cwin_asyncDlg::OnBnClickedTestButton() {
    GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(FALSE);
    //execute one request asynchronously
    ExecuteTask();
    //execute multiple requests asynchronously in batch
    ExecuteTasksInBatch();
    GetDlgItem(IDC_TEST_BUTTON)->EnableWindow(TRUE);
}
