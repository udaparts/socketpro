#pragma once

#include "muqueue.h"
#include "jobcontext.h"
#include "usocket.h"

namespace SocketProAdapter
{
namespace ClientSide
{

ref class CJobManager : public IJobManager
{
public:
	CJobManager();
	virtual ~CJobManager();

internal:
	void CleanTasks(bool bSoft);
	void EmptyJob(CJobContext ^pJC);

private:
	void RemoveTasksInternally(CJobContext ^pJC);
	static void MyCancel(CAsyncServiceHandler ^pHandler);

public:
	virtual IJobContext^ CreateJob();
	virtual IJobContext^ CreateJob(Object ^Identity);
	virtual IJobContext^ CreateJob(Object ^Identity, AsyncCallback ^cb);
	virtual bool EnqueueJob(IJobContext^ JobContext);
	virtual bool DestroyJob(IJobContext^ JobContext);
	virtual int CancelJobs(Object^ Identity);
	virtual bool CancelJob(__int64 jobId);
	virtual IJobContext^ SeekJob(__int64 jobId);
	virtual int ResetPosition(__int64 jobId, int nNewPosition);
	virtual void ShrinkMemory();
	virtual void ShrinkMemory(int nMemoryChunkSize);
	virtual tagJobStatus GetJobStatus(__int64 jobId);
	virtual bool WaitAll();
	virtual bool WaitAll(int nTimeout);
	virtual List<__int64>^ GetJobs(Object ^Identity);
	virtual CAsyncServiceHandler^ LockIdentity();
	virtual CAsyncServiceHandler^ LockIdentity(int nTimeout);
	virtual void UnlockIdentity(CAsyncServiceHandler ^Identity);
	virtual bool Wait(Object ^Identity);
	virtual bool Wait(Object ^Identity, int nTimeout);
	virtual bool Wait(array<__int64>^ jobs);
	virtual bool Wait(array<__int64>^ jobs, int nTimeout);
	virtual bool WaitAny(array<__int64> ^jobs, int nTimeout, List<__int64> ^%completedJobs);
	virtual bool WaitAny(Object ^Identity, int nTimeout, List<__int64> ^%completedJobs);
	virtual bool WaitAny(array<__int64> ^jobs, List<__int64> ^%completedJobs);
	virtual bool WaitAny(Object ^Identity, List<__int64> ^%jobsCompleted);

	property List<Object^>^ Identities
	{
		virtual List<Object^>^ get()
		{
			List<Object^> ^lst = gcnew List<Object^>();
			CAutoLock	AutoLock(&g_cs.m_sec);
			for each(CJobContext ^jb in m_aJobProcessing)
			{
				if(!lst->Contains(jb->m_pIdentity))
					lst->Add(jb->m_pIdentity);
			}

			for each(Object^ identity in m_mapQueuedIdentityJob->Keys)
			{
				if(!lst->Contains(identity))
					lst->Add(identity);
			}
			return lst;
		}
	}

	property int CountOfJobs
	{
		virtual int get()
		{
			int nSize = 0;
			CAutoLock	AutoLock(&g_cs.m_sec);
			for each(CJobContext ^jb in m_aJobProcessing)
			{
				if(jb->m_Status == tagJobStatus::jsRunning)
					nSize++;
			}
			return (nSize + CountOfQueuedJobs);
		}
	}

	property int CountOfQueuedJobs
	{
		virtual int get()
		{
			int nSize = 0;
			CAutoLock	AutoLock(&g_cs.m_sec);
			for each(List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
			{
				nSize += jobs->Count;
			}
			return nSize;
		}
	}
	
	property __int64 MemoryConsumed
	{
		virtual __int64 get()
		{
			CJobContext ^job;
			__int64 nSize = 0;
			CAutoLock	AutoLock(&g_cs.m_sec);
			for each(List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
			{
				for each (job in jobs)
				{
					nSize += job->m_Tasks->BufferSize;
				}
			}

			for each (job in m_aJobEmpty)
			{
				nSize += job->m_Tasks->BufferSize;
			}

			for each (job in m_aJobProcessing)
			{
				nSize += job->m_Tasks->BufferSize;
			}

			return nSize;
		}
	}

	property USOCKETLib::USocketPoolClass^ SocketPool
	{
		virtual USOCKETLib::USocketPoolClass^ get()
		{
			return m_upc;
		}
	}

	property int RecycleBinSize
	{
		virtual int get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_nRecycleBinSize;
		}

		virtual void set(int nSize)
		{
			if(nSize < 0 || nSize > 10240)
				nSize = 10240;
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_nRecycleBinSize = nSize;
		}
	}

internal:
	Dictionary<ManualResetEvent^, List<ManualResetEvent^>^>	^m_mapHandleJobs;
	HANDLE										m_hIdentityWait;
	List<CAsyncServiceHandler^>				^m_lstAsyncHandler;
	List<Object^>								^m_aLockedIdentity;
	bool										m_bServerLoadingBalance;
	__int64										m_jobIndex;
	Dictionary<Object^, List<CJobContext^>^>	^m_mapQueuedIdentityJob;
	List<CJobContext^>							^m_aJobEmpty;
	List<CJobContext^>							^m_aJobProcessing;
	IUSocketPool								*m_pIUSocketPool;				
	USOCKETLib::USocketPoolClass				^m_upc;
	IWaitAll									^m_pIWaitAll;
	int											m_nRecycleBinSize;
};

}
}
