// CeDBDlg.h : header file
//

#if !defined(AFX_CEDBDLG_H__1DA12719_8D09_42D3_B59A_86549C1933CC__INCLUDED_)
#define AFX_CEDBDLG_H__1DA12719_8D09_42D3_B59A_86549C1933CC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CCeDBDlg dialog

class CCeDBDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CCeDBDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

public:
	virtual HRESULT __stdcall OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag);
	
// Dialog Data
	//{{AFX_DATA(CCeDBDlg)
	enum { IDD = IDD_CEDB_DIALOG };
	CListCtrl	m_lstRecords;
	CString	m_strSQL;
	UINT	m_nPort;
	CString	m_strHost;
	BOOL	m_bZip;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CString					m_strOLEDBConnection;
	CString					m_strUserID;
	CString					m_strPassword;

	CComPtr<IUDataSource>	m_pIUDataSource;
	CComPtr<IUSession>		m_pIUSession;
	CComPtr<IUCommand>		m_pIUCommand;
	CComPtr<IURowset>		m_pIURowset;

	CClientServiceEvent<CCeDBDlg> m_DSEvent;
	CClientServiceEvent<CCeDBDlg> m_SessionEvent;
	CClientServiceEvent<CCeDBDlg> m_CmndEvent;
	CClientServiceEvent<CCeDBDlg> m_RowsetEvent;

	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);

	// Generated message map functions
	//{{AFX_MSG(CCeDBDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnConnectButton();
	afx_msg void OnDisconnectButton();
	afx_msg void OnGoButton();
	afx_msg void OnFirstButton();
	afx_msg void OnLastButton();
	afx_msg void OnPrevButton();
	afx_msg void OnNextButton();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEDBDLG_H__1DA12719_8D09_42D3_B59A_86549C1933CC__INCLUDED_)
