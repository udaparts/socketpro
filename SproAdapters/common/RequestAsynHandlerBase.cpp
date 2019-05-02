#include "stdafx.h"

namespace SocketProAdapter
{
extern CComAutoCriticalSection	g_cs;
namespace ClientSide
{
	CAsyncServiceHandler::CAsyncServiceHandler(int nServiceId) 
		: m_nSvsId(nServiceId), m_pClientSocket(nullptr), m_pIAsyncResultsHandler(nullptr), 
		m_pCS(new CComAutoCriticalSection()), m_lstFunc(gcnew List<CPair^>()), m_nSyncIndex(0)
	{
		m_Copy = gcnew DAsyncResultHandler(this, &CAsyncServiceHandler::Copy);
		m_mapSync = gcnew List<CIndexQueue^>();
	}

	CAsyncServiceHandler::CAsyncServiceHandler(int nServiceId, CClientSocket ^cs)
		: m_nSvsId(nServiceId), m_pClientSocket(cs), m_pIAsyncResultsHandler(nullptr), 
		m_pCS(new CComAutoCriticalSection()), m_lstFunc(gcnew List<CPair^>()), m_nSyncIndex(0)
	{
		m_Copy = gcnew DAsyncResultHandler(this, &CAsyncServiceHandler::Copy);
		if(m_pClientSocket != nullptr)
			Attach(m_pClientSocket);
		m_mapSync = gcnew List<CIndexQueue^>();
	}

	CAsyncServiceHandler::CAsyncServiceHandler(int nServiceId, CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler)
		: m_nSvsId(nServiceId), m_pClientSocket(cs), m_pIAsyncResultsHandler(DefaultAsyncResultsHandler), 
		m_pCS(new CComAutoCriticalSection()), m_lstFunc(gcnew List<CPair^>()), m_nSyncIndex(0)
	{
		m_Copy = gcnew DAsyncResultHandler(this, &CAsyncServiceHandler::Copy);
		if(m_pClientSocket != nullptr)
			Attach(m_pClientSocket);
		m_mapSync = gcnew List<CIndexQueue^>();
	}

