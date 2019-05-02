#ifndef __SOCKETPRO_ADAPTER_REQUEST_ASYNC_HANDLER_BASE_H__
#define __SOCKETPRO_ADAPTER_REQUEST_ASYNC_HANDLER_BASE_H__

//#include <atlcore.h>
//#include "sockutil.h"


namespace SocketProAdapter
{
namespace ClientSide
{
	ref struct CAsyncResult;
	ref class CClientSocket;
	ref class CAsyncServiceHandler;
	public delegate void DAsyncResultHandler(CAsyncResult ^AsyncResult);
	
	[CLSCompliantAttribute(true)] 
	public ref struct CAsyncResult
	{
	internal:
		CAsyncResult(CAsyncServiceHandler ^pAsyncServiceHandler, short ReqId, CUQueue ^q)
			: AsyncServiceHandler(pAsyncServiceHandler), RequestId(ReqId), UQueue(q)
		{
		}
	public:
		short					RequestId;
		CAsyncServiceHandler	^AsyncServiceHandler;
		CUQueue					^UQueue;
		DAsyncResultHandler		^CurrentAsyncResultHandler;
	};
	
	[CLSCompliantAttribute(true)] 
	public interface class IAsyncResultsHandler
	{
	public:
		void Process(CAsyncResult ^AsyncResult);
		void OnExceptionFromServer(CAsyncServiceHandler ^AsyncServiceHandler, CSocketProServerException ^Exception);
	};

