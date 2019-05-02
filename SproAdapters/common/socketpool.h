#ifndef ___SOCKETPOOL_UDAPARTS_H
#define ___SOCKETPOOL_UDAPARTS_H


#include <vcclr.h>
#include <msclr\lock.h>
using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Threading;

#include "jobcontext.h"
#include "jobmanager.h"

namespace SocketProAdapter
{
	/// <summary>
	/// A structure for making a socket connection to a remote SocketPro server.
	/// </summary>
	public ref struct CConnectionContext
	{
	public:
		/// <summary>
		/// A remote host string like "111.222.212.121", MyServerName or www.MyHostSample.com.
		/// </summary>
		String							^m_strHost;

		/// <summary>
		/// A port number which a SocketPro server is running on.
		/// </summary>
		int								m_nPort;

		/// <summary>
		/// A case insensitive user id for login.
		/// </summary>
		String							^m_strUID;

		/// <summary>
		/// A case-sensitive password for login.
		/// </summary>
		String							^m_strPassword;
		
		/// <summary>
		/// An encryption method.
		/// </summary>
		USOCKETLib::tagEncryptionMethod	m_EncrytionMethod;

		/// <summary>
		/// A value indicating if online compression is enabled.
		/// </summary>
		bool							m_bZip;
	};

	
	namespace ClientSide
	{
		/// <summary>
		/// A class for managing a pool of sockets so that you can easily process multiple remote requests in parallel and batching with asynchrony computation and loading balance style. 
		/// The class wraps the methods and properties of COM object USocketPool.
		/// </summary>
		generic<typename THandler> where THandler : CAsyncServiceHandler, gcnew()
		public ref class CSocketPool
		{
			void HandleException(COMException ^ex)
			{
				const char *str;
#ifdef _WIN64
				str = "ATL COM object can not be created! This may happen because the core library npUSocket.dll (64bit) is not registerred (regsvr32 npUSocket.dll) properly or COM environment is not initialized (MTAThread or STAThread) yet!";
#else
				str = "ATL COM object can not be created! This may happen because the core library npUSocket.dll (32bit) is not registerred (regsvr32 npUSocket.dll) properly or COM environment is not initialized (MTAThread or STAThread) yet!";
#endif
				throw gcnew System::InvalidProgramException(gcnew String(str), ex);
			}

		public:
			/// <summary>
			/// Instantiate a socket pool object. Internally, it creates a COM object USocketPool.
			/// </summary>
			CSocketPool() : m_ulRecvTimeout(30000), m_hr(S_OK), m_pIAsyncResultsHandler(nullptr)
			{
				m_mapSocket = gcnew Dictionary<CClientSocket^, THandler>();
				try
				{
					m_USocketPool = gcnew USOCKETLib::USocketPoolClass();
				}
				catch(COMException ^ex)
				{
					HandleException(ex);
				}
				m_USocketPool->OnSocketPoolEvent += gcnew USOCKETLib::_IUSocketPoolEvents_OnSocketPoolEventEventHandler(this, &CSocketPool::OnSocketPoolEvent);
				m_pIUSocketPool = (IUSocketPool*)System::Runtime::InteropServices::Marshal::GetIUnknownForObject(m_USocketPool).ToPointer();
			}

			/// <summary>
			/// Instantiate a socket pool object with a given interface to a default async results handler. Internally, it creates a COM object USocketPool.
			/// </summary>
			CSocketPool(IAsyncResultsHandler ^pDefaultAsyncResultsHandler) : m_ulRecvTimeout(30000), m_hr(S_OK), m_pIAsyncResultsHandler(pDefaultAsyncResultsHandler)
			{
				m_mapSocket = gcnew Dictionary<CClientSocket^, THandler>();
				try
				{
					m_USocketPool = gcnew USOCKETLib::USocketPoolClass();
				}
				catch(COMException ^ex)
				{
					HandleException(ex);
				}
				m_USocketPool->OnSocketPoolEvent += gcnew USOCKETLib::_IUSocketPoolEvents_OnSocketPoolEventEventHandler(this, &CSocketPool::OnSocketPoolEvent);
				m_pIUSocketPool = (IUSocketPool*)System::Runtime::InteropServices::Marshal::GetIUnknownForObject(m_USocketPool).ToPointer();
			}

			/// <summary>
			/// Shutdown the socket pool if it is started, release all of resources, and destroy a COM object USocketPool.
			/// </summary>
			virtual ~CSocketPool()
			{
				ShutdownPool();
				m_USocketPool->OnSocketPoolEvent -= gcnew USOCKETLib::_IUSocketPoolEvents_OnSocketPoolEventEventHandler(this, &CSocketPool::OnSocketPoolEvent);
				if(m_pIUSocketPool != NULL)
				{
					m_pIUSocketPool->Release();
					m_pIUSocketPool = NULL;
				}
				System::Runtime::InteropServices::Marshal::ReleaseComObject(m_USocketPool);
				m_USocketPool = nullptr;
			}
		
		public:
			/// <summary>
			/// Lock a socket with infinite timeout.
			/// If the function returns nothing, refer to m_hr for error code.
			/// </summary>
			THandler Lock()
			{
				return Lock(-1, IntPtr::Zero);
			}
			
			/// <summary>
			/// Lock a socket with a given timeout (nTimeout) in ms.
			/// If the function returns nothing, refer to m_hr for error code.
			/// </summary>
			THandler Lock(long nTimeout)
			{
				return Lock(nTimeout, IntPtr::Zero);
			}
			
			/// <summary>
			/// Set a request receiving timeout in ms to all of pooled USocket objects. 
			/// Timout will not be less than 1000 ms.
			/// </summary>
			void SetRecvTimeout(long nTimeout)
			{
				if(nTimeout >= 0 && nTimeout < 1000)
					nTimeout = 1000;
				m_ulRecvTimeout = (unsigned long)nTimeout;
				msclr::lock AutoLock((Object^)m_mapSocket);
				for each (CClientSocket ^cs in m_mapSocket->Keys)
				{
					IUSocket *pIUSocket = (IUSocket*)cs->GetIUSocket().ToPointer();
					m_hr = pIUSocket->put_RecvTimeout(nTimeout);
				}
			}
			
			/// <summary>
			/// Lock a socket with a given timeout (nTimeout) in ms. 
			/// If pIUSocketSameThread is zero, pool will lock anyone of sockets available.
			/// If pIUSocketSameThread is pointed to an locked socket, pool will lock a socket that hosted with the same thread for the locked thread pointed by pIUSocketSameThread.
			/// If the function returns nothing, refer to m_hr for error code.
			/// </summary>
			virtual THandler Lock(long nTimeout, IntPtr pIUSocketSameThread)
			{
				Object ^obj = nullptr;
				long hSocket = 0;
				if(m_pIUSocketPool == NULL)
					return (THandler)obj;
				CComPtr<IUSocket> pIUSocket;
				m_hr = m_pIUSocketPool->LockASocket(nTimeout, (IUSocket*)pIUSocketSameThread.ToPointer(), &pIUSocket);
				if(FAILED(m_hr))
				{
					return (THandler)obj;
				}
				m_hr = pIUSocket->get_Socket(&hSocket);
				msclr::lock AutoLock((Object^)m_mapSocket);
				for each (CClientSocket ^cs in m_mapSocket->Keys)
				{
					long hMy = 0;
					IUSocket *pIUSocket = (IUSocket*)cs->GetIUSocket().ToPointer();
					m_hr = pIUSocket->get_Socket(&hMy);
					if(hMy == hSocket)
					{
						obj = m_mapSocket[cs];
						break;
					}
				}
				return (THandler)obj;
			}
			
			/// <summary>
			/// Unlock a previously locked socket with a given asynchronous handler (Handler).
			/// </summary>
			void Unlock(THandler Handler)
			{
				if (Handler != nullptr)
				{
					Unlock(Handler->GetAttachedClientSocket());
				}
			}
			
			/// <summary>
			/// Unlock a previously locked socket (ClientSocket).
			/// </summary>
			virtual void Unlock(CClientSocket ^ClientSocket)
			{
				if (ClientSocket != nullptr && m_pIUSocketPool != NULL)
				{
					m_hr = m_pIUSocketPool->UnlockASocket(ClientSocket->m_pIUSocket);
				}
			}
			
			/// <summary>
			/// Check if the handler (Handler) has processed all of requests.
			/// </summary>
			bool IsCompleted(THandler Handler)
			{
				if (Handler == nullptr)
					return true;
				return IsCompleted(Handler->GetAttachedClientSocket());
			}
			
			/// <summary>
			/// Check if the socket (ClientSocket) has processed all of requests.
			/// </summary>
			bool IsCompleted(CClientSocket ^ClientSocket)
			{
				if (ClientSocket != nullptr)
				{
					return (ClientSocket->GetCountOfRequestsInQueue() == 0);
				}
				return true;
			}

			///	<summary>
			/// Start a socket pool. 
			/// All of sockets will be connected to a remote SocketPro server (strHost, lPort) with given credentials (strUID, strPassword).
			/// The bSocketsPerThread indicates the number of sockets hosted by a thread.
			/// The number of threads in socket pool will equal to the number of processors of a machine. 
			/// If it returns false, refer to m_hr for error code.
			/// </summary>
			bool StartSocketPool(String ^strHost, long lPort, String ^strUID, String ^strPassword, BYTE bSocketsPerThread)
			{
				return StartSocketPool(strHost, lPort, strUID, strPassword, bSocketsPerThread, 0, USOCKETLib::tagEncryptionMethod::NoEncryption, false);
			}
			
			/// <summary>
			/// Start a socket pool without connecting to any remote host. 
			/// The input bSocketPerThread should be between 1 and 63.
			/// The pool will contain bSocketPerThread*(the number of processors) sockets totally.
			/// </summary>
			bool StartSocketPool(BYTE bSocketPerThread)
			{
				return StartSocketPool(bSocketPerThread, 0);
			}
			
			/// <summary>
			/// Start a socket pool without connecting to any remote host. 
			/// If the input bThread is zero, socket pool will use the number of processors instead.
			/// The input bSocketPerThread should be between 1 and 63.
			/// The pool will contain bSocketPerThread*bThreads sockets totally.
			/// If failed, refer to m_hr for error code.
			/// </summary>
			bool StartSocketPool(BYTE bSocketPerThread, BYTE bThreads)
			{
				if(m_USocketPool == nullptr)
					return false;
				m_hr = S_OK;
				if (IsStarted())
				{
					return true;
				}
				try
				{
					m_USocketPool->StartPool(bThreads, bSocketPerThread);
				}
				catch (COMException ^myerr)
				{
					m_hr = myerr->ErrorCode;
					return false;
				}
				return IsStarted();
			}
			
			/// <summary>
			/// Start a socket pool with the number of threads equal to the number of processors. 
			/// The array of connection contexts (ConnectionContexts) defines what and how these remote server are connected.
			/// The parameter bSocketsPerThread indicates the max number of sockets can be hosted per thread. It should not exceed 63.
			/// </summary>
			bool StartSocketPool(array<CConnectionContext^> ^ConnectionContexts, BYTE bSocketsPerThread)
			{
				return StartSocketPool(ConnectionContexts, bSocketsPerThread, 0);
			}
			
			/// <summary>
			/// Start a socket pool. 
			/// The array of connection contexts (ConnectionContexts) defines what and how these remote server are connected.
			/// The parameter bSocketsPerThread indicates the max number of sockets can be hosted per thread. It should not exceed 63.
			/// The parameter bThreads indicates the number of threads within the pool. As usual, it should be equal to the number of processors by setting the parameter to zero.
			/// If failed, refer to m_hr for error code.
			/// </summary>
			bool StartSocketPool(array<CConnectionContext^> ^ConnectionContexts, BYTE bSocketsPerThread, BYTE bThreads)
			{
				CComBSTR bstrHost;
				CComBSTR bstrUID;
				CComBSTR bstrPWD;
				if(ConnectionContexts == nullptr || ConnectionContexts->Length == 0 || bSocketsPerThread == 0)
					return false;
				if(!StartSocketPool(bSocketsPerThread, bThreads))
					return false;

				THandler Handler = gcnew THandler();
				long lSvsId = Handler->GetSvsID();

				unsigned int nConnectionContextCount = (unsigned int)ConnectionContexts->Length;
				unsigned int nIndex = 0;
				while(nConnectionContextCount > nIndex)
				{
					CConnectionContext ^cc = ConnectionContexts[nIndex];
					CComPtr<IUSocket> pIUSocket;
					m_pIUSocketPool->FindAClosedSocket(&pIUSocket);
					if(pIUSocket == NULL)
						break;
					if(cc->m_strHost != nullptr)
					{
						pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strHost);
						bstrHost = str;
					}
					else
						bstrHost.Empty();
					
					if(cc->m_strUID != nullptr)
					{
						pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strUID);
						bstrUID = str;
					}
					else
						bstrUID.Empty();

					if(cc->m_strPassword != nullptr)
					{
						pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strPassword);
						bstrPWD = str;
					}
					else
						bstrPWD.Empty();

