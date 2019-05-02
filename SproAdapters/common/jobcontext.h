#pragma once

#include "jobmanagerinterface.h"

namespace SocketProAdapter
{
namespace ClientSide{

interface class IWaitAll
{
	bool WaitUntilFinished(int nTimeout);
};

ref class CJobManager;

ref class CJobContext : public IJobContext
{
public:
	CJobContext(): m_Status(tagJobStatus::jsInitial), m_jobId(0), m_pJobManager(nullptr), 
		m_pIdentity(nullptr), m_idTask(0), m_bBundled(false), m_Callback(nullptr)
	{
		m_Tasks = gcnew CUQueue;
		m_mre = gcnew ManualResetEvent(true);
	}

	virtual ~CJobContext()
	{
		delete m_Tasks;
	}

public:
	property bool IsCompleted
	{
		virtual bool get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return (m_Status == tagJobStatus::jsInitial);
		}
	}

	property Object^ AsyncState
	{
		virtual Object^ get()
		{
			return m_pIdentity;
		}
	}

	property WaitHandle^ AsyncWaitHandle
	{
		virtual WaitHandle^ get()
		{
			return m_mre;
		}
	}

	property bool CompletedSynchronously
	{
		virtual bool get()
		{
			return false;
		}
	}

	property AsyncCallback^ Callback
	{
		virtual AsyncCallback^ get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_Callback;
		}

		virtual void set(AsyncCallback ^cb)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_Callback = cb;
		}
	}

	virtual int AddTask(short sRequestId, IntPtr pBuffer, int nSize);
	virtual bool RemoveTask(int idTask);
	virtual bool AllocateLargeMemory(long nSize);
	virtual bool Wait();
	virtual bool Wait(int nTimeout);
	property IJobManager^ JobManager
	{
		virtual IJobManager^ get()
		{
			return (IJobManager^)m_pJobManager;
		}
	}

	
	property int Size
	{
		virtual int get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_Tasks->GetSize();
		}
	}

	
	property int BufferSize
	{
		virtual int get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_Tasks->BufferSize;
		}
	}

	property __int64 JobId
	{
		virtual __int64 get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_jobId;
		}
	}
	property int CountOfTasks
	{
		virtual int get()
		{
			int	idTask;
			unsigned short usRequestId;
			unsigned long *pReqSize;
			unsigned long ulPos = 0;
			unsigned long ulCount = 0;
			CAutoLock	AutoLock(&g_cs.m_sec);
			CInternalUQueue &UQueue = *(m_Tasks->GetInternalUQueue());
			unsigned long ulSize = UQueue.GetSize();
			while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool)))
			{
				idTask = *((int*)UQueue.GetBuffer(ulPos));
				ulPos += sizeof(idTask);
				usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
				switch(usRequestId)
				{
				case idStartBatching:
				case idCommitBatching:
					break;
				default:
					++ulCount;
					break;
				}
				ulPos += sizeof(unsigned short);
				pReqSize = (unsigned long*)UQueue.GetBuffer(ulPos);
				ulPos += sizeof(unsigned long);
				ulPos += (*pReqSize);
				ulPos += sizeof(bool); //bInternal
			}
			return (int)ulCount;
		}
	}
	property tagJobStatus JobStatus
	{
		virtual tagJobStatus get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_Status;
		}
	}
	property Object^ Identity
	{
		virtual Object^ get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_pIdentity;
		}
	}
	property Dictionary<int, CTaskContext^>^ Tasks
	{
		virtual Dictionary<int, CTaskContext^>^ get()
		{
			int nTask;
			short usRequestId;
			unsigned long nReqSize;
			unsigned long ulPos = 0;
			Dictionary<int, CTaskContext^> ^mapTask = gcnew Dictionary<int, CTaskContext^>();
			CAutoLock	AutoLock(&g_cs.m_sec);
			if(m_Status == tagJobStatus::jsInitial)
				return mapTask;
			CInternalUQueue &UQueue = *(m_Tasks->GetInternalUQueue());
			unsigned long ulSize = UQueue.GetSize();
			while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool)))
			{
				nTask = *((int*)UQueue.GetBuffer(ulPos));
				ulPos += sizeof(nTask);
				usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
				ulPos += sizeof(short);
				nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos));
				ulPos += sizeof(unsigned long);
				ulPos += nReqSize;
				switch(usRequestId)
				{
				case idStartBatching:
				case idCommitBatching:
					break;
				default:
					{
						CTaskContext ^tc = gcnew CTaskContext();
						tc->m_nSize = (long)nReqSize;
						tc->m_sRequestId = (short)usRequestId;
						tc->m_bClient = *((bool*)UQueue.GetBuffer(ulPos));
						mapTask->Add(nTask, tc);
					}
					break;
				}
				ulPos += sizeof(bool); //Client request or not
			}
			return mapTask;
		}
	}
	property bool Bundled
	{
		virtual bool get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_bBundled;
		}
		virtual void set(bool b)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			if(m_Status != tagJobStatus::jsCreating && m_Status != tagJobStatus::jsQueued)
				return;
			m_bBundled = b;
		}
	}

	property SocketProAdapter::ClientSide::CAsyncServiceHandler ^Handler
	{
		virtual SocketProAdapter::ClientSide::CAsyncServiceHandler ^get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_pHandler;
		}
	}


internal:
	virtual int AddTask(short sRequestId, IntPtr pBuffer, int nSize, bool bClient);
	virtual bool RemoveTask(bool bRandomResult, unsigned short usRequestId, bool &bClient);

internal:
	bool					m_bBundled;
	int						m_idTask;
	tagJobStatus			m_Status;
	__int64					m_jobId;
	CAsyncServiceHandler	^m_pHandler;
	CJobManager				^m_pJobManager;
	Object					^m_pIdentity;
	CUQueue					^m_Tasks;
	ManualResetEvent		^m_mre;
	AsyncCallback			^m_Callback;
	static Object			^m_empty = gcnew Object();
};

}
}
