#include "StdAfx.h"


namespace SocketProAdapter
{
#ifndef _WIN32_WCE
namespace ServerSide
{

CBaseService::CBaseService()
{
	if(!g_SocketProLoader.IsLoaded())
	{
		if(!g_SocketProLoader.LoadSocketProServer())
		{
#ifdef _WIN64
			throw gcnew System::InvalidProgramException("SocketPro core server library usktpror.dll (x64) is not available!");
#else
			throw gcnew System::InvalidProgramException("SocketPro core server library usktpror.dll (x32) is not available!");
#endif
		}
	}

	m_bPool = false;
	
	m_aClientPeer = gcnew List<CClientPeer^>();
	m_Pool = gcnew List<CClientPeer^>();

	m_pSvsContext = new CSvsContext();
	memset(m_pSvsContext, 0, sizeof(CSvsContext));
	m_ulSvsID = 0;
	
	CAutoLock	AutoLock(&g_cs.m_sec);
	if(m_aService == nullptr)
	{
		m_aService = gcnew List<CBaseService^>();
	}
	m_aService->Add(this);
}

CBaseService::~CBaseService()
{
	m_Pool->Clear();

	{
		CAutoLock	AutoLock(&g_cs.m_sec);
		m_aClientPeer->Clear();
		m_aService->Remove(this);
	}
	m_Pool = nullptr;
	m_aClientPeer = nullptr;
	if(!m_pSvsContext)
	{
		delete m_pSvsContext;
	}
}

IntPtr CBaseService::AddALibrary(String^ strLibFile)
{
	return AddALibrary(strLibFile, 0);
}

IntPtr CBaseService::AddALibrary(String^ strLibFile, int nParam)
{
	if(strLibFile == nullptr || strLibFile->Length == 0)
		throw gcnew ArgumentException("Must set a file name to a library!");
	pin_ptr<const wchar_t> wch = PtrToStringChars(strLibFile);
	return IntPtr((int)g_SocketProLoader.AddADll(wch, nParam));
}

BOOL CALLBACK OnPMessage(MSG* pMsg)
{
	if(pMsg == NULL)
		return FALSE;
	unsigned int hSocket = g_SocketProLoader.GetAssociatedSocket();
	CClientPeer ^p = CBaseService::SeekClientPeerGlobally((int)hSocket);
	if(p != nullptr)
	{
		return p->GetBaseService()->m_OnPretranslateMessage((int)pMsg->hwnd, (int)pMsg->message, (int)pMsg->wParam, (int)pMsg->lParam, (int)pMsg->time, (int)pMsg->pt.x, (int)pMsg->pt.y);
	}
	return FALSE;
}

void CBaseService::OnBaseRequestCame(int hSocket, short sRequestID)
{

}

void CBaseService::OnFastRequestArrive(int hSocket, short sRequestID)
{

}

void CBaseService::OnSlowRequestArrive(int hSocket, short sRequestID)
{

}

void CBaseService::OnRelease(int hSocket, bool bClose, long lInfo)
{

}

void CBaseService::SetDelegates(unsigned int ulEvents)
{
	if(ulEvents & (int)tagEvent::eOnClose)
	{
		m_pSvsContext->m_OnClose = (POnClose)Marshal::GetFunctionPointerForDelegate(m_OnClose).ToPointer();
	}
	
	if(ulEvents & (int)tagEvent::eOnIsPermitted)
	{
		m_pSvsContext->m_OnIsPermitted = (POnIsPermitted)Marshal::GetFunctionPointerForDelegate(m_OnIsPermitted).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnThreadCreated)
	{
		m_pSvsContext->m_OnThreadCreated = (POnThreadCreated)Marshal::GetFunctionPointerForDelegate(m_OnThreadCreated).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnSwitchTo)
	{
		m_pSvsContext->m_OnSwitchTo = (POnSwitchTo)Marshal::GetFunctionPointerForDelegate(m_OnSwitchTo).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnSend)
	{
		m_pSvsContext->m_OnSend = (POnSend)Marshal::GetFunctionPointerForDelegate(m_OnSend).ToPointer();
	}
	
	if(ulEvents & (int)tagEvent::eOnBaseRequestCame)
	{
		m_pSvsContext->m_OnBaseRequestCame = (POnBaseRequestCame)Marshal::GetFunctionPointerForDelegate(m_OnBaseRequestCame).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnCleanPool)
	{
		m_pSvsContext->m_OnCleanPool = (POnCleanPool)Marshal::GetFunctionPointerForDelegate(m_OnCleanPool).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnFastRequestArrive)
	{
		m_pSvsContext->m_OnFastRequestArrive = (POnFastRequestArrive)Marshal::GetFunctionPointerForDelegate(m_OnFastRequestArrive).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnSlowRequestArrive)
	{
		m_pSvsContext->m_SlowProcess = (PSLOW_PROCESS)Marshal::GetFunctionPointerForDelegate(m_SLOW_PROCESS).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnThreadShuttingDown)
	{
		m_pSvsContext->m_ThreadDying = (PTHREAD_DYING)Marshal::GetFunctionPointerForDelegate(m_Thread_DYING).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnThreadStarted)
	{
		m_pSvsContext->m_ThreadStarted = (PTHREAD_STARTED)Marshal::GetFunctionPointerForDelegate(m_THREAD_STARTED).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnReceive)
	{
		m_pSvsContext->m_OnReceive = (POnReceive)Marshal::GetFunctionPointerForDelegate(m_OnReceive).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnSlowRequestProcessed)
	{
		m_pSvsContext->m_OnRequestProcessed = (POnRequestProcessed)Marshal::GetFunctionPointerForDelegate(m_OnSlowRequestProcessed).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnSendReturnData)
	{
		m_pSvsContext->m_OnSendReturnData = (POnSendReturnData)Marshal::GetFunctionPointerForDelegate(m_OnSendReturnData).ToPointer();
	}

	if(ulEvents & (int)tagEvent::eOnPretranslateMessage)
	{
		m_pSvsContext->m_PreTranslateMessage = OnPMessage;
	}
}

bool CBaseService::AddMe(int nSvsID, int nEvents, tagThreadApartment taWhatTA)
{
	if(m_OnClose == nullptr)
	{
		m_OnClose = gcnew DOnClose(this, &CBaseService::OnClose);
	}
	if(m_OnIsPermitted == nullptr)
	{
		m_OnIsPermitted = gcnew DOnIsPermitted(this, &CBaseService::OnPermitted);
	}
	if(m_OnSwitchTo == nullptr)
	{
		m_OnSwitchTo = gcnew DOnSwitchTo(this, &CBaseService::OnSwitch);
	}

	if(m_OnSend == nullptr)
	{
		m_OnSend = gcnew DOnSend(this, &CBaseService::OnSend);
	}

	if(m_OnBaseRequestCame == nullptr)
	{
		m_OnBaseRequestCame = gcnew DOnBaseRequestCame(this, &CBaseService::OnBaseCame);
	}

	if(m_OnCleanPool == nullptr)
	{
		m_OnCleanPool = gcnew DOnCleanPool(this, &CBaseService::OnCleanPool);
	}

	if(m_OnFastRequestArrive == nullptr)
	{
		m_OnFastRequestArrive = gcnew DOnFastRequestArrive(this, &CBaseService::OnFast);
	}

	if(m_SLOW_PROCESS == nullptr)
	{
		m_SLOW_PROCESS = gcnew DSLOW_PROCESS(this, &CBaseService::OnSlow);
	}

	if(m_Thread_DYING == nullptr)
	{
		m_Thread_DYING = gcnew DTHREAD_DYING(this, &CBaseService::OnThreadShuttingDown);
	}
	
	if(m_THREAD_STARTED == nullptr)
	{
		m_THREAD_STARTED = gcnew DTHREAD_STARTED(this, &CBaseService::OnThreadStarted);
	}

	if(m_OnThreadCreated == nullptr)
	{
		m_OnThreadCreated = gcnew DOnThreadCreated(this, &CBaseService::OnThreadCreated);
	}

	if(m_OnReceive == nullptr)
	{
		m_OnReceive = gcnew DOnReceive(this, &CBaseService::OnReceive);
	}

	if(m_OnSlowRequestProcessed == nullptr)
	{
		m_OnSlowRequestProcessed = gcnew DOnRequestProcessed(this, &CBaseService::OnSlowRequestProcessed);
	}

	if(m_OnSendReturnData == nullptr)
	{
		m_OnSendReturnData = gcnew DOnSendReturnData(this, &CBaseService::OnSendReturnData);
	}

	if(m_OnPretranslateMessage == nullptr)
	{
		m_OnPretranslateMessage = gcnew DOnPretranslateMessage(this, &CBaseService::OnPretranslateMessage);
	}

	if(m_ulSvsID != 0)
		return false;

	nEvents |= ((int)tagEvent::eOnSwitchTo + (int)tagEvent::eOnFastRequestArrive + (int)tagEvent::eOnSlowRequestArrive + (int)tagEvent::eOnClose + (int)tagEvent::eOnIsPermitted + (int)tagEvent::eOnCleanPool + (int)tagEvent::eOnBaseRequestCame + (int)tagEvent::eOnSlowRequestProcessed);
	m_pSvsContext->m_enumTA = (enumThreadApartment)taWhatTA;
	SetDelegates(nEvents);
	
	if(g_SocketProLoader.AddSvsContext(nSvsID, *m_pSvsContext))
	{
		m_ulSvsID = (unsigned int)nSvsID;
		return true;
	}
	else
	{
		m_ulSvsID = 0;
	}
	return false;
}

void CBaseService::OnFast(int hSocket, short usRequestID, int ulLen)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p->m_bAutoBuffer)
	{
		CInternalUQueue *UQueue = p->GetUQueue()->GetInternalUQueue();
		if((ULONG)ulLen > UQueue->GetMaxSize())
		{
			UQueue->ReallocBuffer(ulLen);	
		}
		UQueue->SetSize(0);
		if(ulLen > 0)
		{
			BYTE *pBuffer = (BYTE*)UQueue->GetBuffer();
			unsigned long ulGet = g_SocketProLoader.RetrieveBuffer(hSocket, ulLen, pBuffer, false);
			ATLASSERT(ulGet == (ULONG)ulLen);
			UQueue->SetSize(ulLen);
		}
	}
	if(!p->TransferServerException)
	{
		p->OnFRA(usRequestID, ulLen);
	}
	else
	{
		try
		{
			p->OnFRA(usRequestID, ulLen);
		}
		catch(CSocketProServerException ^err)
		{
			CScopeUQueue	UQueue;
			if(err->HResult == S_OK)
			{
				err = gcnew CSocketProServerException(E_FAIL, err->Message, err->m_nSvsID, err->m_sRequestID);
			}
			if(err->m_sRequestID == 0)
			{
				err->m_sRequestID = usRequestID;
			}
			if(err->m_nSvsID == 0)
			{
				err->m_nSvsID = (int)m_ulSvsID;
			}
			UQueue.m_UQueue->Push(err);
			p->SendReturnData(usRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
			return;
		}
		catch(Exception ^e)
		{
			CScopeUQueue	UQueue;
			CSocketProServerException ^err = gcnew CSocketProServerException(E_FAIL, e->Message);
			err->m_sRequestID = usRequestID;
			err->m_nSvsID = (int)m_ulSvsID;
			UQueue.m_UQueue->Push(err);
			p->SendReturnData(usRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
			return;
		}
	}
	OnFastRequestArrive(hSocket, usRequestID);
}

bool CBaseService::OnPermitted(int hSocket, int ulSvsID)
{
	return OnIsPermitted(hSocket);
}

void CBaseService::OnSwitch(int hSocket, int ulPrevSvsID, int ulCurrSvsID)
{
	bool bServerException;
	CSwitchInfo SInfo;
	int nSvsID = (int)g_SocketProLoader.GetSvsID((unsigned int)hSocket);
	if(nSvsID == ulCurrSvsID)
	{
		memset(&SInfo, 0, sizeof(SInfo));
		g_SocketProLoader.GetClientInfo(hSocket, &SInfo);
		bServerException = ((SInfo.m_ulParam5 & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
		memset(&SInfo, 0, sizeof(SInfo));
		g_SocketProLoader.GetServerInfo(hSocket, &SInfo);
		if(bServerException)
		{
			SInfo.m_ulParam5 |= TRANSFER_SERVER_EXCEPTION;
		}
		else
		{
			SInfo.m_ulParam5 &= (~TRANSFER_SERVER_EXCEPTION);
		}
		g_SocketProLoader.SetServerInfo(hSocket, &SInfo);

		CClientPeer ^p = GetClientPeerFromPool();
		if(p == nullptr)
		{
			//if no instance of CClientPeer is found in pool, create a new instance
			p = GetPeerSocket(hSocket);
		}
		if(p != nullptr)
		{
			p->m_hSocket = hSocket;
			if(nSvsID == ulCurrSvsID)
			{
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_aClientPeer->Add(p);
				}
				p->OnSF(ulPrevSvsID);
			}
		}
	}
	else
	{
		CClientPeer ^p = SeekClientPeerGlobally(hSocket);
		ATLASSERT(p != nullptr);
		if(p != nullptr)
		{
			OnRelease(hSocket, false, ulCurrSvsID);
			p->OnRR(false, ulCurrSvsID);
			RemoveClientPeer(p);
		}
	}
}

void CBaseService::OnThreadCreated(int ulSvsID, int ulThreadID, int hSocket)
{
	OnThreadCreated(ulThreadID, hSocket);
}

int CBaseService::OnSlow(short usRequestID, int ulLen, int hSocket)
{
	HRESULT hr = S_OK;
	CClientPeer ^p = SeekClientPeer(hSocket);
	if(p != nullptr)
	{
		CUQueue^ UQueue = p->GetUQueue();
		if(p->m_bAutoBuffer)
		{
			ulLen = UQueue->GetSize();
		}
		if(!p->TransferServerException)
		{
			hr = p->OnSRA(usRequestID, ulLen);
		}
		else
		{
			try
			{
				hr = p->OnSRA(usRequestID, ulLen);
			}
			catch(CSocketProServerException ^err)
			{
				CScopeUQueue	temp;
				if(err->HResult == S_OK)
				{
					err = gcnew CSocketProServerException(E_FAIL, err->Message, err->m_nSvsID, err->m_sRequestID);
				}
				if(err->m_sRequestID == 0)
				{
					err->m_sRequestID = usRequestID;
				}
				if(err->m_nSvsID == 0)
				{
					err->m_nSvsID = (int)m_ulSvsID;
				}
				temp.m_UQueue->Push(err);
				hr = p->SendReturnData(usRequestID, temp.m_UQueue->GetBuffer(), temp.m_UQueue->GetSize());
				return hr;
			}
			catch(Exception ^e)
			{
				CScopeUQueue	temp;
				CSocketProServerException ^err = gcnew CSocketProServerException(E_FAIL, e->Message);
				err->m_sRequestID = usRequestID;
				err->m_nSvsID = (int)m_ulSvsID;
				temp.m_UQueue->Push(err);
				hr = p->SendReturnData(usRequestID, temp.m_UQueue->GetBuffer(), temp.m_UQueue->GetSize());
				return hr;
			}
		}
		OnSlowRequestArrive(hSocket, usRequestID);
	}
	return hr;
}

void CBaseService::OnClose(int hSocket, int nError)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p == nullptr)
		return;
	OnRelease(hSocket, true, nError);
	p->OnRR(true, nError);
	RemoveClientPeer(p);
}

void CBaseService::OnBaseCame(int hSocket, short usRequestID)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
//	ATLASSERT(p != nullptr);
	if(p != nullptr)
	{
		if(usRequestID <= SOCKETPRO_MAX_BASE_REQUEST_ID)
		{
			p->OnBRC(usRequestID);
		}
		else
		{
			if(p->m_bAutoBuffer)
			{
				CInternalUQueue *pQueue = p->GetUQueue()->GetInternalUQueue();
				ULONG ulLen = g_SocketProLoader.GetCurrentRequestLen(hSocket);
				if(ulLen > pQueue->GetMaxSize())
				{
					pQueue->ReallocBuffer((int)ulLen);	
				}
				pQueue->SetSize(0);
				if(ulLen > 0)
				{
					unsigned long ulGet = g_SocketProLoader.RetrieveBuffer(hSocket, ulLen, (BYTE*)pQueue->GetBuffer(), false);
					ATLASSERT(ulGet == ulLen);
					pQueue->SetSize(ulLen);
				}
			}
			//starting with SocketPro version 4.4.1.1
			p->OnDSR(usRequestID);
		}
		OnBaseRequestCame(hSocket, usRequestID);
	}
}

