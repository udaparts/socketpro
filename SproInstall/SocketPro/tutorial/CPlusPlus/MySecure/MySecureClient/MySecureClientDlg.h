// MySecureClientDlg.h : header file
//

#if !defined(AFX_MYSECURECLIENTDLG_H__917DB1AC_D148_4925_AABF_DA8CB68512B7__INCLUDED_)
#define AFX_MYSECURECLIENTDLG_H__917DB1AC_D148_4925_AABF_DA8CB68512B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMySecureClientDlg dialog

class CMySecureClientDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CMySecureClientDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMySecureClientDlg)
	enum { IDD = IDD_MYSECURECLIENT_DIALOG };
	UINT	m_nPort;
	CString	m_strHost;
	CString	m_strPassword;
	CString	m_strSQL;
	CString	m_strUserID;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySecureClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);
	CMySecure	m_MySecure;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMySecureClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnectButton();
	afx_msg void OnDisconnectButton();
	afx_msg void OnExecutesqlButton();
	afx_msg void OnTodbButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSECURECLIENTDLG_H__917DB1AC_D148_4925_AABF_DA8CB68512B7__INCLUDED_)
