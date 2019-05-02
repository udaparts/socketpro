// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com

#include "stdafx.h"
#ifndef _WIN32_DCOM
	#define _WIN32_DCOM
#endif
#include <objbase.h>
#include "sprowrap.h"
#include <stdio.h>

//#include <iostream>
//using namespace std;

#pragma warning(disable: 4996) // warning C4996: 'swprintf': swprintf has been changed to conform with the ISO C standard, adding an extra character count parameter. To use traditional Microsoft swprintf, set _CRT_NON_CONFORMING_SWPRINTFS.


namespace SocketProAdapter
{
	CComAutoCriticalSection	g_cs;

	CSimpleArray<CUQueue*> CScopeUQueue::m_aUQueue;
	CSimpleArray<CUQueue*> CScopeUQueue::m_aLargeUQueue;
	CComAutoCriticalSection	CScopeUQueue::m_cs;
	unsigned long CScopeUQueue::m_ulLargeSize = 10*1024*1024; //10 mega bytes

	CScopeUQueue::CScopeUQueue(bool bLarge) : m_pUQueue(Lock(bLarge)), m_bLarge(bLarge)
	{
		m_pUQueue->m_bScope = true;
	}

	CScopeUQueue::~CScopeUQueue()
	{
		m_pUQueue->m_bScope = false;
		Unlock(m_pUQueue, m_bLarge);
	}

	CUQueue* CScopeUQueue::Lock()
	{
		return Lock(false);
	}

	void CScopeUQueue::Unlock(CUQueue *pUQueue)
	{
		Unlock(pUQueue, false);
	}

	CUQueue* CScopeUQueue::Lock(bool bLarge)
	{
		int nSize;
		CUQueue *p = UNULL_PTR;
		m_cs.Lock();
		if(bLarge)
		{
			nSize = m_aLargeUQueue.GetSize();
			if(nSize != 0)
			{
				--nSize;
				p = m_aLargeUQueue[nSize];
				m_aLargeUQueue.RemoveAt(nSize);
			}
			m_cs.Unlock();
		}
		else
		{
			nSize = m_aUQueue.GetSize();
			if(nSize != 0)
			{
				--nSize;
				p = m_aUQueue[nSize];
				m_aUQueue.RemoveAt(nSize);
			}
			m_cs.Unlock();
		}
		if(p)
			return p;
		return new CUQueue();
	}

	void CScopeUQueue::DestroyUQueuePool()
	{
		int n;
		CUQueue *p;
		CAutoLock AutoLock(&m_cs.m_sec);
		for(n=0; n<m_aUQueue.m_nSize; n++)
		{
			p = m_aUQueue[n];
			delete p;
		}
		m_aUQueue.RemoveAll();
		for(n=0; n<m_aLargeUQueue.m_nSize; n++)
		{
			p = m_aLargeUQueue[n];
			delete p;
		}
		m_aLargeUQueue.RemoveAll();
	}

	__int64 CScopeUQueue::GetMemoryConsumed()
	{
		int n;
		CUQueue *p;
		__int64 lSize = 0;
		CAutoLock AutoLock(&m_cs.m_sec);
		for(n=0; n<m_aUQueue.m_nSize; n++)
		{
			p = m_aUQueue[n];
			lSize += p->GetMaxSize();
		}
		return lSize;
	}

	void CScopeUQueue::CleanUQueuePool()
	{
		int n;
		CUQueue *p;
		CAutoLock AutoLock(&m_cs.m_sec);
		for(n=0; n<m_aUQueue.m_nSize; n++)
		{
			p = m_aUQueue[n];
			p->CleanTrack();
		}
		for(n=0; n<m_aLargeUQueue.m_nSize; n++)
		{
			p = m_aLargeUQueue[n];
			p->CleanTrack();
		}
	}

	void CScopeUQueue::Unlock(CUQueue *pUQueue, bool bLarge)
	{
		if(pUQueue == UNULL_PTR)
			return;

		//don't unlock the internal m_pUQueue of CScopeUQueue!!!
		ATLASSERT(!pUQueue->m_bScope);

		pUQueue->SetSize(0);
		m_cs.Lock();
		if(bLarge)
			 m_aLargeUQueue.Add(pUQueue);
		else
			m_aUQueue.Add(pUQueue);
		m_cs.Unlock();
	}

	CUQueue& operator << (CUQueue &UQueue, const CSocketProServerException &err )
	{
		UQueue << err.m_hr;
		if(err.m_hr != S_OK)
		{
			UQueue << err.m_ulSvsID;
			UQueue << err.m_usRequestID;
			UQueue << err.m_strMessage;
		}
		return UQueue;
	}
	
	CUQueue& operator >> (CUQueue &UQueue, CSocketProServerException &err )
	{		
		UQueue >> err.m_hr;
		if(err.m_hr != S_OK)
		{
			UQueue >> err.m_ulSvsID;
			UQueue >> err.m_usRequestID;
			UQueue >> err.m_strMessage;
		}
		return UQueue;
	}

	CUPerformanceQuery::CUPerformanceQuery()
	{
		::QueryPerformanceFrequency((LARGE_INTEGER*)&m_liFreq);
		::memset(&m_liCount, 0, sizeof(__int64));
	}

