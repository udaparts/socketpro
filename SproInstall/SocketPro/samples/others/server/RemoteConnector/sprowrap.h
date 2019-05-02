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
		CClientSocketException(HRESULT hr, LPCWSTR strMessage) : m_strMessage(strMessage), m_hr(hr)
		{
				
		}

		CClientSocketException(const CClientSocketException &err)
		{
			m_strMessage = err.m_strMessage;
			m_hr = err.m_hr;
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
			: CClientSocketException(err)
		{
			m_ulSvsID = err.m_ulSvsID;
			m_usRequestID = err.m_usRequestID;
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

		inline CUQueue* operator&() const
		{
			return m_pUQueue;
		}

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

	private:
		static CUQueue* Lock(bool bLarge);
		static void Unlock(CUQueue *pUQueue, bool bLarge);
		
	private:
		bool							m_bLarge;
		CUQueue							*m_pUQueue;
		static CSimpleArray<CUQueue*>	m_aUQueue;
		static CSimpleArray<CUQueue*>	m_aLargeUQueue;
		static CComAutoCriticalSection	m_cs;
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

		#define	IDC_SRCUSOCKETEVENT		2
		static _ATL_FUNC_INFO OnSockEventFuncInfo = {CC_STDCALL, VT_I4, 2, {VT_I4, VT_I4}};
		static _ATL_FUNC_INFO OnSockEventWinMsgInfo = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_I4, VT_I4, VT_I4}};
		static _ATL_FUNC_INFO OnDataAvailableInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
		static _ATL_FUNC_INFO OnSendingDataInfo = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_I4}};
		static _ATL_FUNC_INFO OnSockGetHostByAddr = {CC_STDCALL, VT_I4, 4, {VT_I4, VT_BSTR, VT_BSTR, VT_I4}};
		static _ATL_FUNC_INFO OnSockGetHostByName = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_BSTR, VT_BSTR, VT_BSTR, VT_I4}};
		static _ATL_FUNC_INFO OnRequestProcessedInfo = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_I2, VT_I4, VT_I4, VT_I2}};
		
		namespace Internal
		{
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
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 1, &CCSEvent::OnDataAvailable, &OnDataAvailableInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 2, &CCSEvent::OnOtherMessage, &OnSockEventWinMsgInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 3, &CCSEvent::OnSocketClosed, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 4, &CCSEvent::OnSocketConnected, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 5, &CCSEvent::OnConnecting, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 6, &CCSEvent::OnSendingData, &OnSendingDataInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 7, &CCSEvent::OnGetHostByAddr, &OnSockGetHostByAddr)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 8, &CCSEvent::OnGetHostByName, &OnSockGetHostByName)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 9, &CCSEvent::OnClosing, &OnSockEventFuncInfo)
				SINK_ENTRY_INFO(IDC_SRCUSOCKETEVENT, __uuidof(_IUSocketEvent), 10, &CCSEvent::OnRequestProcessed, &OnRequestProcessedInfo)
			END_SINK_MAP()

			protected:
				virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError) = 0;
				virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam) = 0;
				virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError) = 0;
				virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd) = 0;
				virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent) = 0;
				virtual HRESULT __stdcall OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError) = 0;
				virtual HRESULT __stdcall OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError) = 0;
				virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd) = 0;
				virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag) = 0;
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
		class CRequestAsynHandlerBase;
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
		virtual ClientSide::CRequestAsynHandlerBase* GetHandler() = 0;
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
		virtual ClientSide::CRequestAsynHandlerBase* LockIdentity(unsigned long ulTimeout = INFINITE) = 0;
		virtual void UnlockIdentity(ClientSide::CRequestAsynHandlerBase *pIdentity) = 0;
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
			bool SwitchTo(CRequestAsynHandlerBase *pAsynHandler, bool bAutoTransferServerException = false);
			
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
			
			//depreciated. Use CScopeQueue instead
			inline CUQueue& GetMemoryQueue()
			{
				return (*m_UQueue);
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
				if(m_pIUSocket.p == NULL)
					return 0;
				long lSocket;
				m_pIUSocket->get_Socket(&lSocket);
				return lSocket;
			}

			bool GetReturnRandom();

			unsigned short GetServerPingTime();

			void Cancel(long lRequests = -1);

			IJobContext* GetCurrentJobContext();
			IJobManager* GetJobManager();
			
			//assign this class instance with a valid existing USocket object
			CClientSocket& operator=(IUnknown *pIUnknownToUSocket);

		protected:
			virtual HRESULT __stdcall OnDataAvailable(long hSocket, long lBytes, long lError);
			virtual HRESULT __stdcall OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam);
			virtual HRESULT __stdcall OnSocketClosed(long hSocket, long lError);
			virtual HRESULT __stdcall OnSocketConnected(long hSocket, long lError);
			virtual HRESULT __stdcall OnConnecting(long hSocket, long hWnd);
			virtual HRESULT __stdcall OnSendingData(long hSocket, long lError, long lSent);
			virtual HRESULT __stdcall OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError);
			virtual HRESULT __stdcall OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError);
			virtual HRESULT __stdcall OnClosing(long hSocket, long hWnd);
			virtual HRESULT __stdcall OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);
			virtual void OnBaseRequestProcessed(unsigned short usBaseRequestID);

			//Beginning from SocketPro version 4.8.1.12
			virtual void OnAllRequestsProcessed(unsigned int hSocket, unsigned short usLastRequestID);
			
		private:
			void InitEx(IUnknown *pIUnknown);
			void Init();
			void Uninit();
			void DetachAll();
			void ThrowException(HRESULT hr);
			void InvokeAllProcessed(unsigned short usReqID);
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
			CSimpleMap<unsigned long, CRequestAsynHandlerBase*> m_mapSvsIDHandler;
			CComAutoCriticalSection		m_cs;

		private:
			CUPushClientImpl	m_Push;
			CScopeUQueue	m_UQueue;
			CComPtr<IUFast> m_pIUFast;
			CComPtr<IUChat> m_pIUChat;
			CComPtr<IUSocket> m_pIUSocket;
			unsigned long	m_ulCurSvsID;
			bool			m_bServerException;
			friend CRequestAsynHandlerBase;

