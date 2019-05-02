// SOneClientDlg.h : header file
//

#if !defined(AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_)
#define AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CSOneClientDlg dialog

class CSOneClientDlg : public CDialog, public CClientSocket, public IAsyncResultsHandler
{
// Construction
public:
	CSOneClientDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSOneClientDlg)
	enum { IDD = IDD_SONECLIENT_DIALOG };
	CString	m_strHost;
	CString	m_strPassword;
	UINT	m_nPort;
	UINT	m_nSleep;
	CString	m_strUID;
	BOOL	m_bZip;
	BOOL	m_bFrozen;
	INT	m_nCount;
	INT	m_nGlobalCount;
	INT	m_nGlobalFastCount;
	BOOL	m_bUseSSL;
	CComVariant m_vtOut;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSOneClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	HRESULT OnSocketClosed(long hSocket, long lError);
	HRESULT OnSocketConnected(long hSocket, long lError);
	void OnBaseRequestProcessed(unsigned short usBaseRequestID);
	virtual void Process(CAsyncResult &AsyncResult);
	virtual void OnExceptionFromServer(CAsyncServiceHandler &AsyncServiceHandler, CSocketProServerException &Exception);
	static void MyProcess(CAsyncResult &AsyncResult);
			

// Implementation
protected:
	HICON m_hIcon;
	CAsyncServiceHandler	m_MySvsHandler;

	// Generated message map functions
	//{{AFX_MSG(CSOneClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnZipCheck();
	afx_msg void OnFrozenCheck();
	afx_msg void OnConnectButton();
	afx_msg void OnDisconnectButton();
	afx_msg void OnSleepButton();
	afx_msg void OnGetallcountsButton();
	afx_msg void OnDuerycountButton();
	afx_msg void OnGlobalcountButton();
	afx_msg void OnGlobalfastcountButton();
	afx_msg void OnEchodataButton();
	afx_msg void OnUsesslCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_)
