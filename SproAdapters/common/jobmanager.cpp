#include "StdAfx.h"
#include "jobmanager.h"

namespace SocketProAdapter
{
namespace ClientSide
{
	CJobManager::CJobManager() : m_jobIndex(0), m_pIUSocketPool(NULL), m_upc(nullptr), m_bServerLoadingBalance(false), m_nRecycleBinSize(10)
	{
		m_mapQueuedIdentityJob = gcnew Dictionary<Object^, List<CJobContext^>^>();
		m_aJobEmpty = gcnew List<CJobContext^>();
		m_aJobProcessing = gcnew List<CJobContext^>();
		m_lstAsyncHandler = gcnew List<CAsyncServiceHandler^>();
		m_aLockedIdentity = gcnew List<Object^>();
		m_hIdentityWait = ::CreateEvent(NULL, TRUE, TRUE, NULL);
		m_mapHandleJobs = gcnew Dictionary<ManualResetEvent^, List<ManualResetEvent^>^>();
	}

	CJobManager::~CJobManager()
	{
		CJobContext ^job;
		CAutoLock	AutoLock(&g_cs.m_sec);
		::SetEvent(m_hIdentityWait);
		::Sleep(0);
		::CloseHandle(m_hIdentityWait);
		for each (List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
		{
			for each(job in jobs)
			{
				delete job;
			}
		}
		m_mapQueuedIdentityJob->Clear();

		for each(job in m_aJobEmpty)
		{
			delete job;
		}
		m_aJobEmpty->Clear();

		for each(job in m_aJobProcessing)
		{
			delete job;
		}
		m_aJobProcessing->Clear();
	}

	void CJobManager::CleanTasks(bool bSoft)
	{
		CJobContext ^job;
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
		{
			for each (job in jobs)
			{
				EmptyJob(job);
			}
		}
		m_mapQueuedIdentityJob->Clear();
		if(!bSoft)
		{
			for each(job in m_aJobProcessing)
			{
				EmptyJob(job);
			}
			m_aJobProcessing->Clear();
		}
	}

	void CJobManager::MyCancel(CAsyncServiceHandler ^pHandler)
	{
		pHandler->GetAttachedClientSocket()->Cancel();
		pHandler->GetAttachedClientSocket()->GetUSocket()->DoEcho();
	}

	void CJobManager::RemoveTasksInternally(CJobContext ^pJC)
	{
		if(pJC != nullptr && m_bServerLoadingBalance)
		{
			Dictionary<int, CTaskContext^>	^myTasks = pJC->Tasks;
			for each(int nTask in myTasks->Keys)
			{
				pJC->RemoveTask(nTask);
			}
		}
	}

	void CJobManager::EmptyJob(CJobContext ^pJC)
	{
		Object ^obj = nullptr;
		RemoveTasksInternally(pJC);
		pJC->m_pHandler = (CAsyncServiceHandler^)obj;
		pJC->m_Status = tagJobStatus::jsInitial;
		pJC->m_Tasks->SetSize(0);
		pJC->m_pIdentity = nullptr;
		pJC->m_Callback = nullptr;
		pJC->m_mre->Set();
		m_aJobEmpty->Add(pJC);
		for each(ManualResetEvent ^hEvent in m_mapHandleJobs->Keys)
		{
			List<ManualResetEvent^> ^lstHandle = m_mapHandleJobs[hEvent];
			for each(ManualResetEvent ^h in lstHandle)
			{
				if(h == pJC->m_mre)
				{
					hEvent->Set();
					return;
				}
			}
		}
	}

	IJobContext^ CJobManager::CreateJob()
	{
		return CreateJob(nullptr, nullptr);
	}
	IJobContext^ CJobManager::CreateJob(Object ^Identity)
	{
		return CreateJob(Identity, nullptr);
	}

	IJobContext^ CJobManager::CreateJob(Object ^Identity, AsyncCallback ^cb)
	{
		CJobContext ^pJob;
		if(Identity == nullptr)
		{
			Identity = CJobContext::m_empty;
		}
		int nIndex = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		++m_jobIndex;
		for each (pJob in m_aJobEmpty)
		{
			if(pJob->m_Status == tagJobStatus::jsInitial)
			{
				if((m_aJobEmpty->Count - nIndex - 1) < m_nRecycleBinSize)
					break;
				pJob->m_jobId = m_jobIndex;
				pJob->m_Status = tagJobStatus::jsCreating;
				pJob->m_pIdentity = Identity;
				pJob->m_Callback = cb;
				pJob->m_mre->Reset();
				return pJob;
			}
			nIndex++;
		}
		pJob = gcnew CJobContext();
		m_aJobEmpty->Add(pJob);
		pJob->m_pJobManager = this;
		pJob->m_jobId = m_jobIndex;
		pJob->m_Status = tagJobStatus::jsCreating;
		pJob->m_pIdentity = Identity;
		pJob->m_Callback = cb;
		pJob->m_mre->Reset();
		return pJob;
	}

	bool CJobManager::EnqueueJob(IJobContext^ JobContext)
	{
		if(JobContext == nullptr || JobContext->JobStatus != tagJobStatus::jsCreating)
			return false;
		CAutoLock	AutoLock(&g_cs.m_sec);
		if(m_pIUSocketPool == NULL)
			return false;
		CJobContext ^pJC = (CJobContext^)JobContext;
		if(m_aJobEmpty->Remove(pJC))
		{
			long lConnected = 0;
			m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
			if(pJC->CountOfTasks > 0 && lConnected > 0)
			{
				pJC->m_Status = tagJobStatus::jsQueued;
				List<CJobContext^> ^jobs;
				if(!m_mapQueuedIdentityJob->ContainsKey(pJC->Identity))
				{
					jobs = gcnew List<CJobContext^>();
					m_mapQueuedIdentityJob->Add(pJC->Identity, jobs);
				}
				else
				{
					jobs = m_mapQueuedIdentityJob[pJC->Identity];
				}
				jobs->Add(pJC);
				return true;
			}
			else
			{
				EmptyJob(pJC);
				return false;
			}
		}
		return false;
	}

	bool CJobManager::DestroyJob(IJobContext^ JobContext)
	{
		bool b;
		Dictionary<int, CTaskContext^> ^tasks;
		if(JobContext == nullptr)
			return false;
		bool bSuc = false;
		CAutoLock	AutoLock(&g_cs.m_sec);
		CJobContext ^pJob = (CJobContext ^)JobContext;
		switch(pJob->m_Status)
		{
		case tagJobStatus::jsCreating:
			tasks = pJob->Tasks;
			b = true;
			for each (CTaskContext ^tc in tasks->Values)
			{
				if(tc->m_bClient)
				{
					b = false;
					break;
				}
			}
			if(b)
			{
				pJob->m_Status = tagJobStatus::jsInitial;
				pJob->m_pIdentity = nullptr;
				pJob->m_Tasks->SetSize(0);
				pJob->m_Callback = nullptr;
				pJob->m_mre->Set();
				bSuc = true;
			}
			break;
		case tagJobStatus::jsQueued:
			bSuc = CancelJob(pJob->m_jobId);
			break;
		case tagJobStatus::jsRunning:
		case tagJobStatus::jsInitial:
			//can't do anything
			break;
		default:
			break;
		}
		return bSuc;
	}

	int CJobManager::CancelJobs(Object^ Identity)
	{
		CJobContext ^pJC;
		if(Identity == nullptr)
			Identity = CJobContext::m_empty;
		int nJobs = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(Object ^obj in m_mapQueuedIdentityJob->Keys)
		{
			if(obj != Identity)
				continue;
			for each(pJC in m_mapQueuedIdentityJob[obj])
			{
				EmptyJob(pJC);
				nJobs++;
			}
			m_mapQueuedIdentityJob->Remove(obj);
			break;
		}
		for each (pJC in m_aJobProcessing)
		{
			if(pJC->Identity != Identity)
				continue;
			if(pJC->m_Status != tagJobStatus::jsRunning)
				continue;
			pJC->m_Status = tagJobStatus::jsInitial;
			MyCancel(pJC->m_pHandler);
			nJobs++;
		}
		return nJobs;
	}

	bool CJobManager::CancelJob(__int64 jobId)
	{
		CAutoLock	AutoLock(&g_cs.m_sec);
		CJobContext ^pJC = (CJobContext^)SeekJob(jobId);
		if(pJC == nullptr)
			return false;
		if(pJC->m_Status == tagJobStatus::jsQueued)
		{
			for each (List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
			{
				for each (CJobContext ^job in jobs)
				{
					if(job == pJC)
					{
						EmptyJob(pJC);
						jobs->Remove(job);
						if(jobs->Count == 0)
							m_mapQueuedIdentityJob->Remove(job->m_pIdentity);
						return true;
					}
				}
			}
		}
		else if(pJC->m_Status == tagJobStatus::jsRunning)
		{
			pJC->m_Status = tagJobStatus::jsInitial;
			MyCancel(pJC->m_pHandler);
			return true;
		}
		return false;
	}

	int CJobManager::ResetPosition(__int64 jobId, int nNewPosition)
	{
		if(nNewPosition < 0)
			nNewPosition = 0;
		CAutoLock	AutoLock(&g_cs.m_sec);
		CJobContext ^job = (CJobContext ^)SeekJob(jobId);
		if(job == nullptr || job->m_Status != tagJobStatus::jsQueued)
			return -1;
		List<CJobContext^> ^jobs = m_mapQueuedIdentityJob[job->m_pIdentity];
		jobs->Remove(job);
		if(nNewPosition >= jobs->Count)
		{
			jobs->Add(job);
			return (jobs->Count - 1);
		}
		jobs->Insert(nNewPosition, job);
		return nNewPosition;
	}

	IJobContext^ CJobManager::SeekJob(__int64 jobId)
	{
		CJobContext ^job;
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each (List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
		{
			for each (job in jobs)
			{
				if(job->m_jobId == jobId)
					return job;
			}
		}

		for each(job in m_aJobProcessing)
		{
			if(job->m_Status != tagJobStatus::jsRunning)
				continue;
			if(job->m_jobId == jobId)
				return job;
		}
		return nullptr;
	}

	void CJobManager::ShrinkMemory()
	{
		ShrinkMemory(DEFAULT_UQUEUE_BLOCK_SIZE);
	}

	bool CJobManager::WaitAll()
	{
		return WaitAll((int)-1);
	}

	bool CJobManager::WaitAll(int nTimeout)
	{
		return m_pIWaitAll->WaitUntilFinished(nTimeout);
	}

	bool CJobManager::Wait(array<__int64>^ jobs)
	{
		return Wait(jobs, (int)-1);
	}

	bool CJobManager::WaitAny(array<__int64> ^jobs, List<__int64> ^%completedJobs)
	{
		return WaitAny(jobs, (int)-1, completedJobs);
	}

	bool CJobManager::WaitAny(array<__int64> ^jobs, int nTimeout, List<__int64> ^%completedJobs)
	{
		int n;
		__int64 jobId;
		int dwRtn;
		int nCount;
		bool bSuc = true;
		if(completedJobs == nullptr)
			completedJobs = gcnew List<__int64>();
		else
			completedJobs->Clear();
		if(jobs == nullptr || jobs->Length == 0)
			return true;
		nCount = (unsigned int)jobs->Length;
		ManualResetEvent ^temp = nullptr;
		List<ManualResetEvent^> ^lstHandle = gcnew List<ManualResetEvent^>();
		Dictionary<ManualResetEvent^, __int64> mapEventJobId;
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			for(n=0; n<nCount; n++)
			{
				jobId = jobs[n];
				CJobContext ^pJob = (CJobContext^)SeekJob(jobId);
				if(pJob == nullptr || pJob->m_Status == tagJobStatus::jsInitial)
				{
					completedJobs->Add(jobId);
					continue;
				}
				ManualResetEvent ^hEvent = pJob->m_mre;
				switch(pJob->m_Status)
				{
				case tagJobStatus::jsCreating:
				case tagJobStatus::jsQueued:
				case tagJobStatus::jsRunning:
					mapEventJobId.Add(hEvent, jobId);
					lstHandle->Add(hEvent);
					break;
				default:
					ATLASSERT(FALSE);
					break;
				}
			}
		}
		n = lstHandle->Count;
		if(n == 0)
			return true;
		if(n <= MAXIMUM_WAIT_OBJECTS)
		{
			array<ManualResetEvent^> ^waitHandles = lstHandle->ToArray();
			dwRtn = System::Threading::WaitHandle::WaitAny(waitHandles, nTimeout, false);
			if(dwRtn >= n) //
			{
				if(completedJobs->Count == 0)
					bSuc = false;
			}
		}
		else
		{
			temp = gcnew ManualResetEvent(false);
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_mapHandleJobs->Add(temp, lstHandle);
			}
			if(nTimeout < -1)
				nTimeout = -1;
			if(!temp->WaitOne(nTimeout, false))
			{
				if(completedJobs->Count == 0)
					bSuc = false;
			}
		}
		
		if(bSuc)
		{
			nCount = lstHandle->Count;
			CAutoLock	AutoLock(&g_cs.m_sec);
			for(n=0; n<nCount; n++)
			{
				ManualResetEvent ^h = lstHandle[n];
				jobId = mapEventJobId[h];
				CJobContext ^pJob = (CJobContext^)SeekJob(jobId);
				if(pJob == nullptr || pJob->m_Status == tagJobStatus::jsInitial)
				{
					completedJobs->Add(jobId);
				}
			}
		}

		if(temp != nullptr)
		{
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_mapHandleJobs->Remove(temp);
			}
			delete temp;
		}
		delete lstHandle;
		return bSuc;
	}

