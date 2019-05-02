#ifndef ___SOCKETPRO_ADAPTER_PLG_SERVICE_H__
#define ___SOCKETPRO_ADAPTER_PLG_SERVICE_H__

#include "ClientPeer.h"
#include "RequestAsynHandlerBase.h"
#include "jobcontext.h"

namespace SocketProAdapter
{
namespace ServerSide
{
namespace PLG
{
	/// <summary>
	/// An interface for notifying a peer job creation processing and its returning results from real servers.
	/// The interface must be implemented for CPLGService generic class when you need SocketPro loading balance, parallel and grid computing service at SocketPro server side.
	/// </summary>
	[CLSCompliantAttribute(true)] 
	public interface class IPeerJobContext
	{
		/// <summary>
		/// Called when a job (JobContext) has just been created with a request id from a peer.
		/// </summary>
		void OnJobJustCreated(IJobContext ^JobContext, short sRequestId);

		/// <summary>
		/// Called when a job (JobContext) is about to be enqueued with a request id from a peer.
		/// </summary>
		void OnEnqueuingJob(IJobContext ^JobContext, short sRequestId);
		
		/// <summary>
		/// Called when a job (JobContext) has just been enqueued with a request id from a peer.
		/// </summary>
		void OnJobEnqueued(IJobContext ^JobContext, short sRequestId);
		
		/// <summary>
		/// Called when a task is about to be added into a job (JobContext) with a request id from a peer.
		/// </summary>
		void OnAddingTask(IJobContext ^JobContext, short sRequestId);
		
		/// <summary>
		/// Called when a task (nTaskId) has just been added into a job (JobContext) with a request id from a peer.
		/// </summary>
		void OnTaskJustAdded(IJobContext ^JobContext, int nTaskId, short sRequestId);
		
		/// <summary>
		/// Called when a returning result inside UQueue is about to be sent to a peer for a request (sRequestId).
		/// If the callback returns false, the returning result is discarded. Otherwise, the result will be sent to the client (peer).
		/// </summary>
		bool OnSendingPeerData(IJobContext ^JobContext, short sRequestId, CUQueue ^UQueue);
		
		/// <summary>
		/// Called when a returning result from a real server has just been sent to the peer.
		/// </summary>
		void OnPeerDataSent(IJobContext ^JobContext, short sRequestId);

		/// <summary>
		/// Called when you can call IJobContext.AsyncWaitHandle for waiting until the job is finished after a task (nTaskId) has just been added into a job (JobContext) with a request id from a peer.
		/// </summary>
		void OnWaitable(IJobContext ^JobContext, int nTaskId, short sRequestId);
		
		/// <summary>
		/// The job manager. 
		/// </summary>
		property IJobManager^ JobManager
		{
			IJobManager^ get();
			void set(IJobManager ^jobManager);
		}
	};

