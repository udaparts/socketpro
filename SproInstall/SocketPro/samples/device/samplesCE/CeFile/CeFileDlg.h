// CeFileDlg.h : header file
//

#if !defined(AFX_CEFILEDLG_H__E59DA97A_CFDB_4462_8A0D_543E69597DB0__INCLUDED_)
#define AFX_CEFILEDLG_H__E59DA97A_CFDB_4462_8A0D_543E69597DB0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CCeFileDlg dialog

class CCeFileDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CCeFileDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

public:
	virtual HRESULT __stdcall OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag);

	
// Dialog Data
	//{{AFX_DATA(CCeFileDlg)
	enum { IDD = IDD_CEFILE_DIALOG };
	CListCtrl	m_lstFile;
	CString	m_strHost;
	CString	m_strLocal;
	UINT	m_nPort;
	CString	m_strRemote;
	double	m_dPercent;
	long	m_lRecv;
	CString	m_strDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

private:
	ULONG	m_ulFileSize;

// Implementation
protected:
	HICON m_hIcon;

	CString					m_strUserID;
	CString					m_strPassword;
	
	CComPtr<IUFile>			m_pIUFile;
	CClientServiceEvent<CCeFileDlg> m_FileEvent;

	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);
	virtual HRESULT OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);

	// Generated message map functions
	//{{AFX_MSG(CCeFileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDisconnectButton();
	afx_msg void OnConnectButton();
	afx_msg void OnDownloadButton();
	afx_msg void OnUploadButton();
	afx_msg void OnCancelButton();
	afx_msg void OnFirstButton();
	afx_msg void OnNextButton();
	afx_msg void OnAllButton();
	afx_msg void OnChangeDirEdit();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEFILEDLG_H__E59DA97A_CFDB_4462_8A0D_543E69597DB0__INCLUDED_)