	bool CJobManager::WaitAny(Object ^Identity, List<__int64> ^%jobsCompleted)
	{
		return WaitAny(GetJobs(Identity)->ToArray(), (int)-1, jobsCompleted);
	}

	bool CJobManager::WaitAny(Object ^Identity, int nTimeout, List<__int64> ^%completedJobs)
	{
		return WaitAny(GetJobs(Identity)->ToArray(), nTimeout, completedJobs);
	}

	bool CJobManager::Wait(array<__int64>^ jobs, int nTimeout)
	{
		if(jobs == nullptr)
			return true;
		unsigned int ulTimeout = (unsigned int)nTimeout;
		unsigned int ulMyTimeout = ulTimeout;
		unsigned long ulStart = ::GetTickCount();
		for each(__int64 jobId in jobs)
		{
			IJobContext ^pIJobContext = SeekJob(jobId);
			if(pIJobContext == nullptr)
				continue;
			int nMyTimeout = (int)ulMyTimeout;
			if(!pIJobContext->Wait(nMyTimeout))
				return false;
			unsigned long ulNow = ::GetTickCount();
			if(ulNow > ulStart)
				ulMyTimeout = ulTimeout - (ulNow - ulStart);
			if(ulMyTimeout > ulTimeout)
				ulMyTimeout = 0;
		}
		return true;
	}

