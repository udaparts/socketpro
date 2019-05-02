// ClientDlg.h : header file
//

#pragma once


// CClientDlg dialog
class CClientDlg : public CDialog, public CClientSocket, public IAsyncResultsHandler
{
// Construction
public:
	CClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


protected:
	virtual void Process(CAsyncResult &AsyncResult);
	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);
	CAsyncServiceHandler	m_ash;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDosumButton();
	afx_msg void OnBnClickedRedosumButton();
	afx_msg void OnBnClickedPauseButton();
};
