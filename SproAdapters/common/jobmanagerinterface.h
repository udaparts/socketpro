#ifndef __SOCKETPROADAPTER_JOB_CONTExt_MANAGER_INTERFACE_H__
#define __SOCKETPROADAPTER_JOB_CONTExt_MANAGER_INTERFACE_H__


namespace SocketProAdapter
{
	[Serializable]
	ref class CRet
	{
	public:
		String ^method;
		Object ^ret;
	};

	[CLSCompliantAttribute(true)] 
	public enum class tagJobStatus : int
	{
		/// <summary>
		/// Job in recycle pool for reuse. 
		/// </summary>
		jsInitial = 0,

		/// <summary>
		/// Job in creating state.
		/// </summary>
		jsCreating = 1,

		/// <summary>
		/// Job queued but not in processing.
		/// </summary>
		jsQueued = 2,

		/// <summary>
		/// Job under processing.
		/// </summary>
		jsRunning = 3,
	};

	interface class IProcessBySocketPoolEx
	{
		bool Process();
	};
	
	[CLSCompliantAttribute(true)] 
	public ref class CTaskContext
	{
	public:
		CTaskContext()
		{
			m_sRequestId = 0;
			m_nSize = 0;
			m_bClient = false;
		}

		/// <summary>
		/// A request identification number.
		/// </summary>
		property short RequestId
		{
			short get()
			{
				return m_sRequestId;
			}
		}

		/// <summary>
		/// A request input parameters size in byte.
		/// </summary>
		property int Size
		{
			int get()
			{
				return m_nSize;
			}

		}

		/// <summary>
		/// A boolean indicating if the request originally comes from a remote client.
		/// </summary>
		property bool Client
		{
			bool get()
			{
				return m_bClient;
			}
		}

	internal:
		short	m_sRequestId;	//request id
		long	m_nSize;		//request input parameters size in byte
		bool	m_bClient;		//Client request
	};
	/// <summary>
	/// An interface for sending chat messages. 
	/// Note that all of methods are thread-safe.
	/// </summary>
	[CLSCompliantAttribute(true)] 
	public interface class IUPush
	{
		/// <summary>
		/// Enter an array of chat groups (Groups). The method returns true if successful.
		/// </summary>
		bool Enter(array<int> ^Groups);

		/// <summary>
		/// Send a message (Message) onto an array of chat groups (Groups) of clients. The method returns true if successful.
		/// </summary>
		bool Broadcast(Object ^Message, array<int> ^Groups);

		/// <summary>
		/// Send an array of bytes (Message) onto an array of chat groups (Groups) of clients. The method returns true if successful.
		/// </summary>
		bool Broadcast(array<unsigned char> ^Message, array<int> ^Groups);
		
		/// <summary>
		/// Send a message (Message) onto one client identified with user id (UserId). The method returns true if successful.
		/// </summary>
		bool SendUserMessage(Object ^Message, String ^UserId);
		
		/// <summary>
		/// Send an array of bytes (Message) onto one client identified with user id (UserId). The method returns true if successful.
		/// </summary>
		bool SendUserMessage(String ^UserId, array<unsigned char> ^Message);

		/// <summary>
		/// Leave chat groups. The method returns true if successful.
		/// </summary>
		bool Exit();
	};

	interface class IJobManager;
	/// <summary>
	/// An interface for managing jobs, and each of them may contain many tasks. 
	/// Note that all of methods and properties are thread-safe.
	/// </summary>
	[CLSCompliantAttribute(true)] 
	public interface class IJobContext : public IAsyncResult
	{
		/// <summary>
		/// Add a task into the job. It returns a non-zero task id if successful.
		/// A task consists of a request id (sRequestId), a buffer (pBuffer), and its size (nSize).
		/// </summary>
		int AddTask(short sRequestId, IntPtr pBuffer, int nSize);
		
		/// <summary>
		/// Remove a task specified by idTask. It returns true if successful.
		/// </summary>
		bool RemoveTask(int idTask);
		
