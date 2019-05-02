// PoolSvr.h: interface for the CPoolSvr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POOLSVR_H__13BD0A90_69EF_4EF0_A1A3_DF46020D672C__INCLUDED_)
#define AFX_POOLSVR_H__13BD0A90_69EF_4EF0_A1A3_DF46020D672C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "sprowrap.h"

#define WM_MY_EVENT_ONACCEPT		(WM_USER + 0x0200)
#define WM_MY_EVENT_ONCLOSE			(WM_USER + 0x0201)
#define WM_MY_EVENT_ONISPERMITTED	(WM_USER + 0x0202)
#define WM_UPDATE_LB_STATUS			(WM_USER + 0x0203)

#define sidCRAdo		(odUserServiceIDMin + 202)
const unsigned long		MAX_JOB_QUEUE_SIZE = 5;
const long				Failover = 11111;
const long				ExceedingMaxJobQueueSize = 11112;
const long				JobQueueNormal = 11113;
const long				NoRealServerAvailable = 11114;

class CMyPLGPeer : public CPLGPeer<CClientPeer>
{
protected:
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		//intercept data inside m_UQueue, and modify it here if neccessary
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		//intercept data inside m_UQueue, and modify it here if neccessary 

		return S_OK;
	}

	virtual void OnChatRequestComing(tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2)
	{
		switch(ChatRequestId)
		{
		case idEnter:
			{
				long lConnected;
				WCHAR strUserId[256] = {0};
				CComPtr<IUSocketPool> pIUSocketPool = GetJobManager()->GetSocketPool();
				GetUID(strUserId, sizeof(strUserId)/sizeof(WCHAR));
				pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
				if(lConnected == 0)
				{
					CComVariant vtNoRealServerAvailable(NoRealServerAvailable);
					GetPush()->SendUserMessage(vtNoRealServerAvailable, strUserId);
				}
				else if(GetJobManager()->GetCountOfJobs() >= MAX_JOB_QUEUE_SIZE)
				{
					CComVariant vtMax(ExceedingMaxJobQueueSize);
					GetPush()->SendUserMessage(vtMax, strUserId);
				}
				else
				{
					CComVariant vtMax(JobQueueNormal);
					GetPush()->SendUserMessage(vtMax, strUserId);
				}
			}
			break;
		default:
			break;
		}
	}

public:
	virtual bool OnSendingPeerData(IJobContext *pJobContext, unsigned short usRequestId, CUQueue &UQueue)
	{
		//you can modify data inside UQueue here if neccessary

		return true; //true, will send result data in UQueue onto client peer; false, will not
	}
};

class CMyPLGService : public CPLGService<sidCRAdo, CMyPLGPeer>
{
public:
	HWND					m_hDlg;

protected:
	virtual bool OnFailover(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
	{
		WCHAR strUserId[256] = {0};
		CMyPLGPeer *pPeer = (CMyPLGPeer*)pJobContext->GetIdentity();
		pPeer->GetUID(strUserId, sizeof(strUserId)/sizeof(WCHAR));
		CComVariant vtFailover(Failover);
		pPeer->GetPush()->SendUserMessage(vtFailover, strUserId);
		if(m_hDlg != UNULL_PTR)
			::PostMessage(m_hDlg, WM_UPDATE_LB_STATUS, 0, 0);
		return true; //true, make disaster recovery; false, no disaster recovery
	}

	virtual void OnJobDone(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
	{
		unsigned long ulJobs = pJobContext->GetJobManager()->GetCountOfJobs();
		if(ulJobs < (MAX_JOB_QUEUE_SIZE - 1))
		{
			CComVariant vtMin(JobQueueNormal);
			CMyPLGPeer *pPeer = (CMyPLGPeer*)pJobContext->GetIdentity();
			unsigned long pGroup[] = {1};
			pPeer->GetPush()->Broadcast(vtMin, pGroup, 1);
		}
		if(m_hDlg != UNULL_PTR)
			::PostMessage(m_hDlg, WM_UPDATE_LB_STATUS, 0, 0);
	}

	virtual void OnAllSocketsDisconnected()
	{
		CComVariant vtNoRealServerAvailable(NoRealServerAvailable);
		
		//just get any one of peer sockets
		unsigned int hSocket = CSocketProServer::GetClient(0);
		if(hSocket != 0 && hSocket != (~0))
		{
			unsigned long pGroup[] = {1};
			CComVariant vtGroups;
			IUPush::CreateVtGroups(pGroup, 1, vtGroups);
			g_SocketProLoader.XSpeak(hSocket, &vtNoRealServerAvailable, &vtGroups);
		}

		if(m_hDlg != UNULL_PTR)
			::PostMessage(m_hDlg, WM_UPDATE_LB_STATUS, 0, 0);
	}

	virtual bool OnExecutingJob(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
	{
		unsigned long ulJobs = pJobContext->GetJobManager()->GetCountOfJobs();
		if(ulJobs == MAX_JOB_QUEUE_SIZE)
		{
			unsigned long pGroup[] = {1};
			CComVariant vtMax(ExceedingMaxJobQueueSize);
			CMyPLGPeer *pPeer = (CMyPLGPeer*)pJobContext->GetIdentity();
			pPeer->GetPush()->Broadcast(vtMax, pGroup, 1);
		}
		if(m_hDlg != UNULL_PTR)
			::PostMessage(m_hDlg, WM_UPDATE_LB_STATUS, 0, 0);
		return true; //true, execute the job pJobContext; false, the job discarded.
	}
};	


class CPoolSvr : public CSocketProServer  
{
public:
	CPoolSvr();
	virtual ~CPoolSvr();

public:
	bool Run(int nPort, BYTE bThreadCount, BYTE bSocketsPerThread);
	void Stop();
	
protected:
	void OnAccept(unsigned int hSocket, int nError);
	void OnClose(unsigned int hSocket, int nError);
	bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID);
	bool OnSettingServer();

private:
	bool IsAllowed(LPCWSTR strUserID, LPCWSTR strPassword);
	bool AddService();
	
public: 
	CMyPLGService			m_RadoPoolSvs;
	HWND					m_hDlg;

private:
	bool					m_bRunning;
	int						m_nPort;
	BYTE					m_bThreadCount;
    BYTE					m_bSocketsPerThread;
};

#endif // !defined(AFX_POOLSVR_H__13BD0A90_69EF_4EF0_A1A3_DF46020D672C__INCLUDED_)