	__int64 CUPerformanceQuery::Now()
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_liCount);
		return m_liCount;
	}
	
	__int64 CUPerformanceQuery::Diff(__int64 liNow, __int64 liOld)
	{
		return (liNow - liOld) * 1000000 / m_liFreq;
	}

	__int64 CUPerformanceQuery::Diff(__int64 liOldCount)
	{
		Now();
		return Diff(m_liCount, liOldCount);
	}

	void IUPush::CreateVtGroups(unsigned long *pGroups, unsigned long ulCount, VARIANT &vtGroups)
	{
		unsigned long *p;
		SAFEARRAYBOUND sab[1] = {ulCount, 0};
		vtGroups.vt = (VT_ARRAY|VT_UI4);
		vtGroups.parray = ::SafeArrayCreate(VT_UI4, 1, sab);
		::SafeArrayAccessData(vtGroups.parray, (void**)&p);
		::memcpy(p, pGroups, ulCount*sizeof(unsigned long));
		::SafeArrayUnaccessData(vtGroups.parray);
	}

	namespace ClientSide
	{
		CClientSocket::CClientSocket(bool bCreateOne) 
			: m_ulCurSvsID(0), m_pIJobManager(UNULL_PTR), m_pIJobContext(UNULL_PTR), m_pIExtenalSocketEvent(UNULL_PTR), m_ulBatchBalance(0)
		{
			m_Push.m_pClientSocket = this;
			if(bCreateOne)
			{
				Init();
#ifdef _WIN32_WCE
				//make sure you have registerred usocket.dll properly on your devices!
				//you can use the visual tool regsvr.zip at http://www.udaparts.com/document/articles/regsvr.zip
				ATLASSERT(m_pIUSocket != UNULL_PTR);
#elif defined(_WIN64) || defined(_W64) || defined(WIN64) || defined(x64) || defined(_M_IA64) || defined(_M_AMD64) || defined(_M_ALPHA64) || defined(_M_IA64)
				//make sure you have registerred 64bit npUsocket.dll properly!
				//Note you can register both 32bit and 64bit versions of npUSocket.dll 
				//on one machine at the same time with the command regsvr32 npUSocket.dll
				ATLASSERT(m_pIUSocket != UNULL_PTR);		
#else //32 bit
				//make sure you have registerred 32bit npUsocket.dll properly!
				//Note you can register both 32bit and 64bit versions of npUSocket.dll 
				//on one machine at the same time with the command regsvr32 npUSocket.dll
				ATLASSERT(m_pIUSocket != UNULL_PTR);	
#endif
			}
			if(m_pIUSocket != UNULL_PTR)
			{
				Advise();
			}
		}
		
		CClientSocket::CClientSocket(IUnknown *pIUnknownToUSocket) 
			: m_ulCurSvsID(0), m_pIJobManager(UNULL_PTR), m_pIJobContext(UNULL_PTR), m_pIExtenalSocketEvent(UNULL_PTR), m_ulBatchBalance(0)
		{
			m_Push.m_pClientSocket = this;
			InitEx(pIUnknownToUSocket);
			if(pIUnknownToUSocket != UNULL_PTR)
			{
				Advise();
			}
		}

		bool CClientSocket::CUPushClientImpl::Broadcast(const unsigned char *pMessage, unsigned long ulMessageSize, unsigned long *pGroups, unsigned long ulCount)
		{
			return (m_pClientSocket->GetIUFast()->XSpeakEx(ulMessageSize, (unsigned char*)pMessage, ulCount, pGroups) == S_OK);
		}

		bool CClientSocket::CUPushClientImpl::Broadcast(const VARIANT &vtMessage, unsigned long *pGroups, unsigned long ulCount)
		{
			if(pGroups == UNULL_PTR || ulCount == 0)
				return false;
			CComVariant vtGroups;
			CreateVtGroups(pGroups, ulCount, vtGroups);
			return (m_pClientSocket->GetIUChat()->XSpeak(vtMessage, vtGroups) == S_OK);
		}

		bool CClientSocket::CUPushClientImpl::Enter(unsigned long *pGroups, unsigned long ulCount)
		{
			if(pGroups == UNULL_PTR || ulCount == 0)
				return Exit();
			CComVariant vtGroups;
			CreateVtGroups(pGroups, ulCount, vtGroups);
			return (m_pClientSocket->GetIUChat()->XEnter(vtGroups) == S_OK);
		}
		
		bool CClientSocket::CUPushClientImpl::Exit()
		{
			return (m_pClientSocket->GetIUChat()->Exit() == S_OK);
		}
		
		bool CClientSocket::CUPushClientImpl::SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId)
		{
			return (m_pClientSocket->GetIUChat()->SendUserMessage(CComBSTR(strUserId), vtMessage) == S_OK);
		}

		bool CClientSocket::CUPushClientImpl::SendUserMessage(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulMessageSize)
		{
			return (m_pClientSocket->GetIUFast()->SendUserMessageEx(CComBSTR(strUserID), ulMessageSize, (unsigned char*)pMessage) == S_OK);
		}

		void CAsyncServiceHandler::Copy(CAsyncResult &ar)
		{

		}

		CClientSocket::~CClientSocket()
		{
			DetachAll();
			Uninit();
			CAutoLock AutoLock(&m_cs.m_sec);
			if(m_pIJobManager != UNULL_PTR && m_pIJobContext != UNULL_PTR && m_pIJobContext->GetJobStatus() == jsCreating)
			{
				m_pIJobManager->DestroyJob(m_pIJobContext);
			}
		}

		CClientSocket& CClientSocket::operator=(IUnknown *pIUnknownToUSocket)
		{
			m_ulCurSvsID = 0;
			Uninit();
			if(pIUnknownToUSocket != UNULL_PTR)
			{
				InitEx(pIUnknownToUSocket);
				Advise();
			}
			return *this;
		}

		void CClientSocket::Advise()
		{
			if(m_dwEventCookie == 0xFEFEFEFE)
			{
				ATLASSERT(m_pIUSocket != UNULL_PTR);
				if(m_pIUSocket != UNULL_PTR)
				{
					HRESULT hr = CCSEvent::DispEventAdvise(m_pIUSocket.p);
					ATLASSERT(hr == S_OK);
				}
			}
		}

		void CClientSocket::Disadvise()
		{
			if(m_dwEventCookie != 0xFEFEFEFE)
			{
				ATLASSERT(m_pIUSocket != UNULL_PTR);
				if(m_pIUSocket != UNULL_PTR)
				{
					HRESULT hr = CCSEvent::DispEventUnadvise(m_pIUSocket.p);
					ATLASSERT(hr == S_OK);
				}
			}
		}

		void CClientSocket::DetachAll()
		{
			CAsyncServiceHandler *p;
			CAutoLock AutoLock(&m_cs.m_sec);
			int nSize = m_mapSvsIDHandler.GetSize();
			while(nSize > 0)
			{
				p = m_mapSvsIDHandler.GetValueAt(0);
				ATLASSERT(p != UNULL_PTR);
				p->Attach(UNULL_PTR);
				nSize--;
			}
		}

		void CClientSocket::InitEx(IUnknown *pIUnknown)
		{
			if(pIUnknown)
			{
				HRESULT hr = pIUnknown->QueryInterface(__uuidof(IUSocket), (void**)&m_pIUSocket);
				
				//an interface to a valid USocket object expected
				ATLASSERT(hr == S_OK && m_pIUSocket.p != UNULL_PTR);
				if(hr == S_OK)
				{
					long lSvsID = 0;
					hr = m_pIUSocket->get_CurrentSvsID(&lSvsID);
					m_ulCurSvsID = (unsigned long)lSvsID;

					
					ULONG *pulParameters;
					CComVariant vtServerParameters;
					hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
					::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
					m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
					::SafeArrayUnaccessData(vtServerParameters.parray);
					
#ifdef _DEBUG
					//1000 second. 
					//If too short, client will automatically disconnect socket connection when you debug at server side.
					hr = m_pIUSocket->put_RecvTimeout(1000000);
#endif
					hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&m_pIUFast);

					hr = m_pIUSocket->QueryInterface(__uuidof(IUChat), (void**)&m_pIUChat);
					ATLASSERT(m_pIUChat.p != UNULL_PTR);
				}
			}
		}

		void CClientSocket::Init()
		{
			HRESULT hr = m_pIUSocket.CoCreateInstance(__uuidof(USocket));
			
			//make sure either CoInitialize or CoInitializeEx called
			//make sure usocket.dll registered properly
			ATLASSERT(hr == S_OK);

			if(hr == S_OK)
			{
				long lSvsID = 0;
				hr = m_pIUSocket->get_CurrentSvsID(&lSvsID);
				m_ulCurSvsID = (unsigned long)lSvsID;

				ULONG *pulParameters;
				CComVariant vtServerParameters;
				hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
				::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
				m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
				::SafeArrayUnaccessData(vtServerParameters.parray);

#ifdef _DEBUG
				//1000 second. 
				//If too short, client will automatically disconnect socket connection when you debug at server side.
				hr = m_pIUSocket->put_RecvTimeout(1000000);
#endif

				hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&m_pIUFast);

				hr = m_pIUSocket->QueryInterface(__uuidof(IUChat), (void**)&m_pIUChat);
				ATLASSERT(m_pIUChat.p != UNULL_PTR);
			}
		}
		void CClientSocket::Uninit()
		{
			m_ulCurSvsID = 0;
			Disadvise();
			if(m_pIUChat.p != UNULL_PTR)
			{
				m_pIUChat.Release();
			}
			if(m_pIUFast.p != UNULL_PTR)
			{
				m_pIUFast.Release();
			}
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket.Release();
			}
		}

		IUPush* CClientSocket::GetPush()
		{
			return &m_Push;
		}

		bool CClientSocket::IsConnected()
		{
			if(m_pIUSocket != UNULL_PTR)
			{
				long hSocket = 0;
				m_pIUSocket->get_Socket(&hSocket);
				return (hSocket != 0 && hSocket != -1);
			}
			return false;
		}

		void CClientSocket::SetUSocket(IUnknown *pIUnknownToUSocket)
		{
			CAutoLock al(&m_cs.m_sec);
			//Make sure BeginBatching and Commit/Rollback are balanced before calling this method.
			//Otherwise, there is bug in code.
			ATLASSERT(m_ulBatchBalance == 0);

			m_ulCurSvsID = 0;
			Uninit();
			if(pIUnknownToUSocket != UNULL_PTR)
			{
				InitEx(pIUnknownToUSocket);
				Advise();
			}
		}
		
		bool CClientSocket::Commit(bool bBatchingAtServer)
		{
			{
				CAutoLock al(&m_cs.m_sec);
				m_ulBatchBalance--;
				if(m_ulBatchBalance > 0)
				{
					//we return here because the methods BeginBatching and Commit/Rollback not balanced yet
					return true;
				}
			}

			if(m_pIJobManager != UNULL_PTR)
			{
				CAutoLock al(&m_cs.m_sec);
				if(m_pIJobContext == UNULL_PTR)
					return false;
				if(m_pIJobContext->GetJobStatus() != jsCreating)
					return false;
				CSimpleMap<int, CTaskContext> aTasks;
				m_pIJobContext->GetTasks(aTasks);
				if(IsBatchingBalanced(aTasks))
					return false;
				VARIANT_BOOL vb = bBatchingAtServer ? VARIANT_TRUE : VARIANT_FALSE;
				return (m_pIJobContext->AddTask(idCommitBatching, (BYTE*)&vb, sizeof(vb)) != 0);
			}
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				return (m_pIUSocket->CommitBatching(bBatchingAtServer ? VARIANT_TRUE : VARIANT_FALSE) == S_OK);
			}
			return false;
		}

		bool CClientSocket::Rollback()
		{
			//you can call the method only after calling the method BeginBatching one time
			ATLASSERT(m_ulBatchBalance == 1);
			{
				CAsyncServiceHandler *p;
				CAutoLock al(&m_cs.m_sec);
				m_ulBatchBalance--;
				if(m_ulBatchBalance)
					return false;
#ifdef	_DEBUG
	#ifdef _WIN32_WCE
				ATLTRACE(L"Call the method Rollback carefully, which may lead to 'lost' expected async results.\r\n");
				ATLTRACE(L"'Lost' async results can be tracked from CAsyncServiceHandler::OnResultReturned or m_pIAsyncResultsHandler if it is set.\r\n");
	#else
				ATLTRACE("Call the method Rollback carefully, which may lead to 'lost' expected async results.\r\n");
				ATLTRACE("'Lost' async results can be tracked from CAsyncServiceHandler::OnResultReturned or m_pIAsyncResultsHandler if it is set.\r\n");
	#endif
#endif
				int n, nSize = m_mapSvsIDHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					p = m_mapSvsIDHandler.GetValueAt(n);
					p->RemoveAsyncHandlers();
				}
			}
			if(m_pIJobManager != UNULL_PTR)
			{
				int n;
				CAutoLock al(&m_cs.m_sec);
				if(m_pIJobContext == UNULL_PTR)
					return false;
				if(m_pIJobContext->GetJobStatus() != jsCreating)
					return false;
				CSimpleMap<int, CTaskContext> aTasks;
				m_pIJobContext->GetTasks(aTasks);
				if(IsBatchingBalanced(aTasks))
					return true;
				int nSize = aTasks.GetSize();
				for(n=nSize-1; n>=0; n--)
				{
					int idTask = aTasks.GetKeyAt(n);
					CTaskContext &tc = aTasks.GetValueAt(n);
					m_pIJobContext->RemoveTask(idTask);
					if(tc.m_usRequestId == idStartBatching)
						break;
				}
				return true;
			}
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				return (m_pIUSocket->AbortBatching() == S_OK);
			}
			return false;
		}

		bool CClientSocket::StartJob()
		{
			if(m_pIJobManager != UNULL_PTR)
			{
				CAutoLock al(&m_cs.m_sec);
				if(m_pIJobContext != UNULL_PTR && m_pIJobContext->GetJobStatus() == jsCreating)
				{
					//job context not enqueued or destroyed yet
					return false;
				}
				int nIndex = m_mapSvsIDHandler.FindKey(m_ulCurSvsID);
				CAsyncServiceHandler *p = UNULL_PTR;
				if(nIndex != -1)
					p = m_mapSvsIDHandler.GetValueAt(nIndex);
				m_pIJobContext = m_pIJobManager->CreateJob(p);
				return (m_pIJobContext != UNULL_PTR);
			}
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				return (m_pIUSocket->StartJob() == S_OK);
			}
			return false;
		}

		IJobContext* CClientSocket::GetCurrentJobContext()
		{
			CAutoLock al(&m_cs.m_sec);
			return m_pIJobContext;
		}

		IJobManager* CClientSocket::GetJobManager()
		{
			return m_pIJobManager;
		}

		bool CClientSocket::EndJob()
		{
			if(m_pIJobManager != UNULL_PTR)
			{
				CAutoLock al(&m_cs.m_sec);
				if(m_pIJobContext== UNULL_PTR)
					return false;
				CSimpleMap<int, CTaskContext> tasks;
				m_pIJobContext->GetTasks(tasks);
				if(!IsBatchingBalanced(tasks))
				{
					//Batching not balanced!!!
					return false;
				}
				if(m_pIJobContext->GetJobStatus() != jsCreating || !m_pIJobManager->EnqueueJob(m_pIJobContext))
				{
					//job context has already been enqueued or destroyed.
					return false;
				}
				m_pIExtenalSocketEvent->Process();
				return true;
			}
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				return (m_pIUSocket->EndJob() == S_OK);
			}
			return false;
		}

		bool CClientSocket::IsBatchingBalanced(const CSimpleMap<int, CTaskContext> &aTasks)
		{
			int n;
			bool bBalanced = true;
			int nSize = aTasks.GetSize();
			for(n=0; n<nSize; n++)
			{
				const CTaskContext &tc = aTasks.GetValueAt(n);
				if(tc.m_usRequestId == idStartBatching)
					bBalanced = false;
				else if(tc.m_usRequestId == idCommitBatching)
					bBalanced = true;
			}
			return bBalanced;
		}

		bool CClientSocket::BeginBatching()
		{
			{
				CAutoLock al(&m_cs.m_sec);
				m_ulBatchBalance++;
				if(m_ulBatchBalance > 1)
					return true;
			}
			if(m_pIJobManager != UNULL_PTR)
			{
				CAutoLock al(&m_cs.m_sec);
				if(m_pIJobContext == UNULL_PTR)
					return false;
				if(m_pIJobContext->GetJobStatus() != jsCreating)
					return false;
				CSimpleMap<int, CTaskContext> aTasks;
				m_pIJobContext->GetTasks(aTasks);
				if(!IsBatchingBalanced(aTasks))
					return false;
				return (m_pIJobContext->AddTask(idStartBatching, UNULL_PTR, 0) != 0);
			}
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				CAutoLock al(&m_cs.m_sec);
				return (m_pIUSocket->StartBatching() == S_OK);
			}
			return false;
		}

		unsigned short CClientSocket::GetServerPingTime()
		{
			unsigned short usServerPingTime = 0;
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				ULONG *pulParameters;
				CComVariant vtServerParameters;
				HRESULT hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
				if(hr == S_OK)
				{
					::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
					usServerPingTime = (unsigned short)pulParameters[1];
					::SafeArrayUnaccessData(vtServerParameters.parray);
				}
			}
			return usServerPingTime;
		}

		bool CClientSocket::IsBatching()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				VARIANT_BOOL bBatching;
				m_pIUSocket->get_IsBatching(&bBatching);
				return (!(bBatching == VARIANT_FALSE));
			}
			return false;
		}
		
		bool CClientSocket::SwitchTo(unsigned long ulSvsID, bool bAutoTransferServerException)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				Advise();
				HRESULT hr;
				ULONG *pulBuffer;
				ULONG pulParameters[3];
				CComVariant vtParameters;
				hr = m_pIUSocket->get_ClientParams(&vtParameters);
				::SafeArrayAccessData(vtParameters.parray, (void**)&pulBuffer);
				memcpy(pulParameters, pulBuffer + 2, sizeof(pulParameters));
				::SafeArrayUnaccessData(vtParameters.parray);
				if(bAutoTransferServerException)
				{
					pulParameters[2] |= TRANSFER_SERVER_EXCEPTION;
				}
				else
				{
					pulParameters[2] &= (~TRANSFER_SERVER_EXCEPTION);
				}
				vtParameters.Clear();
				SAFEARRAYBOUND sab[1] = {3, 0};
				vtParameters.vt = (VT_ARRAY | VT_I4);
				vtParameters.parray = ::SafeArrayCreate(VT_I4, 1, sab);
				::SafeArrayAccessData(vtParameters.parray, (void**)&pulBuffer);
				memcpy(pulBuffer, pulParameters, sizeof(pulParameters));
				::SafeArrayUnaccessData(vtParameters.parray);
				hr = m_pIUSocket->put_ClientParams(vtParameters);
			
				hr = m_pIUSocket->SwitchTo((long)ulSvsID);

				bool bOk = (hr == S_OK);
				
				//clean password right after calling IUSocket::SwitchTo
				//so that there is no possibility for other codes to peek a password
				hr = m_pIUSocket->put_Password(UNULL_PTR); 

				return bOk;
			}
			return false;
		}

		bool CClientSocket::SwitchTo(CAsyncServiceHandler *pAsynHandler, bool bAutoTransferServerException)
		{
			ATLASSERT(pAsynHandler != UNULL_PTR);
			if(pAsynHandler)
			{
				return SwitchTo(pAsynHandler->GetSvsID(), bAutoTransferServerException);
			}
			return false;
		}

		void CClientSocket::CleanTrack()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->CleanTrack();
			}
		}

		void CClientSocket::Disconnect()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->Disconnect();
			}
		}

		void CClientSocket::Shutdown()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->Shutdown();
			}
		}

		bool CClientSocket::Connect(LPCTSTR strHost, unsigned long ulPort, bool bSyn)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				Advise();
				return (m_pIUSocket->Connect(CComBSTR(strHost), (long)ulPort, bSyn ? VARIANT_TRUE : VARIANT_FALSE) == S_OK);
			}
			return false;
		}
	
		void CClientSocket::DisableUI(bool bDisable)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->put_Frozen(bDisable ? VARIANT_TRUE : VARIANT_FALSE);
			}
		}

		void CClientSocket::SetUID(LPCTSTR strUID)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->put_UserID(CComBSTR(strUID));
			}
		}

		void CClientSocket::SetPassword(LPCTSTR strPassword)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->put_Password(CComBSTR(strPassword));
			}
		}

		bool CClientSocket::Wait(unsigned short usRequestID, unsigned long ulTimeout, unsigned long ulSvsID)
		{
#ifdef _DEBUG
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				//Call this method only after methods BeginBatching and Rollback/Commit are balanced!
				ATLASSERT(m_ulBatchBalance == 0);
			}
#endif
			ATLASSERT(!IsBatching());

			VARIANT_BOOL bTimeout = VARIANT_FALSE;
			if(ulSvsID == 0)
				ulSvsID = GetCurrentServiceID();
			HRESULT hr = m_pIUSocket->Wait((short)usRequestID, (long)ulTimeout, (long)ulSvsID, &bTimeout);
			bool bWait = (hr == S_OK && bTimeout == VARIANT_FALSE);
			return bWait;
		}

		bool CClientSocket::WaitAll(unsigned long ulTimeout)
		{
#ifdef _DEBUG
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				//Call this method only after methods BeginBatching and Rollback/Commit are balanced!
				ATLASSERT(m_ulBatchBalance == 0);
			}