void CBaseService::OnReceive(int hSocket, int nError)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p)
	{
		p->OnR(nError);
	}
}

void CBaseService::OnSend(int hSocket, int nError)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p)
	{
		p->OnS(nError);
	}
}

void CBaseService::OnSlowRequestProcessed(int hSocket, short usRequestID)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p)
	{
		p->OnSRP(usRequestID);
	}
}

bool CBaseService::OnSendReturnData(int hSocket, short usRequestId, int ulLen, unsigned char *pBuffer)
{
	CClientPeer ^p = SeekClientPeer(hSocket);
	ATLASSERT(p != nullptr);
	if(p)
	{
		return p->OnSRData(usRequestId, ulLen, IntPtr((void*)pBuffer));
	}
	return false;
}

void CBaseService::OnCleanPool(int ulSvsID, int ulTickCount)
{
	int n;
	CClientPeer ^p;
	for(n=0; n<m_Pool->Count; n++)
	{
		p = m_Pool[n];
		ATLASSERT(p != nullptr);
		if(p)
		{
			if((unsigned long)ulTickCount > p->m_ulTickCountReleased && ((unsigned long)ulTickCount - p->m_ulTickCountReleased) > g_SocketProLoader.GetCleanPoolInterval())
			{
				delete p;
				m_Pool->RemoveAt(n);
				n--;
			}
		}
	}
}