	generic<typename TClientPeer> 
		where TClientPeer : CClientPeer, IPeerJobContext, gcnew()
	public ref class CPLGService : public CBaseService
	{
	public:
		CPLGService()
		{
			m_bUsePool = true;
			m_SP = gcnew CSPOnServer();
			m_SP->m_pPLGSvs = this;
			m_mapIdentityJob = gcnew Dictionary<TClientPeer, IJobContext^>();
		}

		static property long ServiceIdOnRealServer
		{
			long get()
			{
				return m_nServiceIdOnRealServer;
			}
			void set(long nServiceIdOnRealServer)
			{
				m_nServiceIdOnRealServer = nServiceIdOnRealServer;
			}
		}

		virtual bool AddMe(int nSvsID, int nEvents, tagThreadApartment taWhatTA) override
		{
			bool bSuc = CBaseService::AddMe(nSvsID, nEvents, taWhatTA);
			if(!bSuc)
				return false;
			ReturnRandom = true;
			if(!m_bUsePool)
				throw gcnew Exception("Must set the m_bUsePool to true!");
			return bSuc;
		}



	protected:
		virtual CClientPeer^ GetPeerSocket(int hSocket) override
		{
			if(!m_bUsePool)
				throw gcnew Exception("Must set the m_bUsePool to true!");
			TClientPeer clientPeer = gcnew TClientPeer();
			clientPeer->JobManager = m_SP->JobManager;
			return clientPeer;
		}
		
		/// <summary>
		/// Called when a fail happens. If the callback returns true, there is a fail recovery. Otherwise, there is no fail recovery.
		/// </summary>
		virtual bool OnFailover(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext)
		{
			return true;
		}
		
		/// <summary>
		/// Called when all the tasks in the job (JobContext) are finished.
		/// </summary>
		virtual void OnJobDone(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext)
		{

		}
		
		/// <summary>
		/// Called when a job (JobContext) is about to be sent to SocketPro real server for processing. Specifically, if the callback returns false, the job (JobContext) will be discarded.
		/// </summary>
		virtual bool OnExecutingJob(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext)
		{
			return true;
		}
		
		/// <summary>
		/// Called when a job (JobContext) is processing.
		/// </summary>
		virtual void OnJobProcessing(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext)
		{
			
		}
		
		/// <summary>
		/// Called when a return result has been just returned and processed
		/// </summary>
		virtual void OnReturnedResultProcessed(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext, short sRequestId)
		{

		}

		/// <summary>
		/// Called when all of sockets in pool are disconnected.
		/// </summary>
		virtual void OnAllSocketsDisconnected()
		{

		}
		
		/// <summary>
		/// Called when a base request is processed from a remote SocketPro server. 
		/// Note that JobContext can be null or nothing.
		/// The parameter sBaseRequestId is the base request id, which must be less than 46.
		/// </summary>
		virtual void OnBaseRequestProcessed(ClientSide::CAsyncServiceHandler ^Handler, IJobContext ^JobContext, short sBaseRequestId)
		{

		}

	private:
		int AddTask(TClientPeer pClientPeer, short usRequestID, IJobContext ^%MyJobContext)
		{
			bool bCreated = false;
			ClientSide::CJobContext ^pIJobContext;
			if(m_mapIdentityJob->ContainsKey(pClientPeer))
			{
				pIJobContext = (ClientSide::CJobContext^)m_mapIdentityJob[pClientPeer];
				ATLASSERT(pIJobContext != nullptr);
				bCreated = true;
			}
			else
			{
				pIJobContext = (ClientSide::CJobContext^)m_SP->JobManager->CreateJob(pClientPeer);
				ATLASSERT(pIJobContext != nullptr);
				{
					CAutoReverseLock arl(&g_cs.m_sec);
					pClientPeer->OnJobJustCreated(pIJobContext, usRequestID);
				}
			}
			CUQueue ^UQueue = pClientPeer->GetUQueue();
			int idTask;
			{
				CAutoReverseLock arl(&g_cs.m_sec);
				pClientPeer->OnAddingTask(pIJobContext, usRequestID);
				idTask = pIJobContext->AddTask(usRequestID, UQueue->GetBuffer(), UQueue->GetSize(), true);
				ATLASSERT(idTask != 0);
				pClientPeer->OnTaskJustAdded(pIJobContext, idTask, usRequestID);
				if(!bCreated)
				{
					pClientPeer->OnEnqueuingJob(pIJobContext, usRequestID);
					bool bSuc = m_SP->JobManager->EnqueueJob(pIJobContext);
					ATLASSERT(bSuc);
					pClientPeer->OnJobEnqueued(pIJobContext, usRequestID);
					if(bSuc)
						m_SP->Process();
					MyJobContext = pIJobContext;
				}
				else
					MyJobContext = nullptr;
			}
			UQueue->SetSize(0);
			return idTask;
		}

	internal:
		virtual void OnBaseRequestCame(int hSocket, short sRequestID) override
		{
			CAutoLock	AutoLock(&g_cs.m_sec);

			//Don't set m_bUserPool to false!!!
			ATLASSERT(m_bUsePool); 

			TClientPeer pClientPeer = (TClientPeer)SeekClientPeer(hSocket);
			ATLASSERT(pClientPeer != nullptr);
	#ifdef _DEBUG
			//Automatical buffering must be enabled!
			ATLASSERT(pClientPeer->m_bAutoBuffer);
	#endif
			switch(sRequestID)
			{
			case idStartBatching:
				{
					if(m_mapIdentityJob->ContainsKey(pClientPeer))
					{
						ClientSide::CJobContext ^pIJobContext = (ClientSide::CJobContext^)m_mapIdentityJob[pClientPeer];
						ATLASSERT(pIJobContext != nullptr);
						int idTask = pIJobContext->AddTask(sRequestID, IntPtr::Zero, 0, true);
						ATLASSERT(idTask != 0);
					}
				}
				break;
			case idCommitBatching:
				{
					if(m_mapIdentityJob->ContainsKey(pClientPeer))
					{
						VARIANT_BOOL b = VARIANT_TRUE;
						ClientSide::CJobContext ^pIJobContext = (ClientSide::CJobContext^)m_mapIdentityJob[pClientPeer];
						ATLASSERT(pIJobContext != nullptr);
						int idTask = pIJobContext->AddTask(sRequestID, IntPtr(&b), sizeof(b), true);
						ATLASSERT(idTask != 0);
					}
				}
				break;
			case idCancel:
				{
					bool b = m_SP->m_bServerLoadingBalance;
					m_SP->m_bServerLoadingBalance = false;
					m_SP->JobManager->CancelJobs(pClientPeer);
					m_mapIdentityJob->Remove(pClientPeer);
					m_SP->m_bServerLoadingBalance = b;
				}
				break;
			default:
				break;
			}
		}

		virtual void OnRelease(int hSocket, bool bClose, long lInfo) override
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			TClientPeer pClientPeer = (TClientPeer)SeekClientPeer(hSocket);
			if(pClientPeer == nullptr)
				return;
			bool b = m_SP->m_bServerLoadingBalance;
			m_SP->m_bServerLoadingBalance = false;
			m_SP->JobManager->CancelJobs(pClientPeer);
			m_mapIdentityJob->Remove(pClientPeer);
			m_SP->m_bServerLoadingBalance = b;
		}