#endif
			ATLASSERT(!IsBatching());

			VARIANT_BOOL bTimeout = VARIANT_FALSE;
			HRESULT hr = m_pIUSocket->WaitAll((long)ulTimeout, &bTimeout);
			bool bWait = (hr == S_OK && bTimeout == VARIANT_FALSE && IsConnected());
			return bWait;
		}

		HRESULT CClientSocket::GetErrorCode()
		{
			long lRtn = S_OK;
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->get_Rtn(&lRtn);
			}
			return lRtn;
		}

		CComBSTR CClientSocket::GetErrorMsg()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			CComBSTR bstrErrorMsg;
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->get_ErrorMsg(&bstrErrorMsg);
			}
			return bstrErrorMsg;
		}

		bool CClientSocket::AutoTransferServerException()
		{
			return m_bServerException;
		}

		unsigned long CClientSocket::GetBytesBatched()
		{
			long lBytes = 0;
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->get_BytesBatched(&lBytes);
			}
			return (unsigned long)lBytes;
		}

		unsigned long CClientSocket::GetCountOfRequestsInQueue()
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			long lCount = 0;
			if(m_pIUSocket.p != UNULL_PTR)
			{
				m_pIUSocket->get_CountOfRequestsInQueue(&lCount);
			}
			return (unsigned long)lCount;
		}
		
		void CClientSocket::Cancel(long lRequests)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(m_pIUSocket.p != UNULL_PTR)
			{
				do
				{
					CAsyncServiceHandler *p;
					HRESULT hr = m_pIUSocket->Cancel(lRequests);
					if(hr != S_OK)
						break;
					unsigned long ulRemoved = 0;
					unsigned long ulCancel = (unsigned long)lRequests;
					if(ulCancel == 0)
						break;
					//The following logic is right in most cases.
					//For example, it will be correct if you mostly call CClientSocket::Cancel(-1) by default.
					//It will be wrong because there is no way to track what types of callbacks you used before, if you call CClientSocket::Cancel(1)
					//However, the canceled requests must be caught by either CAsyncServiceHandler::OnResultReturned or CAsyncServiceHandler::m_pIAsyncResultsHandler if it is set.
					CAutoLock AutoLock(&m_cs.m_sec);
					int n, nSize = m_mapSvsIDHandler.GetSize();
					for(n=0; n<nSize; n++)
					{
						if(ulRemoved >= ulCancel)
							break;
						p = m_mapSvsIDHandler.GetValueAt(n);
						ulRemoved += p->RemoveAsyncHandlers();
					}
				}while(false);
			}
		}

		HRESULT CClientSocket::OnDataAvailable(long hSocket, long lBytes, long lError)
		{
			return S_OK;
		}

		void CClientSocket::OnAllRequestsProcessed(unsigned int hSocket, unsigned short usLastRequestID)
		{
			
		}
		
		HRESULT __stdcall CClientSocket::OnDataAvailableInternal(long hSocket, long lBytes, long lError)
		{
			return OnDataAvailable(hSocket, lBytes, lError);
		}

		HRESULT CClientSocket::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnOtherMessageInternal(long hSocket, long nMsg, long wParam, long lParam)
		{
			if(nMsg == msgAllRequestsProcessed) //Beginning from SocketPro version 4.8.1.12
				OnAllRequestsProcessed((unsigned int)hSocket, (unsigned short)wParam);
			if(m_pIExtenalSocketEvent != UNULL_PTR)
				m_pIExtenalSocketEvent->OnOtherMessage(hSocket, nMsg, wParam, lParam);
			return OnOtherMessage(hSocket, nMsg, wParam, lParam);
		}

		HRESULT CClientSocket::OnSocketClosed(long hSocket, long lError)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnSocketClosedInternal(long hSocket, long lError)
		{
			{
				CAsyncServiceHandler *p;
				CAutoLock AutoLock(&m_cs.m_sec);
				int n, nSize = m_mapSvsIDHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					p = m_mapSvsIDHandler.GetValueAt(n);
					ATLASSERT(p != UNULL_PTR);
					p->RemoveAsyncHandlers();
				}
			}
			if(m_pIExtenalSocketEvent != UNULL_PTR)
				m_pIExtenalSocketEvent->OnSocketClosed(hSocket, lError);
			return OnSocketClosed(hSocket, lError);
		}

		HRESULT CClientSocket::OnSocketConnected(long hSocket, long lError)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnSocketConnectedInternal(long hSocket, long lError)
		{
			if(m_pIExtenalSocketEvent != UNULL_PTR)
				m_pIExtenalSocketEvent->OnSocketConnected(hSocket, lError);
			return OnSocketConnected(hSocket, lError);
		}

		HRESULT CClientSocket::OnConnecting(long hSocket, long hWnd)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnConnectingInternal(long hSocket, long hWnd)
		{
			{
				CAsyncServiceHandler *p;
				CAutoLock AutoLock(&m_cs.m_sec);
				int n, nSize = m_mapSvsIDHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					p = m_mapSvsIDHandler.GetValueAt(n);
					ATLASSERT(p != UNULL_PTR);
					p->RemoveAsyncHandlers();
				}
			}
			return OnConnecting(hSocket, hWnd);
		}

		HRESULT CClientSocket::OnSendingData(long hSocket, long lError, long lSent)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnSendingDataInternal(long hSocket, long lError, long lSent)
		{
			return OnSendingData(hSocket, lError, lSent);
		}

		HRESULT CClientSocket::OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnGetHostByAddrInternal(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError)
		{
			return OnGetHostByAddr(lHandle, bstrHostName, bstrHostAlias, lError);
		}

		HRESULT CClientSocket::OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnGetHostByNameInternal(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError)
		{
			return OnGetHostByName(lHandle, bstrHostName, bstrAlias, bstrIPAddr, lError);
		}

		HRESULT CClientSocket::OnClosing(long hSocket, long hWnd)
		{
			return S_OK;
		}

		HRESULT __stdcall CClientSocket::OnClosingInternal(long hSocket, long hWnd)
		{
			if(m_pIExtenalSocketEvent != UNULL_PTR)
				m_pIExtenalSocketEvent->OnClosing(hSocket, hWnd);
			return OnClosing(hSocket, hWnd);
		}

		void CClientSocket::OnBaseRequestProcessed(unsigned short usBaseRequestID)
		{
			
		}

		HRESULT CClientSocket::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
		{
			return S_OK;
		}

		void CClientSocket::NotifyProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
		{
			if(m_pIExtenalSocketEvent != UNULL_PTR)
				m_pIExtenalSocketEvent->OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
			OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
		}

		HRESULT __stdcall CClientSocket::OnRequestProcessedInternal(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
		{
			ATLASSERT(m_pIUSocket.p != UNULL_PTR);
			if(sFlag != rfCompleted)
			{
				//don't process return result unless sFlag == rfCompleted
				return OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
			}
			if(nRequestID == idSwitchTo)
			{
				long lSvsID = 0;
				m_pIUSocket->get_CurrentSvsID(&lSvsID);
				m_ulCurSvsID = (unsigned long)lSvsID;
				ULONG *pulParameters;
				CComVariant vtServerParameters;
				HRESULT hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
				::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
				m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
				::SafeArrayUnaccessData(vtServerParameters.parray);
			}

			if(nRequestID >= 0 && nRequestID <= SOCKETPRO_MAX_BASE_REQUEST_ID) //46 -- idClose of UFile
			{
				OnBaseRequestProcessed((unsigned short)nRequestID);
				if(m_pIExtenalSocketEvent != UNULL_PTR)
					m_pIExtenalSocketEvent->OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
				return OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
			}
			CAsyncServiceHandler *p = UNULL_PTR;
			
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				int nFind = m_mapSvsIDHandler.FindKey(m_ulCurSvsID);
				if(nFind != -1)
				{
					p = m_mapSvsIDHandler.GetValueAt(nFind);
					ATLASSERT(p != UNULL_PTR);
				}
			}
			unsigned __int64 index = 0;
			if(p != UNULL_PTR)
			{
				ATLASSERT(lLen == lLenInBuffer);
				index = p->OnRR(hSocket, nRequestID, lLen, sFlag);
			}
			if(!index) //disable virtual function call if ProcessRx is used for sync request
				NotifyProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
			return S_OK;
		}
	} //ClientSide

