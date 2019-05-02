// SOneClientDlg.h : header file
//

#if !defined(AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_)
#define AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CSOneClientDlg dialog

class CSOneClientDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CSOneClientDlg(CWnd* pParent = UNULL_PTR);	// standard constructor
	~CSOneClientDlg();

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
	INT		m_nCount;
	INT		m_nGlobalCount;
	INT		m_nGlobalFastCount;
	UINT	m_nTimeRequired;
	long	m_lBytesRecv;
	long	m_lBytesSent;
	UINT	m_lGetALot;
	UINT	m_lLatency;
	UINT	m_lSendALot;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSOneClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	HRESULT OnSocketClosed(long hSocket, long lError);
	HRESULT OnSocketConnected(long hSocket, long lError);
	HRESULT OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);

	void OnBaseRequestProcessed(unsigned short usBaseRequestID);
	void UpdateBytes();
	

// Implementation
protected:
	void EnableControls(BOOL bEnable = TRUE);
	HICON m_hIcon;
	CTOne						m_MySvsHandler;
	CTThree						m_S3Handler;
	//CAsyncServiceHandlerEx<CSOneClientDlg> m_ashEx;
	CUPerformanceQuery			m_PerfQuery;
	LONGLONG					m_lPrev;
	stack<CTestItem*>			m_Stack;
	void PrepareStack(int nSize);
	void CleanStack();

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
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnUsesslCheck();
	afx_msg void OnGetoneButton();
	afx_msg void OnGetalotButton();
	afx_msg void OnSendoneButton();
	afx_msg void OnSendalotButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONECLIENTDLG_H__5F90032F_E8CA_45D9_ACAE_EE1AA0C9996D__INCLUDED_)