		/// <summary>
		/// Wait infinitely until this job is finished or canceled. This method is preferred over the property AsyncWaitHandle.
		/// Don't call this method from the virtual functions OnFailover, OnJobDone, OnExecutingJob, OnJobProcessing, OnReturnedResultProcessed and OnSocketPoolEvent. Otherwise, there maybe is a dead lock.
		/// </summary>
		bool Wait();
		
		/// <summary>
		/// Wait until this job is finished or canceled or timed out within a given timeout (nTimeout). This method is preferred over the property AsyncWaitHandle.
		/// The method returns false only if the job is timed out. If nTimeout equals to -1, the method waits infinitely until the job is either finished or canceled. 
		/// Don't call this method from the virtual functions OnFailover, OnJobDone, OnExecutingJob, OnJobProcessing, OnReturnedResultProcessed and OnSocketPoolEvent. Otherwise, there maybe is a dead lock if nTimeout is large.
		/// </summary>
		bool Wait(int nTimeout);
		
		/// <summary>
		/// Allocate a large memory chunk for this job. If successful, it returns true.
		/// If nSize is less than zero or very large, the method may be fail or cause other unknown problems.
		/// You must be very careful to allocate a large memory chunk.
		/// </summary>
		bool AllocateLargeMemory(long nSize);
		
		/// <summary>
		/// A property indicating a reference to a job manager which creates this job.
		/// </summary>
		property IJobManager^ JobManager
		{
			IJobManager^ get();
		}
		
		/// <summary>
		/// A property indicating job id.
		/// </summary>
		property __int64 JobId
		{
			__int64 get();
		}
		
		/// <summary>
		/// The number of tasks in a job.
		/// </summary>
		property int CountOfTasks
		{
			int get();
		}
		
		/// <summary>
		/// The job status. Note that the property may indicate the order of queued jobs to be processed.
		/// </summary>
		property tagJobStatus JobStatus
		{
			tagJobStatus get();
		}
		
		/// <summary>
		/// An identity object associated with this job. 
		/// Note that SocketPro loading balance round robin computation is dependent on this property.
		/// </summary>
		property Object^ Identity
		{
			Object^ get();
		}
		
		/// <summary>
		/// A collection of the pairs of task id and its request context.
		/// </summary>
		property Dictionary<int, CTaskContext^>^ Tasks
		{
			Dictionary<int, CTaskContext^>^ get();
		}
		
		/// <summary>
		/// A property indicating if all of tasks are bundled. 
		/// If all of tasks are bundled, all of tasks are passed from one loading balancer to another one. As usual, you should not set it to true.
		/// The property defaults to false.
		/// </summary>
		property bool Bundled
		{
			bool get();
			void set(bool b);
		}

		/// <summary>
		/// A property indicating the size of tasks in byte.
		/// </summary>
		property int Size
		{
			int get();
		}

		/// <summary>
		/// A property indicating the size of buffer used by this job in byte.
		/// </summary>
		property int BufferSize
		{
			int get();
		}
		
		/// <summary>
		/// A property indicating a callback. If it is not null or nothing, there will be a callback from a job when completed.
		/// </summary>
		property AsyncCallback^ Callback
		{
			AsyncCallback^ get();
			void set(AsyncCallback ^cb);
		}
		
		/// <summary>
		/// A property for an associated aysnc handler. Note that the property will be null or nothing if the job status is not in running.
		/// </summary>
		property SocketProAdapter::ClientSide::CAsyncServiceHandler ^Handler
		{
			SocketProAdapter::ClientSide::CAsyncServiceHandler ^get();
		}
	};

	/// <summary>
	/// An interface for managing a set of jobs. It is important that you should not call any one of IJobManager::Wait(Any or All) methods from the functions OnFailover, OnJobDone, OnExecutingJob, OnJobProcessing, OnReturnedResultProcessed and OnSocketPoolEvent. 
	/// Otherwise, maybe there is a dead lock if timeout is large.
	/// Note that all of methods and properties are thread-safe.
	/// </summary>
	[CLSCompliantAttribute(true)] 
	public interface class IJobManager
	{
		/// <summary>
		/// Create a job without associating it with an identity object or setting a callback. If successful, it returns an interface to a job.
		/// After getting an interface to a job, you must call its method either EnqueueJob or DestroyJob. 
		/// Without doing so, it may lead to memory leak or undesired effect.
		/// </summary>
		IJobContext^ CreateJob();