					m_pIUSocketPool->Connect(	pIUSocket.p, 
												bstrHost, 
												cc->m_nPort, 
												lSvsId, 
												bstrUID, 
												bstrPWD, 
												(short)cc->m_EncrytionMethod, 
												cc->m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
					pIUSocket.Release();
					nIndex++;
				}
				long lConnected = 0;
				m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
				return (lConnected != 0);
			}
			
			/// <summary>
			/// Start a socket pool. 
			/// All of sockets will be connected to one remote SocketPro server (strHost, lPort) with given credentials (strUID, strPassword).
			/// The bSocketsPerThread indicates the number of sockets hosted by a thread.
			/// The parameter bThreads determines the number of threads in pool. 
			/// Specifically if bThread is zero, the number of threads in socket pool will equal to the number of processors of a machine.
			/// If it returns false, refer to m_hr for error code.
			/// </summary>
			bool StartSocketPool(String ^strHost, long lPort, String ^strUID, String ^strPassword, BYTE bSocketsPerThread, BYTE bThreads, USOCKETLib::tagEncryptionMethod EncrytionMethod, bool bZip)
			{
				if(!StartSocketPool(bSocketsPerThread, bThreads))
					return false;
				try
				{
					THandler temp = gcnew THandler();
					m_USocketPool->ConnectAll(strHost, lPort, temp->GetSvsID(), strUID, strPassword, (short)EncrytionMethod, bZip);
				}
				catch (COMException ^myerr)
				{
					m_hr = myerr->ErrorCode;
					ShutdownPool();
					return false;
				}
				if(m_USocketPool->ConnectedSocketsEx != 0)
					return true;
				m_hr = specNoOpenedSocket;
				return false;
			}
			
