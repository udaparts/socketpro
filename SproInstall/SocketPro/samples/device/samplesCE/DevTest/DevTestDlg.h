// DevTestDlg.h : header file
//

#if !defined(AFX_DEVTESTDLG_H__D2D9C815_BE85_4505_8FE2_F1692AFC58D9__INCLUDED_)
#define AFX_DEVTESTDLG_H__D2D9C815_BE85_4505_8FE2_F1692AFC58D9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDevTestDlg dialog

class CDevTestDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CDevTestDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDevTestDlg)
	enum { IDD = IDD_DEVTEST_DIALOG };
	CString	m_strHost;
	UINT	m_nPort;
	CString	m_strMsg;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	HICON m_hIcon;
	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);
	virtual void OnBaseRequestProcessed(unsigned short nRequestID);

	// Generated message map functions
	//{{AFX_MSG(CDevTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnConnectButton();
	afx_msg void OnDisconnectButton();
	afx_msg void OnSendButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVTESTDLG_H__D2D9C815_BE85_4505_8FE2_F1692AFC58D9__INCLUDED_)