		/// <summary>
		/// Create a job and associate it with an identity object (Identity) without setting a callback. If successful, it returns an interface to a job.
		/// After getting an interface to a job, you must call the method either EnqueueJob or DestroyJob. 
		/// Without doing so, it may lead to memory leak or undesired effect.
		/// Note that SocketPro loading balance round robin computation is dependent on the given identity.
		/// </summary>
		IJobContext^ CreateJob(Object ^Identity);

		/// <summary>
		/// Create a job and associate it with an identity object (Identity) with a given callback (cb). If successful, it returns an interface to a job.
		/// After getting an interface to a job, you must call the method either EnqueueJob or DestroyJob. 
		/// Without doing so, it may lead to memory leak.
		/// Note that SocketPro loading balance round robin computation is dependent on the given identity.
		/// </summary>
		IJobContext^ CreateJob(Object ^Identity, AsyncCallback ^cb);

		/// <summary>
		/// Enqueue a job (JobContext). If successful, it returns true.
		/// Typically calling this method after calling the method CreateJob.
		/// </summary>
		bool EnqueueJob(IJobContext^ JobContext);

		/// <summary>
		/// Destroy a job (JobContext). If successful, it returns true.
		/// You may call this method after calling the method CreateJob.
		/// </summary>
		bool DestroyJob(IJobContext^ JobContext);

		/// <summary>
		/// The method returns the number of jobs canceled with a given associated identity (Identity).
		/// </summary>
		int CancelJobs(Object^ Identity);

		/// <summary>
		/// The method returns true if there is indeed a job canceled with a given job id.
		/// </summary>
		bool CancelJob(__int64 jobId);
		
		/// <summary>
		/// Seek a job from a given job id (jobId).
		/// If found, it returns a job.
		/// </summary>
		IJobContext^ SeekJob(__int64 jobId);

		/// <summary>
		/// Safely shrink a set of memory pools.
		/// Don't call this method very often. Call it only if it is indeed needed.
		/// </summary>
		void ShrinkMemory();
		
		/// <summary>
		/// Safely shrink a set of memory pools down to the given memory chunk size (nMemoryChunkSize).
		/// Don't call this method very often. Call it only if it is indeed needed.
		/// </summary>
		void ShrinkMemory(int nMemoryChunkSize);
		
		/// <summary>
		/// Check a job status for a given job id (jobId)
		/// </summary>
		tagJobStatus GetJobStatus(__int64 jobId);
		
		/// <summary>
		/// Call this method to wait until all of jobs and tasks are processed with infinite time. If all jobs are indeed processed, the method returns true. Otherwise, it returns false.
		/// </summary>
		bool WaitAll();

		/// <summary>
		/// Call this method to wait until a given timeout (nTimeout). If all jobs are indeed processed within the given time (nTimeout), the method returns true. Otherwise, it returns false.
		/// </summary>
		bool WaitAll(int nTimeout);
		
		/// <summary>
		/// Call this method to wait until all of jobs associated with a given identity (Identity) are finished. 
		/// If all jobs are indeed processed, the method returns true. Otherwise, it returns false.
		/// </summary>
		bool Wait(Object ^Identity);

		/// <summary>
		/// Call this method to wait until all of jobs associated with a given identity (Identity) are finished with a given timeout (nTimeout). 
		/// If all jobs are indeed processed within the given time (nTimeout), the method returns true. Otherwise, it returns false.
		/// </summary>
		bool Wait(Object ^Identity, int nTimeout);

		/// <summary>
		/// Call this method to wait until all of jobs (jobs) are finished. 
		/// If all jobs are indeed processed, the method returns true. Otherwise, it returns false.
		/// </summary>
		bool Wait(array<__int64>^ jobs);
		
		/// <summary>
		/// Wait until one or more jobs are finished during a given time frame nTimeout.
		/// The method returns true and finished jobs jobsDone if successful. Otherwise, it returns false.
		/// </summary>
		bool WaitAny(array<__int64> ^jobs, int nTimeout, List<__int64> ^%jobsDone);

