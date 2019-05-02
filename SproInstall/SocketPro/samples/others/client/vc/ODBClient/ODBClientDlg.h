// ODBClientDlg.h : header file
//

#if !defined(AFX_ODBCLIENTDLG_H__E2168E61_F266_45C0_9075_748C72A43196__INCLUDED_)
#define AFX_ODBCLIENTDLG_H__E2168E61_F266_45C0_9075_748C72A43196__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CODBClientDlg dialog

#include "usocketevent.h"
#include "svsevent.h"


class CODBClientDlg : public CDialog
{
// Construction
public:
	CODBClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CODBClientDlg)
	enum { IDD = IDD_ODBCLIENT_DIALOG };
	CString	m_strToDB;
	CString	m_strSQL;
	float	m_fPercent;
	CString	m_strLocalFile;
	CString	m_strRemoteFile;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CODBClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:
	CClientServiceEvent<CODBClientDlg> m_DSEvent;
	CClientServiceEvent<CODBClientDlg> m_SessionEvent;
	CClientServiceEvent<CODBClientDlg> m_CmndEvent;
	CClientServiceEvent<CODBClientDlg> m_RowsetEvent;
	CClientServiceEvent<CODBClientDlg> m_FileEvent;
	
	CComPtr<IUSocket> m_pIUSocketDB;
	CComPtr<IUDataSource>	m_pIUDataSource;
	CComPtr<IUSession> m_pIUSession;
	CComPtr<IUCommand> m_pIUCommand;
	CComPtr<IURowset> m_pIURowset;
	
	CClientSocketEvent<CODBClientDlg> m_DBSocketEvent;
	CClientSocketEvent<CODBClientDlg> m_FileSocketEvent;

	CComPtr<IUSocket> m_pIUSocketFile;
	CComPtr<IUFile>	m_pIUFile;

	BOOL	m_bClientZip;
	CString	m_strHostAddr;
	UINT	m_unPort;
	BOOL	m_bServerZip;
	BOOL	m_bUseSSL;
	CString	m_strPassword;
	CString	m_strUserID;
	unsigned int m_hSocketDB;
	unsigned int m_hSocketFile;
	DWORD	m_dwFileSize;

public:
	virtual HRESULT __stdcall OnSvsRequestProcessed(long hSocket, unsigned short unRequestID, unsigned long ulLen, unsigned long ulLenInBuffer, short sFlag);
	virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError);
	virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam);
	virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError);
	virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError);
	virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent);
	virtual HRESULT __stdcall OnGetHostByAddr(LONG nHandle, BSTR bstrHostName, BSTR bstrHostAlias, LONG lError);
	virtual HRESULT __stdcall OnGetHostByName(LONG hHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, LONG lError);
	virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd);
	virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);

protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CODBClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnConnectButton();
	afx_msg void OnShutdownButton();
	afx_msg void OnTodbButton();
	afx_msg void OnGetpropButton();
	afx_msg void OnExecutesqlButton();
	afx_msg void OnCancelButton();
	afx_msg void OnSendButton();
	afx_msg void OnGetButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ODBCLIENTDLG_H__E2168E61_F266_45C0_9075_748C72A43196__INCLUDED_)
