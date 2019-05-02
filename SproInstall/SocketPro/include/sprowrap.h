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


#ifndef ___SOCKETPRO_WRAPPER_C_PLUS_PLUS_DEVELOPMENT_H____
#define ___SOCKETPRO_WRAPPER_C_PLUS_PLUS_DEVELOPMENT_H____


#include <atlbase.h>

#ifndef __ATLCOM_H__
	extern CComModule _Module;
	#include <atlcom.h>
#endif

#include "sockutil.h"
#include "uqueue.h"

#if _MSC_VER >= 1500
	#include <functional>
	#include <utility>
	#include<vector>
#endif

namespace SocketProAdapter
{
	#define SOCKETPRO_MAX_BASE_REQUEST_ID		45
//	#pragma warning(disable: 4100)
	#include "usocket.h"
#ifndef _WIN32_WCE
	#pragma comment(lib, "shlwapi.lib")
#endif
	extern CComAutoCriticalSection	g_cs;

	#define	TRANSFER_SERVER_EXCEPTION	(0x40000000)

	struct IUPush
	{
		virtual bool Enter(unsigned long *pGroups, unsigned long ulCount) = 0;
		virtual bool Broadcast(const VARIANT& vtMessage, unsigned long *pGroups, unsigned long ulGroupCount) = 0;
		virtual bool Broadcast(const unsigned char *pMessage, unsigned long ulMessageSize, unsigned long *pGroups, unsigned long ulGroupCount) = 0;
		virtual bool SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId) = 0;
		virtual bool SendUserMessage(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulMessageSize) = 0;
		virtual bool Exit() = 0;
		static void CreateVtGroups(unsigned long *pGroups, unsigned long ulCount, VARIANT &vtGroup);
	};

	class CClientSocketException
	{
	public:
		CClientSocketException(HRESULT hr, LPCWSTR strMessage) 
			: m_strMessage(strMessage), m_hr(hr)
		{
				
		}

		CClientSocketException(const CClientSocketException &err) 
			: m_strMessage(err.m_strMessage), m_hr(err.m_hr)
		{

		}

	public:
		CClientSocketException& operator=(const CClientSocketException& err)
		{
			m_strMessage = err.m_strMessage;
			m_hr = err.m_hr;
			return *this;
		}

	public:
		HRESULT 			m_hr;
		CComBSTR			m_strMessage;
	};

	class CSocketProServerException : public CClientSocketException
	{
	public:
		CSocketProServerException(HRESULT hr, LPCWSTR strMessage, ULONG ulSvsID = 0, unsigned short usRequestID = 0)
			: CClientSocketException(hr, strMessage), m_ulSvsID(ulSvsID), m_usRequestID(usRequestID)
		{

		}

		CSocketProServerException(const CSocketProServerException& err)
			: CClientSocketException(err), m_ulSvsID(err.m_ulSvsID), m_usRequestID(err.m_usRequestID)
		{
			
		}
		
	public:
		CSocketProServerException& operator=(const CSocketProServerException& err)
		{
			m_strMessage = err.m_strMessage;
			m_hr = err.m_hr;
			m_ulSvsID = err.m_ulSvsID;
			m_usRequestID = err.m_usRequestID;
			return *this;
		}

	public:
		unsigned long		m_ulSvsID;
		unsigned short		m_usRequestID;
	};

	CUQueue& operator << (CUQueue &UQueue, const CSocketProServerException &err);
	CUQueue& operator >> (CUQueue &UQueue, CSocketProServerException &err);
	
	class CScopeUQueue
	{
	public:
		CScopeUQueue(bool bLarge = false);
		~CScopeUQueue();
		
		//disable copy constructor and assignment operator
		CScopeUQueue(const CScopeUQueue& ScopeUQueue);
		CScopeUQueue& operator=(const CScopeUQueue& ScopeUQueue);

	public:
		inline CUQueue* operator->() const
		{
			return m_pUQueue;
		}

		inline CUQueue& operator*() const
		{
			return (*m_pUQueue);
		}


		/*inline CUQueue* operator&() const
		{
			return m_pUQueue;
		}*/

		template<class ctype>
		inline CUQueue& operator << (const ctype &data)
		{
			return (*m_pUQueue << data);
		}
	
		template<class ctype>
		inline CUQueue& operator >> (ctype &data)
		{
			return (*m_pUQueue >> data);
		}
		static void DestroyUQueuePool();
		static void CleanUQueuePool();
		static __int64 GetMemoryConsumed();

		//You should call the method Unlock to put back the instance of CUQueue into pool for reuse
		//after calling the method Lock for a pointer to an instance of CUQueue
		static CUQueue* Lock();
		static void Unlock(CUQueue *pUQueue);

	private:
		static CUQueue* Lock(bool bLarge);
		static void Unlock(CUQueue *pUQueue, bool bLarge);
		
	private:
		bool							m_bLarge;
		CUQueue							*m_pUQueue;
		static CSimpleArray<CUQueue*>	m_aUQueue;
		static CSimpleArray<CUQueue*>	m_aLargeUQueue;
		static CComAutoCriticalSection	m_cs;
		static unsigned long			m_ulLargeSize;
	};

/*	template<class ctype>
	CUQueue& operator << (CScopeUQueue &su, const ctype &data)
	{
		return (*su << data);
	}
	
	template<class ctype>
	CUQueue& operator >> (CScopeUQueue &su, ctype &data)
	{
		return (*su >> data);
	}*/

	class CUPerformanceQuery
	{
	public:
		CUPerformanceQuery();
		
		//disable copy constructor
		CUPerformanceQuery(const CUPerformanceQuery& perf);
		
		//disable assignment operator
		CUPerformanceQuery& operator=(const CUPerformanceQuery& perf);

	public:
		//return high frequency time at this time
		__int64 Now();

		__int64 Diff(__int64 liNow, __int64 liOld);

		//return time difference from current time in micro-second
		__int64 Diff(__int64 liPrevCount);

	private:
		__int64 m_liFreq;
		__int64 m_liCount;
	};


	struct CConnectionContext
	{
		CConnectionContext()
		{
			memset(this, 0, sizeof(CConnectionContext));
		}
		BSTR		m_strHost;
		long		m_nPort;
		BSTR		m_strUID;
		BSTR		m_strPassword;
		short		m_EncrytionMethod;
		bool		m_bZip;
	};

	enum tagJobStatus
	{
		jsInitial = 0,
		jsCreating = 1,
		jsQueued = 2,
		jsRunning = 3,
	};
	struct CTaskContext
	{
		unsigned short	m_usRequestId;	//request id
		unsigned long	m_ulSize;		//request input parameters size in byte
		bool			m_bClient;	//Client request
	};
	struct IJobManager;

	namespace ClientSide
	{

#ifndef RETURN_RESULT_RANDOM
	#define RETURN_RESULT_RANDOM	((unsigned short)0x4000)
#endif
		namespace Internal
		{
			#define	IDC_SRCUSOCKETEVENT		2
			static _ATL_FUNC_INFO OnSockEventFuncInfo = {CC_STDCALL, VT_I4, 2, {VT_I4, VT_I4}};
			static _ATL_FUNC_INFO OnSockEventWinMsgInfo = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_I4, VT_I4, VT_I4}};
			static _ATL_FUNC_INFO OnDataAvailableInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
			static _ATL_FUNC_INFO OnSendingDataInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
			static _ATL_FUNC_INFO OnSockGetHostByAddr = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_BSTR, VT_BSTR, VT_I4}};
			static _ATL_FUNC_INFO OnSockGetHostByName = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_BSTR, VT_BSTR, VT_BSTR, VT_I4}};
			static _ATL_FUNC_INFO OnRequestProcessedInfo = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_I2, VT_I4, VT_I4, VT_I2}};

			struct IExtenalSocketEvent
			{
				virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam) = 0;
				virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd) = 0;
				virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag) = 0;
				virtual bool Process() = 0;
			};

			template <typename T>
			class CUSimpleArray : public CSimpleArray<T>
			{
			public:
				BOOL Insert(int nIndex, T& t)
				{
					int n;
					ATLASSERT(nIndex >= 0);
					BOOL b = CSimpleArray<T>::Add(t);
					if(!b)
						return b;
					int nStart = GetSize() - 2;
					if(nStart >= nIndex)
					{
						T last = (*this)[nStart + 1]; 
						for(n=nStart; n>=nIndex; n--)
						{
							(*this)[n+1] = (*this)[n];
						}
						(*this)[nIndex] = last;
					}
					return b;
				}
			};

			class CCSEvent : public IDispEventSimpleImpl<IDC_SRCUSOCKETEVENT, CCSEvent, &__uuidof(_IUSocketEvent)>							
			{
			public:
			BEGIN_SINK_MAP(CCSEvent)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 1, &CCSEvent::OnDataAvailableInternal, &OnDataAvailableInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 2, &CCSEvent::OnOtherMessageInternal, &OnSockEventWinMsgInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 3, &CCSEvent::OnSocketClosedInternal, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 4, &CCSEvent::OnSocketConnectedInternal, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 5, &CCSEvent::OnConnectingInternal, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 6, &CCSEvent::OnSendingDataInternal, &OnSendingDataInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 7, &CCSEvent::OnGetHostByAddrInternal, &OnSockGetHostByAddr)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 8, &CCSEvent::OnGetHostByNameInternal, &OnSockGetHostByName)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 9, &CCSEvent::OnClosingInternal, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 10, &CCSEvent::OnRequestProcessedInternal, &OnRequestProcessedInfo)
			END_SINK_MAP()

			private:
				virtual HRESULT __stdcall OnDataAvailableInternal(long hSocket, long lBytes, long lError) = 0;
				virtual HRESULT __stdcall OnOtherMessageInternal(long hSocket, long nMsg, long wParam, long lParam) = 0;
				virtual HRESULT __stdcall OnSocketClosedInternal(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnSocketConnectedInternal(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnConnectingInternal(long hSocket, long hWnd) = 0;
				virtual HRESULT __stdcall OnSendingDataInternal(long hSocket, long lError, long lSent) = 0;
				virtual HRESULT __stdcall OnGetHostByAddrInternal(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError) = 0;
				virtual HRESULT __stdcall OnGetHostByNameInternal(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError) = 0;
				virtual HRESULT __stdcall OnClosingInternal(long hSocket, long hWnd) = 0;
				virtual HRESULT __stdcall OnRequestProcessedInternal(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag) = 0;
			}; //CCSEvent

			#define	IDC_SRC_SOCKETPOOL_EVENT	3
			static _ATL_FUNC_INFO OnSocketPoolEventFuncInfo = {CC_STDCALL, VT_I4, 2, {VT_I4, VT_UNKNOWN}};

			class CSocketPoolEvent : public IDispEventSimpleImpl<IDC_SRC_SOCKETPOOL_EVENT, CSocketPoolEvent, &__uuidof(_IUSocketPoolEvents)>
			{
			public:
			BEGIN_SINK_MAP(CSocketPoolEvent)
				SINK_ENTRY_INFO(IDC_SRC_SOCKETPOOL_EVENT, __uuidof(_IUSocketPoolEvents), 1, &CSocketPoolEvent::OnSocketPoolEvent, &OnSocketPoolEventFuncInfo)
			END_SINK_MAP()

			protected:
				virtual HRESULT __stdcall OnSocketPoolEvent(tagSocketPoolEvent spe, IUSocket *pIUSocket) = 0;
			};

		}; //Internal

		
		/*
			CClientSocket makes your code simpler, but it doesn't not wrap all of methods USocket.
			If you need more methods, you can derive your own client socket and easily add more methods by yourself.
		*/
		class CAsyncServiceHandler;
	};

	struct IJobContext
	{
		virtual int AddTask(unsigned short usRequestId, const unsigned char *pBuffer, unsigned long ulSize) = 0;
		virtual bool RemoveTask(int idTask) = 0;
		virtual bool AllocateLargeMemory(unsigned long ulSize) = 0;
		virtual bool Wait(unsigned long ulTimeout = (~0)) = 0;
		virtual IJobManager* GetJobManager() = 0;
		virtual __int64 GetJobId() = 0;
		virtual unsigned long GetCountOfTasks() = 0;
		virtual tagJobStatus GetJobStatus() = 0;
		virtual void* GetIdentity() = 0;
		virtual void GetTasks(CSimpleMap<int, CTaskContext> &mapTask) = 0;
		virtual bool GetBundled() = 0;
		virtual void SetBundled(bool bBundled) = 0;
		virtual unsigned long GetSize() = 0;
		virtual unsigned long GetBufferSize() = 0;
		virtual ClientSide::CAsyncServiceHandler* GetHandler() = 0;
	};

	struct IJobManager
	{
		virtual IJobContext* CreateJob(void *pIdentity) = 0;
		virtual bool EnqueueJob(IJobContext *pJobContext) = 0;
		virtual bool DestroyJob(IJobContext *pJobContext) = 0;
		virtual unsigned int CancelJobs(void *pIdentity) = 0;
		virtual bool CancelJob(__int64 jobId) = 0;
		virtual IJobContext* SeekJob(__int64 jobId) = 0;
		virtual void ShrinkMemory(unsigned long ulMemoryChunkSize) = 0;
		virtual bool WaitAll(unsigned long ulTimeout = INFINITE) = 0;
		virtual bool Wait(void *pIdentity, unsigned long ulTimeout = INFINITE) = 0;
		virtual bool Wait(const __int64 *pJobId, unsigned int nCount, unsigned long ulTimeout = INFINITE) = 0;
		virtual bool WaitAny(const __int64 *pJobId, unsigned int nCount, CSimpleArray<__int64> &jobs, unsigned long ulTimeout = INFINITE) = 0;
		virtual bool WaitAny(void *pIdentity, CSimpleArray<__int64> &jobs, unsigned long ulTimeout = INFINITE) = 0;
		virtual ClientSide::CAsyncServiceHandler* LockIdentity(unsigned long ulTimeout = INFINITE) = 0;
		virtual void UnlockIdentity(ClientSide::CAsyncServiceHandler *pIdentity) = 0;
		virtual void GetJobs(void *pIdentity, CSimpleArray<__int64> &jobs) = 0;
		virtual tagJobStatus GetJobStatus(__int64 jobId) = 0;
		virtual int ResetPosition(__int64 jobId, int nNewPosition) = 0;
		virtual void GetIdentities(CSimpleArray<void*> &Identities) = 0;
		virtual unsigned int GetCountOfJobs() = 0;
		virtual unsigned int GetCountOfQueuedJobs() = 0;
		virtual __int64 GetMemoryConsumed() = 0;
		virtual IUSocketPool* GetSocketPool() = 0;
		virtual unsigned int GetRecycleBinSize() = 0;
		virtual void SetRecycleBinSize(unsigned int nRecycleBinSize) = 0;
	};

	namespace ClientSide
	{
		class CClientSocket : protected Internal::CCSEvent
		{
		private:
			class CUPushClientImpl : public IUPush
			{
			public:
				virtual bool Enter(unsigned long *pGroups, unsigned long ulCount);
				virtual bool Broadcast(const VARIANT& vtMessage, unsigned long *pGroups, unsigned long ulCount);
				virtual bool Broadcast(const unsigned char *pMessage, unsigned long ulMessageSize, unsigned long *pGroups, unsigned long ulCount);
				virtual bool SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId);
				virtual bool SendUserMessage(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulMessageSize);
				virtual bool Exit();

			private:
				friend CClientSocket;
				CClientSocket *m_pClientSocket;
			};

		public:
			CClientSocket(bool bCreateOne = true);
			virtual ~CClientSocket();

			//construct this class instance with a valid existing USocket object
			CClientSocket(IUnknown *pIUnknownToUSocket);
			
			//no copy constructor
			CClientSocket(const CClientSocket &ClientSocket);
		public:
			//no assignment operator
			CClientSocket& operator=(const CClientSocket &ClientSocket);

			bool Connect(LPCTSTR strHost, unsigned long ulPort, bool bSyn = false);
			
			//abort a socket connection
			void Disconnect();

			void CleanTrack();
			
			//gracefully close a socket connection
			void Shutdown();

			//switch for a service
			virtual bool SwitchTo(unsigned long ulSvsID, bool bAutoTransferServerException = false);
			bool SwitchTo(CAsyncServiceHandler *pAsynHandler, bool bAutoTransferServerException = false);
			
			//Batching requests
			bool StartJob();
			bool EndJob();
			bool BeginBatching();
			bool Commit(bool bBatchingAtServer = false);
			bool Rollback();
			bool IsBatching();
			unsigned long GetBytesBatched();

			//freeze GUI
			void DisableUI(bool bDisable = true);

			//if ulSvsID = 0, it waits a request for the current service id 
			bool Wait(unsigned short usRequestID, unsigned long ulTimeout = INFINITE, unsigned long ulSvsID = 0);
			bool WaitAll(unsigned long ulTimeout = INFINITE);
			
			//client credentials
			void SetUID(LPCTSTR strUID);
			void SetPassword(LPCTSTR strPassword);

			bool IsConnected();
			HRESULT GetErrorCode();
			CComBSTR GetErrorMsg();

			IUPush* GetPush();
			
			//get the number of requests in queue in processing
			unsigned long GetCountOfRequestsInQueue();

			inline const CComPtr<IUSocket>& GetIUSocket()
			{
				return m_pIUSocket;
			}

			void SetUSocket(IUnknown *pIUnknownToUSocket);

			inline const CComPtr<IUFast>& GetIUFast()
			{
				return m_pIUFast;
			}

			inline const CComPtr<IUChat>& GetIUChat()
			{
				return m_pIUChat;
			}
			
			virtual unsigned long GetCurrentServiceID()
			{
				return m_ulCurSvsID;
			}

			inline unsigned int GetCountOfAttachedServiceHandlers()
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				return (unsigned int)m_mapSvsIDHandler.GetSize();
			}

			inline bool AutoTransferServerException();

			inline long GetSocket()
			{
				if(m_pIUSocket.p == UNULL_PTR)
					return 0;
				long lSocket;
				m_pIUSocket->get_Socket(&lSocket);
				return lSocket;
			}

			inline bool GetReturnRandom()
			{
				ATLASSERT(m_pIUSocket.p != UNULL_PTR);
				if(m_pIUSocket.p != UNULL_PTR)
				{
					short sMajor;
					short sMinor = 0;
					m_pIUSocket->GetServerUSockVersion(&sMinor, &sMajor);
					return ((((unsigned short)sMinor) & RETURN_RESULT_RANDOM) == RETURN_RESULT_RANDOM);
				}
				return false;
			}

			unsigned short GetServerPingTime();

			void Cancel(long lRequests = -1);

			IJobContext* GetCurrentJobContext();
			IJobManager* GetJobManager();
			
			//assign this class instance with a valid existing USocket object
			CClientSocket& operator=(IUnknown *pIUnknownToUSocket);

		protected:
			virtual HRESULT OnDataAvailable(long hSocket, long lBytes, long lError);
			virtual HRESULT OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam);
			virtual HRESULT OnSocketClosed(long hSocket, long lError);
			virtual HRESULT OnSocketConnected(long hSocket, long lError);
			virtual HRESULT OnConnecting(long hSocket, long hWnd);
			virtual HRESULT OnSendingData(long hSocket, long lError, long lSent);
			virtual HRESULT OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError);
			virtual HRESULT OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError);
			virtual HRESULT OnClosing(long hSocket, long hWnd);
			virtual HRESULT OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);
			virtual void OnBaseRequestProcessed(unsigned short usBaseRequestID);

			//Beginning from SocketPro version 4.8.1.12
			virtual void OnAllRequestsProcessed(unsigned int hSocket, unsigned short usLastRequestID);

		private:
			virtual HRESULT __stdcall OnDataAvailableInternal(long hSocket, long lBytes, long lError);
			virtual HRESULT __stdcall OnOtherMessageInternal(long hSocket, long nMsg, long wParam, long lParam);
			virtual HRESULT __stdcall OnSocketClosedInternal(long hSocket, long lError);
			virtual HRESULT __stdcall OnSocketConnectedInternal(long hSocket, long lError);
			virtual HRESULT __stdcall OnConnectingInternal(long hSocket, long hWnd);
			virtual HRESULT __stdcall OnSendingDataInternal(long hSocket, long lError, long lSent);
			virtual HRESULT __stdcall OnGetHostByAddrInternal(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError);
			virtual HRESULT __stdcall OnGetHostByNameInternal(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError);
			virtual HRESULT __stdcall OnClosingInternal(long hSocket, long hWnd);
			virtual HRESULT __stdcall OnRequestProcessedInternal(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);

		private:
			void InitEx(IUnknown *pIUnknown);
			void Init();
			void Uninit();
			void DetachAll();
			void InvokeAllProcessed(unsigned short usReqID);
			void NotifyProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);
			static bool IsBatchingBalanced(const CSimpleMap<int, CTaskContext> &aTasks);