	bool CJobManager::Wait(Object ^Identity)
	{
		return Wait(Identity, (int)-1);
	}

	bool CJobManager::Wait(Object ^Identity, int nTimeout)
	{
		int nSize;
		unsigned int ulTimeout = (unsigned int)nTimeout;
		unsigned int ulStart = ::GetTickCount();
		unsigned int ulMyTimeout = ulTimeout;
		List<__int64> ^jobs;
		do
		{
			jobs = GetJobs(Identity);
			nSize = jobs->Count;
			if(nSize == 0)
				break;
			IJobContext ^jb = SeekJob(jobs[nSize-1]);
			if(jb == nullptr)
				break;
			int nMyTimeout = (int)ulMyTimeout;
			if(!jb->Wait(nMyTimeout))
				return false;
			unsigned long ulNow = ::GetTickCount();
			if(ulNow > ulStart)
				ulMyTimeout = ulTimeout - (ulNow - ulStart);
			if(ulMyTimeout > ulTimeout)
				ulMyTimeout = 0;
		}while(true);
		return true;
	}

	void CJobManager::UnlockIdentity(CAsyncServiceHandler ^Identity)
	{
		if(Identity == nullptr)
			return;
		CAutoLock	AutoLock(&g_cs.m_sec);
		if(m_aLockedIdentity->Remove(Identity))
			::SetEvent(m_hIdentityWait);
	}