		virtual void OnFastRequestArrive(int hSocket, short sRequestID) override
		{
			OnMyProcess(hSocket, sRequestID);
		}

		void OnMyProcess(int hSocket, short sRequestID)
		{
			int nTaskId = 0;
			IJobContext ^myJobContext = nullptr;
			CAutoLock	AutoLock(&g_cs.m_sec);
			TClientPeer pClientPeer = (TClientPeer)SeekClientPeer(hSocket);
			ATLASSERT(pClientPeer != nullptr);
			switch(sRequestID)
			{
			case idStartJob:
				{
					ClientSide::CJobContext ^pIJobContext = (ClientSide::CJobContext^)m_SP->JobManager->CreateJob(pClientPeer);
					ATLASSERT(pIJobContext != nullptr);
					if(pIJobContext != nullptr)
					{
						m_mapIdentityJob->Add(pClientPeer, pIJobContext);
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							pClientPeer->OnJobJustCreated(pIJobContext, sRequestID);
						}
					}
				}
				break;
			case idEndJob:
				{
					if(m_mapIdentityJob->ContainsKey(pClientPeer))
					{
						IJobContext ^pIJobContext = m_mapIdentityJob[pClientPeer];
						m_mapIdentityJob->Remove(pClientPeer);
						pClientPeer->OnEnqueuingJob(pIJobContext, sRequestID);
						bool bSuc = m_SP->JobManager->EnqueueJob(pIJobContext);
						ATLASSERT(bSuc);
						pClientPeer->OnJobEnqueued(pIJobContext, sRequestID);
						if(bSuc)
							m_SP->Process();
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							pClientPeer->OnWaitable(pIJobContext, 0, sRequestID);
						}
					}
				}
				break;
			default:
				nTaskId = AddTask(pClientPeer, sRequestID, myJobContext);
				break;
			}
			if(nTaskId != 0 && myJobContext != nullptr)
			{
				CAutoReverseLock arl(&g_cs.m_sec);
				pClientPeer->OnWaitable(myJobContext, nTaskId, sRequestID);	
			}
		}

		virtual void OnSlowRequestArrive(int hSocket, short sRequestID) override
		{
			OnMyProcess(hSocket, sRequestID);
		}