#if _MSC_VER < 1300
		public:
#endif
			//don't access them from your code of VC++ 6
			void Advise();
			void Disadvise();
			Internal::IExtenalSocketEvent	*m_pIExtenalSocketEvent;
			IJobManager						*m_pIJobManager;
			IJobContext						*m_pIJobContext;

		protected:
			CSimpleMap<unsigned long, CAsyncServiceHandler*> m_mapSvsIDHandler;
			CComAutoCriticalSection		m_cs;

		private:
			CUPushClientImpl	m_Push;
			//CScopeUQueue	m_UQueue;
			CComPtr<IUFast> m_pIUFast;
			CComPtr<IUChat> m_pIUChat;
			CComPtr<IUSocket> m_pIUSocket;
			unsigned long	m_ulCurSvsID;
			bool			m_bServerException;
			unsigned long	m_ulBatchBalance;
			friend class CAsyncServiceHandler;

#if _MSC_VER >= 1300
			template<typename THandler, typename TCS>
			friend class CSocketPool;      // unbound friend class

			template<typename THandler, typename TCS>
			friend class CSocketPoolEx;      // unbound friend class
#endif	
		}; //CClientSocket

		struct CAsyncResult; 

#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
		typedef std::tr1::function<void(CAsyncResult&)> LF;
#endif
		
		struct CAsyncResult
		{
#if _MSC_VER >= 1300
		private:
#else
		public:
#endif
			CAsyncResult(CAsyncServiceHandler *pAsyncServiceHandler, unsigned short ReqId, CUQueue &q) 
				: AsyncServiceHandler(pAsyncServiceHandler), RequestId(ReqId), UQueue(q)
			{
			}

		public:
			unsigned short			RequestId;
			CAsyncServiceHandler	*AsyncServiceHandler;
			CUQueue					&UQueue;
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
			LF						CurrentAsyncResultHandler;
#endif
		private:
			friend class CAsyncServiceHandler;
#if _MSC_VER >= 1300
			template <typename TClass>
			friend class CAsyncServiceHandlerEx;
#endif
		};

#if _MSC_VER >= 1600
		#define TA_R(ar) [this](CAsyncResult &ar)
		#define A_R(ar) [](CAsyncResult &ar)
#endif
		typedef void (*PAsyncResultHandler)(CAsyncResult &AsyncResult);
		struct IAsyncResultsHandler
		{
		protected:
			virtual void Process(CAsyncResult &AsyncResult) = 0;
			virtual void OnExceptionFromServer(CAsyncServiceHandler &AsyncServiceHandler, CSocketProServerException &Exception)
			{
			}
			friend class CAsyncServiceHandler;
		};
		
		class CAsyncServiceHandler
		{
		private:
			unsigned __int64		m_nSyncIndex;
			CSimpleMap<unsigned __int64, CUQueue*> m_mapSync;
			inline bool SendLock(unsigned short usRequestId, const unsigned char *pBuffer, unsigned long ulSize)
			{
				m_cs.Lock();
				if(!Send(usRequestId, pBuffer, ulSize))
				{
					m_cs.Unlock();
					return false;
				}
				return true;
			}

			inline CUQueue* Look(unsigned __int64 index)
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				int nIndex = m_mapSync.FindKey(index);
				if(nIndex == -1)
					return UNULL_PTR;
				CUQueue *p = m_mapSync.GetValueAt(nIndex);
#if _MSC_VER >= 1300
				m_mapSync.RemoveAt(nIndex);
#else
				m_mapSync.Remove(index);
#endif
				return p;
			}

			struct CPair
			{
				unsigned short		m_usRequestId;
				PAsyncResultHandler	m_arh;
				unsigned __int64	m_nIndex;
			};

			static void Copy(CAsyncResult &ar);

		public:
			CAsyncServiceHandler(unsigned long ulServiceId, CClientSocket *pClientSocket = UNULL_PTR,
				IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR) 
				: m_sq(false), 
				m_ulSvsId(ulServiceId), 
				m_pClientSocket(pClientSocket), 
				m_pIAsyncResultsHandler(pDefaultAsyncResultsHandler),
				m_qRequestHandlerPair(*m_sq), m_nSyncIndex(0)
			{
				//SocketPro requires your service id is not less than odUserServiceIDMin(0x10000000)
				ATLASSERT(ulServiceId >= odUserServiceIDMin); 

				if(m_pClientSocket)
				{
					CAutoLock AutoLock(&m_pClientSocket->m_cs.m_sec);
					m_pClientSocket->m_mapSvsIDHandler.Add(m_ulSvsId, this);
				}
			}

			virtual ~CAsyncServiceHandler()
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				if(m_pClientSocket)
				{
					CAutoLock AutoLock(&m_pClientSocket->m_cs.m_sec);
					m_pClientSocket->m_mapSvsIDHandler.Remove(m_ulSvsId);
				}
				int n, nSize = m_mapSync.GetSize();
				for(n=0; n<nSize; ++n)
				{
					CScopeUQueue::Unlock(m_mapSync.GetValueAt(n));
				}
			}
			
		private:
			//no copy and assignment!
			CAsyncServiceHandler(const CAsyncServiceHandler &ash);
			CAsyncServiceHandler& operator=(const CAsyncServiceHandler &ash);

		public:
			inline bool BeginBatching()
			{
				return GetAttachedClientSocket()->BeginBatching();
			}

			inline bool CommitBatch(bool bServerBatch = false)
			{
				return GetAttachedClientSocket()->Commit(bServerBatch);
			}

			inline bool RollbackBatch()
			{
				return GetAttachedClientSocket()->Rollback();
			}

			inline unsigned long GetSvsID() const {return m_ulSvsId;}
			inline CClientSocket *GetAttachedClientSocket()
			{
				return m_pClientSocket;
			}

			inline bool WaitAll(unsigned long ulTimeout = INFINITE)
			{
				return GetAttachedClientSocket()->WaitAll(ulTimeout);
			}

			inline bool Wait(unsigned short usReqId = 0, unsigned long ulTimeout = INFINITE)
			{
				if(usReqId == 0)
				{
					
					CComVariant vtReq;
					GetAttachedClientSocket()->GetIUSocket()->GetRequestsInQueue(&vtReq);
					if((vtReq.vt == (VT_ARRAY|VT_UI2) || vtReq.vt == (VT_ARRAY|VT_I2)) && vtReq.parray->cbElements > 0)
					{
						unsigned short *p;
						::SafeArrayAccessData(vtReq.parray, (void**)&p);
						usReqId = p[vtReq.parray->cbElements -1];
						::SafeArrayUnaccessData(vtReq.parray);
					}
				}
				if(usReqId == 0)
					return true;
				return GetAttachedClientSocket()->Wait(usReqId, ulTimeout, GetSvsID());
			}

			void Attach(CClientSocket *pClientSocket)
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				if(m_pClientSocket)
				{
					int n;
					CAutoLock AutoLock(&m_pClientSocket->m_cs.m_sec);
					m_pClientSocket->m_mapSvsIDHandler.Remove(m_ulSvsId);
					m_qRequestHandlerPair.SetSize(0);
					int nSize = m_mapSync.GetSize();
					for(n=0; n<nSize; ++n)
					{
						CScopeUQueue::Unlock(m_mapSync.GetValueAt(n));
					}
					m_mapSync.RemoveAll();
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
					m_aLF.clear();
#endif
				}

				m_pClientSocket = pClientSocket;
				if(m_pClientSocket)
				{
					CAutoLock AutoLock(&m_pClientSocket->m_cs.m_sec);
					m_pClientSocket->m_mapSvsIDHandler.Add(m_ulSvsId, this);
				}
			}

			void Detach()
			{
				Attach(UNULL_PTR);
			}

			inline void SetAsyncResultsHandler(IAsyncResultsHandler *pIAsyncResultsHandler)
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				m_pIAsyncResultsHandler = pIAsyncResultsHandler;
			}

			inline IAsyncResultsHandler* GetAsyncResultsHandler()
			{
				CAutoLock AutoLock(&m_cs.m_sec);
				return m_pIAsyncResultsHandler;
			}

		private:
			inline bool SetAndWait(unsigned short usRequestId, unsigned __int64 &index)
			{
				CPair p;
				p.m_arh = &CAsyncServiceHandler::Copy;
				p.m_usRequestId = usRequestId;
				index = (++m_nSyncIndex);
				p.m_nIndex = index;
				m_qRequestHandlerPair << p;
				m_cs.Unlock();
				return WaitAll();
			}

			inline bool PostProcessSync(unsigned short usRequestId)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

			template<class R0>
			inline bool PostProcessSync(unsigned short usRequestId, R0 &r0)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					if(p->GetSize() > 0)
						*p >> r0;
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

			template<class R0, class R1>
			inline bool PostProcessSync(unsigned short usRequestId, R0 &r0, R1 &r1)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					if(p->GetSize() > 0)
						*p >> r0 >> r1;
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

			template<class R0, class R1, class R2>
			inline bool PostProcessSync(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					if(p->GetSize() > 0)
						*p >> r0 >> r1 >> r2;
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

			template<class R0, class R1, class R2, class R3>
			inline bool PostProcessSync(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					if(p->GetSize() > 0)
						*p >> r0 >> r1 >> r2 >> r3;
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

			template<class R0, class R1, class R2, class R3, class R4>
			inline bool PostProcessSync(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				unsigned __int64 index;
				bool b = SetAndWait(usRequestId, index);
				CUQueue *p = Look(index);
				ATLASSERT(b); //socket closed ????
				if(b)
				{
					ATLASSERT(p != UNULL_PTR);
					if(p->GetSize() > 0)
						*p >> r0 >> r1 >> r2 >> r3 >> r4;
					m_pClientSocket->NotifyProcessed(m_pClientSocket->GetSocket(), (short)usRequestId, p->GetSize(), p->GetSize(), rfCompleted);
				}
				CScopeUQueue::Unlock(p);
				return b;
			}

		public:
			inline CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const CUQueue &UQueue, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, UQueue.GetBuffer(), UQueue.GetSize(), arh);
			}

			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const CUQueue &UQueue, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, UQueue.GetBuffer(), UQueue.GetSize(), arh);
			}

			inline CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const CScopeUQueue &UQueue, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, UQueue->GetBuffer(), UQueue->GetSize(), arh);
			}

			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const CScopeUQueue &UQueue, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, UQueue->GetBuffer(), UQueue->GetSize(), arh);
			}

#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
			template<typename T0>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0)
			{
				return SendRequest(lf, usRequestId, t0);
			}
			
			template<typename T0, typename T1>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1)
			{
				return SendRequest(lf, usRequestId, t0, t1);
			}

			template<typename T0, typename T1, typename T2>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2);
			}

			template<typename T0, typename T1, typename T2, typename T3>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4, t5);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4, t5, t6);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t8);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
			{
				return SendRequest(lf, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t9);
			}

			inline CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId)
			{
				return SendRequest(lf, usRequestId, (const BYTE *)UNULL_PTR, (unsigned long)0);
			}

			inline CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const CUQueue &UQueue)
			{
				return SendRequest(lf, usRequestId, UQueue.GetBuffer(), UQueue.GetSize());
			}

			inline CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId, const CScopeUQueue &UQueue)
			{
				return SendRequest(lf, usRequestId, UQueue->GetBuffer(), UQueue->GetSize());
			}

			inline CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const CUQueue &UQueue)
			{
				return SendRequest(lf, usRequestId, UQueue.GetBuffer(), UQueue.GetSize());
			}

			inline CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const CScopeUQueue &UQueue)
			{
				return SendRequest(lf, usRequestId, UQueue->GetBuffer(), UQueue->GetSize());
			}

			inline CAsyncServiceHandler& operator()(LF lf, unsigned short usRequestId,  const BYTE *pBuffer, unsigned long ulSizeInByte)
			{
				return SendRequest(lf, usRequestId,  pBuffer, ulSizeInByte);
			}

			inline CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId)
			{
				return SendRequest(lf, usRequestId, (const BYTE *)UNULL_PTR, (unsigned long)0);
			}

			inline CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId,  const BYTE *pBuffer, unsigned long ulSizeInByte)
			{
				SendRequest(usRequestId, pBuffer, ulSizeInByte, (PAsyncResultHandler)UNULL_PTR);
				if(lf)
				{
					std::pair<unsigned short, LF> p(usRequestId, lf);
					CAutoLock AutoLock(&m_cs.m_sec);
					m_aLF.push_back(p);
				}
				return *this;
			}

			template<typename T0>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0)
			{
				CScopeUQueue su;
				su<<t0;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}
			
			template<typename T0, typename T1>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			CAsyncServiceHandler& SendRequest(LF lf, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				return SendRequest(lf, usRequestId, su->GetBuffer(), su->GetSize());
			}
#endif

			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, (const BYTE*)UNULL_PTR, (unsigned long)0, arh);
			}

			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const BYTE *pBuffer, unsigned long ulSizeInByte, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, pBuffer, ulSizeInByte, arh);
			}
			
			template<typename T0>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, arh);
			}

			template<typename T0, typename T1>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, arh);
			}

			template<typename T0, typename T1, typename T2>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, t5, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, t5, t6, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t8, arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			inline CAsyncServiceHandler& operator()(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, arh);
			}

			bool ProcessR0(unsigned short usRequestId)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			bool ProcessR0(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId);
				return b;
			}

			//ProcessR1
			template<typename R0>
			bool ProcessR1(unsigned short usRequestId, R0 &r0)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0>
			bool ProcessR1(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0);
				return b;
			}

			//ProcessR2
			template<typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, R0 &r0, R1 &r1)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1>
			bool ProcessR2(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1);
				return b;
			}

			//ProcessR3
			template<typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2>
			bool ProcessR3(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2);
				return b;
			}

			//ProcessR4
			template<typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3>
			bool ProcessR4(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3);
				return b;
			}

			//ProcessR5
			template<typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				if(!SendLock(usRequestId, UNULL_PTR, 0))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}
			
			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3, typename R4>
			bool ProcessR5(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				if(!SendLock(usRequestId, su->GetBuffer(),  su->GetSize()))
					return false;
				bool b = PostProcessSync(usRequestId, r0, r1, r2, r3, r4);
				return b;
			}

			template<typename T0>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}
			
			template<typename T0, typename T1>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, PAsyncResultHandler arh = UNULL_PTR)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				return SendRequest(usRequestId, su->GetBuffer(), su->GetSize(), arh);
			}

			inline CAsyncServiceHandler& SendRequest(unsigned short usRequestId, PAsyncResultHandler arh = UNULL_PTR)
			{
				return SendRequest(usRequestId, (const BYTE*)UNULL_PTR, (unsigned long)0, arh);
			}

			CAsyncServiceHandler& SendRequest(unsigned short usRequestId, const unsigned char *pBuffer, unsigned long ulLenInByte, PAsyncResultHandler arh = UNULL_PTR)
			{
				//SocketPro requires your request id is large than idMultiPart(55)
				ATLASSERT(usRequestId > idMultiPart);
				
				//You forget attaching this async service handler onto a client socket
				//MyAsyncServiceHandler.Attach(pClientSocket);
				ATLASSERT(m_pClientSocket != UNULL_PTR);

		#ifdef _DEBUG
				if(usRequestId < odUserRequestIDMin)
				{
#ifdef _WIN32_WCE
					ATLTRACE(L"Request Id is less than odUserRequestIDMin (%d)", odUserRequestIDMin);
#else
					ATLTRACE("Request Id is less than odUserRequestIDMin (%d)", odUserRequestIDMin);
#endif
				}
		#endif
				bool b = Send(usRequestId, pBuffer, ulLenInByte);

				//check if client socket is connected to a remote host and usRequestId is larger than idMultiPart)!
				ATLASSERT(b);

				if(b && arh != UNULL_PTR)
				{
					CPair p;
					p.m_usRequestId = usRequestId;
					p.m_arh = arh;
					CAutoLock AutoLock(&m_cs.m_sec);
					if(arh == (&CAsyncServiceHandler::Copy))
						p.m_nIndex = (++m_nSyncIndex);
					else
						p.m_nIndex = 0;
					m_qRequestHandlerPair << p;
				}
				return *this;
			}

		protected:
			virtual void OnResultReturned(unsigned short usRequestId, CUQueue &UQueue)
			{

			}

			virtual void OnExceptionFromServer(CSocketProServerException &Exception)
			{

			}
#if _MSC_VER < 1300
		protected:
#else
		private:
#endif
			virtual unsigned long  RemoveAsyncHandlers()
			{
				int n;
				unsigned long ulCount = 0;
				m_cs.Lock();
				ulCount += m_qRequestHandlerPair.GetSize()/sizeof(CPair);
				m_qRequestHandlerPair.SetSize(0);
				int nSize = m_mapSync.GetSize();
				for(n=0; n<nSize; ++n)
				{
					CScopeUQueue::Unlock(m_mapSync.GetValueAt(n));
				}
				m_mapSync.RemoveAll();

#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
				ulCount += (unsigned long)m_aLF.size();
				m_aLF.clear();
#endif
				m_cs.Unlock();
				return ulCount;
			}

			unsigned __int64 OnRR(long hSocket, unsigned short nRequestID, long lLen, short sFlag)
			{
				CUQueue *pUQueue = CScopeUQueue::Lock();
				unsigned long ulSize = (unsigned long)lLen;
				if(ulSize > pUQueue->GetMaxSize())
				{
					pUQueue->ReallocBuffer((ulSize/256 + 1) * 256);
				}
				if(ulSize > 0)
				{
					HRESULT hr;
					unsigned long ulGet;
					IUFast *pIUFast = m_pClientSocket->GetIUFast().p;
					hr = pIUFast->GetRtnBufferEx(ulSize, (BYTE*)pUQueue->GetBuffer(), &ulGet);
					pUQueue->SetSize(ulSize);
					ATLASSERT(ulGet == ulSize);
					ATLASSERT(hr == S_OK);
				}
				unsigned __int64 index;
				bool bException = false;
				if(m_pClientSocket->AutoTransferServerException()) 
				{
					/* ###### ENHANCEMENT FOR SOCKETPROADAPTER VERSION 2.6.0.1 ###### */
					//All request methods must return HRESULT at least!!!!!!
					//If this assert fails here, it means that returned result does not contain HRESULT
					ATLASSERT(pUQueue->GetSize() >= sizeof(long));
					
					CSocketProServerException err(S_OK, UNULL_PTR);
					*pUQueue >> err;
					if(err.m_hr != S_OK)
					{
						bException = true;
						index = OnE((unsigned short)nRequestID, err);
						ATLASSERT(pUQueue->GetSize() == 0);
					}
				}

				//once an handler is found, call its following virtual function
				//for processing returned results
				if(!bException)
					index = OnRR((unsigned short)nRequestID, *pUQueue);
#ifdef _DEBUG
				if(!index && pUQueue->GetSize() != 0)
					ATLTRACE(_T("Warning: %d byte(s) remained in queue and not processed!\n"), pUQueue->GetSize());
#endif
				if(index > 0)
				{
					CAutoLock AutoLock(&m_cs.m_sec);
					m_mapSync.Add(index, pUQueue);
				}
				else
					CScopeUQueue::Unlock(pUQueue);
				return index;
			}

			virtual unsigned __int64 OnRR(unsigned short usRequestId, CUQueue &UQueue)
			{
				do
				{
					CPair p;
					if(GetAsyncResultHandler(usRequestId, p))
					{
						CAsyncResult ar(this, usRequestId, UQueue);
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
						ar.CurrentAsyncResultHandler = p.m_arh;
#endif
						p.m_arh(ar);
						return p.m_nIndex;
						break;
					}

#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
					std::pair<unsigned short, LF> f;
					if(GetAsyncResultHandler(usRequestId, f))
					{
						CAsyncResult ar(this, usRequestId, UQueue);
						ar.CurrentAsyncResultHandler = f.second;
						f.second(ar);
						break;
					}
#endif
					
					IAsyncResultsHandler *pIAsyncResultsHandler = GetAsyncResultsHandler();
					if(pIAsyncResultsHandler)
					{
						CAsyncResult ar(this, usRequestId, UQueue);
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
						ar.CurrentAsyncResultHandler = std::tr1::bind(&IAsyncResultsHandler::Process, pIAsyncResultsHandler, std::tr1::placeholders::_1);
#endif
						pIAsyncResultsHandler->Process(ar);
						break;
					}
					OnResultReturned(usRequestId, UQueue);
				}while(false);
				return 0;
			}

			virtual unsigned __int64 OnE(unsigned short usRequestId, CSocketProServerException &Exception)
			{
				IAsyncResultsHandler *pIAsyncResultsHandler = GetAsyncResultsHandler();
				if(pIAsyncResultsHandler)
					pIAsyncResultsHandler->OnExceptionFromServer(*this, Exception);
				else
					OnExceptionFromServer(Exception);

				//remove stored result handler
				CPair p;
				if(GetAsyncResultHandler(usRequestId, p))
					return p.m_nIndex;
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
				//remove stored result handler
				std::pair<unsigned short, LF> f;
				if(GetAsyncResultHandler(usRequestId, f))
					return false;
#endif
				return 0;
			}

		private:
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
			bool GetAsyncResultHandler(unsigned short usReqId, std::pair<unsigned short, LF> &p)
			{
				bool bRandom = m_pClientSocket->GetReturnRandom();
				CAutoLock AutoLock(&m_cs.m_sec);
				if(bRandom)
				{
					std::vector<std::pair<unsigned short, LF>>::iterator it = m_aLF.begin();
					std::vector<std::pair<unsigned short, LF>>::iterator end = m_aLF.end();
					for(it = m_aLF.begin(); it != end; ++it)
					{
						if(it->first == usReqId)
						{
							p = (*it);
							m_aLF.erase(it);
							return true;
						}
					}
				}
				else
				{
					std::vector<std::pair<unsigned short, LF>>::iterator it = m_aLF.begin();
					if(it !=  m_aLF.end() && it->first == usReqId)
					{
						p = (*it);
						m_aLF.erase(it);
						return true;
					}
				}
				return false;
			}
