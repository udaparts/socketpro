
// WinClientTestDlg.h : header file
//

#pragma once

#include "myclient.h"


// CWinClientTestDlg dialog
class CWinClientTestDlg : public CDialogEx
{
// Construction
public:
	CWinClientTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WINCLIENTTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	MB::ClientSide::CUSocketPool	m_SocketPool;
	CMySocket						*m_pSocket;
	CMyServiceHandler				*m_pServiceHander;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};