#ifndef _WIN32_WCE
	namespace ServerSide
	{
		CSimpleValArray<CBaseService*> g_aService;
		CSocketProServer	*g_pSocketProServer = UNULL_PTR;
		CSocketProServerLoader	g_SocketProLoader;
		CComAutoCriticalSection	CSocketProServer::m_cs;
	
		CSocketProServer::CSocketProServer(int nParam) 
			: m_ulEvents(0), m_hThread(UNULL_PTR), m_nPort(0), m_unMaxBacklog(64)
		{
			m_hEvent = ::CreateEvent(UNULL_PTR, FALSE, FALSE, UNULL_PTR);
			if(!g_SocketProLoader.IsLoaded())
			{
				g_SocketProLoader.LoadSocketProServer();
			}
			
			if(g_pSocketProServer == UNULL_PTR)
			{
				g_pSocketProServer = this;
			}
			else
			{
				ATLASSERT(FALSE); //an application can create ONE instance CSocketProServer only
			}
			
			//make sure that the server core library usktpror.dll (32bit or 64bit) is accessable.
			ATLASSERT(g_SocketProLoader.IsLoaded());

			InitSocketProServer(nParam);
		}

		CSocketProServer::~CSocketProServer()
		{
			::Sleep(250);
			StopSocketProServer();
			UninitSocketProServer();
			g_pSocketProServer = UNULL_PTR;
			::CloseHandle(m_hEvent);
		}

		bool CSocketProServer::PushManager::AddAChatGroup(unsigned long ulGroupID, const wchar_t *strDescription)
		{
			return g_SocketProLoader.AddAChatGroup(ulGroupID, strDescription);
		}

		void CSocketProServer::AskForEvents(unsigned long ulEvents)
		{
			CAutoLock al(&m_cs.m_sec);
			m_ulEvents = 0;
			if((ulEvents & eOnChatRequestComing) > 0)
			{
				g_SocketProLoader.SetOnChatRequestComing(OnChatComing);
				m_ulEvents += eOnChatRequestComing;
			}
			if((ulEvents & eOnChatRequestCame) > 0)
			{
				g_SocketProLoader.SetOnChatRequestCame(OnChatCame);
				m_ulEvents += eOnChatRequestCame;
			}
			if((ulEvents & eOnAccept) > 0)
			{
				g_SocketProLoader.SetOnAccept(OnAccepted);
				m_ulEvents += eOnAccept;
			}
			if((ulEvents & eOnWinMessage) > 0)
			{
				g_SocketProLoader.SetOnWinMessage(OnMessage);
				m_ulEvents += eOnWinMessage;
			}
			if((ulEvents & eOnClose) > 0)
			{
				g_SocketProLoader.SetOnClose(OnDown);
				m_ulEvents += eOnClose;
			}
			if((ulEvents & eOnSwitchTo) > 0)
			{
				g_SocketProLoader.SetOnSwitchTo(OnSTo);
				m_ulEvents += eOnSwitchTo;
			}
			if((ulEvents & eOnIsPermitted) > 0)
			{
				g_SocketProLoader.SetOnIsPermitted(OnPermitted);
				m_ulEvents += eOnIsPermitted;
			}
			if((ulEvents & eOnSend) > 0)
			{
				g_SocketProLoader.SetOnSend(OnSnd);
				m_ulEvents += eOnSend;
			}
			if((ulEvents & eOnSSLEvent) > 0)
			{
				g_SocketProLoader.SetOnSSLEvent(OnSSL);
				m_ulEvents += eOnSSLEvent;
			}
		}

		DWORD WINAPI CSocketProServer::ThreadProc(LPVOID lpParameter)
		{
			ULONG ulError = 1;
			CoInitializeEx(UNULL_PTR, COINIT_MULTITHREADED);
			do
			{
				if(!g_SocketProLoader.StartSocketProServer(g_pSocketProServer->m_nPort, g_pSocketProServer->m_unMaxBacklog))
				{
					::SetEvent(g_pSocketProServer->m_hEvent);
					break;
				}
				g_pSocketProServer->StartMessagePump();
				g_SocketProLoader.StopSocketProServer();
				g_SocketProLoader.UninitSocketProServer();
				g_SocketProLoader.InitSocketProServer(0);
				ulError = 0;
			}while(false);
			CoUninitialize();
			if(ulError)
				ulError = ::GetLastError();
			return ulError;
		}

		bool CSocketProServer::Run(unsigned int nPort, unsigned long ulEvents, UINT unMaxBacklog)
		{
			bool b = true;
			{
				CAutoLock al(&m_cs.m_sec);
				if(m_hThread != UNULL_PTR)
				{
					//bad operation because SocketPro server already started.
					ATLASSERT(false);
					return false;
				}
				m_nPort = nPort;
				m_unMaxBacklog = unMaxBacklog;
			}
			do
			{
				AskForEvents(ulEvents|eOnClose|eOnIsPermitted|eOnAccept|eOnChatRequestComing);
				b = g_pSocketProServer->OnSettingServer();
				if(!b)
					break;
				DWORD dwThreadId;
				m_hThread = ::CreateThread(UNULL_PTR, 0, ThreadProc, UNULL_PTR, 0, &dwThreadId);
				b = (m_hThread != UNULL_PTR);
				if(!b)
					break;
				::WaitForSingleObject(m_hEvent, INFINITE);
				b = (::WaitForSingleObject(m_hThread, 150) == WAIT_TIMEOUT);
				if(!b)
				{
					::CloseHandle(m_hThread);
					m_hThread = UNULL_PTR;
				}
			}while(false);
			return b;
		}

		bool CSocketProServer::InitSocketProServer(int nParam)
		{
			return g_SocketProLoader.InitSocketProServer(nParam);
		}

		void CSocketProServer::StopSocketProServer()
		{
			CAutoLock al(&m_cs.m_sec);
			if(g_pSocketProServer == UNULL_PTR)
				return;
			if(g_pSocketProServer->m_hThread != UNULL_PTR && ::GetCurrentThreadId() != g_SocketProLoader.GetMainThreadID())
			{
				if(!g_SocketProLoader.GetUseWindowMessagePump())
					//Quit IO completion port or window message pump
					g_SocketProLoader.PostQuitPump(g_SocketProLoader.GetMainThreadID()); 
				else
					::PostThreadMessage(g_SocketProLoader.GetMainThreadID(), WM_QUIT, 0, 0) ? true : false;
				::WaitForSingleObject(g_pSocketProServer->m_hThread, INFINITE);
				::CloseHandle(g_pSocketProServer->m_hThread);
				g_pSocketProServer->m_hThread = UNULL_PTR;
			}
		}

		void CSocketProServer::UninitSocketProServer()
		{
			g_SocketProLoader.UninitSocketProServer();
		}

		unsigned long CSocketProServer::PushManager::GetAChatGroupDiscription(unsigned long ulGroupID, wchar_t *strDescription, unsigned long ulChars) //return the number of chars
		{
			return g_SocketProLoader.GetAChatGroup(ulGroupID, strDescription, ulChars);
		}

		bool CSocketProServer::Config::GetHTTPServerPush()
		{
			return g_SocketProLoader.GetHTTPServerPush();
		}

		void CSocketProServer::Config::SetHTTPServerPush(bool bPush)
		{
			g_SocketProLoader.SetHTTPServerPush(bPush);
		}

		unsigned long CSocketProServer::PushManager::GetCountOfChatGroups()
		{
			if(g_SocketProLoader.XGetCountOfChatGroups)
				return g_SocketProLoader.XGetCountOfChatGroups();
			return g_SocketProLoader.GetCountOfChatGroups();
		}

		unsigned long CSocketProServer::PushManager::GetGroupID(unsigned long ulIndex)
		{
			if(g_SocketProLoader.XGetGroupID)
				return g_SocketProLoader.XGetGroupID(ulIndex);
			if(ulIndex > 255)
				return 0;
			return g_SocketProLoader.GetGroupID((unsigned char)ulIndex);
		}
		unsigned long CSocketProServer::PushManager::GetCountOfChatters(unsigned long ulGroupId)
		{
			return g_SocketProLoader.GetCountOfChatters(ulGroupId);
		}

		void CSocketProServer::HttpPush::GetHTTPChatIds(unsigned long ulGroupId, CSimpleArray<CComBSTR> &aChatIds)
		{
			unsigned long ulIndex;
			aChatIds.RemoveAll();
			unsigned long ulCount = g_SocketProLoader.GetCountOfHTTPChatters(ulGroupId);
			for(ulIndex=0; ulIndex<ulCount; ulIndex++)
			{
				const wchar_t *strChatId = g_SocketProLoader.GetHTTPChatId(ulGroupId, ulIndex);
				if(strChatId != UNULL_PTR)
				{
					aChatIds.Add(CComBSTR(strChatId));
				}
			}
		}

		bool CSocketProServer::HttpPush::GetHTTPChatContext(const wchar_t *strChatId, wchar_t **pstrUserID, wchar_t **pstrIpAddr, unsigned long *pLeaseTime, VARIANT *pvtGroups, unsigned long *pTimeout, unsigned long *pCountOfMessages)
		{
			return g_SocketProLoader.GetHTTPChatContext(strChatId, pstrUserID, pstrIpAddr, pLeaseTime, pvtGroups, pTimeout, pCountOfMessages);
		}

		unsigned int CSocketProServer::PushManager::GetChatterSocket(unsigned long ulGroupID, unsigned long ulIndex) //return a socket handle
		{
			return g_SocketProLoader.GetChatterSocket(ulGroupID, ulIndex);
		}

		void CSocketProServer::UseSSL(tagEncryptionMethod EncryptionMethod, const wchar_t *strSubject, bool bMachine, bool bRoot)
		{
			ATLASSERT(EncryptionMethod != SSL23 && EncryptionMethod != TLSv1); //OpenSSL not supported with this method
			g_SocketProLoader.SetDefaultEncryptionMethod(EncryptionMethod);
			g_SocketProLoader.UseMSCert(bMachine, bRoot, strSubject);
		}

		void CSocketProServer::UseSSL(const wchar_t *strPfxFile, const wchar_t *strPassword, const wchar_t *strSubject, tagEncryptionMethod EncryptionMethod)
		{
			g_SocketProLoader.SetDefaultEncryptionMethod(EncryptionMethod);
			g_SocketProLoader.SetPfxFile(strPfxFile, strPassword);
			if(EncryptionMethod == MSSSL || EncryptionMethod == MSTLSv1)
			{
				g_SocketProLoader.UseMSCert(false, true, strSubject); //the first two parameters are ignored
			}
		}

		void CSocketProServer::UseSSL(tagEncryptionMethod EncryptionMethod, const wchar_t *strCertFile, const wchar_t *strPrivateKeyFile)
		{
			ATLASSERT(EncryptionMethod == SSL23 || EncryptionMethod == TLSv1); //the function works only for OpenSSL
			g_SocketProLoader.SetDefaultEncryptionMethod(EncryptionMethod);
			g_SocketProLoader.SetCertFile(strCertFile);
			g_SocketProLoader.SetPrivateKeyFile(strPrivateKeyFile);
		}

		CClientPeer *CBaseService::GetClientPeerFromPool()
		{
			int n;
			CAutoLock	AutoLock(&g_cs.m_sec);
			int nCount = m_Pool.GetSize();
			for(n=0; n<nCount; n++)
			{
				CClientPeer *p = m_Pool[m_Pool.GetSize()-1];
				ATLASSERT(p != UNULL_PTR);
				m_Pool.RemoveAt(m_Pool.GetSize()-1);
				return p;
			}
			return UNULL_PTR;
		}

		void CSocketProServer::Config::SetMaxThreadIdleTimeBeforeSuicide(unsigned long ulMaxThreadIdleTimeBeforeSuicide)
		{
			g_SocketProLoader.SetMaxThreadIdleTimeBeforeSuicide(ulMaxThreadIdleTimeBeforeSuicide);
		}

		void CSocketProServer::Config::SetMaxConnectionsPerClient(unsigned long ulMaxConnectionsPerClient)
		{
			g_SocketProLoader.SetMaxConnectionsPerClient(ulMaxConnectionsPerClient);
		}

		unsigned long CSocketProServer::Config::GetMaxThreadIdleTimeBeforeSuicide()
		{
			return g_SocketProLoader.GetMaxThreadIdleTimeBeforeSuicide();
		}

		unsigned long CSocketProServer::Config::GetMaxConnectionsPerClient()
		{
			return g_SocketProLoader.GetMaxConnectionsPerClient();
		}
		void CSocketProServer::Config::SetTimerElapse(unsigned long ulTimerElapse)
		{
			g_SocketProLoader.SetTimerElapse(ulTimerElapse); 
		}

		unsigned long WINAPI CSocketProServer::Config::GetTimerElapse()
		{
			return g_SocketProLoader.GetTimerElapse();
		}

		void CSocketProServer::Config::SetSMInterval(unsigned long ulSMInterval)
		{
			g_SocketProLoader.SetSMInterval(ulSMInterval);
		}

		unsigned long CSocketProServer::Config::GetSMInterval()
		{
			return g_SocketProLoader.GetSMInterval(); 
		}

		unsigned long CSocketProServer::Config::GetPingInterval()
		{
			return g_SocketProLoader.GetPingInterval();
		}
		
		void CSocketProServer::Config::SetPingInterval(unsigned long ulPingInterval)
		{
			g_SocketProLoader.SetPingInterval(ulPingInterval);
		}
		
		void CSocketProServer::Config::SetCleanPoolInterval(unsigned long ulCleanPoolInterval)
		{
			g_SocketProLoader.SetCleanPoolInterval(ulCleanPoolInterval);
		}
		
		unsigned long CSocketProServer::Config::GetCleanPoolInterval()
		{
			return g_SocketProLoader.GetCleanPoolInterval();
		}

		void CSocketProServer::Config::SetDefaultZip(bool bZip)
		{
			g_SocketProLoader.SetDefaultZip(bZip);
		}

		bool CSocketProServer::Config::GetDefaultZip()
		{
			return g_SocketProLoader.GetDefaultZip();
		}

		void CSocketProServer::Config::SetDefaultEncryptionMethod(tagEncryptionMethod EncryptionMethod)
		{
			g_SocketProLoader.SetDefaultEncryptionMethod(EncryptionMethod);
		}

		tagEncryptionMethod CSocketProServer::Config::GetDefaultEncryptionMethod()
		{
			return g_SocketProLoader.GetDefaultEncryptionMethod();
		}

		unsigned int WINAPI CSocketProServer::GetCountOfClients()
		{
			return g_SocketProLoader.GetCountOfClients();
		}

		unsigned int CSocketProServer::FindClient(unsigned int hSocket)
		{
			return g_SocketProLoader.FindClient(hSocket);
		}

		unsigned int CSocketProServer::GetClient(unsigned int unIndex)
		{
			return g_SocketProLoader.GetClient(unIndex);
		}

		HWND CSocketProServer::GetWin()
		{
			return g_SocketProLoader.GetWin();
		}

		unsigned long CSocketProServer::GetMainThreadID()
		{
			return g_SocketProLoader.GetMainThreadID();
		}

		int CSocketProServer::GetLastSocketError()
		{
			return g_SocketProLoader.GetLastSocketError();
		}

		unsigned int CSocketProServer::GetListeningSocketHandle()
		{
			return g_SocketProLoader.GetListeningSocket();
		}

		bool CSocketProServer::Config::SetUseWindowMessagePump(bool bUseWindowMessagePump)
		{
			if(g_SocketProLoader.SetUseWindowMessagePump != UNULL_PTR)
			{
				return g_SocketProLoader.SetUseWindowMessagePump(bUseWindowMessagePump);
			}
			return false;
		}

		bool CSocketProServer::Config::GetUseWindowMessagePump()
		{
			if(g_SocketProLoader.GetUseWindowMessagePump != UNULL_PTR)
			{
				return g_SocketProLoader.GetUseWindowMessagePump();
			}
			return true;
		}

		bool CSocketProServer::PostQuit()
		{
			CAutoLock al(&m_cs.m_sec);
			if(g_pSocketProServer == UNULL_PTR)
				return true;
			if(g_pSocketProServer->m_hThread != UNULL_PTR)
			{
				if(!g_SocketProLoader.GetUseWindowMessagePump())
					g_SocketProLoader.PostQuitPump(g_SocketProLoader.GetMainThreadID()); 
				else
					::PostThreadMessage(g_SocketProLoader.GetMainThreadID(), WM_QUIT, 0, 0) ? true : false;
				::CloseHandle(g_pSocketProServer->m_hThread);
				g_pSocketProServer->m_hThread = UNULL_PTR;
			}
			return true;
		}

		bool CSocketProServer::GetLocalName(wchar_t *strLocalName, unsigned short usChars)
		{
			return g_SocketProLoader.GetLocalName(strLocalName, usChars);
		}

		void CSocketProServer::Config::SetSwitchTime(unsigned long ulSwitchTime)
		{
			g_SocketProLoader.SetSwitchTime(ulSwitchTime);
		}

		unsigned long CSocketProServer::Config::GetSwitchTime()
		{
			return g_SocketProLoader.GetSwitchTime();
		}
		
		void CSocketProServer::Config::SetAuthenticationMethod(tagAuthenticationMethod am)
		{
			g_SocketProLoader.SetAuthenticationMethod(am);
		}

		tagAuthenticationMethod CSocketProServer::Config::GetAuthenticationMethod()
		{
			return (tagAuthenticationMethod)g_SocketProLoader.GetAuthenticationMethod();
		}

		void CSocketProServer::Config::SetSharedAM(bool bShared)
		{
			g_SocketProLoader.SetSharedAM(bShared);
		}

		bool CSocketProServer::Config::GetSharedAM()
		{
			return g_SocketProLoader.GetSharedAM();
		}

		void CSocketProServer::OnChatRequestComing(unsigned int hSocketSource, tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2)
		{
			
		}

		void CSocketProServer::OnChatRequestCame(unsigned int hSocketSource, tagChatRequestID ChatRequestId)
		{
		
		}

		void CSocketProServer::StartMessagePump()
		{
			if(g_SocketProLoader.GetUseWindowMessagePump != UNULL_PTR && g_SocketProLoader.StartIOPump != UNULL_PTR 
				&& (!g_SocketProLoader.GetUseWindowMessagePump()))
			{
				::SetEvent(m_hEvent);
				g_SocketProLoader.StartIOPump(); //support IO completion port pump from SocketPro version 4.8.0.1
			}
			else
			{
				MSG msg;
				::PeekMessage(&msg, 0, 0, 0, 0); 
				::SetEvent(m_hEvent);
				while(::GetMessage(&msg, 0, 0, 0))
				{
					::TranslateMessage(&msg);
					if(OnWinMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam, msg.time, msg.pt.x, msg.pt.y))
					{
						continue;
					}
					::DispatchMessage(&msg);
				}
			}
		}

		void CALLBACK CSocketProServer::OnMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
		{
			g_pSocketProServer->OnWinMessage(hWnd, nMessage, wParam, lParam);
		}

		void CALLBACK CSocketProServer::OnAccepted(unsigned int hSocket, int nError)
		{
			g_pSocketProServer->OnAccept(hSocket, nError);
		}

		void CALLBACK CSocketProServer::OnDown(unsigned int hSocket, int nError)
		{
			g_pSocketProServer->OnClose(hSocket, nError);
		}
		void CALLBACK CSocketProServer::OnSnd(unsigned int hSocket, int nError)
		{
			g_pSocketProServer->OnSend(hSocket, nError);
		}

		void CALLBACK CSocketProServer::OnSSL(unsigned int hSocket, int nWhere, int nRtn)
		{
			g_pSocketProServer->OnSSLEvent(hSocket, nWhere, nRtn);
		}

		void CALLBACK CSocketProServer::OnSTo(unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID)
		{
			g_pSocketProServer->OnSwitchTo(hSocket, ulPrevSvsID, ulCurrSvsID);
		}

		bool CALLBACK CSocketProServer::OnPermitted(unsigned int hSocket, unsigned long ulSvsID)
		{
			return g_pSocketProServer->OnIsPermitted(hSocket, ulSvsID);
		}

		void CALLBACK CSocketProServer::OnChatComing(unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen)
		{
			CComVariant vtP0, vtP1;
			CClientPeer* p = CBaseService::SeekClientPeerGlobally(hSocketSource);
			if(p)
			{
				VARTYPE vt = (VT_ARRAY|VT_UI1);
				CScopeUQueue sq;
				if(sq->GetMaxSize() < ulLen)
					sq->ReallocBuffer(ulLen + 10);
				unsigned long ulGet = p->RetrieveBuffer((BYTE*)sq->GetBuffer(), ulLen, true);
				ATLASSERT(ulGet == ulLen);
				sq->SetSize(ulGet);
				switch(usRequestID)
				{
				case idExit:
					break;
				case idEnter:
					sq >> vtP0.ulVal;
					vtP0.vt = VT_UI4;
					break;
				case idXEnter:
					sq->PopVT(vtP0);
					break;
				case idSpeak:
					sq >> vtP1.ulVal;
					vtP1.vt = VT_UI4;
					sq->PopVT(vtP0);
					break;
				case idSpeakEx:
					sq >> vtP1.ulVal;
					vtP1.vt = VT_UI4;
					ulGet = sq->GetSize();
					sq->Insert(&ulGet);
					sq->Insert((BYTE*)&vt, sizeof(vt));
					sq->PopVT(vtP0);
					break;
				case idXSpeakEx:
					sq->Insert((BYTE*)&vt, sizeof(vt));
					sq->PopVT(vtP0);
					ulGet = sq->GetSize()/sizeof(unsigned long);
					sq->Insert(&ulGet);
					vt = (VT_ARRAY|VT_UI4);
					sq->Insert((BYTE*)&vt, sizeof(vt));
					sq->PopVT(vtP1);
					break;
				case idXSpeak:
					sq->PopVT(vtP1);
					sq->PopVT(vtP0);
					break;
				case idSpeakTo:
					{
						wchar_t strBuffer[41] = {0};
						wchar_t strIpAddr[21] = {0};
						unsigned long ulAddr;
						unsigned int nPort;
						sq >> ulAddr;
						sq >> nPort;
						p->GetPeerName(&nPort, strIpAddr, 20);
						swprintf(strBuffer, L"%s:%d", strIpAddr, nPort);
						vtP0.vt = VT_BSTR;
						vtP0.bstrVal = CComBSTR(strBuffer).Detach();
					}
					sq->PopVT(vtP1);
					break;
				case idSpeakToEx:
					{
						wchar_t strBuffer[41] = {0};
						wchar_t strIpAddr[21] = {0};
						unsigned long ulAddr;
						unsigned int nPort;
						sq >> ulAddr;
						sq >> nPort;
						p->GetPeerName(&nPort, strIpAddr, 20);
						swprintf(strBuffer, L"%s:%d", strIpAddr, nPort);
						vtP0.vt = VT_BSTR;
						vtP0.bstrVal = CComBSTR(strBuffer).Detach();
					}
					ulGet = sq->GetSize();
					sq->Insert(&ulGet);
					sq->Insert((BYTE*)&vt, sizeof(vt));
					sq->PopVT(vtP1);
					break;
				case idSendUserMessage:
					{
						CComBSTR bstrUserId;
						sq >> bstrUserId;
						vtP0.vt = VT_BSTR;
						vtP0.bstrVal = bstrUserId.Detach();
					}
					sq->PopVT(vtP1);
					break;
				case idSendUserMessageEx:
					{
						CComBSTR bstrUserId;
						sq >> bstrUserId;
						vtP0.vt = VT_BSTR;
						vtP0.bstrVal = bstrUserId.Detach();
					}
					ulGet = sq->GetSize();
					sq->Insert(&ulGet);
					sq->Insert((BYTE*)&vt, sizeof(vt));
					sq->PopVT(vtP1);
					break;
				case idGetAllClients:
					break;
				case idGetAllGroups:
					break;
				case idGetAllListeners:
					if(sq->GetSize() == sizeof(unsigned long))
					{
						sq >> vtP0.ulVal; //group ids
						vtP0.vt = VT_UI4;
					}
					else
						sq->PopVT(vtP0);
					break;
				default:
					break;
				}
				ATLASSERT(sq->GetSize() == 0);
				p->OnChatRequestComing((tagChatRequestID)usRequestID, vtP0, vtP1);
			}
			g_pSocketProServer->OnChatRequestComing(hSocketSource, (tagChatRequestID)usRequestID, vtP0, vtP1);
		}

		void CALLBACK CSocketProServer::OnChatCame(unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen)
		{
			CClientPeer* p = CBaseService::SeekClientPeerGlobally(hSocketSource);
			if(p)
			{
				p->OnChatRequestCame((tagChatRequestID)usRequestID);
			}
			g_pSocketProServer->OnChatRequestCame(hSocketSource, (tagChatRequestID)usRequestID);
		}

		CBaseService::CBaseService() : m_ulSvsID(0), m_PreTranslateMessage(UNULL_PTR), m_ThreadStarted(UNULL_PTR), m_ThreadDying(UNULL_PTR), m_bUsePool(false)
		{
			if(!g_SocketProLoader.IsLoaded())
			{
				g_SocketProLoader.LoadSocketProServer();
			}

			//make sure that the server core library usktpror.dll (32bit or 64bit) is accessable.
			ATLASSERT(g_SocketProLoader.IsLoaded());

			memset(&m_SvsContext, 0, sizeof(m_SvsContext));
			m_ulEvents = (eOnSwitchTo + eOnFastRequestArrive + eOnSlowRequestArrive + eOnClose + eOnIsPermitted + eOnCleanPool + eOnBaseRequestCame);
			m_SvsContext.m_OnClose = OnClose;
			m_SvsContext.m_OnSwitchTo = OnSwitch;
			m_SvsContext.m_SlowProcess = OnSlow;
			m_SvsContext.m_OnFastRequestArrive = OnFast;
			m_SvsContext.m_OnIsPermitted = OnPermitted;
			m_SvsContext.m_OnCleanPool = OnCleanPool;
			m_SvsContext.m_OnBaseRequestCame = OnBaseCame;
			
			g_cs.Lock();
			g_aService.Add(this);
			g_cs.Unlock();
		}

		CBaseService::~CBaseService()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			{
				int  n;
				int  nCount;
				CClientPeer *p;
				nCount = m_Pool.GetSize();
				for(n=0; n<nCount; n++)
				{
					p = m_Pool[n];
					delete p;
				}
				m_Pool.RemoveAll();
				if(m_ulSvsID != 0)
				{
					RemoveMe();
				}
			}
			g_aService.Remove(this);
		}

		unsigned long CBaseService::GetCountOfServices()
		{
			return g_aService.GetSize();
		}

		CClientPeer* CBaseService::SeekClientPeerGlobally(unsigned int hSocket)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			CBaseService *pBase = GetBaseService(hSocket);
			if(pBase != UNULL_PTR)
			{
				return pBase->SeekClientPeer(hSocket);
			}
			//this could happen
			return UNULL_PTR;
		}

		CClientPeer* CBaseService::SeekClientPeer(unsigned int hSocket)
		{
			int n;
			CAutoLock	AutoLock(&g_cs.m_sec);
			int nSize = m_aClientPeer.GetSize();
			for(n=0; n<nSize; n++)
			{
				if(m_aClientPeer[n]->m_hSocket == hSocket)
					return m_aClientPeer[n];
			}
			return UNULL_PTR;
		}

		CBaseService *CBaseService::GetBaseService(unsigned long ulSvsID)
		{
			int n;
			CBaseService	*pBase;
			CAutoLock	AutoLock(&g_cs.m_sec);
			int nSize = g_aService.GetSize();
			for(n=0; n<nSize; n++)
			{
				pBase = g_aService[n];
				if(pBase->m_ulSvsID == ulSvsID)
					return pBase;
			}
			//if this happens, CBaseService::m_ulSvsID is not correct!
			//typically, you may forget setting m_ulSvsID when developing a dll with C/C++
			//or Loading balance + client socket closed.
			ATLTRACE("++++ Service not found! ++++\n");

			//should not come here
			ATLASSERT(FALSE);

			return UNULL_PTR;
		}

		void CBaseService::RemoveMe()
		{
			if(m_ulSvsID == 0)
				return;
			if(::GetCurrentThreadId() != CSocketProServer::GetMainThreadID() && g_SocketProLoader.GetUseWindowMessagePump())
				return; //must call the method within the main thread in case using window message pump
			g_SocketProLoader.RemoveASvsContext(m_ulSvsID);
			m_ulSvsID = 0;
		}

		void CBaseService::RemoveAllSlowRequests()
		{
			g_SocketProLoader.RemoveAllSlowRequests(m_ulSvsID);	
		}

		unsigned short CBaseService::GetSlowRequest(unsigned short usIndex)
		{
			return g_SocketProLoader.GetSlowRequest(m_ulSvsID, usIndex);
		}

		void CBaseService::RemoveSlowRequest(unsigned short usRequestID)
		{
			g_SocketProLoader.RemoveSlowRequest(m_ulSvsID, usRequestID);
		}

		unsigned short CBaseService::GetCountOfSlowRequests()
		{
			return g_SocketProLoader.GetCountOfSlowRequests(m_ulSvsID);
		}

		bool CBaseService::AddSlowRequest(unsigned short usRequestID)
		{
			return g_SocketProLoader.AddSlowRequest(m_ulSvsID, usRequestID);
		}
		
		unsigned long CBaseService::GetSvsID()
		{
			return m_ulSvsID;
		}

		void CBaseService::SetSvsID(unsigned long ulSvsID)
		{
			m_ulSvsID = ulSvsID;
		}

		/*
		bool CBaseService::AddMe(unsigned long ulSvsID, unsigned long ulEvents, enumThreadApartment taWhatTA)
		{
			m_SvsContext.m_enumTA = taWhatTA;
			m_ulEvents = (eOnSwitchTo + eOnFastRequestArrive + eOnSlowRequestArrive + eOnClose + eOnIsPermitted + eOnCleanPool + eOnBaseRequestCame);
			if((ulEvents & eOnReceive) > 0)
			{
				m_SvsContext.m_OnReceive = OnReceive;
				m_ulEvents += eOnReceive;
			}
			else
			{
				m_SvsContext.m_OnReceive = UNULL_PTR;
			}

			if((ulEvents & eOnSend) > 0)
			{
				m_SvsContext.m_OnSend = OnSend;
				m_ulEvents += eOnSend;
			}
			else
			{
				m_SvsContext.m_OnSend = UNULL_PTR;
			}

			if(m_ThreadStarted && (ulEvents & eOnThreadStarted) > 0)
			{
				m_SvsContext.m_ThreadStarted = m_ThreadStarted;
				m_ulEvents += eOnThreadStarted;
			}
			else
			{
				m_SvsContext.m_ThreadStarted = UNULL_PTR;
			}

			if(m_ThreadDying && (ulEvents & eOnThreadShuttingDown) > 0)
			{
				m_SvsContext.m_ThreadDying = m_ThreadDying;
				m_ulEvents += eOnThreadShuttingDown;
			}
			else
			{
				m_SvsContext.m_ThreadDying = UNULL_PTR;
			}
			
			if(m_PreTranslateMessage && (ulEvents & eOnPretranslateMessage) > 0)
			{
				m_SvsContext.m_PreTranslateMessage = m_PreTranslateMessage;
				m_ulEvents += eOnPretranslateMessage;
			}
			else
			{
				m_SvsContext.m_PreTranslateMessage = UNULL_PTR;
			}

			if((ulEvents & eOnSlowRequestProcessed) > 0)
			{
				m_SvsContext.m_OnRequestProcessed = OnSlowRequestProcessed;
				m_ulEvents += eOnSlowRequestProcessed;
			}
			else
			{
				m_SvsContext.m_OnRequestProcessed = UNULL_PTR;
			}
			
			if((ulEvents & eOnSendReturnData) > 0)
			{
				m_SvsContext.m_OnSendReturnData = OnSendReturnData;
				m_ulEvents += eOnSendReturnData;
			}
			else
			{
				m_SvsContext.m_OnSendReturnData = UNULL_PTR;
			}

			if((ulEvents & eOnThreadCreated) > 0)
			{
				m_SvsContext.m_OnThreadCreated = OnThreadCreated;
				m_ulEvents += eOnThreadCreated;
			}
			else
			{
				m_SvsContext.m_OnSendReturnData = UNULL_PTR;
			}
			
			m_ulSvsID = ulSvsID;
			return g_SocketProLoader.AddSvsContext(ulSvsID, m_SvsContext);
		}*/

		void CALLBACK CBaseService::OnFast(unsigned int hSocket, unsigned short usRequestID, unsigned long ulLen)
		{
			CBaseService *pBase = GetBaseService(hSocket);
			ATLASSERT(pBase != UNULL_PTR);
			if(pBase == UNULL_PTR)
				return;
			CClientPeer *p = pBase->SeekClientPeer(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p == UNULL_PTR)
				return;
			p->m_usCurrentRequestId = usRequestID;
			if(p->m_bAutoBuffer)
			{
				if(ulLen > p->m_UQueue.GetMaxSize())
				{
					p->m_UQueue.ReallocBuffer(ulLen);	
				}
				p->m_UQueue.SetSize(0);
				if(ulLen > 0)
				{
					unsigned long ulGet = p->RetrieveBuffer((unsigned char*)p->m_UQueue.GetBuffer(), ulLen, false);
					ATLASSERT(ulGet == ulLen);
					p->m_UQueue.SetSize(ulLen);
				}
			}
			if(!p->TransferServerException())
			{
				p->OnFastRequestArrive(usRequestID, ulLen);
			}
			else
			{
				try
				{
					p->OnFastRequestArrive(usRequestID, ulLen);
				}
				catch(CSocketProServerException &err)
				{
					CUQueue UQueue;
					if(err.m_hr == S_OK)
					{
						err.m_hr = E_FAIL;
					}
					if(err.m_usRequestID == 0)
					{
						err.m_usRequestID = usRequestID;
					}
					if(err.m_ulSvsID == 0)
					{
						err.m_ulSvsID = p->GetSvsID();
					}
					UQueue << err;
					p->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
					return;
				}
				catch(...)
				{
					CUQueue UQueue;
					CSocketProServerException err(E_FAIL, L"Unspecified error");
					err.m_usRequestID = usRequestID;
					err.m_ulSvsID = p->GetSvsID();
					UQueue << err;
					p->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
					return;
				}
			}
			pBase->OnFastRequestArrive(hSocket, usRequestID);
		}

		bool CBaseService::OnIsPermitted(unsigned int hSocket)
		{
			return true;
		}

		void CBaseService::OnThreadCreated(unsigned long ulThreadID, unsigned int hSocket)
		{

		}

		CBaseService *CBaseService::GetBaseService(unsigned int hSocket)
		{
			int n;
			int nCount;
			unsigned long ulSvsID = g_SocketProLoader.GetSvsID(hSocket);
			CAutoLock	AutoLock(&g_cs.m_sec);
			nCount = g_aService.GetSize();
			for(n=0; n<nCount; n++)
			{
				if(g_aService[n]->GetSvsID() == ulSvsID)
					return g_aService[n];
			}
			return UNULL_PTR;
		}

		bool CALLBACK CBaseService::OnPermitted(unsigned int hSocket, unsigned long ulSvsID)
		{
			CBaseService *pBase = GetBaseService(ulSvsID);
			//make sure that you already set CBaseService::m_ulSvsID 
			//by calling either CBaseService::AddMe or CBaseService::SetSvsID
			ATLASSERT(pBase != UNULL_PTR);
			if(pBase)
			{
				ATLASSERT(pBase->GetSvsID() != 0);
				return pBase->OnIsPermitted(hSocket);
			}
			return false;
		}

		void CALLBACK CBaseService::OnBaseCame(unsigned int hSocket, unsigned short usRequestID)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			if(p)
			{
				p->m_usCurrentRequestId = usRequestID;
				if(usRequestID <= SOCKETPRO_MAX_BASE_REQUEST_ID) //Base request id will not exceed the number
				{
					p->OnBaseRequestCame(usRequestID);
				}
				else
				{
					if(p->m_bAutoBuffer)
					{
						ULONG ulLen = g_SocketProLoader.GetCurrentRequestLen(hSocket);
						if(ulLen > p->m_UQueue.GetMaxSize())
						{
							p->m_UQueue.ReallocBuffer(ulLen);	
						}
						p->m_UQueue.SetSize(0);
						if(ulLen > 0)
						{
							unsigned long ulGet = p->RetrieveBuffer((unsigned char*)p->m_UQueue.GetBuffer(), ulLen, false);
							ATLASSERT(ulGet == ulLen);
							p->m_UQueue.SetSize(ulLen);
						}
					}
					//starting with SocketPro version 4.4.1.1
					p->OnDispatchingSlowRequest(usRequestID);
				}
				p->GetBaseService()->OnBaseRequestCame(hSocket, usRequestID);
			}
		}

		void CALLBACK CBaseService::OnReceive(unsigned int hSocket, int nError)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p)
			{
				p->OnReceive(nError);
			}
		}

		void CALLBACK CBaseService::OnThreadCreated(unsigned long ulSvsID, unsigned long ulThreadID, unsigned int hSocket)
		{
			CBaseService *pBaseService = GetBaseService(ulSvsID);
			ATLASSERT(pBaseService != UNULL_PTR);
			if(pBaseService)
			{
				ATLASSERT(pBaseService->GetSvsID() != 0);
				pBaseService->OnThreadCreated(ulThreadID, hSocket);
			}
		}

		void CALLBACK CBaseService::OnCleanPool(unsigned long ulSvsID, unsigned long ulTickCount)
		{
			CBaseService *pBaseService = GetBaseService(ulSvsID);
			if(pBaseService != UNULL_PTR)
			{
				int n;
				CClientPeer *p;
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nCount = pBaseService->m_Pool.GetSize();
				for(n=0; n<nCount; n++)
				{
					p = (pBaseService->m_Pool)[n];
					ATLASSERT(p);
					if(p)
					{
						if(ulTickCount > p->m_ulTickCountReleased && (ulTickCount - p->m_ulTickCountReleased) > g_SocketProLoader.GetCleanPoolInterval())
						{
							delete p;
							pBaseService->m_Pool.RemoveAt(n);
							nCount--;
							n--;
						}
					}
				}
			}
		}

		void CALLBACK CBaseService::OnSend(unsigned int hSocket, int nError)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p)
			{
				p->OnSend(nError);
			}
		}

		void CALLBACK CBaseService::OnSlowRequestProcessed(unsigned int hSocket, unsigned short usRequestID)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p)
			{
				p->OnSlowRequestProcessed(usRequestID);
			}
		}

		bool CALLBACK CBaseService::OnSendReturnData(unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p)
			{
				return p->OnSendReturnData(usRequestId, ulLen, pBuffer);
			}
			return false;
		}

		void CALLBACK CBaseService::OnSwitch(unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID)
		{
			CBaseService *pBaseService = GetBaseService(hSocket);
			ATLASSERT(pBaseService != UNULL_PTR);
			if(pBaseService == UNULL_PTR)
				return;
			ATLASSERT(pBaseService->GetSvsID() != 0);
			if(ulCurrSvsID == pBaseService->GetSvsID())
			{
				bool bServerException;
				CSwitchInfo SInfo;
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

				CClientPeer *pPeerSocket = pBaseService->GetClientPeerFromPool();
				
				//if we find a CClientPeer, we just reuse it from the pool
				//if we can't find one, we need to create one
				if(pPeerSocket == UNULL_PTR)
				{
					pPeerSocket = pBaseService->GetPeerSocket(hSocket);
				}
				ATLASSERT(pPeerSocket != UNULL_PTR);
				if(pPeerSocket)
				{
					pPeerSocket->m_Push.m_hSocket = hSocket;
					//p->m_hSocket = hSocket;
					pPeerSocket->m_hSocket = hSocket;

					//record a pointer into its service
					pBaseService->AddAClientPeer(pPeerSocket);

					pPeerSocket->OnSwitchFrom(ulPrevSvsID);
				}
			}
			else
			{
				CClientPeer *p = SeekClientPeerGlobally(hSocket);
				ATLASSERT(p != UNULL_PTR);
				if(p)
				{
					pBaseService->OnRelease(hSocket, false, ulCurrSvsID);
					p->OnReleaseResource(false, ulCurrSvsID);
					pBaseService->RemoveClientPeer(p);
				}
			}
		}

		HRESULT CALLBACK CBaseService::OnSlow(unsigned short usRequestID, unsigned long ulLen, unsigned int hSocket)
		{
			HRESULT hr = S_OK;
			CClientPeer *p = CBaseService::SeekClientPeerGlobally(hSocket);
			ATLASSERT(p != UNULL_PTR);
			if(p == UNULL_PTR)
				return S_OK;
			p->m_usCurrentRequestId = usRequestID;
			if(p->m_bAutoBuffer)
			{
				ulLen = p->m_UQueue.GetSize();
			}
			if(!p->TransferServerException())
			{
				hr = p->OnSlowRequestArrive(usRequestID, ulLen);
			}
			else
			{
				try
				{
					hr = p->OnSlowRequestArrive(usRequestID, ulLen);
				}
				catch(CSocketProServerException &err)
				{
					CUQueue UQueue;
					if(err.m_hr == S_OK)
					{
						err.m_hr = E_FAIL;
					}
					if(err.m_usRequestID == 0)
					{
						err.m_usRequestID = usRequestID;
					}
					if(err.m_ulSvsID == 0)
					{
						err.m_ulSvsID = p->GetSvsID();
					}
					UQueue << err;
					hr = p->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
				}
				catch(...)
				{
					CUQueue UQueue;
					CSocketProServerException err(E_FAIL, L"Unspecified error");
					err.m_usRequestID = usRequestID;
					err.m_ulSvsID = p->GetSvsID();
					UQueue << err;
					hr = p->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
				}				
			}
			{
				CBaseService *pBase = p->GetBaseService();
				if(pBase != UNULL_PTR)
					pBase->OnSlowRequestArrive(hSocket, usRequestID);
			}
			return hr;
		}

		void CBaseService::OnBaseRequestCame(unsigned int hSocket, unsigned short usRequestID)
		{

		}

		void CBaseService::OnFastRequestArrive(unsigned int hSocket, unsigned short usRequestID)
		{

		}
		
		void CBaseService::OnSlowRequestArrive(unsigned int hSocket, unsigned short usRequestID)
		{

		}

		void CBaseService::OnRelease(unsigned int hSocket, bool bClose, unsigned long ulInfo)
		{

		}

		void CALLBACK CBaseService::OnClose(unsigned int hSocket, int nError)
		{
			CClientPeer *p = SeekClientPeerGlobally(hSocket);
			if(p == UNULL_PTR)
				return;
			p->OnReleaseResource(true, nError);
			CBaseService *pBase = p->GetBaseService();
			if(pBase != UNULL_PTR)
			{
				pBase->OnRelease(hSocket, true, nError);
				pBase->RemoveClientPeer(p);
			}
			else
			{
				ATLASSERT(FALSE);
			}
		}

		
		IUPush* CClientPeer::GetPush()
		{
			return &m_Push;
		}

		CUQueue& CClientPeer::GetUQueue()
		{
			return m_UQueue;
		}

		CBaseService *CClientPeer::GetBaseService()
		{
			return CBaseService::GetBaseService(GetSvsID());
		}

		void CBaseService::AddAClientPeer(CClientPeer *pClientPeer)
		{
			g_cs.Lock();
			m_aClientPeer.Add(pClientPeer);
			g_cs.Unlock();
		}

		void CBaseService::RemoveClientPeer(CClientPeer *pClientPeer)
		{
			ATLASSERT(pClientPeer);
			if(!pClientPeer)
				return;
			{
				g_cs.Lock();
				BOOL b = m_aClientPeer.Remove(pClientPeer);
				g_cs.Unlock();
				ATLASSERT(b);
			}
			pClientPeer->m_hSocket = 0;
			pClientPeer->m_ulTickCountReleased = ::GetTickCount();
			if(m_bUsePool)
				m_Pool.Add(pClientPeer);
			else
				delete pClientPeer;
		}

		unsigned long CBaseService::GetCountOfAllServices()
		{
			return g_SocketProLoader.GetCountOfServices();
		}
				
		HINSTANCE CBaseService::AddALibrary(const wchar_t * strLibFile, int nParam) //return an instance to the added dll
		{
			return g_SocketProLoader.AddADll(strLibFile, nParam);
		}

		bool CBaseService::RemoveALibrary(HINSTANCE hLib)
		{
			if(::GetCurrentThreadId() != CSocketProServer::GetMainThreadID())
				return false; //must call the method within the main thread
			return g_SocketProLoader.RemoveADllByHandle(hLib);
		}

		bool CBaseService::RemoveALibrary(const wchar_t *strLibFile)
		{
			if(::GetCurrentThreadId() != CSocketProServer::GetMainThreadID())
				return false; //must call the method within the main thread
			return g_SocketProLoader.RemoveADll(strLibFile);
		}
		unsigned long CBaseService::GetServiceID(unsigned long ulIndex)
		{
			return g_SocketProLoader.GetServiceID(ulIndex);
		}

		unsigned long CSocketProServer::GetUserID(unsigned int hSocket, wchar_t *strUID, unsigned long ulCharLen)
		{
			return g_SocketProLoader.GetUID(hSocket, strUID, ulCharLen);
		}

		unsigned long CSocketProServer::GetPassword(unsigned int hSocket, wchar_t *strPassword, unsigned long ulCharLen)
		{
			return g_SocketProLoader.GetPassword(hSocket, strPassword, ulCharLen);
		}

		bool CSocketProServer::SetPassword(unsigned int hSocket, const wchar_t *strPassword)
		{
			if(g_SocketProLoader.SetPassword == UNULL_PTR)
				return false;
			return g_SocketProLoader.SetPassword(hSocket, strPassword);
		}

		unsigned long CBaseService::GetCountOfLibraries()
		{
			return g_SocketProLoader.GetCountOfLibraries();
		}

		unsigned long CBaseService::GetEvents()
		{
			return m_ulEvents;
		}

		bool CBaseService::GetReturnRandom()
		{
			if(g_SocketProLoader.GetReturnRandom == UNULL_PTR)
				return false;
			return g_SocketProLoader.GetReturnRandom(m_ulSvsID);
		}

		void CBaseService::SetReturnRandom(bool bRandom)
		{
			if(g_SocketProLoader.SetReturnRandom == UNULL_PTR)
				return;
			g_SocketProLoader.SetReturnRandom(m_ulSvsID, bRandom);
		}

		HINSTANCE CBaseService::GetALibrary(unsigned long ulIndex)
		{
			return g_SocketProLoader.GetADll(ulIndex);
		}

		CClientPeer::CClientPeer() : m_usCurrentRequestId(0), m_bAutoBuffer(true), m_hSocket(0), m_ulTickCountReleased(0), m_UQueue(*m_sq)
		{
			
		}

		CClientPeer::~CClientPeer()
		{

		}

		bool CClientPeer::CUPushServerImpl::Enter(unsigned long *pGroups, unsigned long ulCount)
		{
			if(pGroups == UNULL_PTR || ulCount == 0)
				return Exit();
			CComVariant vtGroups;
			CreateVtGroups(pGroups, ulCount, vtGroups);
			if(g_SocketProLoader.XEnter == UNULL_PTR)
			{
				unsigned long n;
				unsigned long ulGroups = 0;
				for(n=0; n<ulCount; n++)
					ulGroups |= pGroups[n];
				return g_SocketProLoader.Enter(m_hSocket, ulGroups);
			}
			return g_SocketProLoader.XEnter(m_hSocket, &vtGroups);
		}

		bool CClientPeer::CUPushServerImpl::Broadcast(const unsigned char *pMessage, unsigned long ulMessageSize, unsigned long *pGroups, unsigned long ulCount)
		{
			if(pGroups == UNULL_PTR || ulCount == 0)
				return false;
			if(g_SocketProLoader.XSpeakEx == UNULL_PTR)
			{
				unsigned long n;
				unsigned long ulGroups = 0;
				for(n=0; n<ulCount; n++)
					ulGroups |= pGroups[n];
				return g_SocketProLoader.SpeakEx(m_hSocket, pMessage, ulMessageSize, ulGroups);
			}
			return g_SocketProLoader.XSpeakEx(m_hSocket, pMessage, ulMessageSize, ulCount, pGroups);
		}

		bool CClientPeer::CUPushServerImpl::Broadcast(const VARIANT& vtMessage, unsigned long *pGroups, unsigned long ulCount)
		{
			if(pGroups == UNULL_PTR || ulCount == 0)
				return false;
			if(g_SocketProLoader.XSpeak == UNULL_PTR)
			{
				unsigned long n;
				unsigned long ulGroups = 0;
				for(n=0; n<ulCount; n++)
					ulGroups |= pGroups[n];
				return g_SocketProLoader.Speak(m_hSocket, &vtMessage, ulGroups);
			}
			CComVariant vtGroups;
			CreateVtGroups(pGroups, ulCount, vtGroups);
			return g_SocketProLoader.XSpeak(m_hSocket, &vtMessage, &vtGroups);
		}

		bool CClientPeer::CUPushServerImpl::SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId)
		{
			return g_SocketProLoader.SendUserMessage(m_hSocket, strUserId, &vtMessage);
		}

		bool CClientPeer::CUPushServerImpl::SendUserMessage(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulMessageSize)
		{
			return g_SocketProLoader.SendUserMessageEx(m_hSocket, strUserID, pMessage, ulMessageSize);
		}

		bool CClientPeer::CUPushServerImpl::Exit()
		{
			return g_SocketProLoader.Exit(m_hSocket);
		}

		unsigned long CClientPeer::GetSvsID(unsigned int hSocket)
		{
			return g_SocketProLoader.GetSvsID(hSocket);
		}

		unsigned long CClientPeer::GetSvsID()
		{
			return g_SocketProLoader.GetSvsID(m_hSocket);
		}

		unsigned int CClientPeer::GetSocket()
		{
			return m_hSocket;
		}

		void CClientPeer::OnReleaseResource(bool bClosing, unsigned long ulInfo)
		{

		}

		void CClientPeer::OnSwitchFrom(unsigned long ulServiceID)
		{

		}

		void CClientPeer::OnReceive(int nError)
		{

		}

		void CClientPeer:: OnSend(int nError)
		{

		}
		
		void CClientPeer::OnDispatchingSlowRequest(unsigned short usRequestID)
		{

		}

		void CClientPeer::OnBaseRequestCame(unsigned short usRequestID)
		{
			switch(usRequestID)
			{
			case idCleanTrack:
				m_UQueue.CleanTrack();
				break;
			default:
				break;
			}
		}

		void CClientPeer::OnSlowRequestProcessed(unsigned short usRequestID)
		{

		}

		/*
		void CClientPeer::OnChatRequestComing(tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2)
		{

		}

		void CClientPeer::OnChatRequestCame(tagChatRequestID ChatRequestId)
		{

		}*/

		bool CClientPeer::OnSendReturnData(unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer)
		{
			return false;
		}

		unsigned long CClientPeer::SendResult(unsigned short usRequestID, CUQueue &UQueue)
		{
			if(TransferServerException())
			{
				int rtn = 0;
				UQueue.Insert((unsigned char*)&rtn, sizeof(rtn), 0);
			}
			return SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
		}

		unsigned long CClientPeer::SendResult(unsigned short usRequestID, CScopeUQueue &UQueue)
		{
			if(TransferServerException())
			{
				int rtn = 0;
				UQueue->Insert((unsigned char*)&rtn, sizeof(rtn), 0);
			}
			return SendReturnData(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
		}

		unsigned long CClientPeer::SendResult(unsigned short usRequestID)
		{
			CScopeUQueue su;
			if(TransferServerException())
				su<<(int)0; //required by client
			return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
		}


		unsigned long CClientPeer::SendReturnData(unsigned short usRequestID, const unsigned char *pBuffer, unsigned long ulLen)
		{
			return g_SocketProLoader.SendReturnData(m_hSocket, usRequestID, ulLen, pBuffer);
		}

		unsigned long CClientPeer::RetrieveBuffer(unsigned char *pBuffer, unsigned long ulBufferLen, bool bPeek)
		{
			return g_SocketProLoader.RetrieveBuffer(m_hSocket, ulBufferLen, pBuffer, bPeek);
		}

	
		bool CClientPeer::AbortBatching()
		{
			return g_SocketProLoader.AbortBatching(m_hSocket);
		}

		bool CClientPeer::CommitBatching()
		{
			return g_SocketProLoader.CommitBatching(m_hSocket);
		}

		void CClientPeer::DropRequestResult(unsigned short usRequestId)
		{
			if(g_SocketProLoader.DropRequestResult != UNULL_PTR)
			{
				g_SocketProLoader.DropRequestResult(m_hSocket, usRequestId);
			}
		}

		void CClientPeer::DropCurrentSlowRequest()
		{
			if(g_SocketProLoader.DropCurrentSlowRequest != UNULL_PTR)
			{
				g_SocketProLoader.DropCurrentSlowRequest(m_hSocket);
			}
		}

		tagEncryptionMethod CClientPeer::GetEncryptionMethod()
		{
			return g_SocketProLoader.GetEncryptionMethod(m_hSocket);
		
		}

		bool CClientPeer::SetEncryptionMethod(tagEncryptionMethod EncryptionMethod)
		{
			return g_SocketProLoader.SetEncryptionMethod(m_hSocket, EncryptionMethod);
		}

		bool CClientPeer::IsClosing()
		{
			if(g_SocketProLoader.IsClosing != UNULL_PTR)
			{
				return g_SocketProLoader.IsClosing(m_hSocket);
			}
			return false;
		}

		void CClientPeer::CleanTrack()
		{
			g_SocketProLoader.CleanTrack(m_hSocket); 
		}

		unsigned int CClientPeer::GetMaxMessageSize()
		{
			if(g_SocketProLoader.GetHTTPMaxMessageSize == UNULL_PTR)
				return (~0);
			return g_SocketProLoader.GetHTTPMaxMessageSize(GetSocket());
		}

		void CClientPeer::SetMaxMessageSize(unsigned int nNewMax)
		{
			if(g_SocketProLoader.SetHTTPMaxMessageSize != UNULL_PTR)
				g_SocketProLoader.SetHTTPMaxMessageSize(GetSocket(), nNewMax);
		}

		bool CClientPeer::GetInterfaceAttributes(unsigned long *pulMTU, unsigned long *pulMaxSpeed, unsigned long *pulType, unsigned long *pulMask)
		{
			return g_SocketProLoader.GetInterfaceAttributes(m_hSocket, pulMTU, pulMaxSpeed, pulType, pulMask);
		}

		unsigned long CClientPeer::GetCountOfMySpecificBytes()
		{
			return g_SocketProLoader.GetCountOfMySpecificBytes(m_hSocket);
		}

		unsigned long CClientPeer::GetMySpecificBytes(unsigned char *pBuffer, unsigned long ulLen)
		{
			return g_SocketProLoader.GetMySpecificBytes(m_hSocket, pBuffer, ulLen);
		}

		bool CClientPeer::GetPeerName(unsigned int *pnPeerPort, wchar_t *strPeerAddr, unsigned short usChars)
		{
			return g_SocketProLoader.GetPeerName(m_hSocket, pnPeerPort, strPeerAddr, usChars);
		}

		CSwitchInfo CClientPeer::GetServerInfo()
		{
			CSwitchInfo ServerInfo;
			g_SocketProLoader.GetServerInfo(m_hSocket, &ServerInfo);
			return ServerInfo;
		}

		void CClientPeer::SetServerInfo(CSwitchInfo ServerInfo)
		{
			g_SocketProLoader.SetServerInfo(m_hSocket, &ServerInfo);
		}

		bool CClientPeer::GetSockAddr(unsigned int *pnSockPort, wchar_t *strIPAddrBuffer, unsigned short usChars)
		{
			return g_SocketProLoader.GetSockAddr(m_hSocket, pnSockPort, strIPAddrBuffer, usChars);
		}

		bool CClientPeer::PostClose(unsigned short usError)
		{
			return g_SocketProLoader.PostClose(m_hSocket, usError);
		}

		bool CClientPeer::SetBlowFish(unsigned char bKeyLen, unsigned char *strKey)
		{
			return g_SocketProLoader.SetBlowFish(m_hSocket, bKeyLen, strKey);
		}

		void CClientPeer::ShrinkMemory()
		{
			g_SocketProLoader.ShrinkMemory(m_hSocket);
		}

		bool CClientPeer::StartBatching()
		{
			return g_SocketProLoader.StartBatching(m_hSocket);
		}

		unsigned int CClientPeer::GetAssociatedSocket()
		{
			return g_SocketProLoader.GetAssociatedSocket();
		}

		unsigned long CClientPeer::GetRequestIDsInQueue(unsigned short *pusRequestID, unsigned long ulSize)
		{
			return g_SocketProLoader.GetRequestIDsInQueue(m_hSocket, pusRequestID, ulSize);
		}

		unsigned long CClientPeer::GetAssociatedThreadID()
		{
			return g_SocketProLoader.GetAssociatedThreadID(m_hSocket);
		}

		unsigned long CClientPeer::GetBytesBatched()
		{
			return g_SocketProLoader.GetBytesBatched(m_hSocket);
		}

		unsigned long CClientPeer::GetBytesReceived(unsigned long *pulHigh)
		{
			return g_SocketProLoader.GetBytesIn(m_hSocket, pulHigh);
		}

		unsigned long CClientPeer::GetBytesSent(unsigned long *pulHigh)
		{
			return g_SocketProLoader.GetBytesOut(m_hSocket, pulHigh);
		}

		CSwitchInfo CClientPeer::GetClientInfo()
		{
			CSwitchInfo ClientInfo;
			g_SocketProLoader.GetClientInfo(m_hSocket, &ClientInfo);
			return ClientInfo;
		}
		
		void CClientPeer::SetZipLevel(tagZipLevel zl)
		{
			if(g_SocketProLoader.SetZipLevel != UNULL_PTR)
			{
				g_SocketProLoader.SetZipLevel(m_hSocket, zl);
			}
		}

		tagZipLevel CClientPeer::GetZipLevel()
		{
			if(g_SocketProLoader.GetZipLevel != UNULL_PTR)
			{
				return g_SocketProLoader.GetZipLevel(m_hSocket);
			}
			return zlDefault;
		}

		unsigned long CClientPeer::GetConsumedMemory()
		{
			return g_SocketProLoader.GetTotalMemory(m_hSocket);
		}

		unsigned short CClientPeer::GetCurrentRequestID()
		{
			//return g_SocketProLoader.GetCurrentRequestID(m_hSocket);
			return m_usCurrentRequestId;
		}

		unsigned long CClientPeer::GetCurrentRequestLen()
		{
			return g_SocketProLoader.GetCurrentRequestLen(m_hSocket);
		}

		unsigned long CClientPeer::GetJoinedGroups(unsigned long *pGroupId, unsigned long ulGroupCount)
		{
			if(pGroupId == UNULL_PTR || ulGroupCount == 0)
				return 0;
			if(g_SocketProLoader.GetJoinedGroupIds)
				return g_SocketProLoader.GetJoinedGroupIds(m_hSocket, pGroupId, ulGroupCount);
			unsigned long ulGroups = g_SocketProLoader.GetJoinedGroup(m_hSocket);
			int n = 0;
			unsigned long ulCount = 0;
			unsigned long ulGroup = 1;
			while(n<32 && ulGroupCount > ulCount)
			{
				if((ulGroup & ulGroups) == ulGroup)
				{
					pGroupId[ulGroupCount] = ulGroup;
					++ulCount;
				}
				++n;
				ulGroup <<= 1;
			}
			return ulCount;
		}

		bool CClientPeer::IsBatching()
		{
			return g_SocketProLoader.IsBatching(m_hSocket);
		}

		bool CClientPeer::IsCanceled()
		{
			return g_SocketProLoader.IsCanceled();
		}

		bool CClientPeer::IsSameEndian()
		{
			return g_SocketProLoader.IsSameEndian(m_hSocket);
		}

		unsigned long CClientPeer::GetLastRcvTime()
		{
			return g_SocketProLoader.GetLastRcvTime(m_hSocket);
		}

		unsigned long CClientPeer::GetLastSndTime()
		{
			return g_SocketProLoader.GetLastSndTime(m_hSocket);
		}

		int CClientPeer::GetLastSocketError()
		{
			return g_SocketProLoader.GetLastSocketError();
		}

		unsigned long CClientPeer::GetPassword(wchar_t *strPassword, unsigned long ulCharLen)
		{
			return g_SocketProLoader.GetPassword(m_hSocket, strPassword, ulCharLen);
		}

		unsigned long CClientPeer::GetRcvBufferSize()
		{
			return g_SocketProLoader.GetRcvBufferSize(m_hSocket);
		}

		unsigned long CClientPeer::GetRcvBytesInQueue()
		{
			return g_SocketProLoader.GetRcvBytesInQueue(m_hSocket);
		}

		unsigned long CClientPeer::GetSndBytesInQueue()
		{
			return g_SocketProLoader.GetSndBytesInQueue(m_hSocket);
		}

		unsigned long CClientPeer::GetRequestsInQueue()
		{
			return g_SocketProLoader.QueryRequestsInQueue(m_hSocket);
		}

		unsigned long CClientPeer::GetSndBufferSize()
		{
			return g_SocketProLoader.GetSndBufferSize(m_hSocket);
		}

		bool CClientPeer::GetZip()
		{
			return g_SocketProLoader.GetZip(m_hSocket);
		}

		void CClientPeer::SetZip(bool bZip)
		{
			g_SocketProLoader.SetZip(m_hSocket, bZip);
		}

		bool CClientPeer::SetUID(const wchar_t *strUID)
		{
			return g_SocketProLoader.SetUserID(m_hSocket, strUID);
		}

		unsigned long CClientPeer::GetUID(wchar_t *strUID, unsigned long ulCharLen)
		{
			return g_SocketProLoader.GetUID(m_hSocket, strUID, ulCharLen);
		}

		unsigned long CClientPeer::RetrieveBuffer(unsigned int hSocket, unsigned long ulLen, unsigned char* pBuffer, bool bPeek)
		{
			return g_SocketProLoader.RetrieveBuffer(hSocket, ulLen, pBuffer, bPeek);
		}

		unsigned long CClientPeer::SendReturnData(unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char* pBuffer)
		{
			return g_SocketProLoader.SendReturnData(hSocket, usRequestId, ulLen, pBuffer);
		}

		const wchar_t* CHttpPeerBase::CHttpPushImpl::Enter(unsigned long *pGroups, unsigned long ulGroupCount, const wchar_t* strUserID, unsigned long dwLeaseTime, const wchar_t *strIpAddr)
		{
			unsigned long n;
			if(pGroups == UNULL_PTR || ulGroupCount == 0)
				return false;
			g_SocketProLoader.SetUserID(m_pClientPeer->GetSocket(), strUserID);
			if(g_SocketProLoader.XHTTPEnter)
			{
				CComVariant vtGroups;
				IUPush::CreateVtGroups(pGroups, ulGroupCount, vtGroups);
				return g_SocketProLoader.XHTTPEnter(m_pClientPeer->GetSocket(), &vtGroups, dwLeaseTime, strIpAddr);
			}
			unsigned long ulGroups = 0;
			for(n=0; n<ulGroupCount; n++)
			{
				ulGroups |= pGroups[n];
			}
			return g_SocketProLoader.HTTPEnter(m_pClientPeer->GetSocket(), ulGroups, dwLeaseTime, strIpAddr);
		}

		bool CHttpPeerBase::CHttpPushImpl::HTTPSubscribe(const wchar_t* strChatId, unsigned long ulTimeout, const wchar_t *strCrossSiteJSCallback)
		{
			return g_SocketProLoader.HTTPSubscribe(m_pClientPeer->GetSocket(), strChatId, ulTimeout, strCrossSiteJSCallback);	
		}

		bool CHttpPeerBase::CHttpPushImpl::Exit(const wchar_t* strChatId)
		{
			return g_SocketProLoader.HTTPExit(m_pClientPeer->GetSocket(), strChatId);	
		}

		bool CHttpPeerBase::CHttpPushImpl::SendUserMessage(const wchar_t* strChatId, const wchar_t* strUserID, const VARIANT *pvtMsg)
		{
			return g_SocketProLoader.HTTPSendUserMessage(m_pClientPeer->GetSocket(), strChatId, strUserID, pvtMsg);
		}

		const VARIANT* CHttpPeerBase::CHttpPushImpl::GetHTTPChatGroupIds(const wchar_t* strChatId)
		{
			if(g_SocketProLoader.GetHTTPJoinedGroups == UNULL_PTR)
				return UNULL_PTR;
			return g_SocketProLoader.GetHTTPJoinedGroups(m_pClientPeer->GetSocket(), strChatId);
		}

		bool CHttpPeerBase::CHttpPushImpl::Speak(const wchar_t* strChatId, const VARIANT *pvtMsg, unsigned long *pGroups, unsigned long ulGroupCount)
		{
			unsigned long n;
			if(pGroups == UNULL_PTR || ulGroupCount == 0)
				return false;
			if(g_SocketProLoader.XHTTPSpeak)
			{
				CComVariant vtGroups;
				IUPush::CreateVtGroups(pGroups, ulGroupCount, vtGroups);
				return g_SocketProLoader.XHTTPSpeak(m_pClientPeer->GetSocket(), strChatId, pvtMsg, &vtGroups);
			}
			unsigned long ulGroups = 0;
			for(n=0; n<ulGroupCount; n++)
			{
				ulGroups |= pGroups[n];
			}
			return g_SocketProLoader.HTTPSpeak(m_pClientPeer->GetSocket(), strChatId, pvtMsg, ulGroups);
		}

		void CHttpPeerBase::SetAutoPartition(bool bPartition)
		{
			g_SocketProLoader.SetHTTPAutoPartition(GetSocket(), bPartition);
		}

		bool CHttpPeerBase::GetAutoPartition()
		{
			return g_SocketProLoader.GetHTTPAutoPartition(GetSocket());
		}

		bool CHttpPeerBase::SetResponseHeader(const char *strUTF8Header, const char *strUTF8Value)
		{
			return g_SocketProLoader.SetHTTPResponseHeader(GetSocket(), strUTF8Header, strUTF8Value);
		}

		void CHttpPeerBase::SetResponseCode(unsigned int nHttpErrorCode)
		{
			g_SocketProLoader.SetHTTPResponseCode(GetSocket(), nHttpErrorCode);
		}

		enumHTTPRequest CHttpPeerBase::GetRequestMethod()
		{
			return m_HttpRequest;
		}

		double CHttpPeerBase::GetHTTPClientVersion()
		{
			return m_dVersion;
		}

		const char* CHttpPeerBase::GetHeaders()
		{
			return (const char*)m_qHeader.GetBuffer();
		}

		const char* CHttpPeerBase::GetQuery()
		{
			return (const char*)m_qQueue.GetBuffer();
		}

		const char* CHttpPeerBase::GetPathName(unsigned long &nLen)
		{
			char *strQuery = (char*)m_qQueue.GetBuffer();
			const char *str = ::strstr(strQuery, "?");
			if(str)
				nLen = (unsigned long)(str - strQuery);
			else
				nLen = m_qQueue.GetSize() - 1;
			return strQuery;
		}

		const char* CHttpPeerBase::GetParams()
		{
			char *strQuery = (char*)m_qQueue.GetBuffer();
			const char *str = ::strstr(strQuery, "?");
			if(!str)
				return UNULL_PTR;
			return (const char*)(++str);
		}

		unsigned long CHttpPeerBase::GetParamCount()
		{
			const char *strParams = GetParams();
			unsigned long nCount = 0;
			if(strParams != UNULL_PTR)
			{
				++nCount;
				const char *str = ::strstr(strParams, "&");
				while(str)
				{
					strParams = str + 1;
					str = strstr(strParams, "&");
					++nCount;
				}
			}
			return nCount;
		}

		const char* CHttpPeerBase::GetParamValue(const char *strParam, unsigned long &nLen)
		{
			const char *str = UNULL_PTR;
			nLen = 0;
			do
			{
				const char *strParameters = GetParams();
				if(strParameters == UNULL_PTR || strParam == UNULL_PTR)
					break;
				unsigned long len = (unsigned long)::strlen(strParam);
				if(len == 0)
					break;
				const char *p = ::strstr(strParameters, strParam);
				if(p == UNULL_PTR)
					break;
				if(p > strParameters && *(p-1) != '&')
					break;
				p += len; 
				if(strncmp(p, "=", 1))
					break;
				str = p + 1;
				const char *strEnd = ::strstr(str, "&");
				if(strEnd)
					nLen = (unsigned long)(strEnd - str);
				else
					nLen = (unsigned long)strlen(str);
			}while(false);
			return str;
		}

		const char* CHttpPeerBase::GetHeaderValue(const char *strHeaderName, unsigned long &nLen)
		{
			char *str = UNULL_PTR;
			nLen = 0;
			do
			{
				if(m_qHeader.GetSize() == 0 || strHeaderName == UNULL_PTR)
					break;
				unsigned long len = (unsigned long)::strlen(strHeaderName);
				if(len == 0)
					break;
				char *strHeaders = (char*)m_qHeader.GetBuffer();
				char *p = ::StrStrIA(strHeaders, strHeaderName);
				if(p == UNULL_PTR)
					break;
				if(p >= strHeaders + 2 && strncmp(p-2, "\r\n", 2))
					break;
				p += len;
				if(strncmp(p, ": ", 2))
					break;
				str = p + 2;
				const char *strEnd = ::strstr(str, "\r\n");
				if(strEnd)
					nLen = (unsigned long)(strEnd - str);
				else
					nLen = (unsigned long)::strlen(str);
			}while(false);
			return str;
		}

		unsigned long CHttpPeerBase::GetHeaderCount()
		{
			unsigned long nCount = 0;
			if(m_qHeader.GetSize() > 0)
			{
				const char *str;
				++nCount;
				const char *strHeaders = (const char*)m_qHeader.GetBuffer();
				str = ::strstr(strHeaders, "\r\n");
				while(str)
				{
					strHeaders = str + 2;
					str = strstr(strHeaders, "\r\n");
					++nCount;
				}
			}
			return nCount;
		}

		void CHttpPeerBase::OnSwitchFrom(unsigned long ulServiceID)
		{
			m_dVersion = 0.0;
			m_HttpRequest = hrUnknown;
			m_qHeader.SetSize(0);
			m_qQueue.SetSize(0);
		}

		const char HEX2DEC[256] = 
			{
				/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
				/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    
				/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
				/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
				/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
				/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
			};


		unsigned long  CHttpPeerBase::UriDecode(const unsigned char *strIn, unsigned long nLenIn, unsigned char *strOut)
		{
			const unsigned char *pSrc = strIn;
			unsigned long SRC_LEN = nLenIn;
			const unsigned char *SRC_END = pSrc + SRC_LEN;
			const unsigned char *SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

			unsigned char *pStart = strOut;
			unsigned char *pEnd = pStart;
			unsigned long ulGet = 0;

			while (pSrc < SRC_LAST_DEC)
			{
				++ulGet;
				if (*pSrc == '%')
				{
					char dec1, dec2;
					if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
						&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
					{
						*pEnd++ = (dec1 << 4) + dec2;
						pSrc += 3;
						continue;
					}
				}
				*pEnd++ = *pSrc++;
			}

			// the last 2- chars
			while (pSrc < SRC_END)
			{
				++ulGet;
				*pEnd++ = *pSrc++;
			}
			return ulGet;
		}

		void CHttpPeerBase::OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
		{
			bool bOk = false;
			switch(usRequestID)
			{
			case idHeader:
				do
				{
					char *str; 
					char cEnd = 0;
					m_UQueue.Push((unsigned char*)&cEnd, 1);
					str = (char*)m_UQueue.GetBuffer();
					int nPos;
					if(memcmp(str, "GET ", 4) == 0)
					{
						m_HttpRequest = hrGet;
						nPos = 4;
					}
					else if(memcmp(str, "POST ", 5) == 0)
					{
						m_HttpRequest = hrPost;
						nPos = 5;
					}
					else if(memcmp(str, "HEAD ", 5) == 0)
					{
						m_HttpRequest = hrHead;
						nPos = 5;
					}
					else if(memcmp(str, "PUT ", 4) == 0)
					{
						m_HttpRequest = hrPut;
						nPos = 4;
					}
					else if(memcmp(str, "DELETE ", 7) == 0)
					{
						m_HttpRequest = hrDelete;
						nPos = 7;
					}
					else if(memcmp(str, "OPTIONS ", 8) == 0)
					{
						m_HttpRequest = hrOptions;
						nPos = 8;
					}
					else if(memcmp(str, "TRACE ", 6) == 0)
					{
						m_HttpRequest = hrTrace;
						nPos = 6;
					}	
					else if(memcmp(str, "CONNECT ", 8) == 0)
					{
						m_HttpRequest = hrConnect;
						nPos = 8;
					}	
					else
					{
						m_HttpRequest = hrPost; //multi part header
					}

					const char *strHTTP = ::strstr(str, " HTTP/");
					if(strHTTP)
					{
						m_qQueue.SetSize(0);
						m_qQueue.Push((unsigned char*)(str + nPos), (unsigned long)(strHTTP - (str + nPos)));
						m_qHeader.SetSize(0);
						if(m_qHeader.GetMaxSize() < m_qQueue.GetSize())
							m_qHeader.ReallocBuffer(m_qQueue.GetSize());
						
						const char *strTemp = (const char*)m_qQueue.GetBuffer();
						if(::strstr(strTemp, "%") != UNULL_PTR)
						{
							unsigned long ulGet = UriDecode(m_qQueue.GetBuffer(), m_qQueue.GetSize(), (unsigned char*)m_qHeader.GetBuffer());
							m_qQueue.SetSize(0);
							m_qQueue.Push(m_qHeader.GetBuffer(), ulGet);
						}
						m_qQueue.Push((unsigned char*)&cEnd, 1); //null terminated
						
						const char *strHeader = ::strstr(str, "\r\n");
						if(!strHeader)
							break;
						m_dVersion = atof(strHTTP + 6);
						m_qHeader.SetSize(0);
						m_qHeader.Push(strHeader + 2);
						m_qHeader.Push((unsigned char*)&cEnd, 1); //null terminated
					}
					else
					{
						m_qHeader.SetSize(0);
						m_qHeader.Push(m_UQueue.GetBuffer(), m_UQueue.GetSize());
						m_qHeader.Push((unsigned char*)&cEnd, 1); //null terminated
						m_UQueue.SetSize(0);
					}
					bOk = true;
				}while(false);
				if(!bOk)
				{
					SetResponseCode(400); //400 Bad Request
					SetResponseHeader("Connection", "close");
					SendReturnData(usRequestID, UNULL_PTR, 0);
				}
				break;
			default: /*
						case idPut:
						case idDelete:
						case idOptions:
						case idTrace:
						case idMultiPart:
					 */
/*				SetResponseCode(501); //not implemented
				SetResponseHeader("Connection", "close");
				SendReturnData(usRequestID, UNULL_PTR, 0);*/
				break;
			}
			m_UQueue.SetSize(0);
		}
	} //ServerSide
#endif
} //SocketProAdapter

#pragma warning(default: 4996) // warning C4996: 'swprintf': swprintf has been changed to conform with the ISO C standard, adding an extra character count parameter. To use traditional Microsoft swprintf, set _CRT_NON_CONFORMING_SWPRINTFS.


// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com