bool CBaseService::AddMe(int nSvsID)
{
	return AddMe(nSvsID, 0, tagThreadApartment::taNone);
}

bool CBaseService::AddMe(int nSvsID, tagThreadApartment taWhatTA)
{
	return AddMe(nSvsID, 0, taWhatTA);
}

bool CBaseService::AddSlowRequest(short sRequestID)
{
	return g_SocketProLoader.AddSlowRequest(m_ulSvsID, sRequestID);
}

IntPtr CBaseService::GetALibrary(int nIndex)
{
	return IntPtr((int)g_SocketProLoader.GetADll(nIndex));
}

CBaseService^ CBaseService::GetBaseService(int nSvsID)
{
	CAutoLock	AutoLock(&g_cs.m_sec);
	for each (CBaseService ^bs in m_aService)
	{
		if(bs->m_ulSvsID == (unsigned int)nSvsID)
			return bs;
	}
	return nullptr;
}

short CBaseService::GetSlowRequest(short nIndex)
{
	return g_SocketProLoader.GetSlowRequest(m_ulSvsID, nIndex);
}

void CBaseService::RemoveClientPeer(CClientPeer ^p)
{
	ATLASSERT(p != nullptr);
	if(p == nullptr)
		return;
	{
		CAutoLock	AutoLock(&g_cs.m_sec);
		bool bSuc = m_aClientPeer->Remove(p);
		ATLASSERT(bSuc);
	}
	p->m_hSocket = 0;
	p->m_ulTickCountReleased = ::GetTickCount();
	if(m_bPool)
		m_Pool->Add(p);
	else
		delete p;
}