	CAsyncServiceHandler^ CJobManager::LockIdentity()
	{
		return LockIdentity((int)-1);
	}

	CAsyncServiceHandler^ CJobManager::LockIdentity(int nTimeout)
	{
		unsigned long ulStart = ::GetTickCount();
		unsigned long ulTimeout = (unsigned long)nTimeout;
ao:		long lConnected = 0;
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
			if(lConnected == 0)
				return nullptr;
			for each(CAsyncServiceHandler ^p in m_lstAsyncHandler)
			{
				if(!p->GetAttachedClientSocket()->IsConnected())
					continue;
				do
				{
					if(m_mapQueuedIdentityJob->ContainsKey(p))
						break;
					int j, jSize = m_aJobProcessing->Count;
					for(j=0; j<jSize; j++)
					{
						CJobContext ^jb = m_aJobProcessing[j];
						if(jb->m_pIdentity == p)
							break;
					}
					if(jSize && j != jSize)
						break;
					jSize = m_aLockedIdentity->Count;
					for(j=0; j<jSize; j++)
					{
						if(p == m_aLockedIdentity[j])
							break;
					}
					if(jSize && j != jSize)
						break;
					jSize = m_aJobEmpty->Count;
					for(j=0; j<jSize; j++)
					{
						CJobContext ^jb = m_aJobEmpty[j];
						if(jb->m_pIdentity == p)
							break;
					}
					if(jSize && j != jSize)
						break;
					m_aLockedIdentity->Add((Object^)p);
					return p;
				}while(false);
			}
			::ResetEvent(m_hIdentityWait);
		}
		DWORD dwRtn = ::WaitForSingleObject(m_hIdentityWait, ulTimeout);
		if(dwRtn == WAIT_OBJECT_0)
		{
			unsigned long ulNow = ::GetTickCount();
			if(ulNow > ulStart)
				ulTimeout -= (ulNow - ulStart);
			if(ulTimeout > (unsigned long)nTimeout)
				ulTimeout = 0;
			goto ao;
		}
		return nullptr; //all of async handlers are already associated with jobs.
	}

