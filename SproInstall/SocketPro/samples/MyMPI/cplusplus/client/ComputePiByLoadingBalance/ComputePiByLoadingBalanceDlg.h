// ComputePiByLoadingBalanceDlg.h : header file
//

#pragma once

#include "..\MPIOnClientSide\Pi.h"

// CComputePiByLoadingBalanceDlg dialog
class CComputePiByLoadingBalanceDlg : public CDialog, public CClientSocket
{
// Construction
public:
	CComputePiByLoadingBalanceDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COMPUTEPIBYLOADINGBALANCE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON			m_hIcon;
	CPPi			m_PiHandler;
	double			m_dPi;
	int				m_nDivision;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancelButton();
	afx_msg void OnBnClickedConnectButton();
	afx_msg void OnBnClickedCloseButton();
	afx_msg void OnBnClickedComputeButton();

protected:
	HRESULT OnSocketClosed(long hSocket, long lError);
	HRESULT OnSocketConnected(long hSocket, long lError);
	HRESULT OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);
};
