#include "StdAfx.h"
#include "jobmanager.h"
#include "jobcontext.h"


namespace SocketProAdapter
{
namespace ClientSide
{
	int CJobContext::AddTask(short sRequestId, IntPtr pBuffer, int nSize)
	{
		return AddTask(sRequestId, pBuffer, nSize, false);
	}

	int CJobContext::AddTask(short sRequestId, IntPtr pBuffer, int nSize, bool bClient)
	{
		if(sRequestId <= idPublicKeyFromSvr)
			return 0;
		if(pBuffer == IntPtr::Zero)
			nSize = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		if(m_Status == tagJobStatus::jsInitial)
			return 0;
		if(m_Status == tagJobStatus::jsRunning)
			m_Tasks->SetHeadPosition();
		++m_idTask;
		if(m_idTask == 0)
			++m_idTask;
		m_Tasks->Push(m_idTask);
		m_Tasks->Push(sRequestId);
		m_Tasks->Push(nSize);
		if(nSize > 0)
			m_Tasks->Push(pBuffer, nSize);
		m_Tasks->Push(bClient);
		if(m_Status == tagJobStatus::jsRunning)
			m_pHandler->GetAttachedClientSocket()->m_pIUFast->SendRequestEx((unsigned short)sRequestId, (unsigned long)nSize, (BYTE*)pBuffer.ToPointer());
		return m_idTask;
	}

	bool CJobContext::Wait()
	{
		return Wait((int)-1);
	}

	bool CJobContext::Wait(int nTimeout)
	{
		if(nTimeout < -1)
			nTimeout = -1;
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			if(m_Status == tagJobStatus::jsInitial)
				return true;
		}
		return m_mre->WaitOne(nTimeout, false);
	}

	bool CJobContext::AllocateLargeMemory(long nSize)
	{
		CAutoLock	AutoLock(&g_cs.m_sec);
		if((unsigned long)nSize <= m_Tasks->GetInternalUQueue()->GetMaxSize())
			return true;
		m_Tasks->GetInternalUQueue()->ReallocBuffer((unsigned long)nSize);
		return (m_Tasks->GetNativeBuffer(0) != NULL);
	}

	bool CJobContext::RemoveTask(bool bRandomResult, unsigned short usRequestId, bool &bClient)
	{
		int nIndex = 0;
		unsigned short rid;
		unsigned long nReqSize;
		unsigned long ulPos = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		if(m_Status != tagJobStatus::jsRunning)
			return false;
		CInternalUQueue &UQueue = *(m_Tasks->GetInternalUQueue());
		unsigned long ulSize = UQueue.GetSize();
		while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool)))
		{
			rid = *((unsigned short*)UQueue.GetBuffer(ulPos + sizeof(unsigned int)));
			ATLASSERT(rid > 0);
			switch(rid)
			{
			case idStartBatching:
				break;
			case idCommitBatching:
				break;
			default:
				if(rid == usRequestId)
				{
					nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos + sizeof(unsigned int) + sizeof(unsigned short)));
					nReqSize += (sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long));
					UQueue.Pop(nReqSize, ulPos);
					UQueue.Pop((unsigned char*)&bClient, sizeof(bClient), ulPos);
					return true;
				}
				else if(!bRandomResult)
				{
					return false;
				}
				break;
			}
			if(rid == idStartBatching || rid == idCommitBatching)
			{
				nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos + sizeof(unsigned int) + sizeof(unsigned short)));
				nReqSize += (sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool));
				UQueue.Pop(nReqSize, ulPos);
				ulSize -= nReqSize;
				continue;
			}
			ulPos += sizeof(int); //task id
			ulPos += sizeof(unsigned short);
			nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos));
			ulPos += sizeof(unsigned long);
			ulPos += nReqSize;
			ulPos += sizeof(bool); //Client request or not
		}
		ATLASSERT(ulSize == ulPos);
		return false;
	}

	bool CJobContext::RemoveTask(int idTask)
	{
		unsigned short usRequestId;
		unsigned long nReqSize;
		if(idTask == 0)
			return false;
		unsigned long ulPos = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		if(m_Status == tagJobStatus::jsRunning || m_Status == tagJobStatus::jsInitial)
			return false;
		CInternalUQueue &UQueue = *(m_Tasks->GetInternalUQueue());
		unsigned long ulSize = UQueue.GetSize();
		while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool)))
		{
			int nTask = *((int*)UQueue.GetBuffer(ulPos));
			if(nTask == idTask)
			{
				bool bClient = false;
				UQueue.Pop(sizeof(unsigned int), ulPos);
				UQueue.Pop((unsigned char*)&usRequestId, sizeof(unsigned short), ulPos);
				UQueue.Pop((unsigned char*)&nReqSize, sizeof(nReqSize), ulPos);
				UQueue.Pop(nReqSize, ulPos);
				UQueue.Pop((unsigned char*)&bClient, sizeof(bClient), ulPos); //Client request or not
				if(bClient && m_pJobManager->m_bServerLoadingBalance)
				{
					ServerSide::CClientPeer ^p = (ServerSide::CClientPeer^)m_pIdentity;
					if(p != nullptr)
						p->DropRequestResult((short)usRequestId);
				}
				return true;
			}
			ulPos += sizeof(idTask);
			usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
			ulPos += sizeof(unsigned short);
			nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos));
			ulPos += sizeof(unsigned long);
			ulPos += nReqSize;
			ulPos += sizeof(bool); //Client request or not
		}
		return false;
	}

}

}