	List<__int64>^ CJobManager::GetJobs(Object ^Identity)
	{
		CJobContext ^jb;
		if(Identity == nullptr)
			Identity = CJobContext::m_empty;

		List<__int64> ^jobs = gcnew List<__int64>();
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(jb in m_aJobProcessing)
		{
			if(jb->m_pIdentity != Identity)
				continue;
			if(jb->m_Status != tagJobStatus::jsRunning)
				continue;
			jobs->Add(jb->m_jobId);
		}

		for each (Object ^pIdentity in m_mapQueuedIdentityJob->Keys)
		{
			if(pIdentity == Identity)
			{
				List<CJobContext^> ^jbs = m_mapQueuedIdentityJob[pIdentity];
				for each (jb in jbs)
				{
					ATLASSERT(jb->m_Status == tagJobStatus::jsQueued);
					jobs->Add(jb->m_jobId);
				}
			}
		}

		for each(jb in m_aJobEmpty)
		{
			if(jb->m_pIdentity == Identity && jb->m_Status == tagJobStatus::jsCreating)
				jobs->Add(jb->m_jobId);
		}
		return jobs;
	}

	void CJobManager::ShrinkMemory(int nMemoryChunkSize)
	{
		if(nMemoryChunkSize < DEFAULT_UQUEUE_BLOCK_SIZE)
			nMemoryChunkSize = DEFAULT_UQUEUE_BLOCK_SIZE;
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(CJobContext ^pJC in m_aJobEmpty)
		{
			if(pJC->m_Status == tagJobStatus::jsInitial && (unsigned int)pJC->m_Tasks->BufferSize > (unsigned int)nMemoryChunkSize)
				pJC->m_Tasks->ReallocBuffer(nMemoryChunkSize);
		}
	}

	tagJobStatus CJobManager::GetJobStatus(__int64 jobId)
	{
		CJobContext ^pJob;
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(pJob in m_aJobProcessing)
		{
			if(pJob->m_jobId == jobId)
				return pJob->m_Status;
		}

		for each (List<CJobContext^> ^jobs in m_mapQueuedIdentityJob->Values)
		{
			for each (pJob in jobs)
			{
				if(pJob->m_jobId == jobId)
					return pJob->m_Status;
			}
		}

		for each(pJob in m_aJobEmpty)
		{
			if(pJob->m_jobId == jobId && pJob->m_Status == tagJobStatus::jsCreating)
				return tagJobStatus::jsCreating;
		}
		return tagJobStatus::jsInitial;
	}
}



}