	public:
		ref class CPLGHandler: public ClientSide::CAsyncServiceHandler
		{
		public:
			CPLGHandler() : ClientSide::CAsyncServiceHandler(m_nServiceIdOnRealServer),
				m_pJobContext(nullptr), m_pPLGSvs(nullptr)
			{
			}

		public:
			IJobContext^ GetAssocatedJobContext()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_pJobContext;
			}

		internal:
			void AssociateJobContext(IJobContext ^JobContext)
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_pJobContext = JobContext;
			}

			static property long ServiceIdOnRealServer
			{
				long get()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_nServiceIdOnRealServer;
				}
				void set(long nServiceIdOnRealServer)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_nServiceIdOnRealServer = nServiceIdOnRealServer;
				}
			}

		protected:
			virtual void OnExceptionFromServer(CSocketProServerException ^Exception) override
			{
				CScopeUQueue su;
				su.m_UQueue->Push(Exception);
				OnResultReturned(Exception->m_sRequestID, su.m_UQueue);
			}

			virtual void OnResultReturned(short usRequestID, CUQueue ^UQueue) override
			{
				do
				{
					bool bClient = false;
					bool bDrop = false;
					bool bRandomReturn = GetAttachedClientSocket()->ReturnRandom;
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_pJobContext == nullptr || m_pJobContext->JobStatus != tagJobStatus::jsRunning)
						break;
					TClientPeer pClientPeer;
					try
					{
						pClientPeer = (TClientPeer)m_pJobContext->Identity;
					}
					catch(...)
					{
						Object^ myNull = nullptr;
						pClientPeer = (TClientPeer)myNull;
					}
					if(pClientPeer == nullptr)
						break;
					int hSocket = pClientPeer->Socket;
					long ulSvsId = pClientPeer->SvsID;
					{
						CAutoReverseLock arl(&g_cs.m_sec);
						if(pClientPeer->OnSendingPeerData(m_pJobContext, usRequestID, UQueue))
						{
							CBaseService ^pBase = CBaseService::GetBaseService(ulSvsId);
							if(pBase == nullptr)
								break;
							pClientPeer = (TClientPeer)pBase->SeekClientPeer(hSocket);
							if(pClientPeer == nullptr)
								break;
							pClientPeer->SendReturnData(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
							
							pBase = CBaseService::GetBaseService(ulSvsId);
							if(pBase == nullptr)
								break;
							pClientPeer = (TClientPeer)pBase->SeekClientPeer(hSocket);
							if(pClientPeer == nullptr)
								break;
							pClientPeer->OnPeerDataSent(m_pJobContext, usRequestID);
						}
						else
						{
							bDrop = true;
						}
					}
					if(m_pJobContext == nullptr)
						break;
					SocketProAdapter::ClientSide::CJobContext ^pJobContext = (SocketProAdapter::ClientSide::CJobContext^)m_pJobContext;
					pJobContext->RemoveTask(bRandomReturn, usRequestID, bClient);
					if(bClient && bDrop)
					{
						CBaseService ^pBase = CBaseService::GetBaseService(ulSvsId);
						if(pBase == nullptr)
							break;
						pClientPeer = (TClientPeer)pBase->SeekClientPeer(hSocket);
						if(pClientPeer == nullptr)
							break;
						pClientPeer->DropRequestResult(usRequestID);
					}
				}while(false);
				UQueue->SetSize(0);
			}
		internal:
			CPLGService<TClientPeer> ^m_pPLGSvs;

		private:
			IJobContext		^m_pJobContext;
		};

	public:
		typedef ClientSide::CSocketPoolEx<CPLGHandler^> PLGSocketPool;

	private:
		ref class CSPOnServer : public PLGSocketPool
		{
		public:
			CSPOnServer() : m_pPLGSvs(nullptr)
			{
				m_bServerLoadingBalance = true;
				m_JobManager->m_bServerLoadingBalance = m_bServerLoadingBalance;
			}

		public:
			Dictionary<ClientSide::CClientSocket^, CPLGHandler^>^ GetSocketHandlerDictionary()
			{
				msclr::lock AutoLock((Object^)m_mapSocket);
				return m_mapSocket;
			}

			virtual void ShutdownPool() override
			{
				{
					msclr::lock AutoLock((Object^)m_mapSocket);
					for each (SocketProAdapter::ClientSide::CClientSocket ^cs in m_mapSocket->Keys)
					{
						cs->m_OnOtherMessage -= gcnew SocketProAdapter::ClientSide::DOnOtherMessage(this, &CSPOnServer::OnOtherMessage);
					}
				}
				PLGSocketPool::ShutdownPool();
			}

		private:
			void OnOtherMessage(int hSocket, int nMsg, int wParam, int lParam)
			{
				switch(nMsg)
				{
				case msgRequestRemoved:
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						for each (SocketProAdapter::ClientSide::CClientSocket ^cs in m_mapSocket->Keys)
						{
							if(cs->Socket == hSocket)
							{
								TClientPeer pClientPeer;
								try
								{
									pClientPeer = (TClientPeer)m_mapSocket[cs];
								}
								catch(...)
								{
									Object ^myNull = nullptr;
									pClientPeer = (TClientPeer)myNull;
								}
								if(pClientPeer == nullptr)
									break;
								pClientPeer->DropRequestResult((short)wParam);
								break;
							}
						}
					}
					break;
				default:
					break;
				}
			}

		protected:
			virtual void OnSocketPoolEvent(USOCKETLib::tagSocketPoolEvent spe, USOCKETLib::USocket ^pIUSocket) override
			{
				PLGSocketPool::OnSocketPoolEvent(spe, pIUSocket);
				switch(spe)
				{
				case USOCKETLib::tagSocketPoolEvent::speUSocketCreated:
					{
						msclr::lock AutoLock((Object^)m_mapSocket);
						for each (CPLGHandler ^Handler in m_mapSocket->Values)
						{
							if(Handler->m_pPLGSvs == nullptr)
							{
								Handler->m_pPLGSvs = m_pPLGSvs;
								Handler->GetAttachedClientSocket()->m_OnOtherMessage += gcnew SocketProAdapter::ClientSide::DOnOtherMessage(this, &CSPOnServer::OnOtherMessage);
							}
						}
					}
					break;
				default:
					break;
				}
			}

			virtual bool OnFailover(CPLGHandler ^pHandler, IJobContext ^JobContext) override
			{
				return m_pPLGSvs->OnFailover(pHandler, JobContext);
			}

			virtual bool OnExecutingJob(CPLGHandler ^pHandler, IJobContext ^JobContext) override
			{
				return m_pPLGSvs->OnExecutingJob(pHandler, JobContext);
			}

			virtual void OnReturnedResultProcessed(CPLGHandler ^pHandler, IJobContext ^JobContext, short sRequestId) override
			{
				m_pPLGSvs->OnReturnedResultProcessed(pHandler, JobContext, sRequestId);
			}

			virtual void OnAllSocketsDisconnected() override
			{
				m_pPLGSvs->OnAllSocketsDisconnected();
			}

			virtual void OnJobDone(CPLGHandler ^pHandler, IJobContext ^JobContext) override
			{
				m_pPLGSvs->OnJobDone(pHandler, JobContext);
				pHandler->AssociateJobContext(nullptr);
			}

			virtual void OnJobProcessing(CPLGHandler ^pHandler, IJobContext ^JobContext) override
			{
				pHandler->AssociateJobContext(JobContext);
				m_pPLGSvs->OnJobProcessing(pHandler, JobContext);
			}

		internal:
			CPLGService<TClientPeer> ^m_pPLGSvs;
		};

	public:
		property PLGSocketPool^ SocketPool
		{
			PLGSocketPool ^get()
			{
				return m_SP;
			}
		}

		property Dictionary<ClientSide::CClientSocket^, CPLGHandler^>^ SocketHandlerDictionary
		{
			Dictionary<ClientSide::CClientSocket^, CPLGHandler^> ^get()
			{
				return m_SP->GetSocketHandlerDictionary();
			}
		}

	private:
		CSPOnServer	^m_SP;
		Dictionary<TClientPeer, IJobContext^> ^m_mapIdentityJob;
		static long	m_nServiceIdOnRealServer = 0;
	};

}	//PLG

}	//Server Side

}	//SocketProAdapter

#endif