#endif
			bool GetAsyncResultHandler(unsigned short usReqId, CPair &p)
			{
				bool bRandom = m_pClientSocket->GetReturnRandom();
				CAutoLock AutoLock(&m_cs.m_sec);
				unsigned long ulSize = m_qRequestHandlerPair.GetSize();
				ATLASSERT((ulSize%sizeof(CPair)) == 0);
				if(bRandom)
				{
					unsigned long n;
					for(n=0; n<ulSize; n += sizeof(CPair))
					{
						CPair *pPair = (CPair*)m_qRequestHandlerPair.GetBuffer(n);
						if(pPair->m_usRequestId == usReqId)
						{
							m_qRequestHandlerPair.Pop(&p, n);
							return true;
						}
					}
				}
				else if(ulSize >= sizeof(CPair))
				{
					CPair *pPair = (CPair*)m_qRequestHandlerPair.GetBuffer();
					if(pPair->m_usRequestId == usReqId)
					{
						m_qRequestHandlerPair >> p;
						return true;
					}
				}
				return false;
			}

#if _MSC_VER >= 1300
		private:
#else
		protected:
#endif
			bool Send(unsigned short usRequestID, const unsigned char *pBuffer, unsigned long ulLenInByte)
			{
				HRESULT hr;
				if(usRequestID <= idMultiPart)
					return false;
				IJobManager *pIJobManager = m_pClientSocket->GetJobManager();
				if(pIJobManager != UNULL_PTR)
				{
					IJobContext *pIJobContext = m_pClientSocket->GetCurrentJobContext();
					if(pIJobContext != UNULL_PTR && pIJobContext->GetJobStatus() == jsCreating)
					{
						return (pIJobContext->AddTask(usRequestID, pBuffer, ulLenInByte) != 0);
					}
					else
					{
						m_pClientSocket->StartJob();
						pIJobContext = m_pClientSocket->GetCurrentJobContext();
						pIJobContext->AddTask(usRequestID, pBuffer, ulLenInByte);
						return m_pClientSocket->EndJob();
					}
				}

				if(m_pClientSocket->m_pIUFast != UNULL_PTR) 
					hr = m_pClientSocket->m_pIUFast->SendRequestEx(usRequestID, ulLenInByte, (BYTE*)pBuffer);
				else
				{
					CComVariant vtBuffer;
					if(pBuffer != UNULL_PTR && ulLenInByte > 0)
					{
						BYTE *p;
						SAFEARRAYBOUND sab[1] = {ulLenInByte, 0};
						vtBuffer.vt = (VT_ARRAY|VT_UI1);
						vtBuffer.parray = ::SafeArrayCreate(VT_UI1, 1, sab);
						::SafeArrayAccessData(vtBuffer.parray, (void**)&p);
						::memmove(p, pBuffer, ulLenInByte);
						::SafeArrayUnaccessData(vtBuffer.parray);
					}
					hr = m_pClientSocket->GetIUSocket()->SendRequest(usRequestID, vtBuffer, VARIANT_FALSE);
				}
				ATLASSERT(hr == S_OK); //Socket closed ????
				return (hr == S_OK);
			}

		protected:
			CComAutoCriticalSection		m_cs;

		private:
			CScopeUQueue				m_sq;
			unsigned long				m_ulSvsId;
			//an interface to an actual results handler implementation.
			IAsyncResultsHandler		*m_pIAsyncResultsHandler;

			//a pointer to an attached client socket
			CClientSocket				*m_pClientSocket;
			CUQueue						&m_qRequestHandlerPair;
			friend CClientSocket;

#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
			//It is mainly designed for C++ Lambda Expression. 
			//This approach may degrade performance slightly but make client code simpler and nicer to look if C++ Lambda Expression is extensively used.
			std::vector< std::pair<unsigned short, LF> > m_aLF;
#endif
			
#if _MSC_VER >= 1300
			template <typename TClass>
			friend class CAsyncServiceHandlerEx;
#endif
		};

		template<typename TClass>
		class CAsyncServiceHandlerEx : public CAsyncServiceHandler
		{
		public:
			typedef void (TClass::*PFunc)(CAsyncResult&);

		private:
			template<typename T>
			class Bind
			{
			public:
				typedef void (T::*PFunc)(CAsyncResult &);

			public:
				Bind(PFunc f, T *t, unsigned short usReqId) : m_p(f), m_t(t), m_usReqId(usReqId)
				{
				}
			
#if _MSC_VER < 1350
			public:
#else
			private:
#endif
				void Execute(CAsyncResult &ar)
				{
					(m_t->*m_p)(ar);
				}

				T		*m_t;
				PFunc	m_p;
				unsigned short m_usReqId;
				friend class CAsyncServiceHandlerEx;
			};

		public:
			CAsyncServiceHandlerEx(unsigned long ulServiceId, CClientSocket *pClientSocket = UNULL_PTR,
				IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR) 
				: CAsyncServiceHandler(ulServiceId, pClientSocket, pDefaultAsyncResultsHandler), m_sqEx(false), m_qBind(*m_sqEx)
			{

			}

		public:
			template<typename T0>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0)
			{
				return SendRequest(f, pObject, usRequestId, t0);
			}

			template<typename T0, typename T1>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1);
			}

			template<typename T0, typename T1, typename T2>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2);
			}

			template<typename T0, typename T1, typename T2, typename T3>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4, t5);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4, t5, t6);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t8);
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
			{
				return SendRequest(f, pObject, usRequestId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
			}

			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId)
			{
				return SendRequest(f, pObject, usRequestId, (const unsigned char *)UNULL_PTR, (unsigned long)0);
			}

			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const CUQueue &UQueue)
			{
				return SendRequest(f, pObject, usRequestId, UQueue.GetBuffer(), UQueue.GetSize());
			}

			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const CScopeUQueue &UQueue)
			{
				return SendRequest(f, pObject, usRequestId, UQueue->GetBuffer(), UQueue->GetSize());
			}

			inline CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const CUQueue &UQueue)
			{
				return SendRequest(f, pObject, usRequestId, UQueue.GetBuffer(), UQueue.GetSize());
			}

			inline CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const CScopeUQueue &UQueue)
			{
				return SendRequest(f, pObject, usRequestId, UQueue->GetBuffer(), UQueue->GetSize());
			}

			inline CAsyncServiceHandlerEx<TClass>& operator()(PFunc f, TClass *pObject, unsigned short usRequestId, const unsigned char *pBuffer, unsigned long ulLenInByte)
			{
				return SendRequest(f, pObject, usRequestId, pBuffer, ulLenInByte);
			}

			template<typename T0>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0)
			{
				CScopeUQueue su;
				su<<t0;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}
			
			template<typename T0, typename T1>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1)
			{
				CScopeUQueue su;
				su<<t0<<t1;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
			{
				CScopeUQueue su;
				su<<t0<<t1<<t2<<t3<<t4<<t5<<t6<<t7<<t8<<t9;
				return SendRequest(f, pObject, usRequestId, su->GetBuffer(), su->GetSize());
			}

			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId)
			{
				return SendRequest(f, pObject, usRequestId, (const unsigned char *)UNULL_PTR, (unsigned long));
			}

			CAsyncServiceHandlerEx<TClass>& SendRequest(PFunc f, TClass *pObject, unsigned short usRequestId, const unsigned char *pBuffer, unsigned long ulLenInByte)
			{
				//Really strange inputs!!!
				ATLASSERT((!f) == (!pObject)); 

				bool ok = Send(usRequestId, pBuffer, ulLenInByte);
				if(ok && f && pObject)
				{
					Bind<TClass> b(f, pObject, usRequestId);
					m_qBind << b;
				}
				return *this;
			}

#if _MSC_VER < 1300
		protected:
#else
		private:
#endif
			virtual unsigned long  RemoveAsyncHandlers()
			{
				unsigned long ulCount;

				m_cs.Lock();
				ulCount = m_qBind.GetSize()/sizeof(Bind<TClass>);
				m_qBind.SetSize(0);
				m_cs.Unlock();
				ulCount += CAsyncServiceHandler::RemoveAsyncHandlers();
				return ulCount;
			}

			virtual unsigned __int64 OnRR(unsigned short usRequestId, CUQueue &UQueue)
			{
				Bind<TClass> p(UNULL_PTR, UNULL_PTR, usRequestId);
				if(GetARH(usRequestId, p))
				{
					CAsyncResult ar(this, usRequestId, UQueue);
#if (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
					ar.CurrentAsyncResultHandler = std::tr1::bind(p.m_p, p.m_t, std::tr1::placeholders::_1);
#endif
					p.Execute(ar);
					return 0;
				}
				return CAsyncServiceHandler::OnRR(usRequestId, UQueue);
			}

		private:
			virtual unsigned __int64 OnE(unsigned short usRequestId, CSocketProServerException &Exception)
			{
				unsigned __int64 index = CAsyncServiceHandler::OnE(usRequestId, Exception);
				Bind<TClass> p(UNULL_PTR, UNULL_PTR, usRequestId);
				GetARH(usRequestId, p);
				return index;
			}

			bool GetARH(unsigned short usReqId, Bind<TClass> &p)
			{
				bool bRandom = GetAttachedClientSocket()->GetReturnRandom();
				CAutoLock AutoLock(&m_cs.m_sec);
				unsigned long ulSize = m_qBind.GetSize();
				ATLASSERT((ulSize%sizeof(Bind<TClass>)) == 0);
				if(bRandom)
				{
					unsigned long n;
					for(n=0; n<ulSize; n += sizeof(Bind<TClass>))
					{
						Bind<TClass> *pPair = (Bind<TClass>*)m_qBind.GetBuffer(n);
						if(pPair->m_usReqId == usReqId)
						{
							m_qBind.Pop(&p, n);
							return true;
						}
					}
				}
				else if(ulSize >= sizeof(Bind<TClass>))
				{
					Bind<TClass> *pPair = (Bind<TClass>*)m_qBind.GetBuffer();
					if(pPair->m_usReqId == usReqId)
					{
						m_qBind >> p;
						return true;
					}
				}
				return false;
			}

			
		private:
			CScopeUQueue	m_sqEx;
			CUQueue			&m_qBind;
		};

		template<typename THandler, typename TCS = CClientSocket>
		class CSocketPool : protected Internal::CSocketPoolEvent
		{
		public:
			CSocketPool(IAsyncResultsHandler *pIAsyncResultsHandler = UNULL_PTR) 
				: m_ulRecvTimeout(30000), m_pIAsyncResultsHandler(pIAsyncResultsHandler)
			{
				m_hr = m_pIUSocketPool.CoCreateInstance(__uuidof(USocketPool));
				if(m_pIUSocketPool != UNULL_PTR)
				{
					m_hr = CSocketPoolEvent::DispEventAdvise(m_pIUSocketPool);
					if(FAILED(m_hr))
					{
						m_pIUSocketPool.Release();
					}
				}
			}

			virtual ~CSocketPool()
			{
				ShutdownPool();
				if(m_pIUSocketPool != UNULL_PTR)
				{
					CSocketPoolEvent::DispEventUnadvise(m_pIUSocketPool);
					m_pIUSocketPool.Release();
				}	
			}
			
			//no copy constructor
			CSocketPool(const CSocketPool<THandler, TCS>& SocketPool);
		public:
			//no assignment operator
			CSocketPool<THandler, TCS>& operator= (const CSocketPool<THandler, TCS>& SocketPool);

			void SetRecvTimeout(unsigned long ulTimeout = 30000)
			{
				int n;
				if(ulTimeout < 1000)
					ulTimeout = 1000;
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_ulRecvTimeout = ulTimeout;
				int nSize = m_mapSocketHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					TCS *pSocket = m_mapSocketHandler.GetKeyAt(n);
					pSocket->GetIUSocket()->put_RecvTimeout((long)m_ulRecvTimeout);
				}
			}

			bool StartSocketPool(BYTE bSocketsPerThread, BYTE bThreads = 0)
			{
				if(m_pIUSocketPool == UNULL_PTR)
					return false;
				if(IsStarted())
				{
					ShutdownPool();
				}
#ifdef _DEBUG
	#ifdef _WIN32_WCE
				m_hr = ::CoInitializeEx(UNULL_PTR, COINIT_APARTMENTTHREADED);
	#else
				m_hr = ::CoInitialize(UNULL_PTR);
	#endif
				if(m_hr != RPC_E_CHANGED_MODE)
				{
					//SocketPool object can be hosted inside a apartment thread, starting from version 4.8.2.1
	#ifdef _WIN32_WCE
					ATLTRACE(L"Warning: Free-threaded apartment is preferred for hosting socket pool object!\n"); 
	#else
					ATLTRACE("Warning: Free-threaded apartment is preferred for hosting socket pool object!\n"); 
	#endif
				}
#endif			
				m_hr = m_pIUSocketPool->StartPool(bThreads, bSocketsPerThread);
				if(FAILED(m_hr))
				{
					return false;
				}
				return true;
			}

			virtual bool MakeConnection(const CConnectionContext &cc)
			{
				if(!IsStarted())
					return false;
				CComPtr<IUSocket> pIUSocket;
				m_hr = m_pIUSocketPool->FindAClosedSocket(&pIUSocket);
				if(pIUSocket == UNULL_PTR)
					return false;
				THandler Handler;
				m_hr =m_pIUSocketPool->Connect(	pIUSocket.p, 
												cc.m_strHost, 
												cc.m_nPort, 
												Handler.GetSvsID(), 
												cc.m_strUID, 
												cc.m_strPassword, 
												cc.m_EncrytionMethod, 
												cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
				pIUSocket->get_Rtn(&m_hr);
				if(m_hr != S_OK)
					m_hr = specNoOpenedSocket;
				return (m_hr == S_OK);
			}

			bool StartSocketPool(const CConnectionContext *pConnectionContext, unsigned int nConnectionContextCount, BYTE bSocketsPerThread, BYTE bThreads = 0)
			{
				if(pConnectionContext == UNULL_PTR || nConnectionContextCount == 0 || bSocketsPerThread == 0)
					return false;
				if(!StartSocketPool(bSocketsPerThread, bThreads))
					return false;
				THandler Handler;
				unsigned int nIndex = 0;
				while(nConnectionContextCount > nIndex)
				{
					const CConnectionContext &cc = pConnectionContext[nIndex];
					CComPtr<IUSocket> pIUSocket;
					m_pIUSocketPool->FindAClosedSocket(&pIUSocket);
					if(pIUSocket == UNULL_PTR)
						break;
					m_pIUSocketPool->Connect(	pIUSocket.p, 
												cc.m_strHost, 
												cc.m_nPort, 
												Handler.GetSvsID(), 
												cc.m_strUID, 
												cc.m_strPassword, 
												cc.m_EncrytionMethod, 
												cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
					pIUSocket.Release();
					nIndex++;
				}
				long lConnected = 0;
				m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
				return (lConnected != 0);
			}

			bool StartSocketPool(const CConnectionContext &cc, BYTE bSocketsPerThread, BYTE bThreads = 0)
			{
				return StartSocketPool(cc.m_strHost, cc.m_nPort, cc.m_strUID, cc.m_strPassword, bSocketsPerThread, bThreads, cc.m_EncrytionMethod, cc.m_bZip);
			}

			bool StartSocketPool(BSTR strHost, long lPort, BSTR strUID, BSTR strPassword, BYTE bSocketsPerThread, BYTE bThreads = 0, short sEncrytionMethod = NoEncryption, bool bZip = false)
			{
				if(!StartSocketPool(bSocketsPerThread, bThreads))
					return false;
				THandler Handler;
				m_hr = m_pIUSocketPool->ConnectAll(strHost, lPort, Handler.GetSvsID(), strUID, strPassword, sEncrytionMethod, bZip ? VARIANT_TRUE : VARIANT_FALSE);
				if(FAILED(m_hr))
				{
					ShutdownPool();
					return false;
				}
				long lConnected = 0;
				m_pIUSocketPool->get_ConnectedSocketsEx(&lConnected);
				if(lConnected == 0)
					m_hr = specNoOpenedSocket;
				return (lConnected != 0);
			}

			virtual void ShutdownPool()
			{
				if(IsStarted())
				{
					CleanHandlers();
					if(m_pIUSocketPool.p != UNULL_PTR)
					{
						m_pIUSocketPool->ShutdownPool();
					}
				}
			}

			THandler* Lock()
			{
				return Lock(INFINITE, UNULL_PTR);
			}
			
			virtual THandler* Lock(ULONG ulTimeout, IUSocket *pIUSocketSameThreadWith = UNULL_PTR)
			{
				int n;
				CComPtr<IUSocket> pIUSocket;
				if(m_pIUSocketPool == UNULL_PTR)
					return UNULL_PTR;
				m_hr = m_pIUSocketPool->LockASocket(ulTimeout, pIUSocketSameThreadWith, &pIUSocket);
				if(FAILED(m_hr))
					return UNULL_PTR;
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_mapSocketHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					TCS *pKey = m_mapSocketHandler.GetKeyAt(n);
					if(pIUSocket == pKey->GetIUSocket())
					{
						return m_mapSocketHandler.GetValueAt(n);
					}
				}
				ATLASSERT(FALSE); //shouldn't come here!!!
				return UNULL_PTR;
			}

			void Unlock(THandler *pHandler)
			{
				ATLASSERT(pHandler != UNULL_PTR);
				if(pHandler != UNULL_PTR)
				{
					ATLASSERT(pHandler->GetAttachedClientSocket() != UNULL_PTR);
					Unlock((TCS*)pHandler->GetAttachedClientSocket());
				}
			}

			virtual void Unlock(TCS *pClientSocket)
			{
				ATLASSERT(pClientSocket != UNULL_PTR);
				if(pClientSocket != UNULL_PTR)
				{
					m_hr = m_pIUSocketPool->UnlockASocket(pClientSocket->GetIUSocket());
				}
			}

			inline bool IsStarted()
			{
				BYTE bThreads = 0;
				if(m_pIUSocketPool == UNULL_PTR)
					return false;
				m_hr = m_pIUSocketPool->get_ThreadCounts(&bThreads);
				return (bThreads > 0);
			}

			inline const CComPtr<IUSocketPool>& GetUSocketPool()
			{
				return m_pIUSocketPool;
			}

			inline bool IsCompleted(THandler *pHandler)
			{
				ATLASSERT(pHandler != UNULL_PTR);
				if(pHandler != UNULL_PTR)
				{
					ATLASSERT(pHandler->GetAttachedClientSocket() != UNULL_PTR);
					return IsCompleted((TCS*)pHandler->GetAttachedClientSocket());
				}
				return true;
			}
			
			inline bool IsCompleted(TCS *pClientSocket)
			{
				ATLASSERT(pClientSocket != UNULL_PTR);
				if(pClientSocket != UNULL_PTR)
				{
					return (pClientSocket->GetCountOfRequestsInQueue() == 0);
				}
				return true;
			}

		protected:
			virtual HRESULT __stdcall OnSocketPoolEvent(tagSocketPoolEvent spe, IUSocket* pIUSocket)
			{
				switch(spe)
				{
				case speUSocketCreated:
					{
						ATLASSERT(pIUSocket != UNULL_PTR);
						CAutoLock	AutoLock(&g_cs.m_sec);
						TCS *pSocket = new TCS(false);
						pSocket->SetUSocket(pIUSocket);
						THandler *pHandler = new THandler();
						pSocket->Disadvise(); //make sure all of attached handlers subscribe socket events first
						pSocket->Advise(); //re-subscribe socket events
						pHandler->Attach(pSocket);
						pHandler->SetAsyncResultsHandler(m_pIAsyncResultsHandler);
						m_mapSocketHandler.Add(pSocket, pHandler);
						pIUSocket->put_RecvTimeout((long)m_ulRecvTimeout);
					}
					break;
				case speConnected:
					{
						HRESULT hr;

						ATLASSERT(pIUSocket != UNULL_PTR);

						//increase receiving and sending buffer sizes, which leads to better through output
						hr = pIUSocket->StartBatching();
						hr = pIUSocket->SetSockOpt(soRcvBuf, 116800, slSocket);
						hr = pIUSocket->SetSockOpt(soSndBuf, 116800, slSocket);
						hr = pIUSocket->SetSockOptAtSvr(soSndBuf, 116800, slSocket);
						hr = pIUSocket->SetSockOptAtSvr(soRcvBuf, 116800, slSocket);
						hr = pIUSocket->CommitBatching(VARIANT_TRUE);
					}
					break;
				case speShutdown:
					ShutdownPool();
					break;
				default:
					break;
				}

				//With help of this callback and IUSocketPool::Connect, 
				//you are able to create your own locking by use of your own algorithm.
				return S_OK;
			}

		protected:
			void CleanHandlers()
			{
				int n;
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_mapSocketHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					TCS *pKey = m_mapSocketHandler.GetKeyAt(n);
					THandler *pValue = m_mapSocketHandler.GetValueAt(n);
					delete pValue;
					delete pKey;
				}
				m_mapSocketHandler.RemoveAll();
			}

		public:
			HRESULT		m_hr;

		protected:
			CComPtr<IUSocketPool>		m_pIUSocketPool;
			CSimpleMap<TCS*, THandler*> m_mapSocketHandler;

		private:
			IAsyncResultsHandler *m_pIAsyncResultsHandler;
			unsigned long m_ulRecvTimeout;
		}; //CSocketPool

		template<typename THandler, typename TCS = CClientSocket>
		class CSocketPoolEx : public CSocketPool<THandler, TCS>, private Internal::IExtenalSocketEvent
		{
			class CJobManager;
			class CIdentityJobMap;

		public:
			typedef CSocketPoolEx<THandler, TCS> LoadingBalance;

			//Don't directly access this class!!!! 
			//Instead, you should always use its interface IJobContext from your code.
			class CJobContext : public IJobContext
			{
			public:
				CJobContext() : m_Status(jsInitial), m_jobId(0), m_pHandler(UNULL_PTR), m_pJobManager(UNULL_PTR), 
					m_pIdentity(UNULL_PTR), m_idTask(0), m_bBundled(false), m_nWaiting(0)
				{
					m_hEvent = ::CreateEvent(UNULL_PTR, TRUE, TRUE, UNULL_PTR);
				}
				
				virtual ~CJobContext()
				{
					if(m_hEvent != UNULL_PTR)
						::CloseHandle(m_hEvent);
				}

				//disable copy constructor and assignment operator
				CJobContext(const CJobContext &jc);
				CJobContext& operator=(const CJobContext &jc);

			public:
				virtual __int64 GetJobId()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_jobId;
				}

				virtual bool Wait(unsigned long ulTimeout)
				{
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_Status == jsInitial)
							return true;
						::ResetEvent(m_hEvent);
						++m_nWaiting;
					}
					bool b = (::WaitForSingleObject(m_hEvent, ulTimeout) == WAIT_OBJECT_0);
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						--m_nWaiting;
					}
					return b;
				}

				virtual unsigned long GetSize()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_Tasks->GetSize();
				}

				virtual unsigned long GetBufferSize()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_Tasks->GetMaxSize();
				}
				
				//The function is reserved by UDAParts
				virtual int AddTask(unsigned short sRequestId, const unsigned char *pBuffer, unsigned long ulSize, bool bClient)
				{
					if(sRequestId <= idPublicKeyFromSvr)
						return 0;
					if(pBuffer == UNULL_PTR)
						ulSize = 0;
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_Status == jsInitial)
						return 0;
					CUQueue &UQueue = (*m_Tasks);
					if(m_Status == jsRunning)
						UQueue.SetHeadPosition();
					++m_idTask;
					if(m_idTask == 0)
						++m_idTask;
					UQueue.Push((unsigned char*)&m_idTask, sizeof(m_idTask));
					UQueue.Push((unsigned char*)&sRequestId, sizeof(sRequestId));
					UQueue.Push((unsigned char*)&ulSize, sizeof(ulSize));
					if(ulSize > 0)
						UQueue.Push(pBuffer, ulSize);
					UQueue.Push((unsigned char*)&bClient, sizeof(bClient));
					if(m_Status == jsRunning)
					{
						const CComPtr<IUFast> &pIUFast = m_pHandler->GetAttachedClientSocket()->GetIUFast();
						pIUFast->SendRequestEx(sRequestId, ulSize, (unsigned char*)pBuffer);
					}
					return m_idTask;
				}

				virtual int AddTask(unsigned short sRequestId, const unsigned char *pBuffer, unsigned long ulSize)
				{
					return AddTask(sRequestId, pBuffer, ulSize, false);
				}

				virtual IJobManager* GetJobManager()
				{
					return m_pJobManager;
				}

				virtual bool AllocateLargeMemory(unsigned long ulSize)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					CUQueue &UQueue = (*m_Tasks);
					if(ulSize <= UQueue.GetMaxSize())
						return true;
					UQueue.ReallocBuffer(ulSize);
					return (UQueue.GetBuffer() != UNULL_PTR);
				}

				virtual void GetTasks(CSimpleMap<int, CTaskContext> &mapTask)
				{
					int nTask;
					unsigned long ulPos = 0;
					mapTask.RemoveAll();
					CTaskContext task;
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_Status == jsInitial)
						return;
					CUQueue &UQueue = (*m_Tasks);
					unsigned long ulSize = UQueue.GetSize();
					while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long)))
					{
						nTask = *((int*)UQueue.GetBuffer(ulPos));
						ulPos += sizeof(nTask);
						task.m_usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
						ulPos += sizeof(unsigned short);
						task.m_ulSize = *((unsigned long*)UQueue.GetBuffer(ulPos));
						ulPos += sizeof(unsigned long);
						ulPos += task.m_ulSize;
						task.m_bClient = *((bool*)UQueue.GetBuffer(ulPos));
						ulPos += sizeof(bool); //Client request or not
						switch(task.m_usRequestId)
						{
						case idStartBatching:
						case idCommitBatching:
							if(m_pJobManager->m_pSocketPoolEx->m_bServerLoadingBalance)
								break;
						default:
							{
								mapTask.Add(nTask, task);
							}
							break;
						}
					}
				}
				
				//The function is reserved by UDAParts
				virtual bool RemoveTask(bool bRandomResult, unsigned short usRequestId, bool &bClient)
				{
					int nIndex = 0;
					unsigned short rid;
					unsigned long nReqSize;
					unsigned long ulPos = 0;
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_Status != jsRunning)
						return false;
					CUQueue &UQueue = (*m_Tasks);
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

				virtual bool RemoveTask(int idTask)
				{
					int nTask;
					unsigned short usRequestId;
					unsigned long nReqSize;
					if(idTask == 0)
						return false;
					unsigned long ulPos = 0;
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_Status == jsRunning || m_Status == jsInitial)
						return false;
					CUQueue &UQueue = (*m_Tasks);
					unsigned long ulSize = UQueue.GetSize();
					while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long) + sizeof(bool)))
					{
						nTask = *((int*)UQueue.GetBuffer(ulPos));
						if(nTask == idTask)
						{
							usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos + sizeof(unsigned int)));
							nReqSize = *((unsigned long*)UQueue.GetBuffer(ulPos + sizeof(unsigned int) + sizeof(unsigned short)));
							nReqSize += (sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long));
							UQueue.Pop(nReqSize, ulPos);
							bool bClient = false;
							UQueue.Pop((unsigned char*)& bClient, sizeof( bClient), ulPos);
							if(bClient && m_pJobManager->m_pSocketPoolEx->m_bServerLoadingBalance)
							{
#ifndef _WIN32_WCE
								ServerSide::CClientPeer *p = (ServerSide::CClientPeer*)m_pIdentity;
								if(p != UNULL_PTR)
									p->DropRequestResult(usRequestId);
#endif
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

				virtual void* GetIdentity()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_pIdentity;
				}

				virtual ClientSide::CAsyncServiceHandler* GetHandler()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_pHandler;
				}

				virtual unsigned long GetCountOfTasks()
				{
					int	idTask;
					unsigned short usRequestId;
					unsigned long *pReqSize;
					unsigned long ulPos = 0;
					unsigned long ulCount = 0;
					CAutoLock	AutoLock(&g_cs.m_sec);
					CUQueue &UQueue = (*m_Tasks);
					unsigned long ulSize = UQueue.GetSize();
					while(ulSize >= (ulPos + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned long)))
					{
						idTask = *((int*)UQueue.GetBuffer(ulPos));
						ulPos += sizeof(idTask);
						usRequestId = *((unsigned short*)UQueue.GetBuffer(ulPos));
						switch(usRequestId)
						{
						case idStartBatching:
							break;
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
						ulPos += sizeof(bool); //bClient
					}
					return ulCount;
				}

				virtual tagJobStatus GetJobStatus()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_Status;
				}

				virtual bool GetBundled()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_bBundled;
				}

				virtual void SetBundled(bool bBundled)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_Status != jsCreating && m_Status != jsQueued)
						return;
					m_bBundled = bBundled;
				}

			private:
				bool			m_bBundled;
				int				m_idTask;
				tagJobStatus	m_Status;
				__int64			m_jobId;
				THandler		*m_pHandler;
				CJobManager		*m_pJobManager;
				void			*m_pIdentity;
				CScopeUQueue	m_Tasks;
				HANDLE			m_hEvent;
				int				m_nWaiting;
				
				friend			CSocketPoolEx;
				friend			CJobManager;
				friend			CIdentityJobMap;
			};

		private:
			class CIdentityJobMap : public CSimpleMap<void*, Internal::CUSimpleArray<CJobContext *> >
			{
			public:
				CIdentityJobMap()
				{
					m_pJobManager = UNULL_PTR;
					m_lRemoveIndex = 0;
				}

				~CIdentityJobMap()
				{
					RemoveAllJobs();
				}

				unsigned long RemoveAllJobs()
				{
					int n;
					unsigned long ulCount = 0;
					int nSize = GetSize();
					for(n=nSize-1; n>=0; n--)
					{
						ulCount += CleanJobs(GetKeyAt(n));
					}
					return ulCount;
				}

				unsigned long CleanJobs(void *pIdentity)
				{
					unsigned long nSize = 0;
					int nIndex = FindKey(pIdentity);
					if(nIndex != -1)
					{
						unsigned long n;
						CSimpleArray<CJobContext *> &aJob = GetValueAt(nIndex);
						nSize = aJob.GetSize();
						for(n=0; n<nSize; n++)
						{
							CJobContext *p = aJob[n];
							m_pJobManager->EmptyJob(p);
						}
						aJob.RemoveAll();
						Remove(pIdentity);
					}
					return nSize;
				}

				void Add(CJobContext *pJobContext)
				{
					void *p = pJobContext->GetIdentity();
					int nIndex = FindKey(p);
					if(nIndex == -1)
					{
						Internal::CUSimpleArray<CJobContext *> empty;
						CSimpleMap<void*, Internal::CUSimpleArray<CJobContext *> >::Add(p, empty); 
						nIndex = FindKey(p);
					}
					Internal::CUSimpleArray<CJobContext *> &aJob = GetValueAt(nIndex);
					aJob.Add(pJobContext);
				}

				unsigned long GetCountOfJobs()
				{
					int n;
					unsigned long ulCount = 0;
					int nSize = GetSize();
					for(n=0; n<nSize; n++)
					{
						CSimpleArray<CJobContext *> &aJob = GetValueAt(n);
						ulCount += aJob.GetSize();
					}
					return ulCount;
				}

				__int64 GetMemoryConsumed()
				{
					int n;
					int m;
					__int64 llMemory = 0;
					int nSize = GetSize();
					for(n=0; n<nSize; n++)
					{
						CSimpleArray<CJobContext *> &aJob = GetValueAt(n);
						int mSize = aJob.GetSize();
						for(m=0; m<mSize; m++)
						{
							CJobContext *p = aJob[m];
							llMemory += p->m_Tasks->GetMaxSize();
						}
					}
					return llMemory;
				}

				CJobContext* SeekJob(__int64 jobId)
				{
					int n;
					int m;
					int nSize = GetSize();
					for(n=0; n<nSize; n++)
					{
						CSimpleArray<CJobContext *> &aJob = GetValueAt(n);
						int mSize = aJob.GetSize();
						for(m=0; m<mSize; m++)
						{
							CJobContext *p = aJob[m];
							if(p->m_jobId == jobId)
								return p;
						}
					}
					return UNULL_PTR;
				}

				CJobContext *RemoveJobContext()
				{
					unsigned long nSize = (unsigned long)GetSize();
					ATLASSERT(nSize > 0);
					unsigned long nStart = (m_lRemoveIndex%nSize);
					CSimpleArray<CJobContext *> &aJob = GetValueAt(nStart);
					ATLASSERT(aJob.GetSize() > 0);
					CJobContext *p = aJob[0];
					aJob.RemoveAt(0);
					if(aJob.GetSize() == 0)
					{
						Remove(GetKeyAt(nStart));
					}
					else
					{
						m_lRemoveIndex++;
					}
					return p;
				}
				
				bool CancelJob(__int64 jobId)
				{
					int n;
					int m;
					int nSize = GetSize();
					for(n=0; n<nSize; n++)
					{
						CSimpleArray<CJobContext *> &aJob = GetValueAt(n);
						int mSize = aJob.GetSize();
						for(m=0; m<mSize; m++)
						{
							CJobContext *p = aJob[m];
							if(p->m_jobId == jobId)
							{
								aJob.RemoveAt(m);
								if(aJob.GetSize() == 0)
									Remove(GetKeyAt(n));
								return true;
							}
						}
					}
					return false;
				}

			public:
				CJobManager	*m_pJobManager;

			private:
				unsigned long	m_lRemoveIndex;
			};

			class CJobManager : public IJobManager
			{
			public:
				CJobManager() : m_jobIndex(0), m_pSocketPoolEx(UNULL_PTR), m_nRecycleBinSize(10)
				{
					m_mapQueuedIdentityJob.m_pJobManager = this;
					m_hIdentityWait = ::CreateEvent(UNULL_PTR, TRUE, TRUE, UNULL_PTR);
				}

				virtual ~CJobManager()
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_mapQueuedIdentityJob.RemoveAllJobs();
					int nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJC = m_aJobEmpty[n];
						delete pJC;
					}
					m_aJobEmpty.RemoveAll();
					nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJC = m_aJobProcessing[n];
						delete pJC;
					}
					m_aJobProcessing.RemoveAll();
					::SetEvent(m_hIdentityWait);
					::Sleep(0);
					::CloseHandle(m_hIdentityWait);
				}

				//disable copy constructor and assignment operator
				CJobManager(const CJobManager &jm);
				CJobManager& operator=(const CJobManager &jm);

			public:
				void CleanTasks(bool bSoft)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_mapQueuedIdentityJob.RemoveAllJobs();
					if(!bSoft)
					{
						int n;
						int nSize = m_aJobProcessing.GetSize();
						for(n=0; n<nSize; n++)
						{
							CJobContext *pJC = m_aJobProcessing[n];
							EmptyJob(pJC);
						}
						m_aJobProcessing.RemoveAll();
					}
				}

				virtual IUSocketPool* GetSocketPool()
				{
					return m_pSocketPoolEx->GetUSocketPool().p;
				}

				virtual unsigned int CancelJobs(void *pIdentity)
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					unsigned long ulJobs = m_mapQueuedIdentityJob.CleanJobs(pIdentity);
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *jb = m_aJobProcessing[n];
						if(jb->m_pIdentity != pIdentity)
							continue;
						if(jb->m_Status != jsRunning)
							continue;
						jb->m_Status = jsInitial;
						MyCancel(jb->m_pHandler);
						ulJobs++;
					}
					return ulJobs;
				}

				virtual void ShrinkMemory(unsigned long ulMemoryChunkSize)
				{
					int n;
					if(ulMemoryChunkSize < DEFAULT_UQUEUE_BLOCK_SIZE)
						ulMemoryChunkSize = DEFAULT_UQUEUE_BLOCK_SIZE;
					CAutoLock	AutoLock(&g_cs.m_sec);
					int nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJC = m_aJobEmpty[n];
						if(pJC->m_Status == jsInitial && pJC->m_Tasks->GetMaxSize() > ulMemoryChunkSize)
							pJC->m_Tasks->ReallocBuffer(ulMemoryChunkSize);
					}
				}

				virtual void GetIdentities(CSimpleArray<void*> &Identities)
				{
					int n;
					Identities.RemoveAll();
					CAutoLock	AutoLock(&g_cs.m_sec);
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *jb = m_aJobProcessing[n];
						if(Identities.Find(jb->m_pIdentity) == -1)
							Identities.Add(jb->m_pIdentity);
					}
					nSize = m_mapQueuedIdentityJob.GetSize();
					for(n=0; n<nSize; n++)
					{
						void *p = m_mapQueuedIdentityJob.GetKeyAt(n);
						if(Identities.Find(p) == -1)
							Identities.Add(p);
					}
				}

				virtual void GetJobs(void *pIdentity, CSimpleArray<__int64> &jobs)
				{
					int n;
					int j;
					jobs.RemoveAll();
					CAutoLock	AutoLock(&g_cs.m_sec);
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *jb = m_aJobProcessing[n];
						if(jb->m_pIdentity != pIdentity)
							continue;
						if(jb->m_Status != jsRunning)
							continue;
						jobs.Add(jb->m_jobId);
					}

					nSize = m_mapQueuedIdentityJob.GetSize();
					for(n=0; n<nSize; n++)
					{
						void *p = m_mapQueuedIdentityJob.GetKeyAt(n);
						if(p != pIdentity)
							continue;
						Internal::CUSimpleArray<CJobContext *> &myJobContext = m_mapQueuedIdentityJob.GetValueAt(n);
						int jSize = myJobContext.GetSize();
						for(j=0; j<jSize; j++)
						{
							CJobContext *jb = myJobContext[j];
							jobs.Add(jb->m_jobId);
						}
						break;
					}
				}

				virtual unsigned int GetCountOfQueuedJobs()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_mapQueuedIdentityJob.GetCountOfJobs();
				}

				virtual unsigned int GetCountOfJobs()
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					unsigned long ulCount = (unsigned long)m_mapQueuedIdentityJob.GetCountOfJobs();
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *jb = m_aJobProcessing[n];
						if(jb->m_Status == jsRunning)
							ulCount++;
					}
					return ulCount;
				}

				virtual unsigned int GetRecycleBinSize()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_nRecycleBinSize;
				}
				virtual void SetRecycleBinSize(unsigned int nRecycleBinSize)
				{
					if(nRecycleBinSize > 10240)
						nRecycleBinSize = 10240;
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_nRecycleBinSize = nRecycleBinSize;
				}

				virtual __int64 GetMemoryConsumed()
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					__int64 llSize = m_mapQueuedIdentityJob.GetMemoryConsumed();
					int nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJC = m_aJobEmpty[n];
						llSize += pJC->m_Tasks->GetMaxSize();
					}

					nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJC = m_aJobProcessing[n];
						llSize += pJC->m_Tasks->GetMaxSize();
					}
					return llSize;
				}

				virtual int ResetPosition(__int64 jobId, int nNewPosition)
				{
					if (nNewPosition < 0)
						nNewPosition = 0;
					CAutoLock	AutoLock(&g_cs.m_sec);
					CJobContext *pJob = (CJobContext*)SeekJob(jobId);
					if(pJob == UNULL_PTR || pJob->m_Status != jsQueued)
						return -1;
					int nIndex = m_mapQueuedIdentityJob.FindKey(pJob->m_pIdentity);
					Internal::CUSimpleArray<CJobContext*> &jobs = m_mapQueuedIdentityJob.GetValueAt(nIndex);
					nIndex = jobs.Find(pJob);
					if(nIndex == nNewPosition)
						return nIndex;
					jobs.Remove(pJob);
					if(nNewPosition >= jobs.GetSize())
					{
						jobs.Add(pJob);
						return (jobs.GetSize() - 1);
					}
					jobs.Insert(nNewPosition, pJob);
					return nNewPosition;
				}

				virtual IJobContext* CreateJob(void *pIdentity)
				{
					int n;
					CJobContext *pJob;
					CAutoLock	AutoLock(&g_cs.m_sec);
					++m_jobIndex;
					int nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						pJob = m_aJobEmpty[n];
						if(pJob->m_Status == jsInitial)
						{
							if((unsigned int)(nSize - n - 1) < m_nRecycleBinSize)
								break;
							pJob->m_jobId = m_jobIndex;
							pJob->m_Status = jsCreating;
							pJob->m_pIdentity = pIdentity;
							return pJob;
						}
					}
					pJob = new CJobContext();
					m_aJobEmpty.Add(pJob);
					pJob->m_pJobManager = this;
					pJob->m_jobId = m_jobIndex;
					pJob->m_Status = jsCreating;
					pJob->m_pIdentity = pIdentity;
					return pJob;
				}

				virtual bool DestroyJob(IJobContext *pJobContext)
				{
					
					if(pJobContext == UNULL_PTR)
						return false;
					bool ok = false;
					CAutoLock	AutoLock(&g_cs.m_sec);
					CJobContext *pJob = (CJobContext *)pJobContext;
					switch(pJob->m_Status)
					{
					case jsCreating:
						{
							int n;
							bool b = true;
							CSimpleMap<int, CTaskContext> mapIdTask;
							pJob->GetTasks(mapIdTask);
							int nSize = mapIdTask.GetSize();
							for(n=0; n<nSize; n++)
							{
								CTaskContext &tc = mapIdTask.GetValueAt(n);
								if(tc.m_bClient)
								{
									b = false;
									break;
								}
							}
							if(b)
							{
								m_aJobEmpty.Remove(pJob);
								EmptyJob(pJob);
								ok = true;
							}
						}
						break;
					case jsQueued:
						ok = CancelJob(pJob->m_jobId);
						break;
					case jsRunning:
					case jsInitial:
						//can't do anything
						break;
					default:
						ATLASSERT(FALSE); //should not come here
						break;
					}
					return ok;
				}

				virtual bool WaitAll(unsigned long ulTimeout)
				{
					return m_pSocketPoolEx->WaitUntilFinished(ulTimeout);
				}

				virtual bool WaitAny(void *pIdentity, CSimpleArray<__int64> &jobs, unsigned long ulTimeout)
				{
					CSimpleArray<__int64> myJobs;
					GetJobs(pIdentity, myJobs);
					unsigned int nCount = myJobs.GetSize();
					return WaitAny(myJobs.GetData(), nCount, jobs, ulTimeout);
				}

				virtual bool WaitAny(const __int64 *pJobId, unsigned int nCount, CSimpleArray<__int64> &jobs, unsigned long ulTimeout)
				{
					unsigned int n;
					__int64 jobId;
					DWORD dwRtn;
					bool ok = true;
					jobs.RemoveAll();
					if(nCount == 0 || pJobId == UNULL_PTR)
						return true;
					CScopeUQueue su;
					HANDLE hTwmp;
					CSimpleMap<HANDLE, __int64> mapEventJobId;
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						for(n=0; n<nCount; n++)
						{
							jobId = pJobId[n];
							CJobContext *pJob = (CJobContext*)SeekJob(jobId);
							if(pJob == UNULL_PTR || pJob->m_Status == jsInitial)
							{
								jobs.Add(jobId);
								continue;
							}
							HANDLE hEvent = pJob->m_hEvent;
							switch(pJob->m_Status)
							{
							case jsCreating:
							case jsQueued:
							case jsRunning:
								mapEventJobId.Add(hEvent, jobId);
								su << hEvent;
								break;
							default:
								ATLASSERT(FALSE);
								break;
							}
						}
						n = su->GetSize()/sizeof(HANDLE);
						if(n == 0)
							return true;
						CUQueue *pUQueue = &(*su);
						hTwmp = ::CreateEvent(UNULL_PTR, TRUE, FALSE, UNULL_PTR);
						m_mapHandleJobs.Add(hTwmp, pUQueue);
					}
					dwRtn = ::WaitForSingleObject(hTwmp, ulTimeout);
					if(dwRtn > 0)
					{
						if(jobs.GetSize() == 0)
							ok = false;
					}

					if(ok)
					{
						nCount = su->GetSize()/sizeof(HANDLE);
						CAutoLock	AutoLock(&g_cs.m_sec);
						for(n=0; n<nCount; n++)
						{
							HANDLE *p = (HANDLE*)su->GetBuffer();
							HANDLE h = p[n];
							jobId = mapEventJobId.Lookup(h);
							CJobContext *pJob = (CJobContext*)SeekJob(jobId);
							if(pJob == UNULL_PTR || pJob->m_Status == jsInitial)
							{
								jobs.Add(jobId);
							}
						}
					}
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						m_mapHandleJobs.Remove(hTwmp);
					}
					::CloseHandle(hTwmp);
					return ok;
				}

				virtual bool Wait(const __int64 *pJobId, unsigned int nCount, unsigned long ulTimeout)
				{
					unsigned int n;
					if(nCount == 0 || pJobId == UNULL_PTR)
						return true;
					unsigned ulMyTimeout = ulTimeout;
					unsigned long ulStart = ::GetTickCount();
					for(n=0; n<nCount; n++)
					{
						IJobContext *pIJobContext = SeekJob(pJobId[n]);
						if(pIJobContext == UNULL_PTR)
							continue;
						if(!pIJobContext->Wait(ulMyTimeout))
							return false;
						unsigned long ulNow = ::GetTickCount();
						if(ulNow > ulStart)
							ulMyTimeout = ulTimeout - (ulNow - ulStart);
						if(ulMyTimeout > ulTimeout)
							ulMyTimeout = 0;
					}
					return true;
				}

				virtual bool Wait(void *pIdentity, unsigned long ulTimeout)
				{
					unsigned long ulStart = ::GetTickCount();
					unsigned long ulMyTimeout = ulTimeout;
					CSimpleArray<__int64> jobs;
					do
					{
						GetJobs(pIdentity, jobs);
						int nSize = jobs.GetSize();
						if(nSize == 0)
							break;
						IJobContext *jb = SeekJob(jobs[nSize-1]);
						if(jb == UNULL_PTR)
							break;
						if(!jb->Wait(ulMyTimeout))
							return false;
						unsigned long ulNow = ::GetTickCount();
						if(ulNow > ulStart)
							ulMyTimeout = ulTimeout - (ulNow - ulStart);
						if(ulMyTimeout > ulTimeout)
							ulMyTimeout = 0;
					}while(true);
					return true;
				}

				virtual void UnlockIdentity(ClientSide::CAsyncServiceHandler *pIdentity)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					void *p = pIdentity;
					if(m_aLockedIdentity.Remove(p))
						::SetEvent(m_hIdentityWait);
				}

				virtual ClientSide::CAsyncServiceHandler* LockIdentity(unsigned long ulTimeout)
				{
					unsigned long ulMyTimeout = ulTimeout;
					unsigned long ulStart = ::GetTickCount();
ao:					int n = 0;
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_pSocketPoolEx->m_mapSocketHandler.GetSize() == 0)
							return UNULL_PTR;
						long lConnected = 0;
						m_pSocketPoolEx->GetUSocketPool()->get_ConnectedSocketsEx(&lConnected);
						if(lConnected == 0)
							return UNULL_PTR;
						int nSize = m_pSocketPoolEx->m_mapSocketHandler.GetSize();
						for(n=0; n<nSize; n++)
						{
							CAsyncServiceHandler *p = m_pSocketPoolEx->m_mapSocketHandler.GetValueAt(n);
							if(!p->GetAttachedClientSocket()->IsConnected())
								continue;
							do
							{
								void *pIdentity  = p;
								if(m_mapQueuedIdentityJob.FindKey(pIdentity) != -1)
									break;
								int j, jSize = m_aJobProcessing.GetSize();
								for(j=0; j<jSize; j++)
								{
									CJobContext *jb = m_aJobProcessing[j];
									if(jb->m_pIdentity == p)
										break;
								}
								if(jSize && j != jSize)
									break;
								jSize = m_aLockedIdentity.GetSize();
								for(j=0; j<jSize; j++)
								{
									if(p == m_aLockedIdentity[j])
										break;
								}
								if(jSize && j != jSize)
									break;
								jSize = m_aJobEmpty.GetSize();
								for(j=0; j<jSize; j++)
								{
									CJobContext *jb = m_aJobEmpty[j];
									if(jb->m_pIdentity == p)
										break;
								}
								if(jSize && j != jSize)
									break;
								m_aLockedIdentity.Add(pIdentity);
								return p;
							}while(false);
						}
						::ResetEvent(m_hIdentityWait);
					}
					DWORD dwRtn = ::WaitForSingleObject(m_hIdentityWait, ulMyTimeout);
					if(dwRtn == WAIT_OBJECT_0)
					{
						unsigned long ulNow = ::GetTickCount();
						if(ulNow > ulStart)
							ulMyTimeout -= (ulNow - ulStart);
						if(ulMyTimeout > ulTimeout)
							ulMyTimeout = 0;
						goto ao;
					}
					return UNULL_PTR; //all of async handlers are already associated with jobs.
				}

				virtual bool EnqueueJob(IJobContext *pJobContext)
				{
					if(pJobContext == UNULL_PTR || pJobContext->GetJobStatus() != jsCreating || m_pSocketPoolEx->GetUSocketPool() == UNULL_PTR)
						return false;
					CAutoLock	AutoLock(&g_cs.m_sec);
					CJobContext *pJC = (CJobContext*)pJobContext;
					if(m_aJobEmpty.Remove(pJC))
					{
						long lConnected = 0;
						m_pSocketPoolEx->GetUSocketPool()->get_ConnectedSocketsEx(&lConnected);
						if(pJC->GetCountOfTasks() > 0 && lConnected > 0)
						{
							pJC->m_Status = jsQueued;
							m_mapQueuedIdentityJob.Add(pJC);
							return true;
						}
						else
						{
							EmptyJob(pJC);
							if(lConnected == 0)
								m_pSocketPoolEx->m_hr = specNoOpenedSocket;
							else
								m_pSocketPoolEx->m_hr = specUnexpected;
							return false;
						}
					}
					return false;
				}

				virtual IJobContext* SeekJob(__int64 jobId)
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					IJobContext *p = m_mapQueuedIdentityJob.SeekJob(jobId);
					if(p != UNULL_PTR)
						return p;
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJob = m_aJobProcessing[n];
						if(pJob->m_Status != jsRunning)
							continue;
						ATLASSERT(pJob->m_Status != jsInitial);
						ATLASSERT(pJob->m_pHandler != UNULL_PTR);
						ATLASSERT(pJob->m_pJobManager != UNULL_PTR);
						if(pJob->m_jobId == jobId)
							return pJob;
					}
					return UNULL_PTR;
				}

				virtual tagJobStatus GetJobStatus(__int64 jobId)
				{
					int n;
					CAutoLock	AutoLock(&g_cs.m_sec);
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJob = m_aJobProcessing[n];
						ATLASSERT(pJob->m_Status != jsInitial);
						ATLASSERT(pJob->m_pHandler != UNULL_PTR);
						ATLASSERT(pJob->m_pJobManager != UNULL_PTR);
						if(pJob->m_jobId == jobId)
							return pJob->m_Status;
					}
					
					{
						CJobContext *pJob = m_mapQueuedIdentityJob.SeekJob(jobId);
						if(pJob != UNULL_PTR)
							return pJob->m_Status;
					}

					nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJob = m_aJobEmpty[n];
						ATLASSERT(pJob->m_pHandler == UNULL_PTR);
						ATLASSERT(pJob->m_pJobManager != UNULL_PTR);
						ATLASSERT(pJob->GetCountOfTasks() == 0);
						if(pJob->m_jobId == jobId && pJob->m_Status == jsCreating)
							return jsCreating;
					}
					return jsInitial;
				}

				virtual bool CancelJob(__int64 jobId)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					CJobContext *pJC = (CJobContext*)SeekJob(jobId);
					if(pJC == UNULL_PTR)
						return false;
					if(pJC->m_Status == jsQueued && m_mapQueuedIdentityJob.CancelJob(pJC->m_jobId))
					{
						EmptyJob(pJC);
						return true;
					}
					if(pJC->m_Status == jsRunning)
					{
						pJC->m_Status = jsInitial;
						MyCancel(pJC->m_pHandler);
						return true;
					}
					return false;
				}
			private:
				void RemoveTasksInternally(CJobContext *pJC)
				{
					if(pJC != UNULL_PTR && m_pSocketPoolEx->m_bServerLoadingBalance)
					{
						int n;
						CSimpleMap<int, CTaskContext> mapTask;
						pJC->GetTasks(mapTask);
						int nSize = mapTask.GetSize();
						for(n=0; n<nSize; n++)
						{
							int nTaskId = mapTask.GetKeyAt(n);
							pJC->RemoveTask(nTaskId);
						}
					}
				}
				void EmptyJob(CJobContext *pJC)
				{
					int n;
					RemoveTasksInternally(pJC);
					pJC->m_pHandler = UNULL_PTR;
					pJC->m_Status = jsInitial;
					pJC->m_Tasks->SetSize(0);
					pJC->m_pIdentity = UNULL_PTR;
					m_aJobEmpty.Add(pJC);
					if(pJC->m_nWaiting != 0)
						::SetEvent(pJC->m_hEvent);
					int nSize = m_mapHandleJobs.GetSize();
					for(n=0; n<nSize; n++)
					{
						int j;
						CUQueue *pUQueue = m_mapHandleJobs.GetValueAt(n);
						int jSize = pUQueue->GetSize()/sizeof(HANDLE);
						HANDLE *p = (HANDLE*)(pUQueue->GetBuffer());
						for(j=0; j<jSize; j++)
						{
							if(p[j] == pJC->m_hEvent)
							{
								HANDLE h = m_mapHandleJobs.GetKeyAt(n);
								::SetEvent(h);
								return;
							}
						}
					}
				}

			private:
				CSimpleMap<HANDLE, CUQueue*>	m_mapHandleJobs;
				unsigned int					m_nRecycleBinSize;
				HANDLE							m_hIdentityWait;
				__int64							m_jobIndex;
				CIdentityJobMap					m_mapQueuedIdentityJob;
				CSimpleArray<CJobContext *>		m_aJobEmpty;
				CSimpleArray<CJobContext *>		m_aJobProcessing;
				CSocketPoolEx<THandler, TCS>	*m_pSocketPoolEx;
				CSimpleArray<void*>				m_aLockedIdentity;
				friend CSocketPoolEx;
				friend CJobContext;
				friend CIdentityJobMap;
			};

		public:
			CSocketPoolEx(IAsyncResultsHandler *pIAsyncResultsHandler = UNULL_PTR) 
				: CSocketPool<THandler, TCS>(pIAsyncResultsHandler),
				m_nFails(0), m_bWorking(false), m_bPause(false), m_bCallProcess(false), 
				m_bServerLoadingBalance(false)
			{
				m_hWait = ::CreateEvent(UNULL_PTR, TRUE, TRUE, UNULL_PTR);
				m_JobManager.m_pSocketPoolEx = this;
			}

			virtual ~CSocketPoolEx()
			{
				ShutdownPool();
				::CloseHandle(m_hWait);
			}
			//no copy constructor
			CSocketPoolEx(const CSocketPoolEx<THandler, TCS>& PBCNet);

		public:
			//no assignment operator
			CSocketPoolEx<THandler, TCS>& operator= (const CSocketPoolEx<THandler, TCS>& PBCNet);

		public:
			virtual void ShutdownPool()
			{
				Stop(true);
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_aPHandler.RemoveAll();
					m_aPauseHandler.RemoveAll();
				}
				CSocketPool<THandler, TCS>::ShutdownPool();
			}

			IJobManager* GetJobManager()
			{
				return &m_JobManager;
			}

			void ResetFails()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				m_nFails = 0;
			}

			unsigned int GetFails()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_nFails;
			}
			bool IsWorking()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_bWorking;
			}

			virtual THandler* Lock(ULONG ulTimeout, IUSocket *pIUSocketSameThreadWith = UNULL_PTR)
			{
				//should not call this method!
				ATLASSERT(FALSE);

				return UNULL_PTR;
			}

			void Unlock(THandler *pHandler)
			{
				//should not call this method!
				ATLASSERT(FALSE);
			}

			virtual void Unlock(TCS *pClientSocket)
			{
				//should not call this method!
				ATLASSERT(FALSE);
			}

			virtual void Stop(bool bCancelRequestsInQueue = false)
			{
				THandler *pHandler = UNULL_PTR;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(!m_bWorking)
						return;
					m_bWorking = false;
					if(m_aPHandler.GetSize() > 0)
						pHandler = m_aPHandler[0];
				}
				
				m_JobManager.CleanTasks(!bCancelRequestsInQueue);

				while(pHandler != UNULL_PTR)
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
					{
						if(bCancelRequestsInQueue)
						{
							MyCancel(pHandler);
						}
						pHandler->GetAttachedClientSocket()->WaitAll();
					}
					CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						m_aPHandler.Remove(pHandler);
						if(m_aPHandler.GetSize() > 0)
							pHandler = m_aPHandler[0];
						else
							break;
					}
				}

				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_aPauseHandler.GetSize() > 0)
						pHandler = m_aPauseHandler[0];
					else
						pHandler = UNULL_PTR;
				}
				while(pHandler != UNULL_PTR)
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
					{
						if(bCancelRequestsInQueue)
						{
							MyCancel(pHandler);
						}
						pHandler->GetAttachedClientSocket()->WaitAll();
					}
					CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						m_aPauseHandler.Remove(pHandler);
						if(m_aPauseHandler.GetSize() > 0)
							pHandler = m_aPauseHandler[0];
						else
							break;
					}
				}
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_bPause = false;
				}
				::SetEvent(m_hWait);
			}

			bool IsPaused()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_bPause;
			}

			virtual void Pause()
			{
				THandler *pHandler = UNULL_PTR;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_bPause || !m_bWorking)
						return;
					m_bPause = true;
					if(m_aPHandler.GetSize() > 0)
						pHandler = m_aPHandler[0];
				}

				while(pHandler != UNULL_PTR)
				{
					if(pHandler->GetAttachedClientSocket()->GetCountOfRequestsInQueue() > 0)
						pHandler->GetAttachedClientSocket()->WaitAll();
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						m_aPauseHandler.Add(pHandler);
						m_aPHandler.Remove(pHandler);
						if(m_aPHandler.GetSize() > 0)
							pHandler = m_aPHandler[0];
						else
							pHandler = UNULL_PTR;
					}
				}
			}

			virtual void Resume()
			{
				int n;
				CAutoLock	AutoLock(&g_cs.m_sec);
				if(!m_bPause)
					return;
				int nSize = m_aPauseHandler.GetSize();
				if(m_bWorking && nSize > 0)
				{
					for(n=nSize-1; n>=0; --n)
					{
						THandler *pHandler = m_aPauseHandler[n];
						m_aPauseHandler.Remove(pHandler);
						bool b;
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							b = ExecuteJob(pHandler);
						}
						if(!b)
							CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
						else
							m_aPHandler.Add(pHandler);
					}
				}
				m_bPause = false;
				if(m_aPHandler.GetSize() == 0)
					::SetEvent(m_hWait);
				else
					::ResetEvent(m_hWait);
			}

			bool IsAllLoaded()
			{
				long lIdle = 0;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_pIUSocketPool == UNULL_PTR)
						return false;
				}
				m_pIUSocketPool->get_IdleSocketsEx(&lIdle);
				return (lIdle == 0 && GetSocketsInParallel() > 0);
			}

			void GetHandlersInParallel(CSimpleArray<THandler*> &Handlers)
			{
				int n;
				Handlers.RemoveAll();
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_aPHandler.GetSize();
				for(n=0; n<nSize; n++)
					Handlers.Add(m_aPHandler[n]);

				nSize = m_aPauseHandler.GetSize();
				for(n=0; n<nSize; n++)
					Handlers.Add(m_aPauseHandler[n]);
			}

			unsigned int GetSocketsInParallel()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return (unsigned int)(m_aPHandler.GetSize() + m_aPauseHandler.GetSize());
			}

			virtual bool WaitUntilFinished(DWORD dwTime = INFINITE)
			{
				return (::WaitForSingleObject(m_hWait, dwTime) == WAIT_OBJECT_0);
			}

			virtual bool MakeConnection(const CConnectionContext &cc)
			{
				bool ok = CSocketPool<THandler, TCS>::MakeConnection(cc);
				if(!ok)
					return false;
				ok = false;
				CAutoLock	al(&g_cs.m_sec);
				do
				{
					if(!m_bWorking)
						break;
					THandler *pHandler = CSocketPool<THandler, TCS>::Lock(0, UNULL_PTR);
					if(pHandler == UNULL_PTR)
						break;
					if(m_bPause)
					{
						m_aPauseHandler.Add(pHandler);
						break;
					}
					bool ok;
					{
						CAutoReverseLock arl(&g_cs.m_sec);
						ok = ExecuteJob(pHandler);
					}
					if(ok)
						m_aPHandler.Add(pHandler);
					else
						CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
				}while(false);
				if(ok)
					::ResetEvent(m_hWait);
				return true;
			}

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
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(!m_bWorking)
					{
						m_hr = m_pIUSocketPool->get_LockedSocketsEx(&lLocks);
						if(lLocks > 0) //stop previous parallel processing first
							return false;
						m_aPHandler.RemoveAll();
						m_aPauseHandler.RemoveAll();
						m_bWorking = true;
					}
				}
				THandler *pHandler = CSocketPool<THandler, TCS>::Lock(0, UNULL_PTR);
				bool ok = false;
				while(pHandler != UNULL_PTR)
				{
					if(!ExecuteJob(pHandler))
					{
						//if no more tasks available, we stop here
						CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
						break;
					}
					else
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_bPause)
							m_aPauseHandler.Add(pHandler);
						else
							m_aPHandler.Add(pHandler);
					}
					pHandler = CSocketPool<THandler, TCS>::Lock(0, UNULL_PTR);
				}
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_bWorking = (GetSocketsInParallel() > 0);
					if(m_bWorking)
					{
						ok = true;
						::ResetEvent(m_hWait);
					}
					else
						::SetEvent(m_hWait);

					m_bPause = (m_aPauseHandler.GetSize() > 0);
				}
				return ok;
			}
		protected:
			virtual HRESULT __stdcall OnSocketPoolEvent(tagSocketPoolEvent spe, IUSocket* pIUSocket)
			{
				HRESULT hr = CSocketPool<THandler, TCS>::OnSocketPoolEvent(spe, pIUSocket);
				switch(spe)
				{
				case speUSocketCreated:
					{
						int n;
						CAutoLock	AutoLock(&g_cs.m_sec);
						int nSize = m_mapSocketHandler.GetSize();
						for(n=nSize-1; n>=0; n--)
						{
							TCS *cs = m_mapSocketHandler.GetKeyAt(n);
							if(cs->m_pIExtenalSocketEvent != UNULL_PTR)
								break;
							cs->m_pIExtenalSocketEvent = this;
							if(!m_bServerLoadingBalance)
								cs->m_pIJobManager = &m_JobManager;
						}
					}
					break;
				default:
					break;
				}
				return hr;
			}

			virtual bool OnFailover(THandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("A fail with job id = %d, task count = %d, fails = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks(), GetFails());	
#endif
				return true;
			}

			virtual void OnJobDone(THandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Job id = %d done, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
			}

			virtual bool OnExecutingJob(THandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Going to process job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
				return true;
			}

			virtual void OnJobProcessing(THandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Processing job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnReturnedResultProcessed(THandler *pHandler, IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Returned result processed with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnAllSocketsDisconnected()
			{
				
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				long lDead = 0;
				this->m_pIUSocketPool->get_ConnectedSocketsEx(&lDead);
				ATLTRACE("!!! All %d socket(s) disconnected !!!\n", lDead);
#endif					
			}

			virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
			{
				switch(nMsg)
				{
				case msgAllRequestsProcessed: 
					{
						int n;
						THandler *pHandler = UNULL_PTR;
						{
							CAutoLock	AutoLock(&g_cs.m_sec);
							if(!m_bWorking)
								return S_OK;
							int nSize = m_mapSocketHandler.GetSize();
							for(n=0; n<nSize; n++)
							{
								TCS *cs = m_mapSocketHandler.GetKeyAt(n);
								if(cs->GetSocket() == hSocket)
								{
									pHandler = m_mapSocketHandler.GetValueAt(n);
									if(m_bPause)
										pHandler = UNULL_PTR;
									break;
								}
							}
						}
						if(pHandler != UNULL_PTR)
						{
							if(!ExecuteJob(pHandler))
							{
								CAutoLock	AutoLock(&g_cs.m_sec);
								if(m_aPHandler.Remove(pHandler))
								{
									CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
								}
								if(GetSocketsInParallel() == 0)
								{
									m_bWorking = false;
									m_bPause = false;
									::SetEvent(m_hWait);
								}
							}
						}
					}
					break;
				case msgRequestRemoved:
					break;
				default:
					break;
				}
				return S_OK;
			}

		private:
			static void MyCancel(CAsyncServiceHandler *pHandler)
			{
				pHandler->GetAttachedClientSocket()->Cancel();
				pHandler->GetAttachedClientSocket()->GetIUSocket()->DoEcho();
			}

			void Failover(THandler *pHandler)
			{
				int n;
				CJobContext *pJC;
				//no needed CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_JobManager.m_aJobProcessing.GetSize();
				for(n=0; n<nSize; n++)
				{
					pJC = m_JobManager.m_aJobProcessing[n];
					if(pJC->m_pHandler == pHandler)
					{
						bool b = false;
						if(pJC->m_Status == jsRunning)
						{
							m_nFails++;
							{
								CAutoReverseLock arl(&g_cs.m_sec);
								b = OnFailover(pHandler, pJC);
							}
						}
						if(m_JobManager.m_aJobProcessing.Remove(pJC))
						{
							if(b)
							{
								pJC->m_Status = jsQueued;
								m_JobManager.m_mapQueuedIdentityJob.Add(pJC);
								pJC->m_pHandler = UNULL_PTR;
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

			void EmptyJob(CJobContext *pJC)
			{
				m_JobManager.EmptyJob(pJC);
			}

			bool ExecuteJob(THandler *pHandler)
			{
				int n;
				HRESULT	hr;
				unsigned long nReqSize;
				CJobContext *pJC;
				bool bHttpBatching = false;
				bool bNext = false;
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_JobManager.m_aJobProcessing.GetSize();
				for(n=0; n<nSize; n++)
				{
					pJC = m_JobManager.m_aJobProcessing[n];
					if(pJC->m_pHandler == pHandler)
					{
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							OnJobDone(pHandler, pJC);
						}
						
						if(m_JobManager.m_aJobProcessing.Remove(pJC))
						{
							EmptyJob(pJC);
						}
						break;
					}
				}
				
				unsigned int nCountOfTasks;
				do
				{
					if(m_JobManager.m_mapQueuedIdentityJob.GetSize() == 0)
						return false;
					pJC = m_JobManager.m_mapQueuedIdentityJob.RemoveJobContext();
					m_JobManager.m_aJobProcessing.Add(pJC);

					{
						CAutoReverseLock arl(&g_cs.m_sec);
						bNext = OnExecutingJob(pHandler, pJC);
					}

					nCountOfTasks = pJC->GetCountOfTasks();
					if(bNext && nCountOfTasks > 0)
						break;
					else
					{
						m_JobManager.m_aJobProcessing.Remove(pJC);
						EmptyJob(pJC);
					}
				}while(true);
				pJC->m_pHandler = pHandler;
				pJC->m_Status = jsRunning;
				const CComPtr<IUFast> &pIUFast = pHandler->GetAttachedClientSocket()->GetIUFast();
				const CComPtr<IUSocket>& pIUSocket = pHandler->GetAttachedClientSocket()->GetIUSocket();
				if(pJC->m_bBundled)
					hr = pIUFast->SendRequestEx(idStartJob, 0, UNULL_PTR);
				unsigned long ulPos = 0;
				CUQueue &UQueue = (*(pJC->m_Tasks));
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
					case idHeader:
						if(nCountOfTasks > 1 && pJC->m_Tasks->GetSize() < 64*1024)
						{
							hr = pIUSocket->StartBatching();
							bHttpBatching = true;
						}
					default:
						hr = pIUFast->SendRequestEx(usRequestId, nReqSize, (BYTE*)UQueue.GetBuffer(ulPos));
						if(bHttpBatching)
						{
							switch(usRequestId)
							{
							case idGet:
							case idPost:
							case idHead:
							case idPut:
							case idDelete:
							case idOptions:
							case idTrace:
							case idConnect:
							case idMultiPart:
								hr = pIUSocket->CommitBatching(false);
								break;
							default:
								break;
							}
						}
					}
					ulPos += nReqSize;
					ulPos += sizeof(bool); //Client request or not
					if(hr != S_OK)
						break;
				}
				if(pJC->m_bBundled)
					hr = pIUFast->SendRequestEx(idEndJob, 0, UNULL_PTR);
				{
					CAutoReverseLock arl(&g_cs.m_sec);
					OnJobProcessing(pHandler, pJC);
				}
				return true;
			}

			void ResultReturned(THandler *pHandler, unsigned short usRequestId)
			{
				int n;
				CJobContext *pJC;
				//no required here CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_JobManager.m_aJobProcessing.GetSize();
				for(n=0; n<nSize; n++)
				{
					pJC = m_JobManager.m_aJobProcessing[n];
					if(pJC->m_pHandler == pHandler)
					{
						if(pJC->m_Status != jsRunning)
							break;
						if(!m_bServerLoadingBalance)
						{
							bool bClient;
							pJC->RemoveTask(pJC->m_pHandler->GetAttachedClientSocket()->GetReturnRandom(), usRequestId, bClient);
						}

						{
							CAutoReverseLock arl(&g_cs.m_sec);
							OnReturnedResultProcessed(pHandler, pJC, usRequestId);
						}
						break;
					}
				}
			}

			virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd)
			{
				int n;
				CAutoLock	AutoLock(&g_cs.m_sec);
//				ATLTRACE("<--- Socket closing ---> hSocket = %d, error = %d\n", hSocket);
				if(!m_bWorking)
					return S_OK;
				int nSize = m_mapSocketHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					TCS *cs = m_mapSocketHandler.GetKeyAt(n);
					if(cs->GetSocket() == hSocket)
					{
						THandler *pHandler = m_mapSocketHandler.GetValueAt(n);
						if(cs->GetCountOfRequestsInQueue() > 0)
						{
							Failover(pHandler);
						}
						bool bRemove = (m_aPHandler.Remove(pHandler) != FALSE);
						if(bRemove)
							CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
						else if((bRemove = (m_aPauseHandler.Remove(pHandler) != FALSE)))
							CSocketPool<THandler, TCS>::Unlock(pHandler->GetAttachedClientSocket());
						if(bRemove && GetSocketsInParallel() == 0)
							m_bCallProcess = true;
						break;
					}
				}
				return S_OK;
			}

			virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError)
			{
				bool b = false;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
//					ATLTRACE("+++ Socket closed hSocket = %d, error = %d +++\n", hSocket, lError);
					b = m_bCallProcess;
					if(b)
						m_bCallProcess = false;
				}
				if(b)
				{
					if(!Process())
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
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
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_JobManager.CleanTasks(false);
					m_JobManager.ShrinkMemory(DEFAULT_UQUEUE_BLOCK_SIZE);
				}
				::SetEvent(m_JobManager.m_hIdentityWait);
				return S_OK;
			}

			virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError)
			{
				return S_OK;
			}

			virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
			{
				int n;
				if(sFlag != rfCompleted)
					return S_OK;
				CAutoLock	AutoLock(&g_cs.m_sec);
				int nSize = m_mapSocketHandler.GetSize();
				for(n=0; n<nSize; n++)
				{
					TCS *cs = m_mapSocketHandler.GetKeyAt(n);
					if(cs->GetSocket() == hSocket)
					{
						THandler *pHandler = m_mapSocketHandler.GetValueAt(n);
						ATLASSERT(pHandler != UNULL_PTR);
						ResultReturned(pHandler, (unsigned short)nRequestID);
						break;
					}
				}
				return S_OK;
			}