	CAsyncServiceHandler::~CAsyncServiceHandler()
	{
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pClientSocket != nullptr)
			{
				Detach();
			}
			for each(CIndexQueue^ index in m_mapSync)
			{
				CScopeUQueue::Unlock(index->UQueue);
			}
		}
		delete m_pCS;
	}

	CClientSocket^ CAsyncServiceHandler::GetAttachedClientSocket()
	{
		CAutoLock al(&m_pCS->m_sec);
		return m_pClientSocket;
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, DAsyncResultHandler ^AsyncResultHandler)
	{
		return SendRequest(sRequestID, IntPtr::Zero, 0, AsyncResultHandler);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID)
	{
		return SendRequest(sRequestID, IntPtr::Zero, 0, (DAsyncResultHandler^)nullptr);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, IntPtr pBuffer, int nLenInByte)
	{
		return SendRequest(sRequestID, pBuffer, nLenInByte, (DAsyncResultHandler ^)nullptr);
	}

	bool CAsyncServiceHandler::Send(short sRequestID, IntPtr pBuffer, int nLenInByte)
	{
		CAutoLock al(&m_pCS->m_sec);
		if(m_pClientSocket == nullptr)
		{
			throw gcnew System::InvalidOperationException(gcnew String("The handler is not attached with a socket!"));
		}
		if((unsigned short)sRequestID <= idMultiPart && GetSvsID() != sidHTTPOnRealServer)
		{
			//Your request id must be between odUserRequestIDMin and odUserRequestIDMax.
			//UDAParts reserves all request ids from 0 to odUserRequestIDMin - 1.
			throw gcnew Exception(gcnew String("The request id can not be less than idMultiPart (55)!"));
		}
		
		IJobManager ^pIJobManager = m_pClientSocket->JobManager;
		if(pIJobManager != nullptr)
		{
			IJobContext ^pIJobContext = m_pClientSocket->CurrentJobContext;
			if(pIJobContext != nullptr && pIJobContext->JobStatus == tagJobStatus::jsCreating)
			{
				return (pIJobContext->AddTask(sRequestID, pBuffer, nLenInByte) != 0);
			}
			else
			{
				m_pClientSocket->StartJob();
				pIJobContext = m_pClientSocket->CurrentJobContext;
				pIJobContext->AddTask(sRequestID, pBuffer, nLenInByte);
				return m_pClientSocket->EndJob();
			}
		}

		if(m_pClientSocket->m_pIUFast == NULL)
		{
			CComVariant vtBuffer;
			if(pBuffer != IntPtr::Zero && (ULONG)nLenInByte > 0)
			{
				BYTE *p;
				SAFEARRAYBOUND sab[1] = {(ULONG)nLenInByte, 0};
				vtBuffer.vt = (VT_ARRAY|VT_UI1);
				vtBuffer.parray = ::SafeArrayCreate(VT_UI1, 1, sab);
				::SafeArrayAccessData(vtBuffer.parray, (void**)&p);
				::memmove(p, pBuffer.ToPointer(), (ULONG)nLenInByte);
				::SafeArrayUnaccessData(vtBuffer.parray);
			}
			return (m_pClientSocket->m_pIUSocket->SendRequest(sRequestID, vtBuffer, VARIANT_FALSE) == S_OK);
		}
		return (m_pClientSocket->m_pIUFast->SendRequestEx(sRequestID, nLenInByte, (BYTE*)pBuffer.ToPointer()) == S_OK);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, IntPtr pBuffer, int nLenInByte, DAsyncResultHandler ^AsyncResultHandler)
	{
		bool b = Send(sRequestID, pBuffer, nLenInByte);
		if(b && AsyncResultHandler != nullptr)
		{
			unsigned __int64 index = 0;
			bool bSync = (m_Copy == AsyncResultHandler);
			CAutoLock al(&m_pCS->m_sec);
			if(bSync)
			{
				++m_nSyncIndex;
				index = m_nSyncIndex;
			}
			CPair ^p = gcnew CPair(sRequestID, AsyncResultHandler, index);
			m_lstFunc->Add(p);
		}
		return b;
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, CUQueue ^UQueue)
	{
		return SendRequest(sRequestID, UQueue, (DAsyncResultHandler ^)nullptr);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, CUQueue ^UQueue, DAsyncResultHandler ^AsyncResultHandler)
	{
		if(UQueue == nullptr)
			return SendRequest(sRequestID, IntPtr::Zero, 0, AsyncResultHandler);
		return SendRequest(sRequestID, UQueue->GetBuffer(), UQueue->GetSize(), AsyncResultHandler);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, CScopeUQueue ^UQueue)
	{
		return SendRequest(sRequestID, UQueue, (DAsyncResultHandler ^)nullptr);
	}

	bool CAsyncServiceHandler::SendRequest(short sRequestID, CScopeUQueue ^UQueue, DAsyncResultHandler ^AsyncResultHandler)
	{
		if(UQueue == nullptr)
			return SendRequest(sRequestID, IntPtr::Zero, 0, AsyncResultHandler);
		return SendRequest(sRequestID, UQueue->m_UQueue->GetBuffer(), UQueue->m_UQueue->GetSize(), AsyncResultHandler);
	}

	bool CAsyncServiceHandler::ProcessR0(short sRequestID)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			if(!SendRequest(sRequestID, m_Copy))
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index);
	}

	generic <typename R0>
	bool CAsyncServiceHandler::ProcessR1(short sRequestID, [Out]R0 %r0)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			if(!SendRequest(sRequestID, m_Copy))
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index, r0);
	}

	generic <typename R0, typename R1>
	bool CAsyncServiceHandler::ProcessR2(short sRequestID, [Out]R0 %r0, [Out]R1 %r1)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			bool b = SendRequest(sRequestID, m_Copy);
			if(!b)
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index, r0, r1);
	}

	generic <typename R0, typename R1, typename R2>
	bool CAsyncServiceHandler::ProcessR3(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			bool b = SendRequest(sRequestID, m_Copy);
			if(!b)
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index, r0, r1, r2);
	}

	generic <typename R0, typename R1, typename R2, typename R3>
	bool CAsyncServiceHandler::ProcessR4(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			bool b = SendRequest(sRequestID, m_Copy);
			if(!b)
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
	}

	generic <typename R0, typename R1, typename R2, typename R3, typename R4>
	bool CAsyncServiceHandler::ProcessR5(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
	{
		unsigned __int64 index;
		{
			CAutoLock al(&m_pCS->m_sec);
			bool b = SendRequest(sRequestID, m_Copy);
			if(!b)
				return false;
			index = m_nSyncIndex;
		}
		return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
	}

	bool CAsyncServiceHandler::BeginBatching()
	{
		return GetAttachedClientSocket()->BeginBatching();
	}

	bool CAsyncServiceHandler::CommitBatch(bool bServerBatch)
	{
		return GetAttachedClientSocket()->Commit(bServerBatch);
	}

	bool CAsyncServiceHandler::CommitBatch()
	{
		return CommitBatch(false);
	}

	bool CAsyncServiceHandler::RollbackBatch()
	{
		return GetAttachedClientSocket()->Rollback();
	}

	bool CAsyncServiceHandler::Wait(short sRequestId)
	{
		return Wait(sRequestId, -1);
	}

	bool CAsyncServiceHandler::Wait()
	{
		return Wait(0, -1);
	}

	bool CAsyncServiceHandler::WaitAll(int lTimeout)
	{
		return GetAttachedClientSocket()->WaitAll(lTimeout);
	}

	bool CAsyncServiceHandler::WaitAll()
	{
		return WaitAll(-1);
	}

	bool CAsyncServiceHandler::Wait(short sRequestId, int lTimeout)
	{
		if(sRequestId == 0)
		{
			CComVariant vtReq;
			CComQIPtr<IUSocket> pIUSocket = (IUnknown*)GetAttachedClientSocket()->GetIUSocket().ToPointer();
			pIUSocket->GetRequestsInQueue(&vtReq);
			if((vtReq.vt == (VT_ARRAY|VT_UI2) || vtReq.vt == (VT_ARRAY|VT_I2)) && vtReq.parray->cbElements > 0)
			{
				short *p;
				::SafeArrayAccessData(vtReq.parray, (void**)&p);
				sRequestId = p[vtReq.parray->cbElements -1];
				::SafeArrayUnaccessData(vtReq.parray);
			}
		}
		if(sRequestId == 0)
			return true;
		return GetAttachedClientSocket()->Wait(sRequestId, (unsigned long)lTimeout, GetSvsID());
	}

	void CAsyncServiceHandler::Copy(CAsyncResult ^ar)
	{
		/*CInternalUQueue *p = ar->UQueue->GetInternalUQueue();
		CInternalUQueue *pSync = m_qSync->GetInternalUQueue();
		pSync->Push(p->GetBuffer(), p->GetSize());
		p->SetSize(0);*/
	}

	bool CAsyncServiceHandler::Attach(CClientSocket ^pClientSocket)
	{
		if(m_pClientSocket != nullptr)
			Detach();
		if(pClientSocket)
		{
			CAsyncServiceHandler ^p = pClientSocket->Lookup(GetSvsID());
			
			//One ClientSocket can be attached with ONLY ONE CAsyncServiceHandler 
			//for a given service id and a socket connection.
			ATLASSERT(p == nullptr);
			if(p == nullptr)
			{
				CAutoLock AutoLock(&pClientSocket->m_pCS->m_sec);
				pClientSocket->m_lstAsynHandler->Add(this);
				CAutoLock al(&m_pCS->m_sec);
				m_pClientSocket = pClientSocket;
				return true;
			}
			return false;
		}
		return false;
	}

	int CAsyncServiceHandler::GetSvsID()
	{
		return m_nSvsId;
	}

	void CAsyncServiceHandler::NotifyRequestProcessed(int len, short sReqId)
	{
		if(m_pClientSocket && m_pClientSocket->m_OnRequestProcessed)
		{		
			m_pClientSocket->m_OnRequestProcessed->Invoke(m_pClientSocket->Socket, sReqId, len, len, USOCKETLib::tagReturnFlag::rfCompleted);
#ifdef _DEBUG	
			if(len != 0)
			{
				ATLTRACE(_T("Warning: %d byte(s) remained in queue and not processed!\n"), len);
			}
#endif
		}
	}

	CUQueue^ CAsyncServiceHandler::Look(unsigned __int64 index)
	{
		int n = 0;
		CAutoLock al(&m_pCS->m_sec);
		for each(CIndexQueue^ iq in m_mapSync)
		{
			if(iq->Index == index)
			{
				m_mapSync->RemoveAt(n);
				return iq->UQueue;
			}
			++n;
		}
		return nullptr;
	}

	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
			NotifyRequestProcessed(len, sReqId);
		CScopeUQueue::Unlock(p);
		return b;
	}

	generic <typename R0>
	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
		{
			if(len > 0)
				len -= p->Load(r0);
			NotifyRequestProcessed(len, sReqId);
		}
		CScopeUQueue::Unlock(p);
		return b;
	}

	generic <typename R0, typename R1>
	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
		{
			if(len > 0)
			{
				len -= p->Load(r0);
				len -= p->Load(r1);
			}
			NotifyRequestProcessed(len, sReqId);
		}
		CScopeUQueue::Unlock(p);
		return b;
	}

	generic <typename R0, typename R1, typename R2>
	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
		{
			if(len > 0)
			{
				len -= p->Load(r0);
				len -= p->Load(r1);
				len -= p->Load(r2);
			}
			NotifyRequestProcessed(len, sReqId);
		}
		CScopeUQueue::Unlock(p);
		return b;
	}

	generic <typename R0, typename R1, typename R2, typename R3>
	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
		{
			if(len > 0)
			{
				len -= p->Load(r0);
				len -= p->Load(r1);
				len -= p->Load(r2);
				len -= p->Load(r3);
			}
			NotifyRequestProcessed(len, sReqId);
		}
		CScopeUQueue::Unlock(p);
		return b;
	}

	generic <typename R0, typename R1, typename R2, typename R3, typename R4>
	bool CAsyncServiceHandler::PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
	{
		int len;
		bool b = WaitAll();
		CUQueue ^p = Look(index);
		if(p != nullptr)
			len = p->GetSize();
		else
			len = 0;
		if(b)
		{
			if(len > 0)
			{
				len -= p->Load(r0);
				len -= p->Load(r1);
				len -= p->Load(r2);
				len -= p->Load(r3);
				len -= p->Load(r4);
			}
			NotifyRequestProcessed(len, sReqId);
		}
		CScopeUQueue::Unlock(p);
		return b;
	}


	void CAsyncServiceHandler::Detach()
	{
		CAutoLock al(&m_pCS->m_sec);
		m_lstFunc->Clear();
		if(m_pClientSocket != nullptr)
		{
			CAutoLock AutoLock(&m_pClientSocket->m_pCS->m_sec);
			m_pClientSocket->m_lstAsynHandler->Remove(this);
			m_pClientSocket = nullptr;
		}
	}

	CAsyncServiceHandler::CPair^ CAsyncServiceHandler::GetAsyncResultHandler(short sReqId)
	{
		CAutoLock al(&m_pCS->m_sec);
		bool b = (m_pClientSocket->ReturnRandom);
		for each(CPair^ p in m_lstFunc)
		{
			if(p->RequestId == sReqId)
			{
				m_lstFunc->Remove(p);
				return p;
			}
			if(!b)
				break;
		}
		return nullptr;
	}

	unsigned __int64 CAsyncServiceHandler::OnE(short sRequestId, CSocketProServerException ^Exception)
	{
		IAsyncResultsHandler ^pIAsyncResultsHandler = AsyncResultsHandler;
		if(pIAsyncResultsHandler != nullptr)
			pIAsyncResultsHandler->OnExceptionFromServer(this, Exception);
		else
			OnExceptionFromServer(Exception);
		CPair ^p = GetAsyncResultHandler(sRequestId);
		if(p == nullptr)
			return 0;
		return p->m_nIndex;
	}

	unsigned long  CAsyncServiceHandler::RemoveAsyncHandlers(unsigned int nCancel)
	{
		int nTotal;
		if(nCancel == 0)
			return 0;
		unsigned long ulCount = 0;
		m_pCS->Lock();
		nTotal = m_lstFunc->Count;
		if(nCancel >= (unsigned int)nTotal)
		{
			m_lstFunc->Clear();
			ulCount = (unsigned int)nTotal;
		}
		else if(nCancel > 0)
		{
			int nIndex;
			for(nIndex = nTotal - 1; nIndex >= 0; --nIndex)
			{
				m_lstFunc->RemoveAt(nIndex);
				ulCount++;
				if(ulCount >= nCancel)
					break;
			}
		}
		m_pCS->Unlock();
		return ulCount;
	}

	unsigned __int64 CAsyncServiceHandler::OnRR(int hSocket, short nRequestID, int lLen, USOCKETLib::tagReturnFlag sFlag)
	{
		CUQueue ^UQueue = CScopeUQueue::Lock();
		CInternalUQueue *q = UQueue->GetInternalUQueue();
		unsigned long ulSize = (unsigned long)lLen;
		if(ulSize > (unsigned long)q->GetMaxSize())
			q->ReallocBuffer((ulSize/256 + 1) * 256);
		if(ulSize > 0)
		{
			HRESULT hr;
			unsigned long ulGet;
			IUFast *pIUFast = (IUFast *)((void*)(m_pClientSocket->GetIUFast()));
			hr = pIUFast->GetRtnBufferEx(ulSize, (BYTE*)q->GetBuffer(), &ulGet);
			q->SetSize(ulSize);
			ATLASSERT(ulGet == ulSize);
			ATLASSERT(hr == S_OK);
		}
		q->SetSize(ulSize);
		unsigned __int64 index = 0;
		CSocketProServerException ^err = nullptr;
		if(m_pClientSocket->AutoTransferServerException) 
		{
			/* ###### ENHANCEMENT FROM SOCKETPRO VERSION 4.6.0.1 ###### */
			//All request methods must return HRESULT at least!!!!!!
			//If this assert fails here, it means that returned result does not contain HRESULT
			ATLASSERT(q->GetSize() >= sizeof(long));
			UQueue->Pop(err);
			if(err != nullptr && err->HResult != S_OK)
				index = OnE(nRequestID, err);
#ifdef _DEBUG
			if(err->HResult != S_OK)
			{
				ATLASSERT(q->GetSize() == 0);
			}
#endif
		}

		//once an handler is found, call its following virtual function
		//for processing returned results
		if(err == nullptr || err->HResult == S_OK)
			index = OnRR(nRequestID, UQueue);
		if(index == 0 && m_pClientSocket->m_OnRequestProcessed != nullptr)
		{
			m_pClientSocket->m_OnRequestProcessed->Invoke(hSocket, nRequestID, q->GetSize(), q->GetSize(), sFlag);
		}
		if(index == 0 && q->GetSize() != 0)
		{
			ATLTRACE(_T("Warning: %d byte(s) remained in queue and not processed!\n"), q->GetSize());
			q->SetSize(0);
		}
		if(index > 0)
		{
			CIndexQueue ^iq = gcnew CIndexQueue();
			iq->Index = index;
			iq->UQueue = UQueue;
			m_pCS->Lock();
			m_mapSync->Add(iq);
			m_pCS->Unlock();
		}
		else
			CScopeUQueue::Unlock(UQueue);
		return index;
	}

	unsigned __int64 CAsyncServiceHandler::OnRR(short sRequestID, CUQueue ^UQueue)
	{
		do
		{
			CPair ^p = GetAsyncResultHandler(sRequestID);
			if(p != nullptr && p->m_arh != nullptr)
			{
				CAsyncResult ar(this, sRequestID, UQueue);
				ar.CurrentAsyncResultHandler = p->m_arh;
				p->m_arh->Invoke(%ar);
				return p->m_nIndex;
				break;
			}

			IAsyncResultsHandler ^pIAsyncResultsHandler = AsyncResultsHandler;
			if(pIAsyncResultsHandler != nullptr)
			{
				CAsyncResult ar(this, sRequestID, UQueue);
				ar.CurrentAsyncResultHandler = gcnew DAsyncResultHandler(pIAsyncResultsHandler, &IAsyncResultsHandler::Process);
				pIAsyncResultsHandler->Process(%ar);
				break;
			}
			OnResultReturned(sRequestID, UQueue);
		}while(false);
		return 0;
	}
}
}