#if _MSC_VER >= 1300
			template<typename THandler, typename TCS>
			friend class CSocketPool;      // unbound friend class

			template<typename THandler, typename TCS>
			friend class CSocketPoolEx;      // unbound friend class
#endif	
		}; //CClientSocket

		class CRequestAsynHandlerBase
		{
		public:
			CRequestAsynHandlerBase();
			virtual ~CRequestAsynHandlerBase();
			
			//no copy constructor
			CRequestAsynHandlerBase(const CRequestAsynHandlerBase &RequestAsynHandlerBase);

		protected:
			virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue) = 0;

		public:
			virtual unsigned long GetSvsID() = 0;
			
			//no assignment operator
			CRequestAsynHandlerBase& operator= (const CRequestAsynHandlerBase &RequestAsynHandlerBase);

			virtual bool Attach(CClientSocket *pClientSocket);
			virtual void Detach();
			inline const CSocketProServerException& GetServerException() const 
			{
				CAutoLock AutoLock(&g_cs.m_sec);
				return m_err;
			}
			
			//it will return a pointer to a valid client socket if the handler is attached
			inline CClientSocket *GetAttachedClientSocket()
			{
				return m_pClientSocket;
			}
			
			virtual bool SendRequest(unsigned short usRequestID, const unsigned char *pBuffer=NULL, unsigned long ulLenInByte=0);
			virtual bool SendRequest(unsigned short usRequestID, CUQueue &UQueue);
			virtual bool SendRequest(unsigned short usRequestID, CScopeUQueue &UQueue);
			
			template<class ctype0>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0)
			{
				CScopeUQueue UQueue;
				UQueue << data0;
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}
			
			template<class ctype0, class ctype1, class ctype2>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4, class ctype5>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, const ctype5& data5)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4<<data5;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4, class ctype5, class ctype6>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, const ctype5& data5, const ctype6& data6)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4<<data5<<data6;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4, class ctype5, class ctype6, class ctype7>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, const ctype5& data5, const ctype6& data6, const ctype7& data7)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4<<data5<<data6<<data7;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4, class ctype5, class ctype6, class ctype7, class ctype8>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, const ctype5& data5, const ctype6& data6, const ctype7& data7, const ctype8& data8)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4<<data5<<data6<<data7<<data8;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4, class ctype5, class ctype6, class ctype7, class ctype8, class ctype9>
			bool SendRequest(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, const ctype5& data5, const ctype6& data6, const ctype7& data7, const ctype8& data8, const ctype9& data9)
			{
				CScopeUQueue UQueue;
				UQueue<<data0<<data1<<data2<<data3<<data4<<data5<<data6<<data7<<data8<<data9;			
				return SendRequest(usRequestID, UQueue->GetBuffer(), UQueue->GetSize());
			}

			inline void SuppressSocketProServerException();

		protected:
			//overwrite this function for monitoring SocketPro base requests
			virtual void OnBaseRequestProcessed(unsigned short usBaseRequestID);

			CSocketProServerException	m_err;

		private:
			CClientSocket *m_pClientSocket;
			friend CClientSocket;
		}; //CRequestAsynHandlerBase

		template<typename THandler, typename TCS = CClientSocket>
		class CSocketPool : protected Internal::CSocketPoolEvent
		{
		public:
			CSocketPool() : m_ulRecvTimeout(30000)
			{
				m_hr = m_pIUSocketPool.CoCreateInstance(__uuidof(USocketPool));
				if(m_pIUSocketPool != NULL)
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
				if(m_pIUSocketPool != NULL)
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
				if(m_pIUSocketPool == NULL)
					return false;
				if(IsStarted())
				{
					ShutdownPool();
				}
#ifdef _DEBUG
				m_hr = ::CoInitialize(NULL);
				if(m_hr != RPC_E_CHANGED_MODE)
				{
					//SocketPool object can be hosted inside a apartment thread, starting from version 4.8.2.1
					ATLTRACE("Warning: Free-threaded apartment is preferred for hosting socket pool object!\n"); 
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
				if(pIUSocket == NULL)
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
				if(pConnectionContext == NULL || nConnectionContextCount == 0 || bSocketsPerThread == 0)
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
					if(pIUSocket == NULL)
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
					if(m_pIUSocketPool.p != NULL)
					{
						m_pIUSocketPool->ShutdownPool();
					}
				}
			}

			THandler* Lock()
			{
				return Lock(INFINITE, NULL);
			}
			
			virtual THandler* Lock(ULONG ulTimeout, IUSocket *pIUSocketSameThreadWith = NULL)
			{
				int n;
				CComPtr<IUSocket> pIUSocket;
				if(m_pIUSocketPool == NULL)
					return NULL;
				m_hr = m_pIUSocketPool->LockASocket(ulTimeout, pIUSocketSameThreadWith, &pIUSocket);
				if(FAILED(m_hr))
					return NULL;
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
				return NULL;
			}

			void Unlock(THandler *pHandler)
			{
				ATLASSERT(pHandler != NULL);
				if(pHandler != NULL)
				{
					ATLASSERT(pHandler->GetAttachedClientSocket() != NULL);
					Unlock((TCS*)pHandler->GetAttachedClientSocket());
				}
			}

			virtual void Unlock(TCS *pClientSocket)
			{
				ATLASSERT(pClientSocket != NULL);
				if(pClientSocket != NULL)
				{
					m_hr = m_pIUSocketPool->UnlockASocket(pClientSocket->GetIUSocket());
				}
			}

			inline bool IsStarted()
			{
				BYTE bThreads = 0;
				if(m_pIUSocketPool == NULL)
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
				ATLASSERT(pHandler != NULL);
				if(pHandler != NULL)
				{
					ATLASSERT(pHandler->GetAttachedClientSocket() != NULL);
					return IsCompleted((TCS*)pHandler->GetAttachedClientSocket());
				}
				return true;
			}
			
			inline bool IsCompleted(TCS *pClientSocket)
			{
				ATLASSERT(pClientSocket != NULL);
				if(pClientSocket != NULL)
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
						ATLASSERT(pIUSocket != NULL);
						CAutoLock	AutoLock(&g_cs.m_sec);
						TCS *pSocket = new TCS();
						pSocket->SetUSocket(pIUSocket);
						THandler *pHandler = new THandler();
						pSocket->Disadvise(); //make sure all of attached handlers subscribe socket events first
						pHandler->Attach(pSocket);
						pSocket->Advise(); //re-subscribe socket events
						m_mapSocketHandler.Add(pSocket, pHandler);
						pIUSocket->put_RecvTimeout((long)m_ulRecvTimeout);
					}
					break;
				case speConnected:
					{
						HRESULT hr;

						ATLASSERT(pIUSocket != NULL);

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
				CJobContext() : m_Status(jsInitial), m_jobId(0), m_pHandler(NULL), m_pJobManager(NULL), 
					m_pIdentity(NULL), m_idTask(0), m_bBundled(false), m_nWaiting(0)
				{
					m_hEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL);
				}
				
				virtual ~CJobContext()
				{
					if(m_hEvent != NULL)
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
					if(pBuffer == NULL)
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
					return (UQueue.GetBuffer() != NULL);
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
								if(p != NULL)
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

				virtual ClientSide::CRequestAsynHandlerBase* GetHandler()
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
					m_pJobManager = NULL;
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
					return NULL;
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
				CJobManager() : m_jobIndex(0), m_pSocketPoolEx(NULL), m_nRecycleBinSize(10)
				{
					m_mapQueuedIdentityJob.m_pJobManager = this;
					m_hIdentityWait = ::CreateEvent(NULL, TRUE, TRUE, NULL);
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
					if(pJob == NULL || pJob->m_Status != jsQueued)
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
					
					if(pJobContext == NULL)
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
					if(nCount == 0 || pJobId == NULL)
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
							if(pJob == NULL || pJob->m_Status == jsInitial)
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
						CUQueue *pUQueue = &su;
						hTwmp = ::CreateEvent(NULL, TRUE, FALSE, NULL);
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
							if(pJob == NULL || pJob->m_Status == jsInitial)
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
					if(nCount == 0 || pJobId == NULL)
						return true;
					unsigned ulMyTimeout = ulTimeout;
					unsigned long ulStart = ::GetTickCount();
					for(n=0; n<nCount; n++)
					{
						IJobContext *pIJobContext = SeekJob(pJobId[n]);
						if(pIJobContext == NULL)
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
						if(jb == NULL)
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

				virtual void UnlockIdentity(ClientSide::CRequestAsynHandlerBase *pIdentity)
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					void *p = pIdentity;
					if(m_aLockedIdentity.Remove(p))
						::SetEvent(m_hIdentityWait);
				}

				virtual ClientSide::CRequestAsynHandlerBase* LockIdentity(unsigned long ulTimeout)
				{
					unsigned long ulMyTimeout = ulTimeout;
					unsigned long ulStart = ::GetTickCount();
ao:					int n = 0;
					{
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_pSocketPoolEx->m_mapSocketHandler.GetSize() == 0)
							return NULL;
						long lConnected = 0;
						m_pSocketPoolEx->GetUSocketPool()->get_ConnectedSocketsEx(&lConnected);
						if(lConnected == 0)
							return NULL;
						int nSize = m_pSocketPoolEx->m_mapSocketHandler.GetSize();
						for(n=0; n<nSize; n++)
						{
							CRequestAsynHandlerBase *p = m_pSocketPoolEx->m_mapSocketHandler.GetValueAt(n);
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
					return NULL; //all of async handlers are already associated with jobs.
				}

				virtual bool EnqueueJob(IJobContext *pJobContext)
				{
					if(pJobContext == NULL || pJobContext->GetJobStatus() != jsCreating || m_pSocketPoolEx->GetUSocketPool() == NULL)
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
					if(p != NULL)
						return p;
					int nSize = m_aJobProcessing.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJob = m_aJobProcessing[n];
						if(pJob->m_Status != jsRunning)
							continue;
						ATLASSERT(pJob->m_Status != jsInitial);
						ATLASSERT(pJob->m_pHandler != NULL);
						ATLASSERT(pJob->m_pJobManager != NULL);
						if(pJob->m_jobId == jobId)
							return pJob;
					}
					return NULL;
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
						ATLASSERT(pJob->m_pHandler != NULL);
						ATLASSERT(pJob->m_pJobManager != NULL);
						if(pJob->m_jobId == jobId)
							return pJob->m_Status;
					}
					
					{
						CJobContext *pJob = m_mapQueuedIdentityJob.SeekJob(jobId);
						if(pJob != NULL)
							return pJob->m_Status;
					}

					nSize = m_aJobEmpty.GetSize();
					for(n=0; n<nSize; n++)
					{
						CJobContext *pJob = m_aJobEmpty[n];
						ATLASSERT(pJob->m_pHandler == NULL);
						ATLASSERT(pJob->m_pJobManager != NULL);
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
					if(pJC == NULL)
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
					if(pJC != NULL && m_pSocketPoolEx->m_bServerLoadingBalance)
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
					pJC->m_pHandler = NULL;
					pJC->m_Status = jsInitial;
					pJC->m_Tasks->SetSize(0);
					pJC->m_pIdentity = NULL;
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
			CSocketPoolEx() : m_nFails(0), m_bWorking(false), m_bPause(false), m_bCallProcess(false), m_bServerLoadingBalance(false)
			{
				m_hWait = ::CreateEvent(NULL, TRUE, TRUE, NULL);
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

			virtual THandler* Lock(ULONG ulTimeout, IUSocket *pIUSocketSameThreadWith = NULL)
			{
				//should not call this method!
				ATLASSERT(FALSE);

				return NULL;
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
				THandler *pHandler = NULL;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(!m_bWorking)
						return;
					m_bWorking = false;
					if(m_aPHandler.GetSize() > 0)
						pHandler = m_aPHandler[0];
				}
				
				m_JobManager.CleanTasks(!bCancelRequestsInQueue);

				while(pHandler != NULL)
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
						pHandler = NULL;
				}
				while(pHandler != NULL)
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
				THandler *pHandler = NULL;
				{
					CAutoLock	AutoLock(&g_cs.m_sec);
					if(m_bPause || !m_bWorking)
						return;
					m_bPause = true;
					if(m_aPHandler.GetSize() > 0)
						pHandler = m_aPHandler[0];
				}

				while(pHandler != NULL)
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
							pHandler = NULL;
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
					if(m_pIUSocketPool == NULL)
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
					THandler *pHandler = CSocketPool<THandler, TCS>::Lock(0, NULL);
					if(pHandler == NULL)
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
				THandler *pHandler = CSocketPool<THandler, TCS>::Lock(0, NULL);
				bool ok = false;
				while(pHandler != NULL)
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
					pHandler = CSocketPool<THandler, TCS>::Lock(0, NULL);
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
							if(cs->m_pIExtenalSocketEvent != NULL)
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
						THandler *pHandler = NULL;
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
										pHandler = NULL;
									break;
								}
							}
						}
						if(pHandler != NULL)
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
			static void MyCancel(CRequestAsynHandlerBase *pHandler)
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
								pJC->m_pHandler = NULL;
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
					hr = pIUFast->SendRequestEx(idStartJob, 0, NULL);
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
					hr = pIUFast->SendRequestEx(idEndJob, 0, NULL);
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
						ATLASSERT(pHandler != NULL);
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
			unsigned long SendReturnData(unsigned short usRequestID, const unsigned char *pBuffer = NULL, unsigned long ulLen = 0);
			
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
			
			//notification
//			bool Exit();
//			bool Enter(unsigned long ulGroupIDs);
//			bool SpeakToEx(const unsigned char *pMessage, unsigned long ulLen, unsigned int hSocketTarget);
//			bool SpeakTo(const VARIANT *pvtMsg, unsigned int hSocketTarget);
//			bool SpeakEx(const unsigned char *pMessage, unsigned long ulLen, unsigned long ulGroups = odAllGroups);
//			bool Speak(const VARIANT *pvtMsg, unsigned long ulGroups = odAllGroups);
//			bool SendUserMessage(const wchar_t *strUserID, const VARIANT *pvtMessage);
//			bool SendUserMessageEx(const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulLen);
			
			unsigned long GetJoinedGroups(unsigned long *pGroupId, unsigned long ulGroupCount);
			
			//user id and password
			bool SetUID(const wchar_t *strUID);
			unsigned long GetUID(wchar_t *strUID, unsigned long ulCharLen);
			unsigned long GetPassword(wchar_t *strPassword, unsigned long ulCharLen);

			//Switch info is used for exchanging version information and others
			void SetServerInfo(CSwitchInfo ServerInfo);
			CSwitchInfo GetServerInfo();
			CSwitchInfo GetClientInfo();
			bool TransferServerException();
			
			//socket connection attributes
			void SetZip(bool bZip);
			bool GetZip();
			unsigned long GetSndBufferSize();
			unsigned long GetRequestsInQueue();
			unsigned long GetRcvBytesInQueue();
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
			virtual void OnChatRequestComing(tagChatRequestID ChatRequestId, const VARIANT &vtParam1, const VARIANT &vtParam2);
			virtual void OnChatRequestCame(tagChatRequestID ChatRequestId);
			
		public:
			//m_bAutoBuffer is true by default
			//if m_bAutoBuffer is true, all of fast and slow requests are automatically buffered into m_UQueue
			bool m_bAutoBuffer;
			
		private:
			CScopeUQueue	m_sq;
			CUPushServerImpl m_Push;
			unsigned int	m_hSocket;
			unsigned long	m_ulTickCountReleased;
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
			virtual bool	AddMe(unsigned long ulSvsID, unsigned long ulEvents = 0, enumThreadApartment taWhatTA = taNone);
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
			static unsigned long GetUserID(unsigned int hSocket, wchar_t *strUID, unsigned long ulCharLen);
			static unsigned long GetPassword(unsigned int hSocket, wchar_t *strPassword, unsigned long ulCharLen);
			static bool SetPassword(unsigned int hSocket, const wchar_t *strPassword);
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
			void StartMessagePump();
			unsigned long GetEvents()
			{
				return m_ulEvents;
			}
			bool StartSocketProServer(UINT unPort, UINT unMaxBacklog = 64);
			void AskForEvents(unsigned long ulEvents);
			static void StopSocketProServer();
			
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
			static bool PostQuit(unsigned long ulThreadID);
			
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

//			static unsigned long GetCountOfHTTPChatters(unsigned long ulGroupId);
//			static const wchar_t* GetHTTPChatId(unsigned long ulGroupId, unsigned long ulIndex);
/*			static void SetGroupsNotifiedWhenEntering(unsigned long ulGroupsNotifiedWhenEntering);
			static void SetGroupsNotifiedWhenExiting(unsigned long ulGroupsNotifiedWhenExiting);
			static unsigned long GetGroupsNotifiedWhenEntering();
			static unsigned long GetGroupsNotifiedWhenExiting();*/

		protected:
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

		private:
			unsigned long			m_ulEvents;	
		};
		