#if _MSC_VER >= 1300
		protected:
#else
		public:
#endif
			bool					m_bServerLoadingBalance;

		private:
			bool					m_bPause;
			bool					m_bWorking;
			bool					m_bCallProcess;
			unsigned int			m_nFails;
			HANDLE					m_hWait;
			CSimpleArray<THandler*> m_aPHandler;
			CSimpleArray<THandler*> m_aPauseHandler;
			CJobManager				m_JobManager;
			friend class CJobManager;
		};
	}; //ClientSide

#ifndef _WIN32_WCE
	namespace ServerSide
	{
/*		#include "uscktpro.h"
		#ifdef __BORLANDC__	  //Borland compiler
			//	#pragma comment(lib, "usktprob.lib")
		#else					//Visual C++
			#pragma comment(lib, "usktpror.lib")
		#endif */
		#include "SPLoader.h"
		extern CSocketProServerLoader g_SocketProLoader;
		enum tagEvent
		{
			eOnSwitchTo = 0x1,
			eOnFastRequestArrive = 0x2,
			eOnSlowRequestArrive = 0x4,
			eOnClose = 0x8,
			eOnIsPermitted = 0x10,
			eOnChatRequestComing = 0x20,
			eOnChatRequestCame = 0x40,
			eOnSSLEvent = 0x80,
			eOnAccept = 0x100,
			eOnBaseRequestCame = 0x200,
			eOnWinMessage = 0x400,
			eOnThreadStarted = 0x800,
			eOnThreadShuttingDown = 0x1000,
			eOnPretranslateMessage = 0x2000,
			eOnSend = 0x4000,
			eOnReceive = 0x8000,
			eOnSendReturnData = 0x10000,
			eOnSlowRequestProcessed = 0x20000,
			eOnCleanPool = 0x40000,
			eOnThreadCreated = 0x80000,
		};

		class CBaseService;
		class CSocketProServer;

		#define BEGIN_SWITCH(RequestId) switch(RequestId){

		#define M_I0_R0(id, func) case id:{func();SendResult(id);}break;

		#define M_I0_R1(id, func, R0) case id:{R0 r0;func(r0);SendResult(id,r0);}break;

		#define M_I0_R2(id, func, R0, R1) case id:{R0 r0;R1 r1;func(r0,r1);SendResult(id,r0,r1);}break;

		#define M_I0_R3(id, func, R0, R1, R2) case id:{R0 r0;R1 r1;R2 r2;func(r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I0_R4(id, func, R0, R1, R2, R3) case id:{R0 r0;R1 r1;R2 r2;R3 r3;func(r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I0_R5(id, func, R0, R1, R2, R3, R4) case id:{R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;func(r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




		#define M_I1_R0(id, func, A0) case id:{A0 a0;m_UQueue>>a0;func(a0);SendResult(id);}break;

		#define M_I1_R1(id, func, A0, R0) case id:{A0 a0;R0 r0;m_UQueue>>a0;func(a0,r0);SendResult(id,r0);}break;

		#define M_I1_R2(id, func, A0, R0, R1) case id:{A0 a0;R0 r0;R1 r1;m_UQueue>>a0;func(a0,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I1_R3(id, func, A0, R0, R1, R2) case id:{A0 a0;R0 r0;R1 r1;R2 r2;m_UQueue>>a0;func(a0,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I1_R4(id, func, A0, R0, R1, R2, R3) case id:{A0 a0;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0;func(a0,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I1_R5(id, func, A0, R0, R1, R2, R3, R4) case id:{A0 a0;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0;func(a0,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




		#define M_I2_R0(id, func, A0, A1) case id:{A0 a0;A1 a1;m_UQueue>>a0>>a1;func(a0,a1);SendResult(id);}break;

		#define M_I2_R1(id, func, A0, A1, R0) case id:{A0 a0;A1 a1;R0 r0;m_UQueue>>a0>>a1;func(a0,a1,r0);SendResult(id,r0);}break;

		#define M_I2_R2(id, func, A0, A1, R0, R1) case id:{A0 a0;A1 a1;R0 r0;R1 r1;m_UQueue>>a0>>a1;func(a0,a1,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I2_R3(id, func, A0, A1, R0, R1, R2) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I2_R4(id, func, A0, A1, R0, R1, R2, R3) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I2_R5(id, func, A0, A1, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




		#define M_I3_R0(id, func, A0, A1, A2) case id:{A0 a0;A1 a1;A2 a2;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2);SendResult(id);}break;

		#define M_I3_R1(id, func, A0, A1, A2, R0) case id:{A0 a0;A1 a1;A2 a2;R0 r0;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0);SendResult(id,r0);}break;

		#define M_I3_R2(id, func, A0, A1, A2, R0, R1) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I3_R3(id, func, A0, A1, A2, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I3_R4(id, func, A0, A1, A2, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I3_R5(id, func, A0, A1, A2, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




		#define M_I4_R0(id, func, A0, A1, A2, A3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3);SendResult(id);}break;

		#define M_I4_R1(id, func, A0, A1, A2, A3, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0);SendResult(id,r0);}break;

		#define M_I4_R2(id, func, A0, A1, A2, A3, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I4_R3(id, func, A0, A1, A2, A3, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I4_R4(id, func, A0, A1, A2, A3, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I4_R5(id, func, A0, A1, A2, A3, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



		#define M_I5_R0(id, func, A0, A1, A2, A3, A4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4);SendResult(id);}break;

		#define M_I5_R1(id, func, A0, A1, A2, A3, A4, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0);SendResult(id,r0);}break;

		#define M_I5_R2(id, func, A0, A1, A2, A3, A4, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I5_R3(id, func, A0, A1, A2, A3, A4, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I5_R4(id, func, A0, A1, A2, A3, A4, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I5_R5(id, func, A0, A1, A2, A3, A4, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



		#define M_I6_R0(id, func, A0, A1, A2, A3, A4, A5) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5);SendResult(id);}break;

		#define M_I6_R1(id, func, A0, A1, A2, A3, A4, A5, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0);SendResult(id,r0);}break;

		#define M_I6_R2(id, func, A0, A1, A2, A3, A4, A5, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1);SendResult(id,r0,r1);}break;

		#define M_I6_R3(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

		#define M_I6_R4(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

		#define M_I6_R5(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



		#define M_I7_R0(id, func, A0, A1, A2, A3, A4, A5, A6) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6);SendResult(id);}break;

		#define M_I7_R1(id, func, A0, A1, A2, A3, A4, A5, A6, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0);SendResult(id,r0);}break;

		#define M_I7_R2(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1);SendResult(id,r0,r1);}break;
		
		#define M_I7_R3(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2);SendResult(id,r0,r1,r2);}break;
		
		#define M_I7_R4(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;
		
		#define M_I7_R5(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



		#define M_I8_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7);SendResult(id);}break;
		
		#define M_I8_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0);SendResult(id,r0);}break;
		
		#define M_I8_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1);SendResult(id,r0,r1);}break;
		
		#define M_I8_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2);SendResult(id,r0,r1,r2);}break;
		
		#define M_I8_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;
		
		#define M_I8_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



		#define M_I9_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8);SendResult(id);}break;
		
		#define M_I9_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0);SendResult(id,r0);}break;
		
		#define M_I9_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1);SendResult(id,r0,r1);}break;
		
		#define M_I9_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2);SendResult(id,r0,r1,r2);}break;
		
		#define M_I9_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;
		
		#define M_I9_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;


		#define M_I10_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9);SendResult(id);}break;
		
		#define M_I10_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0);SendResult(id,r0);}break;
		
		#define M_I10_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1);SendResult(id,r0,r1);}break;
		
		#define M_I10_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2);SendResult(id,r0,r1,r2);}break;
		
		#define M_I10_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;
		
		#define M_I10_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;
		

		#define END_SWITCH default:ATLTRACE("There is unknown request (%d) came!\r\nWe are going to close connection now .....\r\n", GetCurrentRequestID());PostClose(ERROR_UNKNOWN_REQUEST);break;} 

		class CClientPeer
		{
		private:
			class CUPushServerImpl : public IUPush
			{
			public:
				virtual bool Enter(unsigned long *pGroups, unsigned long ulCount);
				virtual bool Broadcast(const VARIANT& vtMessage, unsigned long *pGroups, unsigned long ulCount);
				virtual bool Broadcast(const unsigned char *pMessage, unsigned long ulMessageSize, unsigned long *pGroups, unsigned long ulCount);
				virtual bool SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId);
				virtual bool SendUserMessage(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulMessageSize);
				virtual bool Exit();
			private:
				friend CBaseService;
				unsigned int m_hSocket;
			};
		public:
			CClientPeer();
			virtual ~CClientPeer();
			
			//disable copy constructor and assignment operator
			CClientPeer& operator=(const CClientPeer &cp);
			CClientPeer(const CClientPeer &cp);

		public:
			//retrieve request and send result back
			unsigned long RetrieveBuffer(unsigned char *pBuffer, unsigned long ulBufferLen, bool bPeek = false);
			unsigned long SendReturnData(unsigned short usRequestID, const unsigned char *pBuffer = UNULL_PTR, unsigned long ulLen = 0);
			
			unsigned long SendResult(unsigned short usRequestID);
			unsigned long SendResult(unsigned short usRequestID, CUQueue &UQueue);
			unsigned long SendResult(unsigned short usRequestID, CScopeUQueue &UQueue);

			template<class ctype0>
			unsigned long SendResult(unsigned short usRequestID, const ctype0& data0)
			{
				CScopeUQueue su;
				if(TransferServerException())
					su<<(HRESULT)0; //required by client
				su<<data0;
				return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
			}

			template<class ctype0, class ctype1>
			unsigned long SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1)
			{
				CScopeUQueue su;
				if(TransferServerException())
					su<<(HRESULT)0; //required by client
				su<<data0<<data1;
				return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2>
			unsigned long SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype1& data2)
			{
				CScopeUQueue su;
				if(TransferServerException())
					su<<(HRESULT)0; //required by client
				su<<data0<<data1<<data2;
				return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3>
			unsigned long SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3)
			{
				CScopeUQueue su;
				if(TransferServerException())
					su<<(HRESULT)0; //required by client
				su<<data0<<data1<<data2<<data3;
				return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
			unsigned long SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4)
			{
				CScopeUQueue su;
				if(TransferServerException())
					su<<(HRESULT)0; //required by client
				su<<data0<<data1<<data2<<data3<<data4;
				return SendReturnData(usRequestID, su->GetBuffer(), su->GetSize());
			}

			//batching at server side
			bool StartBatching();
			bool CommitBatching();
			bool AbortBatching();
			bool IsBatching();
						
			unsigned long GetJoinedGroups(unsigned long *pGroupId, unsigned long ulGroupCount);
			
			//user id and password
			bool SetUID(const wchar_t *strUID);
			unsigned long GetUID(wchar_t *strUID, unsigned long ulCharLen);
			unsigned long GetPassword(wchar_t *strPassword, unsigned long ulCharLen);

			//Switch info is used for exchanging version information and others
			void SetServerInfo(CSwitchInfo ServerInfo);
			inline CSwitchInfo GetServerInfo();
			CSwitchInfo GetClientInfo();
			inline bool TransferServerException()
			{
				CSwitchInfo SF;
				g_SocketProLoader.GetServerInfo(m_hSocket, &SF);
				return ((SF.m_ulParam5 & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
			}

			//socket connection attributes
			void SetZip(bool bZip);
			bool GetZip();
			unsigned long GetSndBufferSize();
			unsigned long GetRequestsInQueue();
			unsigned long GetRcvBytesInQueue();
			unsigned long GetSndBytesInQueue();
			unsigned long GetRcvBufferSize();
			unsigned long GetLastSndTime();
			unsigned long GetLastRcvTime();
			bool IsSameEndian();
			unsigned long GetCurrentRequestLen();
			unsigned short GetCurrentRequestID();
			unsigned long GetConsumedMemory();
			unsigned long GetBytesSent(unsigned long *pulHigh);
			unsigned long GetBytesReceived(unsigned long *pulHigh);
			unsigned long GetBytesBatched();
			unsigned long GetMySpecificBytes(unsigned char* pBuffer, unsigned long ulLen);
			unsigned long GetCountOfMySpecificBytes();
			unsigned long GetAssociatedThreadID();
			bool SetBlowFish(unsigned char bKeyLen, unsigned char *strKey);
			void CleanTrack();
			void DropCurrentSlowRequest();
			void DropRequestResult(unsigned short usRequestId);
			bool IsClosing();
			tagEncryptionMethod GetEncryptionMethod();
			bool SetEncryptionMethod(tagEncryptionMethod EncryptionMethod);
			void SetZipLevel(tagZipLevel zl);
			tagZipLevel GetZipLevel();

			//others
			unsigned long GetRequestIDsInQueue(unsigned short *pusRequestID, unsigned long ulSize);
			bool GetInterfaceAttributes(unsigned long *pulMTU, unsigned long *pulMaxSpeed, unsigned long *pulType, unsigned long *pulMask);
			bool PostClose(unsigned short usError);
			bool GetSockAddr(unsigned int *pnSockPort, wchar_t *strIPAddrBuffer, unsigned short usChars);
			bool GetPeerName(unsigned int *pnPeerPort, wchar_t *strPeerAddr, unsigned short usChars);
			void ShrinkMemory();
			unsigned long GetSvsID();
			unsigned int  GetSocket();
			CBaseService *GetBaseService();
			CUQueue& GetUQueue();
			IUPush* GetPush();
			unsigned int GetMaxMessageSize();
			void SetMaxMessageSize(unsigned int nNewMax);

			static unsigned long GetSvsID(unsigned int hSocket);
			static bool IsCanceled();
			static unsigned int GetAssociatedSocket();
			static int GetLastSocketError();
			static unsigned long RetrieveBuffer(unsigned int hSocket, unsigned long ulLen, unsigned char* pBuffer, bool bPeek = false);
			static unsigned long SendReturnData(unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char* pBuffer);

		protected:
			virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen) = 0;
			virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen) = 0;
			virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo);
			virtual void OnSwitchFrom(unsigned long ulServiceID);
			virtual void OnReceive(int nError);
			virtual void OnSend(int nError);
			virtual void OnBaseRequestCame(unsigned short usRequestID);
			virtual void OnDispatchingSlowRequest(unsigned short usRequestID);
			virtual void OnSlowRequestProcessed(unsigned short usRequestID);
			virtual bool OnSendReturnData(unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer);
			virtual void OnChatRequestComing(tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2)
			{

			}

			virtual void OnChatRequestCame(tagChatRequestID ChatRequestId)
			{

			}
			
		public:
			//m_bAutoBuffer is true by default
			//if m_bAutoBuffer is true, all of fast and slow requests are automatically buffered into m_UQueue
			bool m_bAutoBuffer;
			
		private:
			CScopeUQueue	m_sq;
			CUPushServerImpl m_Push;
			unsigned int	m_hSocket;
			unsigned long	m_ulTickCountReleased;
			unsigned short	m_usCurrentRequestId;
			friend CBaseService;
			friend CSocketProServer;

		protected:
			CUQueue	&m_UQueue;
		};
	
		class CBaseService
		{
		public:
			CBaseService();
			virtual ~CBaseService();
			
		public:
			unsigned long	GetSvsID();
			void			SetSvsID(unsigned long ulSvsID);
			unsigned long	GetEvents();
			virtual bool	AddMe(unsigned long ulSvsID, unsigned long ulEvents = 0, enumThreadApartment taWhatTA = taNone)
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
			}

			bool			AddSlowRequest(unsigned short usRequestID);
			void			RemoveSlowRequest(unsigned short usRequestID);
			void			RemoveAllSlowRequests();
			unsigned short	GetCountOfSlowRequests();
			
			//return a slow request ID
			unsigned short	GetSlowRequest(unsigned short usIndex); 
			
			void			RemoveMe();
			CClientPeer*	SeekClientPeer(unsigned int hSocket);
		
			static CClientPeer* SeekClientPeerGlobally(unsigned int hSocket);
			static CBaseService *GetBaseService(unsigned int hSocket);
			static CBaseService *GetBaseService(unsigned long ulSvsID);
			static unsigned long GetServiceID(unsigned long ulIndex);
			static unsigned long GetCountOfServices();
			static unsigned long GetCountOfAllServices();
			static unsigned long GetCountOfLibraries();
			static HINSTANCE GetALibrary(unsigned long ulIndex);
			static HINSTANCE AddALibrary(const wchar_t * strLibFile, int nParam = 0); //return an instance to the added dll
			static bool RemoveALibrary(HINSTANCE hLib);
			static bool RemoveALibrary(const wchar_t *strLibFile);
			
		protected:
			bool			GetReturnRandom();
			void			SetReturnRandom(bool bRandom);
			virtual CClientPeer* GetPeerSocket(unsigned int hSocket) = 0;
			virtual bool OnIsPermitted(unsigned int hSocket);
			virtual void OnThreadCreated(unsigned long ulThreadID, unsigned int hSocket);
			static void CALLBACK OnThreadCreated(unsigned long ulSvsID, unsigned long ulThreadID, unsigned int hSocket);

		private:
			virtual void OnBaseRequestCame(unsigned int hSocket, unsigned short usRequestID);
			virtual void OnRelease(unsigned int hSocket, bool bClose, unsigned long ulInfo);
			virtual void OnFastRequestArrive(unsigned int hSocket, unsigned short usRequestID);
			virtual void OnSlowRequestArrive(unsigned int hSocket, unsigned short usRequestID);
			void RemoveClientPeer(CClientPeer *pClientPeer);
			void AddAClientPeer(CClientPeer *pClientPeer);
			CClientPeer *GetClientPeerFromPool();
			static void CALLBACK OnFast(unsigned int hSocket, unsigned short usRequestID, unsigned long ulLen);
			static bool CALLBACK OnPermitted(unsigned int hSocket, unsigned long ulSvsID);
			static void CALLBACK OnSwitch(unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID);
			static HRESULT CALLBACK OnSlow(unsigned short usRequestID, unsigned long ulLen, unsigned int hSocket);
			static void CALLBACK OnClose(unsigned int hSocket, int nError);
			static void CALLBACK OnBaseCame(unsigned int hSocket, unsigned short usRequestID);
			static void CALLBACK OnReceive(unsigned int hSocket, int nError);
			static void CALLBACK OnSend(unsigned int hSocket, int nError);
			static void CALLBACK OnSlowRequestProcessed(unsigned int hSocket, unsigned short usRequestID);
			static bool CALLBACK OnSendReturnData(unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer);
			static void CALLBACK OnCleanPool(unsigned long ulSvsID, unsigned long ulTickCount);
			
		public:
			//usually you don't need the three callbacks
			PPRETRANSLATEMESSAGE	m_PreTranslateMessage;	//Optional
			PTHREAD_STARTED			m_ThreadStarted;		//Optional
			PTHREAD_DYING			m_ThreadDying;			//Optional
			
			CSvsContext				m_SvsContext;

		protected:
			//if m_bUsePool is true, all of CClientPeer derived objects are put into m_Pool, 
			//and then deleted after a period of time (60 seconds) when a socket connection is closed
			//When the pool is enabled, it will be helpful for performance when socket connections are repeatedly closed.
			//However, you may have to initialize a CClientPeer derived object by overriding CClientPeer::OnSwitchFrom
			//because a CClientPeer derived object may be in a different state.

			//if m_bUsePool is false, all of CClientPeer derived objects are deleted 
			//immediately when a socket connection is closed or swicthed
			bool					m_bUsePool;

		private:
			unsigned long					m_ulEvents;
			unsigned long					m_ulSvsID;
			CSimpleValArray<CClientPeer*>	m_aClientPeer;
			CSimpleValArray<CClientPeer*>	m_Pool;
		};

		class CSocketProServer
		{
		public:
			CSocketProServer(int nParam = 0);
			virtual ~CSocketProServer();

		public:
			unsigned long GetEvents()
			{
				CAutoLock al(&m_cs.m_sec);
				return m_ulEvents;
			}
			void AskForEvents(unsigned long ulEvents);
			bool Run(unsigned int nPort, unsigned long ulEvents = 0, UINT unMaxBacklog = 64);
			
			static void StopSocketProServer();
			static unsigned long GetUserID(unsigned int hSocket, wchar_t *strUID, unsigned long ulCharLen);
			static unsigned long GetPassword(unsigned int hSocket, wchar_t *strPassword, unsigned long ulCharLen);
			static bool SetPassword(unsigned int hSocket, const wchar_t *strPassword);

			struct Config
			{
				static void SetMaxThreadIdleTimeBeforeSuicide(unsigned long ulMaxThreadIdleTimeBeforeSuicide);
				static void SetMaxConnectionsPerClient(unsigned long ulMaxConnectionsPerClient);
				static unsigned long GetMaxThreadIdleTimeBeforeSuicide();
				static unsigned long GetMaxConnectionsPerClient();
				static void SetTimerElapse(unsigned long ulTimerElapse);
				static unsigned long WINAPI GetTimerElapse();
				static void SetSMInterval(unsigned long ulSMInterval);
				static unsigned long GetSMInterval();
				static unsigned long GetPingInterval();
				static void SetPingInterval(unsigned long ulPingInterval);
				static void SetCleanPoolInterval(unsigned long ulCleanPoolInterval);
				static unsigned long GetCleanPoolInterval();
				static void SetDefaultZip(bool bZip);
				static bool GetDefaultZip();
				static void SetDefaultEncryptionMethod(tagEncryptionMethod EncryptionMethod);
				static tagEncryptionMethod GetDefaultEncryptionMethod();
				static void SetSwitchTime(unsigned long ulSwitchTime);
				static unsigned long GetSwitchTime();
				static tagAuthenticationMethod GetAuthenticationMethod();
				static void SetAuthenticationMethod(tagAuthenticationMethod am);
				static void SetSharedAM(bool bShared);
				static bool GetSharedAM();
				static bool SetUseWindowMessagePump(bool bUseWindowMessagePump);
				static bool GetUseWindowMessagePump();
				static bool GetHTTPServerPush();
				static void SetHTTPServerPush(bool bPush);
			};

			static void UseSSL(tagEncryptionMethod EncryptionMethod, const wchar_t *strCertFile, const wchar_t *strPrivateKeyFile);
			static void UseSSL(tagEncryptionMethod EncryptionMethod, const wchar_t *strSubject, bool bMachine = false, bool bRoot = true);
			static void UseSSL(const wchar_t *strPfxFile, const wchar_t *strPassword, const wchar_t *strSubject, tagEncryptionMethod EncryptionMethod = MSTLSv1);
			static unsigned int WINAPI GetCountOfClients();
			static unsigned int FindClient(unsigned int hSocket);
			static unsigned int GetClient(unsigned int unIndex);
			static HWND GetWin();
			static unsigned long GetMainThreadID();
			static int GetLastSocketError();
			static unsigned int GetListeningSocketHandle();
			static bool GetLocalName(wchar_t *strLocalName, unsigned short usChars);
			static bool PostQuit();
			
			//non-HTTP chat
			struct PushManager
			{
				static unsigned long GetAChatGroupDiscription(unsigned long ulGroupID, wchar_t *strDescription, unsigned long ulChars); //return the number of chars
				static unsigned long GetCountOfChatGroups();
				static unsigned long GetGroupID(unsigned long ulIndex);
				static unsigned long GetCountOfChatters(unsigned long ulGroupId);
				static unsigned int GetChatterSocket(unsigned long ulGroupId, unsigned long ulIndex); //return a socket handle
				static bool AddAChatGroup(unsigned long ulGroupID, const wchar_t *strDescription);
			};

			//HTTP Chat
			struct HttpPush
			{
				static void GetHTTPChatIds(unsigned long ulGroupId, CSimpleArray<CComBSTR> &aChatIds);
				static bool GetHTTPChatContext(const wchar_t *strChatId, wchar_t **pstrUserID, wchar_t **pstrIpAddr, unsigned long *pLeaseTime, VARIANT *pvtGroups, unsigned long *pTimeout, unsigned long *pCountOfMessages);
			};

		protected:
			virtual bool OnSettingServer() = 0;

			//track all of window messages by overriding this function
			virtual bool OnWinMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam, DWORD dwTime, LONG nPointX, LONG nPointY)
			{
				return false;	
			}

			virtual void OnWinMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
			{
			}

			virtual void OnAccept(unsigned int hSocket, int nError)
			{
			}

			virtual void OnClose(unsigned int hSocket, int nError)
			{
			}
			virtual void OnSend(unsigned int hSocket, int nError)
			{
			}
			virtual void OnSwitchTo(unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID)
			{
			}
			virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
			{
				return true;
			}
			virtual void OnSSLEvent(unsigned int hSocket, int nWhere, int nRtn)
			{
			}

			virtual void OnChatRequestComing(unsigned int hSocketSource, tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2);
			virtual void OnChatRequestCame(unsigned int hSocketSource, tagChatRequestID ChatRequestId);
			
		private:
			static void CALLBACK OnMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
			static void CALLBACK OnAccepted(unsigned int hSocket, int nError);
			static void CALLBACK OnDown(unsigned int hSocket, int nError);
			static void CALLBACK OnSnd(unsigned int hSocket, int nError);
			static void CALLBACK OnSTo(unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID);
			static bool CALLBACK OnPermitted(unsigned int hSocket, unsigned long ulSvsID);
			static void CALLBACK OnChatComing(unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen);
			static void CALLBACK OnChatCame(unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen);
			static void CALLBACK OnSSL(unsigned int hSocket, int nWhere, int nRtn);
			static bool InitSocketProServer(int nParam = 0);
			static void UninitSocketProServer();
			static DWORD WINAPI ThreadProc(LPVOID lpParameter);
			void StartMessagePump();

		private:
			unsigned long	m_ulEvents;
			HANDLE			m_hThread;
			HANDLE			m_hEvent;
			UINT			m_nPort;
			UINT			m_unMaxBacklog;
			static CComAutoCriticalSection	m_cs;
		};
		
		struct IUHttpPush
		{
			virtual const wchar_t* Enter(unsigned long *pGroups, unsigned long ulGroupCount, const wchar_t* strUserID, unsigned long dwLeaseTime, const wchar_t *strIpAddr = UNULL_PTR) = 0;
			virtual bool HTTPSubscribe(const wchar_t* strChatId, unsigned long ulTimeout, const wchar_t *strCrossSiteJSCallback) = 0;
			virtual bool Exit(const wchar_t* strChatId) = 0;
			virtual bool SendUserMessage(const wchar_t* strChatId, const wchar_t* strUserID, const VARIANT *pvtMsg) = 0;
			virtual bool Speak(const wchar_t* strChatId, const VARIANT *pvtMsg, unsigned long *pGroups, unsigned long ulGroupCount) = 0;
			virtual const VARIANT* GetHTTPChatGroupIds(const wchar_t* strChatId) = 0;
		};
		
		class CHttpPeerBase : public CClientPeer
		{
		private:
			struct CHttpPushImpl : public IUHttpPush
			{
				virtual const wchar_t* Enter(unsigned long *pGroups, unsigned long ulGroupCount, const wchar_t* strUserID, unsigned long dwLeaseTime, const wchar_t *strIpAddr = UNULL_PTR);
				virtual bool HTTPSubscribe(const wchar_t* strChatId, unsigned long ulTimeout, const wchar_t *strCrossSiteJSCallback);
				virtual bool Exit(const wchar_t* strChatId);
				virtual bool SendUserMessage(const wchar_t* strChatId, const wchar_t* strUserID, const VARIANT *pvtMsg);
				virtual bool Speak(const wchar_t* strChatId, const VARIANT *pvtMsg, unsigned long *pGroups, unsigned long ulGroupCount);
				virtual const VARIANT* GetHTTPChatGroupIds(const wchar_t* strChatId);
				CClientPeer *m_pClientPeer;
			};
			CHttpPushImpl	m_HttpPush;

		public:
			CHttpPeerBase() : m_HttpRequest(hrUnknown), m_dVersion(0.0)
			{
				m_HttpPush.m_pClientPeer = this;
			}

			IUHttpPush* GetHttpPush()
			{
				return &m_HttpPush;
			}
			const char* GetPathName(unsigned long &nLen);
			const char* GetHeaderValue(const char *strHeaderName, unsigned long &nLen);
			bool SetResponseHeader(const char *strUTF8Header, const char *strUTF8Value);
			void SetResponseCode(unsigned int nHttpErrorCode);
			enumHTTPRequest GetRequestMethod();
			double GetHTTPClientVersion();
			const char* GetHeaders();
			const char* GetQuery();
			unsigned long GetHeaderCount();
			const char* GetParams();
			unsigned long GetParamCount();
			const char* GetParamValue(const char *strParam, unsigned long &nLen);
			void SetAutoPartition(bool bPartition);
			bool GetAutoPartition();
			
		protected:
			virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen);
			virtual void OnSwitchFrom(unsigned long ulServiceID);

		private:
			static unsigned long UriDecode(const unsigned char *strIn, unsigned long nLenIn, unsigned char *strOut);
			
		private:
			CUQueue	m_qQueue;	//store HTTP Query
			CUQueue	m_qHeader;	//store HTTP Headers
			enumHTTPRequest	m_HttpRequest;
			double	m_dVersion;
		};
		
		template<typename TClientPeer>
		class CSocketProService : public CBaseService
		{
		public:
			CSocketProService()
			{
				m_bUsePool = true;
			}
			
			//disable copy constructor and assignment operator
			CSocketProService(const CSocketProService &Service);
			CSocketProService& operator=(const CSocketProService &Service);

		protected:
			virtual CClientPeer* GetPeerSocket(unsigned int hSocket)
			{
				return new TClientPeer;
			}
		};

		class CDummyPeer : public CClientPeer
		{
		protected:
			virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
			{

			}

			virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
			{
				return S_OK;
			}
		};