	[CLSCompliantAttribute(true)] 
	public ref class CAsyncServiceHandler
	{
	public:
		CAsyncServiceHandler(int nServiceId);
		CAsyncServiceHandler(int nServiceId, CClientSocket ^cs);
		CAsyncServiceHandler(int nServiceId, CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler);
		
		virtual ~CAsyncServiceHandler();

	protected:
		CClientSocket ^m_pClientSocket;

	private:
		ref class CIndexQueue
		{
		public:
			unsigned __int64 Index;
			CUQueue			 ^UQueue;
		};
		unsigned __int64 m_nSyncIndex;
		List<CIndexQueue^> ^m_mapSync;
		CUQueue^ Look(unsigned __int64 index);

		ref class CPair
		{
		public:
			CPair(short sReqId, DAsyncResultHandler ^arh, unsigned __int64 index) : RequestId(sReqId), m_arh(arh), m_nIndex(index)
			{
			}
			short				RequestId;
			DAsyncResultHandler	^m_arh;
			unsigned __int64 m_nIndex;
		};

		void NotifyRequestProcessed(int len, short sReqId);
		bool PostProcessSync(short sReqId, unsigned __int64 index);
		generic <typename R0>
		bool PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0);
		generic <typename R0, typename R1>
		bool PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1);
		generic <typename R0, typename R1, typename R2>
		bool PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
		generic <typename R0, typename R1, typename R2, typename R3>
		bool PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
		generic <typename R0, typename R1, typename R2, typename R3, typename R4>
		bool PostProcessSync(short sReqId, unsigned __int64 index, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

	public:
		int GetSvsID();
		virtual bool Attach(CClientSocket ^pClientSocket);
		virtual void Detach();
		
		/// <summary>
		/// Refer to the method CClientSocket.BeginBatching.
		/// </summary>
		bool BeginBatching();
		
		/// <summary>
		/// Refer to the method CClientSocket.Commit.
		/// </summary>
		bool CommitBatch(bool bBatchingAtServer);

		/// <summary>
		/// Refer to the method CClientSocket.Commit.
		/// </summary>
		bool CommitBatch();

		/// <summary>
		/// Refer to the method CClientSocket.Rollback.
		/// </summary>
		bool RollbackBatch();

		/// <summary>
		/// Refer to the method CClientSocket.WaitAll.
		/// </summary>
		bool WaitAll(int nTimeout);

		/// <summary>
		/// Refer to the method CClientSocket.WaitAll.
		/// </summary>
		bool WaitAll();

		/// <summary>
		/// Wait until the request (sRequestId) is processed and its result is returned or it is timed out at the given time (nTimeout) in milliseconds. If the request id (sRequestId) is zero, it implies the last request if available. The method returns true if its result is returned. Otherwise, it returns false.
		/// Refer to the method CClientSocket.Wait.
		/// </summary>
		bool Wait(short sRequestId, int nTimeout);
		
		/// <summary>
		/// Wait until the request (sRequestId) is processed and its result is returned. If the request id (sRequestId) is zero, it implies the last request. The method returns true if its result is returned. Otherwise, it returns false.
		/// Refer to the method CClientSocket.Wait.
		/// </summary>
		bool Wait(short sRequestId);
		
		/// <summary>
		/// Wait until the last request is processed and its result is returned. The method returns true if its result is returned. Otherwise, it returns false.
		/// Refer to the method CClientSocket.Wait.
		/// </summary>
		bool Wait();

		CClientSocket ^GetAttachedClientSocket();
		
		/// <summary>
		/// Send an empty request to a remote server. 
		/// </summary>
		bool SendRequest(short sRequestID);

		/// <summary>
		/// Send an empty request with a given callback to a remote server. 
		/// </summary>
		bool SendRequest(short sRequestID, DAsyncResultHandler ^AsyncResultHandler);
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		virtual bool SendRequest(short sRequestID, IntPtr pBuffer, int nLenInByte);

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		virtual bool SendRequest(short sRequestID, IntPtr pBuffer, int nLenInByte, DAsyncResultHandler ^AsyncResultHandler);
		
		/// <summary>
		/// Send a request to a remote server. 
		/// </summary>
		bool SendRequest(short sRequestID, CUQueue ^UQueue);

		/// <summary>
		/// Send a request to a remote server with a given callback. 
		/// </summary>
		bool SendRequest(short sRequestID, CUQueue ^UQueue, DAsyncResultHandler ^AsyncResultHandler);

		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		bool SendRequest(short sRequestID, CScopeUQueue ^UQueue);

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		bool SendRequest(short sRequestID, CScopeUQueue ^UQueue, DAsyncResultHandler ^AsyncResultHandler);
		
	public:
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		bool ProcessR0(short sRequestID);
		
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename R0>
		bool ProcessR1(short sRequestID, [Out]R0 %r0);
		
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename R0, typename R1>
		bool ProcessR2(short sRequestID, [Out]R0 %r0, [Out]R1 %r1);
		
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);
		
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0>
		bool ProcessR0(short sRequestID, T0 t0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				bool b = SendRequest(sRequestID, t0, t1, t2, m_Copy);
				if(!b)
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}

		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			CAutoLock al(&m_pCS->m_sec);
			{
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}
		/// <summary>
		/// Process a request (sRequestID) without any return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
		bool ProcessR0(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index);
		}
		/// <summary>
		/// Process a request (sRequestID) with one return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0>
		bool ProcessR1(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0);
		}
		/// <summary>
		/// Process a request (sRequestID) with two return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1>
		bool ProcessR2(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1);
		}
		/// <summary>
		/// Process a request (sRequestID) with three return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2>
		bool ProcessR3(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2);
		}
		/// <summary>
		/// Process a request (sRequestID) with four return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3>
		bool ProcessR4(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3);
		}
		/// <summary>
		/// Process a request (sRequestID) with five return data synchronously.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3, typename R4>
		bool ProcessR5(short sRequestID, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4)
		{
			unsigned __int64 index;
			{
				CAutoLock al(&m_pCS->m_sec);
				if(!SendRequest(sRequestID, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_Copy))
					return false;
				index = m_nSyncIndex;
			}
			return PostProcessSync(sRequestID, index, r0, r1, r2, r3, r4);
		}

		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0>
		bool SendRequest(short sRequestID, T0 data0)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			return SendRequest(sRequestID, UQueue.m_UQueue);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0>
		bool SendRequest(short sRequestID, T0 data0, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1>
		bool SendRequest(short sRequestID, T0 data0, T1 data1)
		{
			return SendRequest(sRequestID, data0, data1, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2)
		{
			return SendRequest(sRequestID, data0, data1, data2, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, data5, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			UQueue.Save(data5);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, data5, data6, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			UQueue.Save(data5);
			UQueue.Save(data6);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, data5, data6, data7, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a give callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			UQueue.Save(data5);
			UQueue.Save(data6);
			UQueue.Save(data7);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}
		
		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, data5, data6, data7, data8, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			UQueue.Save(data5);
			UQueue.Save(data6);
			UQueue.Save(data7);
			UQueue.Save(data8);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}

		/// <summary>
		/// Send a request to a remote server.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, T9 data9)
		{
			return SendRequest(sRequestID, data0, data1, data2, data3, data4, data5, data6, data7, data8, data9, (DAsyncResultHandler ^)nullptr);
		}

		/// <summary>
		/// Send a request to a remote server with a given callback.
		/// </summary>
		generic <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
		bool SendRequest(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, T9 data9, DAsyncResultHandler ^AsyncResultHandler)
		{
			CScopeUQueue UQueue;
			UQueue.Save(data0);
			UQueue.Save(data1);
			UQueue.Save(data2);
			UQueue.Save(data3);
			UQueue.Save(data4);
			UQueue.Save(data5);
			UQueue.Save(data6);
			UQueue.Save(data7);
			UQueue.Save(data8);
			UQueue.Save(data9);
			return SendRequest(sRequestID, UQueue.m_UQueue, AsyncResultHandler);
		}

	public:
		property IAsyncResultsHandler^ AsyncResultsHandler
		{
			IAsyncResultsHandler^ get()
			{
				CAutoLock al(&m_pCS->m_sec);
				return m_pIAsyncResultsHandler;
			}
			void set(IAsyncResultsHandler ^p)
			{
				CAutoLock al(&m_pCS->m_sec);
				m_pIAsyncResultsHandler = p;
			}
		}
	
	protected:
		virtual void OnResultReturned(short sRequestID, CUQueue ^UQueue)
		{

		}

		virtual void OnExceptionFromServer(CSocketProServerException ^Exception)
		{

		}

	internal:
		unsigned long  RemoveAsyncHandlers(unsigned int nCancel);
		unsigned __int64 OnRR(int hSocket, short sRequestID, int nLen, USOCKETLib::tagReturnFlag sFlag);
	
	private:
		CPair^ GetAsyncResultHandler(short sReqId);
		bool Send(short sRequestID, IntPtr pBuffer, int nLenInByte);
		void Copy(CAsyncResult ^ar);
		unsigned __int64 OnRR(short sRequestID, CUQueue ^UQueue);
		unsigned __int64 OnE(short sRequestId, CSocketProServerException ^Exception);

	private:
		int						m_nSvsId;
		IAsyncResultsHandler	^m_pIAsyncResultsHandler;
		CComAutoCriticalSection	*m_pCS;
		List<CPair^> ^m_lstFunc;
		DAsyncResultHandler		^m_Copy;
	};
}
}

#endif