CClientPeer^ CBaseService::GetClientPeerFromPool()
{
	CClientPeer ^p = nullptr;
	if(m_Pool->Count > 0)
	{
		p = m_Pool[0];
		m_Pool->RemoveAt(0);
	}
	return p;
}
/*
CClientPeer^ CBaseService::GetPS(int hSocket)
{
	try
	{
		return GetPeerSocket(hSocket);
	}
	catch(Exception ^eError)
	{
		String^ strError = eError->Message;
		strError = nullptr;
	}
	catch(...)
	{
		DWORD dwError = ::GetLastError();
		dwError = 0;
	}
	return nullptr;
}*/

int CBaseService::GetServiceID(int nIndex)
{
	return g_SocketProLoader.GetServiceID(nIndex);
}

void CBaseService::RemoveAllSlowRequests()
{
	 g_SocketProLoader.RemoveAllSlowRequests(m_ulSvsID);
}

bool CBaseService::RemoveALibrary(String ^strLibFile)
{
	if(strLibFile == nullptr || strLibFile->Length == 0)
		throw gcnew ArgumentException("Must set a string indicating a file!");
	if(::GetCurrentThreadId() != g_SocketProLoader.GetMainThreadID())
		return false; //must call the method within the main thread
	pin_ptr<const wchar_t> wch = PtrToStringChars(strLibFile);
	return g_SocketProLoader.RemoveADll(wch);
}

