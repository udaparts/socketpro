// PiMFCDlg.h : header file
//

#pragma once
#include "Pi.h"

#define WM_CUST_PI	(WM_USER + 0x222)

class CPiParallel : public CSocketPoolEx<CPPi>
{
	#define m_nDivision 100
public:
	int GetProgress()
	{
		return m_nDivision - GetJobManager()->GetCountOfQueuedJobs();
	}

	double GetPi()
	{
		CAutoLock	AutoLock(&m_cs.m_sec);
		return m_dPi;
	}

	void PrepareAndExecuteJobs()
	{
		int		n;
		double	dStart;
		int		nNum = 10000000;
		double	dStep = 1.0/nNum/m_nDivision;
		
		{
			CAutoLock	AutoLock(&m_cs.m_sec);

			//initialize member
			m_dPi = 0.0;
		}
		
		//get an async handler
		CPPi *pi = (CPPi *)GetJobManager()->LockIdentity();
		//a job containing one task only
		for(n=0; n<m_nDivision; n++)
		{
			dStart = (double)n/m_nDivision;
			pi->ComputeAsyn(dStart, dStep, nNum);
		}
		
		//a job containing two tasks
/*		for(n=0; n<m_nDivision; n++)
		{
			pi->GetAttachedClientSocket()->StartJob();
			dStart = (double)n/m_nDivision;
			pi->ComputeAsyn(dStart, dStep, nNum);
			n += 1;
			dStart = (double)n/m_nDivision;
			pi->ComputeAsyn(dStart, dStep, nNum);
			pi->GetAttachedClientSocket()->EndJob();
		}*/

		GetJobManager()->UnlockIdentity(pi);

		//manually divide a large task into nDivision sub-tasks
/*		int		nTaskId;
		bool	ok;
		CUQueue	UQueue;
		unsigned short usRequestId = (unsigned short)idComputeCPPi;
		for(n=0; n<m_nDivision; n++)
		{
			dStart = (double)n/m_nDivision;
			UQueue.SetSize(0);

			UQueue<<dStart;
			UQueue<<dStep;
			UQueue<<nNum;

			IJobContext *jc = GetJobManager()->CreateJob(UNULL_PTR);
			nTaskId = jc->AddTask(usRequestId, UQueue.GetBuffer(), UQueue.GetSize());
			ok = GetJobManager()->EnqueueJob(jc);
			Process();
		}*/
	}

	bool BuildConnections()
	{
		int n;
		CComBSTR bstrLocal(L"127.0.0.1");
		CComBSTR bstrDesk(L"localhost");
		CComBSTR bstrLaptop(L"127.0.0.1");
		CComBSTR bstrYYEXP(L"localhost");
		CComBSTR bstrSomeOne(L"127.0.0.1");
		
		CComBSTR bstrUserId(L"SocketPro");
		CComBSTR bstrPassword(L"PassOne");

		//set connection contexts
		CConnectionContext pConnectionContext[5];
		pConnectionContext[0].m_strHost = bstrLocal.m_str;
		pConnectionContext[1].m_strHost = bstrDesk.m_str;
		pConnectionContext[2].m_strHost = bstrLaptop.m_str;
		pConnectionContext[3].m_strHost = bstrYYEXP.m_str;
		pConnectionContext[4].m_strHost = bstrSomeOne.m_str;
		for(n=0; n<5; n++)
		{
			pConnectionContext[n].m_nPort = 20901;
			pConnectionContext[n].m_strPassword = bstrPassword.m_str;
			pConnectionContext[n].m_strUID = bstrUserId.m_str;
			pConnectionContext[n].m_EncrytionMethod = NoEncryption;
			pConnectionContext[n].m_bZip = false;
		}
		//start socket pool
		return StartSocketPool(pConnectionContext, 5, 2, 3);
	}

protected:
	//overrite three pure virtual methods
	virtual bool OnFailover(CPPi *pHandler, IJobContext *pJobContext);
	virtual void OnReturnedResultProcessed(CPPi *pHandler, IJobContext *pJobContext, unsigned short usRequestId);
	virtual void OnJobDone(CPPi *pHandler, IJobContext *pJobContext);
public:
	HWND	m_hWnd;

private:
	CComAutoCriticalSection m_cs;
	
	//need to be protected by a critical section
	double m_dPi;
};

// CPiMFCDlg dialog
class CPiMFCDlg : public CDialog
{
// Construction
public:
	CPiMFCDlg(CWnd* pParent = UNULL_PTR);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PIMFC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	CPiParallel	m_PiMPI;

	void UpdateControls();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSs();
	afx_msg void OnBnClickedButtonPr();
};
