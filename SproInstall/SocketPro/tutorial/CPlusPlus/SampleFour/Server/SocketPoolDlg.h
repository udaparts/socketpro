// SocketPoolDlg.h : header file
//

#if !defined(AFX_SOCKETPOOLDLG_H__C847C327_066C_4A5F_A51D_F9D8E39FCAC0__INCLUDED_)
#define AFX_SOCKETPOOLDLG_H__C847C327_066C_4A5F_A51D_F9D8E39FCAC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSocketPoolDlg dialog

#include "PoolSvr.h"

class CSocketPoolDlg : public CDialog
{
// Construction
public:
	CSocketPoolDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSocketPoolDlg)
	enum { IDD = IDD_SOCKETPOOL_DIALOG };
	CListBox		m_lstInfo;
	CString			m_strLBStatus;
	UINT			m_nPort;
	long			m_lConnectedSockets;
	DWORD			m_lSocketConnections;
	BYTE			m_bSocketsPerThread;
	BYTE			m_bThreadCount;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketPoolDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

private:
	void OnAccept(unsigned int hSocket, int nError);
	void OnClose(unsigned int hSocket, int nError);
	bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID);
	void OnUpdateLBStatus();

// Implementation
protected:
	HICON			m_hIcon;
	CPoolSvr		*m_pPoolSvr;
	
	// Generated message map functions
	//{{AFX_MSG(CSocketPoolDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStartButton();
	afx_msg void OnStopButton();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddrealserverButton();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETPOOLDLG_H__C847C327_066C_4A5F_A51D_F9D8E39FCAC0__INCLUDED_)
