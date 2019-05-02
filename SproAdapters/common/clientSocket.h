#ifndef __SOCKETPRO_ADAPTER_CLIENT_SIDE__H__
#define __SOCKETPRO_ADAPTER_CLIENT_SIDE__H__

#include <vcclr.h>
using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Threading;

#include "jobmanagerinterface.h"

namespace SocketProAdapter
{
	namespace ClientSide
	{
		[CLSCompliantAttribute(true)] 
		public delegate void DOnDataAvailable(int hSocket, int nBytes, int nError);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnClosing(int hSocket, int hWnd);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnConnecting(int hSocket, int hWnd);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnGetHostByAddr(int hHandle, String ^strHostName, String ^strHostAlias, int nError);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnGetHostByName(int hHandle, String ^strHostName, String ^strHostAlias, String ^strIPAddr, int nError);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnOtherMessage(int hSocket, int nMsg, int wParam, int lParam);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib::tagReturnFlag ReturnFlag);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnSendingData(int hSocket, int nError, int nSent);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnSocketClosed(int hSocket, int nError);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnSocketConnected(int hSocket, int nError);
		
		[CLSCompliantAttribute(true)] 
		public delegate void DOnBaseRequestProcessed(short sRequestID);

		[CLSCompliantAttribute(true)] 
		public delegate void DOnAllRequestsProcessed(int hSocket, short sRequestID);