#if _MSC_VER >= 1300
	namespace PLG
	{
		template<typename TClientPeer>
		class CPLGPeer: public TClientPeer
		{
		public:
			CPLGPeer() : m_pIJobManager(UNULL_PTR)
			{
			}

			//disable copy constructor and assignment operator
			CPLGPeer(const CPLGPeer &PLGPeer);
			CPLGPeer& operator=(const CPLGPeer &PLGPeer);

		public:
			IJobManager* GetJobManager()
			{
				return m_pIJobManager;
			}

			virtual void OnWaitable(IJobContext *pJobContext, int nTaskId, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("You can call IJobContext::Wait from current worker thread with job id = %d, request id = %d, task count = %d, task id = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks(), nTaskId);
#endif
			}

			virtual void OnJobJustCreated(IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Job just created with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnEnqueuingJob(IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Enqueuing job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnJobEnqueued(IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Job equeued with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnAddingTask(IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Adding a task with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnTaskJustAdded(IJobContext *pJobContext, int nTaskId, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Task added with job id = %d, request id = %d, task count = %d, task id = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks(), nTaskId);
#endif
			}

			virtual bool OnSendingPeerData(IJobContext *pJobContext, unsigned short usRequestId, CUQueue &UQueue)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Going to send data (%d bytes) with job id = %d, request id = %d, task count = %d\n", UQueue.GetSize(), (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
				return true;
			}

			virtual void OnPeerDataSent(IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Data sent with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

		private:
			IJobManager	*m_pIJobManager;

			template<unsigned long, typename TClientPeer, typename TCS>
			friend class CPLGService;
		};
	/*
		class CPLGHttpPeer : public CPLGPeer<CHttpPeerBase>
		{
		public:
			virtual bool OnSendingPeerData(IJobContext *pJobContext, unsigned short usRequestId, CUQueue &UQueue)
			{
				switch(usRequestId)
				{
				case idGet:
				case idPost:
				case idHead:
				case idPut:
				case idDelete:
				case idOptions:
				case idTrace:
				case idConnect:
				case idMultiPart:
					return true;
					break;
				case idHeader:
				default:
					return false;
					break;
				}
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Going to send data (%d bytes) with job id = %d, request id = %d, task count = %d\n", UQueue.GetSize(), (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
				return true;
			}

		protected:
			virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
			{
				return S_OK;
			}
		};
*/
		typedef CPLGPeer<CDummyPeer>	DefaultPLGPeer;
		
		template<unsigned long ulServiceIdOnRealServer, typename TClientPeer = DefaultPLGPeer, typename TCS = ClientSide::CClientSocket>
		class CPLGService : public CBaseService
		{
		public:
			CPLGService()
			{
				m_bUsePool =  true;
				m_SP.m_pPLGSvs = this;
			}

			//disable copy constructor and assignment operator
			CPLGService(const CPLGService &PLGService);
			CPLGService& operator=(const CPLGService &PLGService);
			
		private:
			class CPLGHandler : public ClientSide::CAsyncServiceHandler
			{
			public:
				CPLGHandler() 
					: ClientSide::CAsyncServiceHandler(ulServiceIdOnRealServer), m_pJobContext(UNULL_PTR)
				{
					
				}
				
				//disable copy constructor and assignment operator
				CPLGHandler(const CPLGHandler &PLGHandler);
				CPLGHandler& operator=(const CPLGHandler &PLGHandler);

			public:
				IJobContext* GetAssocatedJobContext()
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					return m_pJobContext;
				}

				void AssociateJobContext(IJobContext *pJobContext)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					m_pJobContext = pJobContext;
				}
				
			protected:
				virtual void OnExceptionFromServer(CSocketProServerException &Exception)
				{
					CScopeUQueue su;
					su << Exception;
					CUQueue &UQueue = (*su);
					OnResultReturned(Exception.m_usRequestID, UQueue);
				}

				virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
				{
					do
					{
						bool bClient = false;
						bool bDrop = false;
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_pJobContext == UNULL_PTR || m_pJobContext->GetJobStatus() != jsRunning)
							break;
						bool bRandomReturn = GetAttachedClientSocket()->GetReturnRandom();
						CClientPeer *p = (CClientPeer *)m_pJobContext->GetIdentity();
						TClientPeer *pClientPeer = UNULL_PTR;
						try
						{
							pClientPeer = dynamic_cast<TClientPeer *>(p);
						}
						catch(...)
						{
							pClientPeer = UNULL_PTR;
						}
						if(pClientPeer == UNULL_PTR)
							break;
						unsigned int hSocket = pClientPeer->GetSocket();
						if(hSocket == 0 || hSocket == (~0))
							break;
						unsigned long ulServiceId = pClientPeer->GetSvsID();
						if(ulServiceId == 0 || ulServiceId == (~0))
							break;
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							if(pClientPeer->OnSendingPeerData(m_pJobContext, usRequestID, UQueue))
							{
								CBaseService *pBase = CBaseService::GetBaseService(ulServiceId);
								if(pBase == UNULL_PTR)
									break;
								pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
								if(pClientPeer == UNULL_PTR)
									break;
								pClientPeer->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
								
								pBase = CBaseService::GetBaseService(ulServiceId);
								if(pBase == UNULL_PTR)
									break;
								pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
								if(pClientPeer == UNULL_PTR)
									break;
								pClientPeer->OnPeerDataSent(m_pJobContext, usRequestID);
							}
							else
							{
								bDrop = true;
							}
						}
						if(m_pJobContext == UNULL_PTR)
							break;
						PLGSocketPool::CJobContext *pJobContext = (PLGSocketPool::CJobContext*)m_pJobContext;
						pJobContext->RemoveTask(bRandomReturn, usRequestID, bClient);
						if(bClient && bDrop)
						{
							CBaseService *pBase = CBaseService::GetBaseService(ulServiceId);
							if(pBase == UNULL_PTR)
								break;
							pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
							if(pClientPeer == UNULL_PTR)
								break;
							pClientPeer->DropRequestResult(usRequestID);
						}
					}while(false);
					UQueue.SetSize(0);
				}

			private:
				IJobContext		*m_pJobContext;
			};

		
		public:
			typedef ClientSide::CSocketPoolEx<CPLGHandler, TCS> PLGSocketPool;
			
			PLGSocketPool& GetSocketPool()
			{
				return m_SP;
			}

			virtual bool AddMe(unsigned long ulSvsID, unsigned long ulEvents = 0, enumThreadApartment taWhatTA = taNone)
			{
				bool ok = CBaseService::AddMe(ulSvsID, ulEvents, taWhatTA);
				if(!ok)
					return false;
				if(ulSvsID == sidHTTP)
				{
					AddSlowRequest(idGet);
					AddSlowRequest(idPost);
					AddSlowRequest(idMultiPart);
				}
				else
					SetReturnRandom(true);
				//Don't set m_bUserPool to false!!!
				ATLASSERT(m_bUsePool); 
				return ok;
			}

		protected:
			virtual bool OnFailover(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("A fail with job id = %d, task count = %d, fails = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks(), m_SP.GetFails());	
#endif
				return true;
			}

			virtual void OnJobDone(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Job id = %d done, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
			}

			virtual bool OnExecutingJob(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Going to process job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
				return true;
			}

			virtual void OnJobProcessing(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Processing job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif				
			}

			virtual void OnReturnedResultProcessed(ClientSide::CAsyncServiceHandler *pHandler, IJobContext *pJobContext, unsigned short usRequestId)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Returned result processed with job id = %d, request id = %d, task count = %d\n", (int)pJobContext->GetJobId(), usRequestId, pJobContext->GetCountOfTasks());
#endif
			}

			virtual void OnAllSocketsDisconnected()
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				long lDead = 0;
				m_SP.GetUSocketPool()->get_ConnectedSocketsEx(&lDead);
				ATLTRACE("!!! All %d socket(s) disconnected !!!\n", lDead);
#endif	
			}
		
			virtual CClientPeer* GetPeerSocket(unsigned int hSocket)
			{
				TClientPeer *p = new TClientPeer;
				p->m_pIJobManager = GetSocketPool().GetJobManager();
				return p;
			}

		private:
			int AddTask(TClientPeer *pClientPeer, unsigned short usRequestID, IJobContext **ppIJobContext)
			{
				PLGSocketPool::CJobContext *pIJobContext;
				int nIndex = m_mapIdentityJob.FindKey(pClientPeer);
				if(nIndex == -1)
				{
					pIJobContext = (PLGSocketPool::CJobContext*)m_SP.GetJobManager()->CreateJob(pClientPeer);
					ATLASSERT(pIJobContext != UNULL_PTR);
					{
						CAutoReverseLock arl(&g_cs.m_sec);
						pClientPeer->OnJobJustCreated(pIJobContext, usRequestID);
					}
				}
				else
				{
					pIJobContext = (PLGSocketPool::CJobContext*)m_mapIdentityJob.GetValueAt(nIndex);
					ATLASSERT(pIJobContext != UNULL_PTR);
				}
				int idTask;
				CUQueue &UQueue = pClientPeer->GetUQueue();
				{
					CAutoReverseLock arl(&g_cs.m_sec);
					pClientPeer->OnAddingTask(pIJobContext, usRequestID);
					idTask = pIJobContext->AddTask(usRequestID, UQueue.GetBuffer(), UQueue.GetSize(), true);
					ATLASSERT(idTask != 0);
					pClientPeer->OnTaskJustAdded(pIJobContext, idTask, usRequestID);
					if(nIndex == -1)
					{
						pClientPeer->OnEnqueuingJob(pIJobContext, usRequestID);
						bool ok = m_SP.GetJobManager()->EnqueueJob(pIJobContext);
						ATLASSERT(ok);
						pClientPeer->OnJobEnqueued(pIJobContext, usRequestID);
						if(ok)
							m_SP.Process();
						if(ppIJobContext != UNULL_PTR)
							*ppIJobContext = pIJobContext;
					}
					else
					{
						if(ppIJobContext != UNULL_PTR)
							*ppIJobContext = UNULL_PTR;
					}
				}
				UQueue.SetSize(0);
				return idTask;
			}

			void EndJob(TClientPeer *pClientPeer, unsigned short usRequestID)
			{
				int nIndex = m_mapIdentityJob.FindKey(pClientPeer);
				if(nIndex != -1)
				{
					PLGSocketPool::CJobContext *pIJobContext = (PLGSocketPool::CJobContext*)m_mapIdentityJob.GetValueAt(nIndex);
					m_mapIdentityJob.Remove(pClientPeer);
					pClientPeer->OnEnqueuingJob(pIJobContext, usRequestID);
					bool ok = m_SP.GetJobManager()->EnqueueJob(pIJobContext);
					ATLASSERT(ok);
					pClientPeer->OnJobEnqueued(pIJobContext, usRequestID);
					ATLASSERT(ok);
					if(ok)
						m_SP.Process();
					{
						CAutoReverseLock arl(&g_cs.m_sec);
#if defined(_DEBUG) && defined(ENABLE_SOCKETPRO_LB_TRACING)
						if(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId())
							ATLTRACE("OnWaitable called with main thread.\n");
						else
							ATLTRACE("OnWaitable called within SocketPro worker thread.\n");
#endif
						pClientPeer->OnWaitable(pIJobContext, 0, idEndJob);
					}
				}
			}

			void StartJob(TClientPeer *pClientPeer, unsigned short usRequestID)
			{
				PLGSocketPool::CJobContext *pIJobContext = (PLGSocketPool::CJobContext*)m_SP.GetJobManager()->CreateJob(pClientPeer);
				ATLASSERT(pIJobContext != UNULL_PTR);
				if(pIJobContext != UNULL_PTR)
				{
					m_mapIdentityJob.Add(pClientPeer, pIJobContext);

					{
						CAutoReverseLock arl(&g_cs.m_sec);
						pClientPeer->OnJobJustCreated(pIJobContext, usRequestID);
					}
				}
			}

			void OnMyProcess(unsigned int hSocket, unsigned short usRequestID)
			{
				int nTaskId = 0;
				bool	bHeader = false;
				TClientPeer *pClientPeer = UNULL_PTR;
				IJobContext *ppIJobContext = UNULL_PTR;
				{
					//Don't set m_bUserPool to false!!!
					ATLASSERT(m_bUsePool); 
					CAutoLock	AutoLock(&g_cs.m_sec);
					pClientPeer = (TClientPeer *)SeekClientPeer(hSocket);
					ATLASSERT(pClientPeer != UNULL_PTR);
					switch(usRequestID)
					{
					case idStartJob:
						StartJob(pClientPeer, usRequestID);
						break;
					case idEndJob:
						EndJob(pClientPeer, usRequestID);
						break;
					case idHeader: //HTTP
						if(GetSvsID() == sidHTTP)
						{
							StartJob(pClientPeer, idStartJob);
							bHeader = true;
						}
					default:
						nTaskId = AddTask(pClientPeer, usRequestID, &ppIJobContext);
						if(GetSvsID() == sidHTTP)
						{
							switch(usRequestID)
							{
							case idGet:
							case idPost:
							case idHead:
							case idPut:
							case idDelete:
							case idOptions:
							case idTrace:
							case idConnect:
							case idMultiPart:
								bHeader = true;
								EndJob(pClientPeer, idEndJob);
								break;
							default:
								break;
							}
						}
						if(!bHeader && pClientPeer != UNULL_PTR && ppIJobContext != UNULL_PTR)
						{
							ATLASSERT(nTaskId != 0);
							PLGSocketPool::CJobContext *job = (PLGSocketPool::CJobContext *)ppIJobContext;
							{
								CAutoReverseLock arl(&g_cs.m_sec);
#if defined(_DEBUG) && defined(ENABLE_SOCKETPRO_LB_TRACING)
								if(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId())
									ATLTRACE("OnWaitable called with main thread.\n");
								else
									ATLTRACE("OnWaitable called within SocketPro worker thread.\n");
#endif
								pClientPeer->OnWaitable(ppIJobContext, nTaskId, usRequestID);
							}
						}
						break;
					}
				}
			}

			virtual void OnFastRequestArrive(unsigned int hSocket, unsigned short usRequestID)
			{
				OnMyProcess(hSocket, usRequestID);
			}

			virtual void OnSlowRequestArrive(unsigned int hSocket, unsigned short usRequestID)
			{
				OnMyProcess(hSocket, usRequestID);
			}

			virtual void OnRelease(unsigned int hSocket, bool bClose, unsigned long ulInfo)
			{
				CAutoLock	AutoLock(&g_cs.m_sec);

				//Don't set m_bUserPool to false!!!
				ATLASSERT(m_bUsePool); 

				TClientPeer *pClientPeer = (TClientPeer *)SeekClientPeer(hSocket);

				bool b = m_SP.m_bServerLoadingBalance;
				m_SP.m_bServerLoadingBalance = false;
				m_SP.GetJobManager()->CancelJobs(pClientPeer);
				m_mapIdentityJob.Remove(pClientPeer);
				m_SP.m_bServerLoadingBalance = b;
			}

			virtual void OnBaseRequestCame(unsigned int hSocket, unsigned short usRequestID)
			{
				CAutoLock	AutoLock(&g_cs.m_sec);

				//Don't set m_bUserPool to false!!!
				ATLASSERT(m_bUsePool); 

				TClientPeer *pClientPeer = (TClientPeer *)SeekClientPeer(hSocket);
				ATLASSERT(pClientPeer != UNULL_PTR);
#ifdef _DEBUG
				//Automatical buffering must be enabled!
				ATLASSERT(pClientPeer->m_bAutoBuffer);
#endif
				switch(usRequestID)
				{
				case idStartBatching:
					{
						int nIndex = m_mapIdentityJob.FindKey(pClientPeer);
						if(nIndex != -1)
						{
							PLGSocketPool::CJobContext *pIJobContext = (PLGSocketPool::CJobContext*)m_mapIdentityJob.GetValueAt(nIndex);
							ATLASSERT(pIJobContext != UNULL_PTR);
							int idTask = pIJobContext->AddTask(usRequestID, UNULL_PTR, 0, true);
							ATLASSERT(idTask != 0);
						}
					}
					break;
				case idCommitBatching:
					{
						int nIndex = m_mapIdentityJob.FindKey(pClientPeer);
						if(nIndex != -1)
						{
							VARIANT_BOOL b = VARIANT_TRUE;
							PLGSocketPool::CJobContext *pIJobContext = (PLGSocketPool::CJobContext*)m_mapIdentityJob.GetValueAt(nIndex);
							ATLASSERT(pIJobContext != UNULL_PTR);
							int idTask = pIJobContext->AddTask(usRequestID, (unsigned char*)&b, sizeof(b), true);
							ATLASSERT(idTask != 0);
						}
					}
					break;
				case idCancel:
					{
						bool b = m_SP.m_bServerLoadingBalance;
						m_SP.m_bServerLoadingBalance = false;
						m_SP.GetJobManager()->CancelJobs(pClientPeer);
						m_mapIdentityJob.Remove(pClientPeer);
						m_SP.m_bServerLoadingBalance = b;
					}
					break;
				default:
					break;
				}
			}
			
		private:
			class CSPOnServer : public PLGSocketPool
			{
			public:
				CSPOnServer()
				{
					m_bServerLoadingBalance = true;
				}
				
				//disable copy constructor and asssignment operator
				CSPOnServer(const CSPOnServer &SPOnServer);
				CSPOnServer& operator=(const CSPOnServer &SPOnServer);;

			protected:
				virtual bool OnFailover(CPLGHandler *pHandler, IJobContext *pJobContext)
				{
					return m_pPLGSvs->OnFailover(pHandler, pJobContext);
				}

				virtual bool OnExecutingJob(CPLGHandler *pHandler, IJobContext *pJobContext)
				{
					return m_pPLGSvs->OnExecutingJob(pHandler, pJobContext);
				}

				virtual void OnReturnedResultProcessed(CPLGHandler *pHandler, IJobContext *pJobContext, unsigned short usRequestId)
				{
					m_pPLGSvs->OnReturnedResultProcessed(pHandler, pJobContext, usRequestId);
				}

				virtual void OnAllSocketsDisconnected()
				{
					m_pPLGSvs->OnAllSocketsDisconnected();
				}

				virtual void OnJobDone(CPLGHandler *pHandler, IJobContext *pJobContext)
				{
					m_pPLGSvs->OnJobDone(pHandler, pJobContext);
					pHandler->AssociateJobContext(UNULL_PTR);
				}

				virtual void OnJobProcessing(CPLGHandler *pHandler, IJobContext *pJobContext)
				{
					pHandler->AssociateJobContext(pJobContext);
					m_pPLGSvs->OnJobProcessing(pHandler, pJobContext);
				}

				virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
				{
					HRESULT hr = PLGSocketPool::OnOtherMessage(hSocket, nMsg, wParam, lParam);
					switch(nMsg)
					{
					case msgRequestRemoved:
						{
							int n;
							CPLGHandler *pHandler = UNULL_PTR;
							CAutoLock	AutoLock(&g_cs.m_sec);
							int nSize = m_mapSocketHandler.GetSize();
							for(n=0; n<nSize; n++)
							{
								TCS *cs = m_mapSocketHandler.GetKeyAt(n);
								if(cs->GetSocket() == hSocket)
								{
									pHandler = m_mapSocketHandler.GetValueAt(n);
									IJobContext *pIJobContext = pHandler->GetAssocatedJobContext();
									if(pIJobContext == UNULL_PTR)
										break;
									void *pIdentity = pIJobContext->GetIdentity();
									if(pIdentity == UNULL_PTR)
										break;
									CClientPeer *pClientPeer = (CClientPeer*)pIdentity;
									if(pClientPeer == UNULL_PTR)
										break;
									TClientPeer *p = UNULL_PTR;
									try
									{
										p = dynamic_cast<TClientPeer *>(pClientPeer);
									}
									catch(...)
									{
										p = UNULL_PTR;
									}
									if(p == UNULL_PTR)
										break;
									p->DropRequestResult((unsigned short)wParam);
									break;
								}
							}
						}
						break;
					default:
						break;
					}
					return hr;
				}

			private:
				CPLGService<ulServiceIdOnRealServer, TClientPeer, TCS> *m_pPLGSvs;
				friend class CPLGService;
			};

		private:
			CSPOnServer	m_SP;
			CSimpleMap<TClientPeer*, IJobContext*> m_mapIdentityJob;
		};
	}; //PLG
#endif
		typedef CDummyPeer CNotifier;
		typedef CSocketProService<CNotifier> CNotificationService;

		extern CSimpleValArray<CBaseService*> g_aService;
		extern CSocketProServer	*g_pSocketProServer;

	}; //ServerSide
#endif
}; //SocketProAdapter

#endif

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