		/// <summary>
		/// Wait infinitely until one or more jobs are finished.
		/// The method returns true and finished jobs jobsDone if successful. Otherwise, it returns false.
		/// </summary>
		bool WaitAny(array<__int64> ^jobs, List<__int64> ^%jobsDone);

		/// <summary>
		/// Wait until one or more jobs with an identity Identity are finished during a time frame nTimeout.
		/// The method returns true and finished jobs jobsDone if successful. Otherwise, it returns false.
		/// </summary>
		bool WaitAny(Object ^Identity, int nTimeout, List<__int64> ^%jobsDone);

		/// <summary>
		/// Wait infinitely until one or more jobs with an identity Identity are finished during a time frame nTimeout.
		/// The method returns true and finished jobs jobsDone if successful. Otherwise, it returns false.
		/// </summary>
		bool WaitAny(Object ^Identity, List<__int64> ^%jobsDone);

		/// <summary>
		/// Wait until all of jobs (jobs) are finished with a given timeout (nTimeout). 
		/// If all jobs are indeed processed within the given time (nTimeout), the method returns true. Otherwise, it returns false.
		/// </summary>
		bool Wait(array<__int64>^ jobs, int nTimeout);

		/// <summary>
		/// Get an array of job ids in creating, queue or processing for a givn identity. 
		/// </summary>
		List<__int64>^ GetJobs(Object ^Identity);

		/// <summary>
		/// Lock an identity infinitely from available async handlers in socket pool. 
		/// You must call the method UnlockIdentity with the returned async handler later. Otherwise, async handler identities will be leaked quickly.
		/// The method will return null or nothing if no async handler is available at the calling time.
		/// Don't call this method from the virtual functions OnFailover, OnJobDone, OnExecutingJob, OnJobProcessing, OnReturnedResultProcessed and OnSocketPoolEvent. 
		/// Otherwise, there maybe is a dead lock.
		/// </summary>
		ClientSide::CAsyncServiceHandler^ LockIdentity();
		
		/// <summary>
		/// Lock an identity with a given timeout from available async handlers in socket pool. 
		/// You must call the method UnlockIdentity with the returned async handler later. Otherwise, async handler identities will be leaked quickly.
		/// The method will return null or nothing if no async handler is available at the calling time.
		/// Don't call this method from the virtual functions OnFailover, OnJobDone, OnExecutingJob, OnJobProcessing, OnReturnedResultProcessed and OnSocketPoolEvent. 
		/// Otherwise, there maybe is a dead lock if nTimeout is large.
		/// </summary>
		ClientSide::CAsyncServiceHandler^ LockIdentity(int nTimeout);
		
		/// <summary>
		/// Unlock an identity.
		/// </summary>
		void UnlockIdentity(ClientSide::CAsyncServiceHandler ^Identity);

		/// <summary>
		/// Reset a queued job (jobId) to a new position (nNewPosition). If failed, the method returns -1.
		/// Otherwise, it returns a new position.
		/// </summary>
		int ResetPosition(__int64 jobId, int nNewPosition);
		
		/// <summary>
		/// Identities under parallel computation at this time.
		/// </summary>
		property List<Object^>^ Identities
		{
			List<Object^>^ get();
		}
		
		/// <summary>
		/// The number of jobs queued and under processing.
		/// </summary>
		property int CountOfJobs
		{
			int get();
		}
		
		/// <summary>
		/// The number of jobs queued.
		/// </summary>
		property int CountOfQueuedJobs
		{
			int get();
		}
		
		/// <summary>
		/// The number of memory consumed by this job manager in byte.
		/// </summary>
		property __int64 MemoryConsumed
		{
			__int64 get();
		}
		
		/// <summary>
		/// A property for job recycle bin size.  
		/// Larger job recycle bin will make calling the methods Wait stable without any error at the cost of some memory. 
		/// The property defaults to 10 but will never be larger than 10240.
		/// </summary>
		property int RecycleBinSize
		{
			int get();
			void set(int nSize);
		}
		
		/// <summary>
		/// A property for raw COM USocketPool object.
		/// </summary>
		property USOCKETLib::USocketPoolClass^ SocketPool
		{
			USOCKETLib::USocketPoolClass^ get();
		}
	};

};

#endif