		/// <summary>
		/// The class CClientSocket wraps all of commonly used functions found in the interfaces ISocketBase, IUSocket and IUFast.
		/// It also wraps the functions that can't be easily used through dotNet COM interop service.
		/// In case you need the functions that are not wrapped, you could use the method GetUSocket to get them through dotNet COM interop service.
		/// Additionally, each of instances of the class CClientSocket contains an instance of class CUQueue, 
		/// which is used to help sending all of requests and retrieving their returned results automatically through the interface IUFast.
		/// The class CClientSocket dispatches all of returned results onto one of attched instances of CAsyncServiceHandler-derived classes on the fly.
		/// To monitor various socket events, you could use delegates m_OnXXXX as shown in samples and tutorials.
		/// </summary>
		[CLSCompliantAttribute(true)] 
		public ref class CClientSocket : public IAsyncResult
		{
		private:
			ref class CUPushClientImpl : public IUPush
			{
			public:
				virtual bool Enter(array<int> ^Groups);
				virtual bool Broadcast(Object ^Message, array<int> ^Groups);
				virtual bool Broadcast(array<unsigned char> ^Message, array<int> ^Groups);
				virtual bool SendUserMessage(Object ^Message, String ^UserId);
				virtual bool SendUserMessage(String ^UserId, array<unsigned char> ^Message);
				virtual bool Exit();

			internal:
				CClientSocket ^m_ClientSocket;
			};

		public:
			CClientSocket();
			CClientSocket(bool bCreateOne);
			virtual ~CClientSocket();

			//construct this class instance with a valid existing USocket object
			CClientSocket(IntPtr pIUnknownToUSocket);

		public:
			property bool IsCompleted
			{
				virtual bool get()
				{
					if(m_pIUSocket == NULL)
						return true;
					long lCount = 0;
					m_pIUSocket->get_CountOfRequestsInQueue(&lCount);
					return (lCount == 0);
				}
			}

			property Object^ AsyncState
			{
				virtual Object^ get()
				{
					return m_AsyncState;
				}

				void set(Object^ AsyncState)
				{
					m_AsyncState = AsyncState;
				}
			}

			property WaitHandle^ AsyncWaitHandle
			{
				virtual WaitHandle^ get()
				{
					return m_mre;
				}
				void set(WaitHandle ^wh)
				{
					m_mre = wh;
				}
			}

			property bool CompletedSynchronously
			{
				virtual bool get()
				{
					return false;
				}
			}

			property IJobManager^ JobManager
			{
				IJobManager^ get()
				{
					return m_pIJobManager;
				}
			}

			property IUPush^ Push
			{
				IUPush^ get()
				{
					return m_pPush;
				}
			}

			property IJobContext^ CurrentJobContext
			{
				IJobContext^ get()
				{
					CAutoLock AutoLock(&m_pCS->m_sec);
					return m_pIJobContext;
				}
			}

		public:
/*			/// <summary>
			/// Send a message (UQueue) in binary onto one or more groups specified by nGroups through builtin chat service.
			/// You can send a message into multiple groups, for example, group 1 and group 2 (3).
			/// </summary>
			bool SpeakEx(int nGroups, CUQueue ^UQueue);
			
			/// <summary>
			/// Send an array of bytes (MessageBuffer) onto one or more groups specified by nGroups through builtin chat service.
			/// You can send a message into multiple groups, for example, group 1 and group 2 (3).
			/// </summary>
			bool SpeakEx(array<BYTE> ^MessageBuffer, int nGroups);
			
			/// <summary>
			/// Send an array of bytes (pBuffer) onto one or more groups specified by nGroups through builtin chat service.
			/// You can send a message into multiple groups, for example, group 1 and group 2 (3).
			/// </summary>
			bool SpeakEx(int nGroups, IntPtr pBuffer, int nLen);
			
			/// <summary>
			/// Send an array of bytes (UQueue) onto one specific client (strIPAddr/nPort) through builtin chat service.
			/// Note that strIPAddr must be in the format 111.222.212.121 only.
			/// </summary>
			bool SpeakToEx(String^ strIPAddr, int nPort, CUQueue ^UQueue);
			
			/// <summary>
			/// Send an array of bytes (MessageBuffer) onto one specific client (strIPAddr/nPort) through builtin chat service.
			/// Note that strIPAddr must be in the format 111.222.212.121 only.
			/// </summary>
			bool SpeakToEx(String^ strIPAddr, int nPort, array<BYTE> ^MessageBuffer);
			
			/// <summary>
			/// Send a given size (nLen) of bytes (pBuffer) onto one specific client (strIPAddr/nPort) through builtin chat service.
			/// Note that strIPAddr must be in the format 111.222.212.121 only.
			/// </summary>
			bool SpeakToEx(String^ strIPAddr, int nPort, IntPtr pBuffer, int nLen);
*/			
			/// <summary>
			/// Asynchronously connect to a remote server with a given server address (strHost) and a given port number (nPort).
			/// Note that strIPAddr can in one of three formats like 111.222.121.212, www.udaparts.com or AComputerName.
			/// </summary>
			bool Connect(String ^strHost, int nPort);
			
			/// <summary>
			/// Connect to a remote server with a given server address (strHost) and a given port number (nPort) either asynchronously and synchronously.
			/// Note that strIPAddr can in one of three formats like 111.222.121.212, www.udaparts.com or AComputerName.
			/// </summary>
			bool Connect(String ^strHost, int nPort, bool bSyn);
			
			//abort a socket connection
			/// <summary>
			/// abort a socket connection.
			/// </summary>
			void Disconnect();
			
			//gracefully close a socket connection
			/// <summary>
			/// gracefully close a socket connection.
			/// </summary>
			void Shutdown();

			/// <summary>
			/// Switch for a specified service (nSvsID) but doesn't transfer SocketProServer exception.
			/// </summary>
			virtual bool SwitchTo(int nSvsID);
			
			/// <summary>
			/// Switch for a specified service (nSvsID).
			/// If bAutoTransferServerException is set to true, it forces SocketPro server to transfer server exception to client.
			/// </summary>
			virtual bool SwitchTo(int nSvsID, bool bAutoTransferServerException);

			/// <summary>
			/// Switch for a specified service (pAsynHandler) but doesn't transfer SocketProServer exception.
			/// </summary>
			bool SwitchTo(CAsyncServiceHandler ^pAsynHandler);
			
			/// <summary>
			/// Switch for a specified service (pAsynHandler).
			/// If bAutoTransferServerException is set to true, it forces SocketPro server to transfer server exception to client.
			/// </summary>
			bool SwitchTo(CAsyncServiceHandler ^pAsynHandler, bool bAutoTransferServerException);
			
			/// <summary>
			/// Attach this CClientSocket instance to an underlying COM USocket object through a given interface (pIUnknownToUSocket).
			/// Note that if pIUnknownToUSocket is null or zero, this CClientSocket instance will be detached from a COM USocket.
			/// </summary>
			void SetUSocket(IntPtr pIUnknownToUSocket);

			/// <summary>
			/// Attach this CClientSocket instance to an underlying COM USocket object through a given dotNet COM interop wrap (USocket).
			/// Note that if USocket is null, this CClientSocket instance will be detached from a COM USocket.
			/// </summary>
			void SetUSocket(USOCKETLib::USocketClass ^USocket);
			
			bool IsConnected();
			
			IntPtr GetIUFast();
			IntPtr GetIUChat();
			IntPtr GetIUSocket();
			USOCKETLib::USocketClass ^GetUSocket();
			int GetCurrentServiceID();
			int GetErrorCode();
			String^ GetErrorMsg();
			
			/// <summary>
			/// Start batching requests.
			/// </summary>
			bool BeginBatching();

			/// <summary>
			/// Commit previously batched requests without asking for batching return results on server side.
			/// </summary>
			bool Commit();

			/// <summary>
			/// Commit previously batched requests. 
			/// If the input parameter bBatchingAtServer is true, the method informs a SocketPro server returns all of return results in batch.
			/// Otherwise, the results will not be returned in batch.
			/// </summary>
			bool Commit(bool bBatchingAtServer);
			
			/// <summary>
			/// Discard all of previously batched results if available.
			/// </summary>
			bool Rollback();
			bool IsBatching();
			int GetBytesBatched();

			/// <summary>
			/// Disable client graphic user interfaces. 
			/// If graphic user interfaces are disabled, graphic user interfaces will be frozen when you either call the method Wait or WaitAll or set the property Syn to true
			/// </summary>
			void DisableUI();
			
			/// <summary>
			/// Enable or disable client graphic user interfaces. 
			/// If graphic user interfaces are disabled, graphic user interfaces will be frozen when you either call the method Wait or WaitAll or set the property Syn to true
			/// </summary>
			void DisableUI(bool bDisable);

			/// <summary>
			/// Wait until one specified request (sRequestID) of current service is processed and returned with an infinite timeout (-1 or 0xFFFFFFFF).
			/// The method returns false if socket is closed. 
			/// The method returns true if socket is not closed.
			/// </summary>
			bool Wait(short sRequestID);

			/// <summary>
			/// Wait until one specified request of current service is processed and returned with a given timeout (nTimeout).
			/// The method returns false if the waiting is timeout or socket is closed. 
			/// The method returns true only if the waiting is not timeout and socket is not closed.
			/// </summary>
			bool Wait(short sRequestID, int nTimeout);

			/// <summary>
			/// Wait until one specified request (sRequestID) is processed and returned with a given timeout (nTimeout) and service id (nSvsID).
			/// The method returns false if the waiting is timeout or socket is closed. 
			/// The method returns true only if the waiting is not timeout and socket is not closed.
			/// </summary>
			bool Wait(short sRequestID, int nTimeout, int nSvsID);
			
			/// <summary>
			/// Wait until all of requests are processed and returned with an infinite timeout (-1 or 0xFFFFFFFF).
			/// The method returns false if socket is closed. 
			/// The method returns true if socket is not closed.
			/// </summary>
			bool WaitAll();

			/// <summary>
			/// Wait until all of requests are processed and returned with a given timeout (nTimeout).
			/// The method returns false if the waiting is timeout or socket is closed. 
			/// The method returns true only if the waiting is not timeout and socket is not closed.
			/// </summary>
			bool WaitAll(int nTimeout);
			
			/// <summary>
			/// Set a user id.
			/// </summary>
			void SetUID(String ^strUID);

			/// <summary>
			/// Set a password. Note that the password will be automatically cleaned right after calling the method SwitchTo for better security.
			/// </summary>
			void SetPassword(String ^strPassword);

			//get the number of requests in queue in processing
			/// <summary>
			/// get the number of requests in queue in processing
			/// </summary>
			int GetCountOfRequestsInQueue();
			
			int GetCountOfAttachedServiceHandlers();
			
			/// <summary>
			/// Release all of interfaces to a COM USocket object, memory queue and other resources. 
			/// </summary>
			void DestroyUSocket();

			/// <summary>
			/// Detach all of attached instances of CAsyncServiceHandler.
			/// </summary>
			void DetachAll();
			
			/// <summary>
			/// Clean all of memory tracks on client side, send a request to a remote SocketPro server so that all of memory tracks on server side are also cleaned.
			/// Call this method if you think that there is sensitive/secret data left in various memory tracks.
			/// </summary>
			void CleanTrack();
			
			/// <summary>
			/// Cancel all of queued requests.
			/// </summary>
			void Cancel();
			
			/// <summary>
			/// Cancel a given number (lRequests) of queued requests.
			/// </summary>
			void Cancel(int nRequests);
			
			/// <summary>
			/// Start a job until the method EndJob is called.
			/// </summary>
			bool StartJob();

			/// <summary>
			/// End a job.
			/// </summary>
			bool EndJob();
		
			property __int64 BytesReceived
			{
				__int64 get()
				{
					if(m_pIUSocket == NULL)
						return 0;
					long lLow = 0, lHigh = 0;
					__int64 llData = 0;
					m_pIUSocket->GetBytesReceived(&lHigh, &lLow);
					if(lHigh != 0)
					{
						llData = lHigh;
						llData = (llData<<32);
					}
					llData += (ULONG)lLow;
					return llData;
				}
			}

			property bool ReturnRandom
			{
				bool get()
				{
					if(m_pIUSocket == NULL)
						return false;
					short sMajor = 0;
					short sMinor = 0;
					m_pIUSocket->GetServerUSockVersion(&sMinor, &sMajor);
					return ((((unsigned short)sMinor) & RETURN_RESULT_RANDOM) == RETURN_RESULT_RANDOM);
				}
			}
			
			property __int64 BytesSent
			{
				__int64 get()
				{
					if(m_pIUSocket == NULL)
						return 0;
					long lLow = 0, lHigh = 0;
					__int64 llData = 0;
					m_pIUSocket->GetBytesSent(&lHigh, &lLow);
					if(lHigh != 0)
					{
						llData = lHigh;
						llData = (llData<<32);
					}
					llData += (ULONG)lLow;
					return llData;
				}
			}

			property USOCKETLib::tagEncryptionMethod EncryptionMethod
			{
				USOCKETLib::tagEncryptionMethod get()
				{
					if(m_pIUSocket == NULL)
						return USOCKETLib::tagEncryptionMethod::NoEncryption;
					short nMethod;
					m_pIUSocket->get_EncryptionMethod(&nMethod);
					return (USOCKETLib::tagEncryptionMethod)nMethod;
				}
				void set(USOCKETLib::tagEncryptionMethod em)
				{
					if(m_pIUSocket != NULL)
					{
						m_pIUSocket->put_EncryptionMethod((short)em);
					}
				}
			}
			
			/// <summary>
			/// A property indicating if transferring server exception is enabled.
			/// </summary>
			property bool AutoTransferServerException
			{
				bool get()
				{
					return m_bServerException;
				}
			}
			
			/// <summary>
			/// A property enable or disable batch processing. It should be false by default.
			/// </summary>
			property bool Syn
			{
				bool get()
				{
					if(m_pIUSocket == NULL)
						return false;
					VARIANT_BOOL bSyn;
					m_pIUSocket->get_Syn(&bSyn);
					return (bSyn == VARIANT_FALSE) ? false : true;
				}
				void set(bool bSyn)
				{
					if(m_pIUSocket != NULL)
					{
						m_pIUSocket->put_Syn(bSyn ? VARIANT_TRUE : VARIANT_FALSE);
					}
				}
			}

			/// <summary>
			/// A property indicating socket handle.
			/// </summary>
			property int Socket
			{
				int get()
				{
					if(m_pIUSocket == NULL)
						return 0;
					long lSocket;
					m_pIUSocket->get_Socket(&lSocket);
					return lSocket;
				}
			}
			
			/// <summary>
			/// A property indicating asynchronous callback.
			/// </summary>
			property AsyncCallback ^Callback
			{
				AsyncCallback ^get()
				{
					return m_cb;
				}
				void set(AsyncCallback ^cb)
				{
					m_cb = cb;
				}
			}

		public:
			DOnDataAvailable		^m_OnDataAvailable;
			DOnClosing				^m_OnClosing;
			DOnConnecting			^m_OnConnecting;
			DOnGetHostByAddr		^m_OnGetHostByAddr;
			DOnGetHostByName		^m_OnGetHostByName;
			DOnOtherMessage			^m_OnOtherMessage;
			DOnRequestProcessed		^m_OnRequestProcessed;
			DOnSendingData			^m_OnSendingData;
			DOnSocketClosed			^m_OnSocketClosed;
			DOnSocketConnected		^m_OnSocketConnected;
			DOnBaseRequestProcessed	^m_OnBaseRequestProcessed;
			DOnAllRequestsProcessed	^m_OnAllRequestsProcessed;
			
		private:
			void InitEx(IntPtr pIUnknownToUSocket);
			void Init();
			void Uninit();
			void OnClosingEventHandler(int hSocket, int hWnd);
			void OnConnectingEventHandler(int hSocket, int hWnd);
			void OnDataAvailableEventHandler(int hSocket, int lBytes, int lError);
			void OnGetHostByAddrEventHandler(int hHandle, String ^strHostName, String ^strHostAlias, int lError);
			void OnGetHostByNameEventHandler(int hHandle, String ^strHostName, String ^strAlias, String ^strIPAddr, int lError);
			void OnOtherMessageEventHandler(int hSocket, int nMsg, int wParam, int lParam);
			void OnRequestProcessedEventHandler(int hSocket, short nRequestID, int lLen, int lLenInBuffer, short sFlag);
			void OnSendingDataEventHandler(int hSocket, int lError, int lSent);
			void OnSocketClosedEventHandler(int hSocket, int lError);
			void OnSocketConnectedEventHandler(int hSocket, int lError);
			bool IsWeb();
			static bool IsBatchingBalanced(Dictionary<int, CTaskContext^>^ aTasks);

		protected:
			List<CAsyncServiceHandler^>^ GetRequestAsynHandlers();

		internal:
			CAsyncServiceHandler^ Lookup(int nSvsID);
			void Disadvise();
			void Advise();
			List<CAsyncServiceHandler^> ^m_lstAsynHandler;
			IUSocket	*m_pIUSocket;
			IUFast		*m_pIUFast;
			CComAutoCriticalSection		*m_pCS;
			void OnRequestProcessed(int hSocket, short nRequestID, int lLen, int lLenInBuffer, USOCKETLib::tagReturnFlag ReturnFlag);
			AsyncCallback	^m_cb;
			IJobContext		^m_pIJobContext;
			IJobManager		^m_pIJobManager;
			IProcessBySocketPoolEx	^m_pIProcess;

		private:
			bool		m_bAdvised;
			bool		m_bServerException;
			IUChat		*m_pIUChat;
			int			m_nCurSvsID;
			USOCKETLib::USocketClass ^m_USocketClass;
			CCSEvent	*m_pCSEvent;
			Object		^m_AsyncState;
			WaitHandle	^m_mre;
			CUPushClientImpl	^m_pPush;
			unsigned long	m_ulBatchBalance;
		};
	}; //ClientSide
}; //SocketProAdapter

#endif