/*		class CNotifier : public CClientPeer
		{
		protected:
			virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
			{
				//shouldn't come here
				ATLASSERT(FALSE);
			}
			virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
			{
				//shouldn't come here
				ATLASSERT(FALSE);
				return 0;
			}
		};*/

		struct IUHttpPush
		{
			virtual const wchar_t* Enter(unsigned long *pGroups, unsigned long ulGroupCount, const wchar_t* strUserID, unsigned long dwLeaseTime, const wchar_t *strIpAddr = NULL) = 0;
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
				virtual const wchar_t* Enter(unsigned long *pGroups, unsigned long ulGroupCount, const wchar_t* strUserID, unsigned long dwLeaseTime, const wchar_t *strIpAddr = NULL);
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
			CPLGPeer() : m_pIJobManager(NULL)
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
				m_bUsePool = true;
				m_SP.m_pPLGSvs = this;
			}

			//disable copy constructor and assignment operator
			CPLGService(const CPLGService &PLGService);
			CPLGService& operator=(const CPLGService &PLGService);
			
		private:
			class CPLGHandler : public ClientSide::CRequestAsynHandlerBase
			{
			public:
				CPLGHandler() : m_pJobContext(NULL)
				{
					
				}
				
				//disable copy constructor and assignment operator
				CPLGHandler(const CPLGHandler &PLGHandler);
				CPLGHandler& operator=(const CPLGHandler &PLGHandler);

			public:
				virtual unsigned long GetSvsID()
				{
					return ulServiceIdOnRealServer;
				}

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

				virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
				{
					do
					{
						bool bClient = false;
						bool bDrop = false;
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_pJobContext == NULL || m_pJobContext->GetJobStatus() != jsRunning)
							break;
						bool bRandomReturn = GetAttachedClientSocket()->GetReturnRandom();
						CClientPeer *p = (CClientPeer *)m_pJobContext->GetIdentity();
						TClientPeer *pClientPeer = NULL;
						try
						{
							pClientPeer = dynamic_cast<TClientPeer *>(p);
						}
						catch(...)
						{
							pClientPeer = NULL;
						}
						if(pClientPeer == NULL)
							break;
						unsigned int hSocket = pClientPeer->GetSocket();
						if(hSocket == 0 || hSocket == (~0))
							break;
						unsigned long ulSvsId = pClientPeer->GetSvsID();
						if(ulSvsId == 0 || ulSvsId == (~0))
							break;
						{
							CAutoReverseLock arl(&g_cs.m_sec);
							if(pClientPeer->OnSendingPeerData(m_pJobContext, usRequestID, UQueue))
							{
								CBaseService *pBase = CBaseService::GetBaseService(ulSvsId);
								if(pBase == NULL)
									break;
								pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
								if(pClientPeer == NULL)
									break;
								pClientPeer->SendReturnData(usRequestID, UQueue.GetBuffer(), UQueue.GetSize());
								
								pBase = CBaseService::GetBaseService(ulSvsId);
								if(pBase == NULL)
									break;
								pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
								if(pClientPeer == NULL)
									break;
								pClientPeer->OnPeerDataSent(m_pJobContext, usRequestID);
							}
							else
							{
								bDrop = true;
							}
						}
						if(m_pJobContext == NULL)
							break;
						PLGSocketPool::CJobContext *pJobContext = (PLGSocketPool::CJobContext*)m_pJobContext;
						pJobContext->RemoveTask(bRandomReturn, usRequestID, bClient);
						if(bClient && bDrop)
						{
							CBaseService *pBase = CBaseService::GetBaseService(ulSvsId);
							if(pBase == NULL)
								break;
							pClientPeer = (TClientPeer *)pBase->SeekClientPeer(hSocket);
							if(pClientPeer == NULL)
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
			virtual bool OnFailover(ClientSide::CRequestAsynHandlerBase *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("A fail with job id = %d, task count = %d, fails = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks(), m_SP.GetFails());	
#endif
				return true;
			}

			virtual void OnJobDone(ClientSide::CRequestAsynHandlerBase *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Job id = %d done, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
			}

			virtual bool OnExecutingJob(ClientSide::CRequestAsynHandlerBase *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Going to process job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif
				return true;
			}

			virtual void OnJobProcessing(ClientSide::CRequestAsynHandlerBase *pHandler, IJobContext *pJobContext)
			{
#ifdef ENABLE_SOCKETPRO_LB_TRACING
				ATLTRACE("Processing job id = %d, task count = %d\n", (int)pJobContext->GetJobId(), pJobContext->GetCountOfTasks());
#endif				
			}

			virtual void OnReturnedResultProcessed(ClientSide::CRequestAsynHandlerBase *pHandler, IJobContext *pJobContext, unsigned short usRequestId)
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
					ATLASSERT(pIJobContext != NULL);
					{
						CAutoReverseLock arl(&g_cs.m_sec);
						pClientPeer->OnJobJustCreated(pIJobContext, usRequestID);
					}
				}
				else
				{
					pIJobContext = (PLGSocketPool::CJobContext*)m_mapIdentityJob.GetValueAt(nIndex);
					ATLASSERT(pIJobContext != NULL);
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
						if(ppIJobContext != NULL)
							*ppIJobContext = pIJobContext;
					}
					else
					{
						if(ppIJobContext != NULL)
							*ppIJobContext = NULL;
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
				ATLASSERT(pIJobContext != NULL);
				if(pIJobContext != NULL)
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
				TClientPeer *pClientPeer = NULL;
				IJobContext *ppIJobContext = NULL;
				{
					//Don't set m_bUserPool to false!!!
					ATLASSERT(m_bUsePool); 
					CAutoLock	AutoLock(&g_cs.m_sec);
					pClientPeer = (TClientPeer *)SeekClientPeer(hSocket);
					ATLASSERT(pClientPeer != NULL);
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
						if(!bHeader && pClientPeer != NULL && ppIJobContext != NULL)
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
				ATLASSERT(pClientPeer != NULL);
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
							ATLASSERT(pIJobContext != NULL);
							int idTask = pIJobContext->AddTask(usRequestID, NULL, 0, true);
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
							ATLASSERT(pIJobContext != NULL);
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
					pHandler->AssociateJobContext(NULL);
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
							CPLGHandler *pHandler = NULL;
							CAutoLock	AutoLock(&g_cs.m_sec);
							int nSize = m_mapSocketHandler.GetSize();
							for(n=0; n<nSize; n++)
							{
								TCS *cs = m_mapSocketHandler.GetKeyAt(n);
								if(cs->GetSocket() == hSocket)
								{
									pHandler = m_mapSocketHandler.GetValueAt(n);
									IJobContext *pIJobContext = pHandler->GetAssocatedJobContext();
									if(pIJobContext == NULL)
										break;
									void *pIdentity = pIJobContext->GetIdentity();
									if(pIdentity == NULL)
										break;
									CClientPeer *pClientPeer = (CClientPeer*)pIdentity;
									if(pClientPeer == NULL)
										break;
									TClientPeer *p = NULL;
									try
									{
										p = dynamic_cast<TClientPeer *>(pClientPeer);
									}
									catch(...)
									{
										p = NULL;
									}
									if(p == NULL)
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
		extern CSocketProServerLoader g_SocketProLoader;
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