			/// <summary>
			/// Make one connection to a remote SocketPro server specified by the parameter cc.
			/// Note if there is no unconnected USocket object available, the method will return false.
			/// If the method returns false, refer to m_hr for error code.
			/// </summary>
			virtual bool MakeConnection(CConnectionContext ^cc)
			{
				if(cc == nullptr || !IsStarted())
					return false;
				CComPtr<IUSocket> pIUSocket;
				m_hr = m_pIUSocketPool->FindAClosedSocket(&pIUSocket);
				if(pIUSocket == NULL)
					return false;

				THandler Handler = gcnew THandler();
				long lSvsId = Handler->GetSvsID();
				CComBSTR bstrHost;
				if(cc->m_strHost != nullptr)
				{
					pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strHost);
					bstrHost = str;
				}
				CComBSTR bstrUID;
				if(cc->m_strUID != nullptr)
				{
					pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strUID);
					bstrUID = str;
				}
				CComBSTR bstrPWD;
				if(cc->m_strPassword != nullptr)
				{
					pin_ptr<const wchar_t> str = PtrToStringChars(cc->m_strPassword);
					bstrPWD = str;
				}

				m_hr =m_pIUSocketPool->Connect( pIUSocket.p, 
												bstrHost, 
												cc->m_nPort, 
												lSvsId, 
												bstrUID, 
												bstrPWD, 
												(short)cc->m_EncrytionMethod, 
												cc->m_bZip ? VARIANT_TRUE : VARIANT_FALSE);

				long hr = S_OK;
				pIUSocket->get_Rtn(&hr);
				if(hr != S_OK)
					m_hr = specNoOpenedSocket;
				return (m_hr == S_OK);
			}
			
			/// <summary>
			/// Shut down socket pool and clean all of associated resources.
			/// </summary>
			virtual void ShutdownPool()
			{
				if(IsStarted())
				{
					CleanHandlers();
					if(m_pIUSocketPool != NULL)
					{
						m_pIUSocketPool->DisconnectAll();
						m_pIUSocketPool->ShutdownPool();
					}
				}
			}
			
			/// <summary>
			/// Check the pool is started.
			/// </summary>
			bool IsStarted()
			{
				if(m_pIUSocketPool == NULL)
					return false;
				BYTE bThreads;
				m_pIUSocketPool->get_ThreadCounts(&bThreads);
				return (bThreads != 0);
			}
			
			/// <summary>
			/// Get a COM interop interface to COM object USocketPool. You may use the interface to debug, see how properties are changed, and extend the pool with more features.
			/// </summary>
			USOCKETLib::USocketPoolClass^ GetUSocketPool()
			{
				return m_USocketPool;
			}

		protected:
			/// <summary>
			/// The virtual function is called when starting and shutting down a socket pool as well as connectiong to remote host.
			/// Note that dead lock may happen if you call one of methods of USocketPool object inside the virtual function.
			/// </summary>
			virtual void OnSocketPoolEvent(USOCKETLib::tagSocketPoolEvent spe, USOCKETLib::USocket ^pIUSocket)
			{
				switch(spe)
				{
				case USOCKETLib::tagSocketPoolEvent::speUSocketCreated:
					{
						ATLASSERT(pIUSocket != nullptr);
						IntPtr p = System::Runtime::InteropServices::Marshal::GetIUnknownForObject(pIUSocket);
						CClientSocket ^ClientSocket = gcnew CClientSocket(p);
						if (p != IntPtr::Zero)
						{
							System::Runtime::InteropServices::Marshal::Release(p);
						}
						THandler Handler = gcnew THandler();
						{
							msclr::lock AutoLock((Object^)m_mapSocket);
							m_mapSocket->Add(ClientSocket, Handler);
						}
						pIUSocket->RecvTimeout = (long)m_ulRecvTimeout;
						ClientSocket->Disadvise(); //make sure all of attached handlers subscribe socket events first
						Handler->Attach(ClientSocket);
						Handler->AsyncResultsHandler = m_pIAsyncResultsHandler;
						ClientSocket->Advise(); //re-subscribe socket events
					}
					break;
				case USOCKETLib::tagSocketPoolEvent::speConnected:
					{
						ATLASSERT(pIUSocket != nullptr);

						//increase receiving and sending buffer sizes, which leads to better through output
						pIUSocket->StartBatching();
						pIUSocket->SetSockOpt(soRcvBuf, 116800, slSocket);
						pIUSocket->SetSockOpt(soSndBuf, 116800, slSocket);
						pIUSocket->SetSockOptAtSvr(soSndBuf, 116800, slSocket);
						pIUSocket->SetSockOptAtSvr(soRcvBuf, 116800, slSocket);
						pIUSocket->CommitBatching(true);
					}
					break;
				case USOCKETLib::tagSocketPoolEvent::speShutdown:
					ShutdownPool();
					break;
				}
			}

		private:
			void CleanHandlers()
			{
				if(m_mapSocket != nullptr)
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					for each (CAsyncServiceHandler ^rahb in m_mapSocket->Values)
					{
						delete rahb;
					}

					for each (CClientSocket ^cs in m_mapSocket->Keys)
					{
						cs->Disconnect();
						cs->DetachAll();
						cs->DestroyUSocket();
						delete cs;
					}
					m_mapSocket->Clear();
				}
			}

		public:
			/// <summary>
			/// Error code.
			/// </summary>
			long						m_hr;

		protected:
			/// <summary>
			/// A dictionary containing socket and handler in pair. You can access it from your derived class cautiously with monitor protection.
			/// </summary>
			Dictionary<CClientSocket^, THandler>					^m_mapSocket;
		
		private:
			USOCKETLib::USocketPoolClass	^m_USocketPool;
			unsigned long					m_ulRecvTimeout;
			IAsyncResultsHandler			^m_pIAsyncResultsHandler;

		internal:
			IUSocketPool				*m_pIUSocketPool;
		};
		
		/// <summary>
		/// A class makes parallel, loading balance and grid computation extremely easy with load balancing and failover protection.
		/// </summary>
		generic<typename THandler> where THandler : CAsyncServiceHandler, gcnew()
		public ref class CSocketPoolEx : public CSocketPool<THandler>, IWaitAll, IProcessBySocketPoolEx
		{
		public:
			CSocketPoolEx() 
				: m_lRemoveIndex(0), m_nFails(0), m_bWorking(false), 
				m_bCallProcess(false), m_bServerLoadingBalance(false)
			{
				m_hWait = ::CreateEvent(NULL, TRUE, TRUE, NULL);
				m_aPHandler = gcnew List<THandler>();
				m_aPauseHandler = gcnew List<THandler>();
				m_JobManager = gcnew CJobManager();
				m_JobManager->m_pIUSocketPool = m_pIUSocketPool;
				m_JobManager->m_upc = GetUSocketPool();
				m_JobManager->m_bServerLoadingBalance = m_bServerLoadingBalance;
				m_JobManager->m_pIWaitAll = this;
			}

			CSocketPoolEx(IAsyncResultsHandler ^pDefaultAsyncResultsHandler) 
				: CSocketPool<THandler>(pDefaultAsyncResultsHandler), m_lRemoveIndex(0), m_nFails(0), 
				m_bWorking(false), m_bCallProcess(false), m_bServerLoadingBalance(false)
			{
				m_hWait = ::CreateEvent(NULL, TRUE, TRUE, NULL);
				m_aPHandler = gcnew List<THandler>();
				m_aPauseHandler = gcnew List<THandler>();
				m_JobManager = gcnew CJobManager();
				m_JobManager->m_pIUSocketPool = m_pIUSocketPool;
				m_JobManager->m_upc = GetUSocketPool();
				m_JobManager->m_bServerLoadingBalance = m_bServerLoadingBalance;
				m_JobManager->m_pIWaitAll = this;
			}
			
			virtual ~CSocketPoolEx()
			{
				m_JobManager->m_upc = nullptr;
				ShutdownPool();
				::CloseHandle(m_hWait);
				delete m_JobManager;
			}

		public:
			/// <summary>
			/// A property indicating a job manager.
			/// </summary>
			property IJobManager^ JobManager
			{
				IJobManager^ get()
				{
					return m_JobManager;
				}
			}

			/// <summary>
			/// Reset the number of failovers to zero.
			/// </summary>
			void ResetFails()
			{
				msclr::lock AutoLock((Object^)m_mapSocket);
				m_nFails = 0;
			}
			
			/// <summary>
			/// The number of failovers.
			/// </summary>
			property int Fails
			{
				int get()
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					return (int)m_nFails;
				}
			}
			
			/// <summary>
			/// Indicating if socket pool is still processing requests.
			/// </summary>
			property bool Working
			{
				bool get()
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					return m_bWorking;
				}
			}
			
			/// <summary>
			/// Indicating if processing requests is paused.
			/// </summary>
			property bool Paused
			{
				bool get()
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					return m_bPause;
				}
			}
			
			/// <summary>
			/// Indicating how many sockets are involved for proessing requests in parallel.
			/// </summary>
			property int SocketsInParallel
			{
				int get()
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					return (m_aPHandler->Count + m_aPauseHandler->Count);
				}
			}
			
			/// <summary>
			/// Lock a socket. This method is disabled. Calling this method will always throw an exception from this class.
			/// </summary>
			virtual THandler Lock(long nTimeout, IntPtr pIUSocketSameThread) override
			{
				throw gcnew System::InvalidOperationException("Lock method disabled!");
			}
			
			/// <summary>
			/// Unlock a socket. This method is disabled. Calling this method will always throw an exception from this class.
			/// </summary>
			virtual void Unlock(CClientSocket ^ClientSocket) override
			{
				throw gcnew System::InvalidOperationException("Lock method disabled!");
			}

			
			/// <summary>
			/// The method returns an array of handlers involved for proessing requests in parallel.
			/// </summary>
			List<THandler>^ GetHandlersInParallel()
			{
				List<THandler> ^handlers = gcnew List<THandler>();
				msclr::lock AutoLock((Object^)m_mapSocket);
				for each (THandler h in m_aPHandler)
					handlers->Add(h);
				for each (THandler h in m_aPauseHandler)
					handlers->Add(h);
				return handlers;
			}
			
			/// <summary>
			/// A property indicating if all of connected sockets in pool are processing requests in parallel.
			/// </summary>
			property bool AllLoaded
			{
				bool get()
				{
					long lLocks = 0;
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						if(m_pIUSocketPool == NULL)
							return false;
					}
					m_pIUSocketPool->get_LockedSocketsEx(&lLocks);
					return (lLocks > 0 && SocketsInParallel >= lLocks);
				}
			}
			
			/// <summary>
			/// Call this method to wait until all of jobs and tasks are processed with infinite time. If all jobs are indeed processed, the method returns true. Otherwise, it returns false.
			/// Don't call this method from the virtual functions, OnFailover, OnExecuteJob, OnResultReturned and OnSocketPoolEvent. Otherwise, there is a dead lock.
			/// </summary>
			bool WaitUntilFinished()
			{
				return WaitUntilFinished((int)-1);
			}
			
			/// <summary>
			/// Call this method to wait until a given timeout (nTimeout). If all jobs are indeed processed within the given time (nTimeout), the method returns true. Otherwise, it returns false.
			/// Don't call this method from the virtual functions, OnFailover, OnExecuteJob, OnResultReturned and OnSocketPoolEvent. Otherwise, there maybe is a dead lock if nTimeout is large.
			/// </summary>
			virtual bool WaitUntilFinished(int nTimeout)
			{
				return (::WaitForSingleObject(m_hWait, (DWORD)nTimeout) == WAIT_OBJECT_0);
			}
			
			/// <summary>
			/// Make one connection to a remote SocketPro server specified by the parameter cc, and join parallel computation immediately if there is a job available.
			/// Note if there is no unconnected USocket object available, the method will return false.
			/// If the method returns false, refer to m_hr for error code.
			/// </summary>
			virtual bool MakeConnection(CConnectionContext ^cc) override
			{
				if(cc == nullptr)
					return false;
				bool bSuc = CSocketPool<THandler>::MakeConnection(cc);
				if(!bSuc)
					return false;
				bSuc = false;
				msclr::lock AutoLock((Object^)m_mapSocket);
				do
				{
					if(!m_bWorking)
						break;
					THandler pHandler = CSocketPool<THandler>::Lock(0, IntPtr::Zero);
					if(pHandler == nullptr)
						break;
					if(m_bPause)
					{
						m_aPauseHandler->Add(pHandler);
						break;
					}
					AutoLock.release();
					bool bSuc = ExecuteJob(pHandler);
					AutoLock.acquire();
					if(bSuc)
						m_aPHandler->Add(pHandler);
					else
						CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
				}while(false);
				if(bSuc)
					::ResetEvent(m_hWait);
				return true;
			}
			
			/// <summary>
			/// Starts to process requests in parallel. If socket pool is not started or there is no job available, the method returns false.
			/// </summary>
			virtual bool Process()
			{
				if(!IsStarted())
					return false;
				long lLocks = 0;
				long lSockets = 0;
				m_pIUSocketPool->get_ConnectedSocketsEx(&lSockets);
				if(lSockets == 0)
				{
					m_hr = specNoOpenedSocket;
					return false;
				}
				
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					if(!m_bWorking)
					{
						m_hr = m_pIUSocketPool->get_LockedSocketsEx(&lLocks);
						if(lLocks > 0) //stop previous parallel processing first
							return false;
						m_aPHandler->Clear();
						m_aPauseHandler->Clear();
						m_bWorking = true;
					}
				}
				THandler pHandler = CSocketPool<THandler>::Lock(0, IntPtr::Zero);
				bool bSuc = false;
				while(pHandler != nullptr)
				{
					if(!ExecuteJob(pHandler))
					{
						//if no more tasks available, we stop here
						CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
						break;
					}
					else
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						if(m_bPause)
							m_aPauseHandler->Add(pHandler);
						else
							m_aPHandler->Add(pHandler);
					}
					pHandler = CSocketPool<THandler>::Lock(0, IntPtr::Zero);
				}
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					m_bWorking = (SocketsInParallel > 0);
					if(m_bWorking)
					{
						bSuc = true;
						::ResetEvent(m_hWait);
					}
					else
						::SetEvent(m_hWait);

					m_bPause = (m_aPauseHandler->Count > 0);
				}
				return bSuc;
			}
			
			/// <summary>
			/// Pause parallel computation gracefully. After pausing, you can call the method Resume to resume parallel computation.
			/// </summary>
			virtual void Pause()
			{
				THandler pHandler;
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					if(m_bPause || !m_bWorking)
						return;
					m_bPause = true;
					if(m_aPHandler->Count > 0)
						pHandler = m_aPHandler[0];
					else
						return;
				}

				do
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
						pHandler->GetAttachedClientSocket()->WaitAll();
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						m_aPauseHandler->Add(pHandler);
						m_aPHandler->Remove(pHandler);
						if(m_aPHandler->Count > 0)
							pHandler = m_aPHandler[0];
						else
							break;
					}
				}while(true);
			}
			
			/// <summary>
			/// Call the method to resume parallel computation after calling the method Pause.
			/// </summary>
			virtual void Resume()
			{
				int n;
				msclr::lock AutoLock((Object^)m_mapSocket);
				if(!m_bPause)
					return;
				int nSize = m_aPauseHandler->Count;
				if(m_bWorking && m_bPause && nSize > 0)
				{
					for(n=nSize-1; n>=0; --n)
					{
						THandler pHandler = m_aPauseHandler[n];
						m_aPauseHandler->Remove(pHandler);
						AutoLock.release();
						if(ExecuteJob(pHandler))
						{
							AutoLock.acquire();
							m_aPHandler->Add(pHandler);
						}
						else
						{
							AutoLock.acquire();
							CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
						}
					}
				}
				m_bPause = false;
				if(m_aPHandler->Count == 0)
					::SetEvent(m_hWait);
				else
					::ResetEvent(m_hWait);
			}
			
			/// <summary>
			/// Stop parallel computation gracefully until all of scheduled jobs are finished.
			/// </summary>
			void Stop()
			{
				return Stop(false);
			}

			/// <summary>
			/// Stop parallel computation. If the parameter bCancelRequestsInQueue is set to true, socket pool will send a request Cancel to server for shutting down computation as soon as possible.
			/// If the parameter bCancelRequestsInQueue is set to false by default, it stops computation gracefully until all of scheduled jobs are finished.
			/// </summary>
			virtual void Stop(bool bCancelRequestsInQueue)
			{
				bool bLoop = false;
				THandler pHandler;
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					if(!m_bWorking)
						return;
					m_bWorking = false;
					if(m_aPHandler->Count > 0)
					{
						pHandler = m_aPHandler[0];
						m_aPHandler->Remove(pHandler);
						bLoop = true;
					}
				}
				m_JobManager->CleanTasks(!bCancelRequestsInQueue);
				while(bLoop)
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
					{
						if(bCancelRequestsInQueue)
							MyCancel(pHandler);
						pHandler->GetAttachedClientSocket()->WaitAll();
					}
					CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						if(m_aPHandler->Count > 0)
						{
							pHandler = m_aPHandler[0];
							m_aPHandler->Remove(pHandler);
						}
						else
							break;
					}
				}

				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					if(m_aPauseHandler->Count > 0)
					{
						pHandler = m_aPauseHandler[0];
						m_aPauseHandler->Remove(pHandler);
						bLoop = true;
					}
					else
						bLoop = false;
				}
				while(bLoop)
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
					{
						if(bCancelRequestsInQueue)
							MyCancel(pHandler);
						pHandler->GetAttachedClientSocket()->WaitAll();
					}
					CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						if(m_aPauseHandler->Count > 0)
						{
							pHandler = m_aPauseHandler[0];
							m_aPauseHandler->Remove(pHandler);
						}
						else
							break;
					}
				}
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					m_bPause = false;
				}
				::SetEvent(m_hWait);
			}
			
			/// <summary>
			/// Shut down socket pool, and clean all of associated resources.
			/// </summary>
			virtual void ShutdownPool() override
			{
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					for each (CClientSocket ^cs in m_mapSocket->Keys)
					{
						cs->m_OnAllRequestsProcessed -= gcnew DOnAllRequestsProcessed(this, &CSocketPoolEx::OnAllRequestsProcessed);
						cs->m_OnClosing -= gcnew DOnClosing(this, &CSocketPoolEx::OnClosing);
						cs->m_OnSocketClosed -= gcnew DOnSocketClosed(this, &CSocketPoolEx::OnSocketClosed);
						cs->m_OnRequestProcessed -= gcnew DOnRequestProcessed(this, &CSocketPoolEx::OnRequestProcessed);
					}
				}
				CSocketPool<THandler>::ShutdownPool();
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_JobManager->m_lstAsyncHandler->Clear();
			}

		protected:
			/// <summary>
			/// Called when a fail happens. If the callback returns true, there is a fail recovery. Otherwise, there is no fail recovery.
			/// </summary>
			virtual bool OnFailover(THandler Handler, IJobContext ^JobContext)
			{
				return true;
			}
			
			/// <summary>
			/// Called when all the tasks in job are finished.
			/// </summary>
			virtual void OnJobDone(THandler Handler, IJobContext ^JobContext)
			{
				
			}
			
			/// <summary>
			/// Called when there is a job which is about to be sent to SocketPro server for processing. Specifically, if the callback returns false, the job (JobContext) is discarded.
			/// </summary>
			virtual bool OnExecutingJob(THandler Handler, IJobContext ^JobContext)
			{
				return true;
			}
			
			/// <summary>
			/// Called when there is a job under processing.
			/// </summary>
			virtual void OnJobProcessing(THandler Handler, IJobContext ^JobContext)
			{
				
			}
			
			/// <summary>
			/// Called when a return result is returned and processed
			/// </summary>
			virtual void OnReturnedResultProcessed(THandler Handler, IJobContext ^JobContext, short sRequestId)
			{

			}

			/// <summary>
			/// Called when all of sockets in pool are disconnected.
			/// </summary>
			virtual void OnAllSocketsDisconnected()
			{
				
			}

			/// <summary>
			/// The virtual method will be called when there is a socket pool related event.
			/// Note that dead lock may happen if you call one of methods of USocketPool object inside the virtual function.
			/// </summary>
			virtual void OnSocketPoolEvent(USOCKETLib::tagSocketPoolEvent spe, USOCKETLib::USocket ^pIUSocket) override
			{
				CSocketPool<THandler>::OnSocketPoolEvent(spe, pIUSocket);
				switch(spe)
				{
				case USOCKETLib::tagSocketPoolEvent::speUSocketCreated:
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						for each (CClientSocket ^cs in m_mapSocket->Keys)
						{
							if(cs->GetUSocket() == pIUSocket)
							{
								cs->m_OnAllRequestsProcessed += gcnew DOnAllRequestsProcessed(this, &CSocketPoolEx::OnAllRequestsProcessed);
								cs->m_OnClosing += gcnew DOnClosing(this, &CSocketPoolEx::OnClosing);
								cs->m_OnSocketClosed += gcnew DOnSocketClosed(this, &CSocketPoolEx::OnSocketClosed);
								cs->m_OnRequestProcessed += gcnew DOnRequestProcessed(this, &CSocketPoolEx::OnRequestProcessed);
								if(!m_bServerLoadingBalance)
								{
									cs->m_pIJobManager = m_JobManager;
									cs->m_pIProcess = this;
								}
								CAutoLock	AutoLock(&g_cs.m_sec);
								m_JobManager->m_lstAsyncHandler->Add(m_mapSocket[cs]);
								break;
							}
						}
					}
					break;
				default:
					break;
				}
			}

		private:
			static void MyCancel(CAsyncServiceHandler ^pHandler)
			{
				pHandler->GetAttachedClientSocket()->Cancel();
				pHandler->GetAttachedClientSocket()->GetUSocket()->DoEcho();
			}

			void Failover(THandler pHandler)
			{
				CJobContext ^pJC;
				CAutoLock	AutoLock(&g_cs.m_sec);
				for each (pJC in m_JobManager->m_aJobProcessing)
				{
					if(pJC->m_pHandler == pHandler)
					{
						bool b = false;
						if(pJC->m_Status == tagJobStatus::jsRunning)
						{
							m_nFails++;
							{
								CAutoReverseLock arl(&g_cs.m_sec);
								b = OnFailover(pHandler, pJC);
							}
						}
						if(m_JobManager->m_aJobProcessing->Remove(pJC))
						{
							if(b)
							{
								pJC->m_Status = tagJobStatus::jsQueued;
								Object ^Identity = pJC->Identity;
								List<CJobContext^> ^jobs = nullptr;
								if(m_JobManager->m_mapQueuedIdentityJob->ContainsKey(Identity))
									jobs = m_JobManager->m_mapQueuedIdentityJob[Identity];
								if(jobs == nullptr)
								{
									jobs = gcnew List<CJobContext^>();
									m_JobManager->m_mapQueuedIdentityJob->Add(Identity, jobs);
								}
								jobs->Add(pJC);
								Object ^obj = nullptr;
								pJC->m_pHandler = (THandler)obj;
							}
							else
							{
								EmptyJob(pJC);
							}
						}
						break;
					}
				}
			}

			CJobContext^ RemoveJobContext()
			{
				int n = 0;
				int nIndex = (m_lRemoveIndex%m_JobManager->m_mapQueuedIdentityJob->Count);
				for each (Object ^Identity in m_JobManager->m_mapQueuedIdentityJob->Keys)
				{
					if(n == nIndex)
					{
						List<CJobContext^> ^jobs = m_JobManager->m_mapQueuedIdentityJob[Identity];
						CJobContext ^job = jobs[0];
						jobs->RemoveAt(0);
						if(jobs->Count == 0)
							m_JobManager->m_mapQueuedIdentityJob->Remove(Identity);
						return job;
					}
					n++;
				}
				return nullptr;
			}

			void ResultReturned(THandler pHandler, short sRequestId)
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				for each(CJobContext ^pJC in m_JobManager->m_aJobProcessing)
				{
					if(pJC->m_pHandler == pHandler)
					{
						if(pJC->m_Status != tagJobStatus::jsRunning)
							break;
						if(!m_bServerLoadingBalance)
						{
							bool bClient;
							pJC->RemoveTask(pJC->m_pHandler->GetAttachedClientSocket()->ReturnRandom, (unsigned short)sRequestId, bClient);
						}
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							OnReturnedResultProcessed(pHandler, pJC, sRequestId);
						}
						break;
					}
				}
			}

			bool ExecuteJob(THandler pHandler)
			{
				HRESULT	hr;
				unsigned long nReqSize;
				CJobContext ^pJC;
				bool bNext = false;
				CAutoLock	AutoLock(&g_cs.m_sec);
				for each(pJC in m_JobManager->m_aJobProcessing)
				{
					if(pJC->m_pHandler == pHandler)
					{
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							OnJobDone(pHandler, pJC);
						}
						if(pJC->m_Callback != nullptr)
							pJC->m_Callback->Invoke(pJC);
						if(m_JobManager->m_aJobProcessing->Remove(pJC))
							EmptyJob(pJC);
						break;
					}
				}

				unsigned int nCountOfTasks;
				do
				{
					if(m_JobManager->m_mapQueuedIdentityJob->Count == 0)
						return false;
					pJC = RemoveJobContext();
					m_JobManager->m_aJobProcessing->Add(pJC);
					{
						CAutoReverseLock arl(&g_cs.m_sec);
						bNext = OnExecutingJob(pHandler, pJC);
					}
					nCountOfTasks = pJC->CountOfTasks;
					m_lRemoveIndex++;
					if(bNext && nCountOfTasks > 0)
						break;
					else
					{
						m_JobManager->m_aJobProcessing->Remove(pJC);
						EmptyJob(pJC);
					}
				}while(true);

				pJC->m_pHandler = pHandler;
				pJC->m_Status = tagJobStatus::jsRunning;
				IUFast *pIUFast = (IUFast*)(pHandler->GetAttachedClientSocket()->GetIUFast().ToPointer());
				IUSocket *pIUSocket = (IUSocket *)(pHandler->GetAttachedClientSocket()->GetIUSocket().ToPointer());
				if(pJC->m_bBundled)
					hr = pIUFast->SendRequestEx(idStartJob, 0, NULL);
				unsigned long ulPos = 0;
				CInternalUQueue &UQueue = *(pJC->m_Tasks->GetInternalUQueue());
				unsigned long ulSize = UQueue.GetSize();
				while(ulSize >= (ulPos + sizeof(unsigned short) + sizeof(unsigned long)))
				{
					ulPos += sizeof(int); //skip task id
					unsigned short usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
					ulPos += sizeof(unsigned short);
					nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos));
					ulPos += sizeof(nReqSize);
					switch(usRequestId)
					{
					case idStartBatching:
						hr = pIUSocket->StartBatching();
						break;
					case idCommitBatching:
						{
							VARIANT_BOOL b = VARIANT_FALSE;
							if (nReqSize == 1)
							{
								BYTE byte = *((BYTE*)UQueue.GetBuffer(ulPos));
								if(byte != 0)
									b = VARIANT_TRUE;
							}
							else if(nReqSize == 2)
							{
								short s = *((short*)UQueue.GetBuffer(ulPos));
								if(s != 0)
									b = VARIANT_TRUE;
							}
							else
							{
								//should not come here
								ATLASSERT(FALSE);
							}
							hr = pIUSocket->CommitBatching(b);
						}
						break;
					default:
						hr = pIUFast->SendRequestEx(usRequestId, nReqSize, (BYTE*)UQueue.GetBuffer(ulPos));
						break;
					}
					ulPos += nReqSize;
					ulPos += sizeof(bool); //Client request or not
					if(hr != S_OK)
						break;
				}
				if(pJC->m_bBundled)
					hr = pIUFast->SendRequestEx(idEndJob, 0, NULL);
				{
					CAutoReverseLock arl(&g_cs.m_sec);
					OnJobProcessing(pHandler, pJC);
				}
				return true;
			}

			void EmptyJob(CJobContext ^pJC)
			{
				m_JobManager->EmptyJob(pJC);
			}

			void OnClosing(int hSocket, int hWnd)
			{
				msclr::lock AutoLock((Object^)m_mapSocket);
				if(!m_bWorking)
					return;
				for each (CClientSocket ^cs in m_mapSocket->Keys)
				{
					if(cs->Socket == hSocket)
					{
						THandler pHandler = m_mapSocket[cs];
						if(cs->GetCountOfRequestsInQueue() > 0)
						{
							AutoLock.release();
							Failover(pHandler);
							AutoLock.acquire();
						}
						bool bRemove = m_aPHandler->Remove(pHandler);
						if(bRemove)
							CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
						else 
						{
							bRemove = m_aPauseHandler->Remove(pHandler);
							if(bRemove)
								CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
						}
						if(bRemove && SocketsInParallel == 0)
							m_bCallProcess = true;
						break;
					}
				}
			}

			void OnAllRequestsProcessed(int hSocket, short sRequestID)
			{
				bool bLoop = false;
				Object ^obj = nullptr;
				THandler pHandler = (THandler)obj;
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					if(!m_bWorking)
						return;
					for each (CClientSocket ^cs in m_mapSocket->Keys)
					{
						if(cs->Socket == hSocket)
						{
							if(!m_bPause)
							{
								pHandler = m_mapSocket[cs];
								bLoop = true;
							}
							break;
						}
					}
				}
				if(bLoop)
				{
					if(!ExecuteJob(pHandler))
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						if(m_aPHandler->Remove(pHandler))
						{
							CSocketPool<THandler>::Unlock(pHandler->GetAttachedClientSocket());
						}
						if(SocketsInParallel == 0)
						{
							m_bWorking = false;
							m_bPause = false;
							::SetEvent(m_hWait);
						}
					}
				}
			}

			void OnSocketClosed(int hSocket, int nError)
			{
				bool b = false;
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					b = m_bCallProcess;
					if(b)
						m_bCallProcess = false;
				}
				if(b)
				{
					if(!Process())
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						m_bWorking = false;
						m_bPause = false;
						::SetEvent(m_hWait);
					}
				}
				long lConnected = 0;
				m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
				if(lConnected == 0)
				{
					OnAllSocketsDisconnected();
					m_JobManager->CancelJobs(false);
					m_JobManager->ShrinkMemory(DEFAULT_UQUEUE_BLOCK_SIZE);
				}
				::SetEvent(m_JobManager->m_hIdentityWait);
			}

			void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib::tagReturnFlag ReturnFlag)
			{
				if(ReturnFlag != USOCKETLib::tagReturnFlag::rfCompleted)
					return;
				msclr::lock AutoLock((Object^)m_mapSocket);
				for each (CClientSocket ^cs in m_mapSocket->Keys)
				{
					if(cs->Socket == hSocket)
					{
						THandler pHandler = m_mapSocket[cs];
						AutoLock.release(); 
						ResultReturned(pHandler, sRequestID);
						AutoLock.acquire();
						break;
					}
				}
			}
		internal:
			bool					m_bServerLoadingBalance;
			CJobManager				^m_JobManager;

		private:
			bool					m_bPause;
			bool					m_bWorking;
			bool					m_bCallProcess;
			unsigned int			m_nFails;
			HANDLE					m_hWait;
			List<THandler>			^m_aPHandler;
			List<THandler>			^m_aPauseHandler;
			unsigned long			m_lRemoveIndex;
		};
	};
}

#endif