bool CBaseService::RemoveALibrary(IntPtr hInstance)
{
	if(::GetCurrentThreadId() != g_SocketProLoader.GetMainThreadID())
		return false; //must call the method within the main thread
	return g_SocketProLoader.RemoveADllByHandle((HINSTANCE)hInstance.ToPointer());
}

void CBaseService::RemoveMe()
{
	 g_SocketProLoader.RemoveASvsContext(m_ulSvsID);
}

void CBaseService::RemoveSlowRequest(short sRequestID)
{
	g_SocketProLoader.RemoveSlowRequest(m_ulSvsID, sRequestID);
}

CClientPeer^ CBaseService::SeekClientPeer(int hSocket)
{
	CAutoLock	AutoLock(&g_cs.m_sec);
	for each (CClientPeer ^peer in m_aClientPeer)
	{
		if((int)peer->m_hSocket == hSocket)
			return peer;
	}
	return nullptr;
}

CClientPeer^ CBaseService::SeekClientPeerGlobally(int hSocket)
{
	CAutoLock	AutoLock(&g_cs.m_sec);
	for each (CBaseService ^bs in m_aService)
	{
		for each (CClientPeer ^peer in bs->m_aClientPeer)
		{
			if((int)peer->m_hSocket == hSocket)
				return peer;
		}
	}
	return nullptr;
}


}
#endif
}
