#pragma once

#ifndef __SOCKETPRO_ADAPTER_BASE_SERVICE_H________
#define __SOCKETPRO_ADAPTER_BASE_SERVICE_H________

namespace SocketProAdapter
{
#ifndef _WIN32_WCE
namespace ServerSide
{

[CLSCompliantAttribute(true)] 
public ref class CBaseService abstract
{
public:
	CBaseService();
	virtual ~CBaseService();

public:
	/// <summary>
	/// Plug this service into a SocketPro server with default events and taNone.
	/// The method returns true or false. 
	/// If false is returned, the SocketPro server may already have a service with the same service id.
	/// </summary>
	/// <param name="nSvsID">A service ID.</param>
	bool AddMe(int nSvsID);

	/// <summary>
	/// Plug this service into a SocketPro server with default events.
	/// The method returns true or false. 
	/// If false is returned, the SocketPro server may already have a service with the same service id.
	/// </summary>
	/// <param name="nSvsID">A service ID.</param>
	/// <param name="taWhatTA">A thread apartment. If no COM object is involved, set the parameter into taNone.</param>
	bool AddMe(int nSvsID, tagThreadApartment taWhatTA);

	/// <summary>
	/// Plug this service into a SocketPro server with default events.
	/// The method returns true or false. 
	/// If false is returned, the SocketPro server may already have a service with the same service id.
	/// </summary>
	/// <param name="nSvsID">A service ID.</param>
	/// <param name="nEvents">An integer indicating one or more events. Note that some events are always subscribed no matter what the input is.</param>
	/// <param name="taWhatTA">A thread apartment. If no COM object is involved, set the parameter into taNone.</param>
	virtual bool AddMe(int nSvsID, int nEvents, tagThreadApartment taWhatTA);
	
	/// <summary>
	/// Inform a SocketPro server that a request will take a long time to process.
	/// The method returns true or false. If it is return false, check if you have already called the method AddMe successfully.
	/// Note that you can call this method ONLY AFTER successfully calling the method AddMe.
	/// </summary>
	/// <param name="sRequestID">A request ID.</param>
	bool AddSlowRequest(short sRequestID);
	
	void RemoveAllSlowRequests();
	void RemoveMe();
	void RemoveSlowRequest(short sRequestID);
	CClientPeer^ SeekClientPeer(int hSocket);
	short GetSlowRequest(short nIndex);
	
	property short CountOfSlowRequests
	{
		short get()
		{
			return g_SocketProLoader.GetCountOfSlowRequests(m_ulSvsID);
		}
	}

internal:
	/// <summary>
	/// A property indicating if returning results will be queued in random. It defaults to false. 
	/// The property is mainly created for parallel, loading balance and grid computing at server side.
	/// Don't set the property from your code!
	/// </summary>
	property bool ReturnRandom
	{
		bool get()
		{
			if(g_SocketProLoader.GetReturnRandom == NULL)
				return false;
			return g_SocketProLoader.GetReturnRandom(m_ulSvsID);
		}

		void set(bool b)
		{
			if(g_SocketProLoader.SetReturnRandom == NULL)
				return;
			g_SocketProLoader.SetReturnRandom(m_ulSvsID, b);
		}
	}

public:
	property int Events
	{
		int get()
		{
			int nEvents = 0;
			if(m_pSvsContext->m_OnBaseRequestCame != NULL)
				nEvents += (int)tagEvent::eOnBaseRequestCame;

			if(m_pSvsContext->m_OnCleanPool != NULL)
				nEvents += (int)tagEvent::eOnCleanPool;

			if(m_pSvsContext->m_OnClose != NULL)
				nEvents += (int)tagEvent::eOnClose;
			
			if(m_pSvsContext->m_OnFastRequestArrive != NULL)
				nEvents += (int)tagEvent::eOnFastRequestArrive;

			if(m_pSvsContext->m_OnIsPermitted != NULL)
				nEvents += (int)tagEvent::eOnIsPermitted;

			if(m_pSvsContext->m_OnReceive != NULL)
				nEvents += (int)tagEvent::eOnReceive;

			if(m_pSvsContext->m_OnRequestProcessed != NULL)
				nEvents += (int)tagEvent::eOnSlowRequestProcessed;

			if(m_pSvsContext->m_OnSend != NULL)
				nEvents += (int)tagEvent::eOnSend;

			if(m_pSvsContext->m_OnSendReturnData != NULL)
				nEvents += (int)tagEvent::eOnSendReturnData;

			if(m_pSvsContext->m_OnSwitchTo != NULL)
				nEvents += (int)tagEvent::eOnSwitchTo;

			if(m_pSvsContext->m_OnThreadCreated != NULL)
				nEvents += (int)tagEvent::eOnThreadCreated;

			if(m_pSvsContext->m_PreTranslateMessage != NULL)
				nEvents += (int)tagEvent::eOnPretranslateMessage;

			if(m_pSvsContext->m_SlowProcess != NULL)
				nEvents += (int)tagEvent::eOnSlowRequestArrive;

			if(m_pSvsContext->m_ThreadDying != NULL)
				nEvents += (int)tagEvent::eOnThreadShuttingDown;

			if(m_pSvsContext->m_ThreadStarted != NULL)
				nEvents += (int)tagEvent::eOnThreadStarted;

			return nEvents;
		}
	}
	
	/// <summary>
	/// A property indicating this service id.
	/// </summary>
	property int SvsID
	{
		int get()
		{
			return m_ulSvsID;
		}
	}
	
	static bool RemoveALibrary(String ^strLibFile);
	static bool RemoveALibrary(IntPtr hInstance);
	static CClientPeer^ SeekClientPeerGlobally(int hSocket);
	
	/// <summary>
	/// Get an library from a zero-based index.
	/// The method returns a HINSTANCE to a libarary. If not found, the return result will be zero.
	/// </summary>
	/// <param name="nIndex">0, 1, 2, .....</param>
	static IntPtr GetALibrary(int nIndex);

	static CBaseService^ GetBaseService(int nSvsID);
	static int GetServiceID(int nIndex);

	/// <summary>
	/// Plug a library into a SocketPro server.
	/// The method returns a HINSTANCE to a libarary. If failed, the return result will be zero.
	/// </summary>
	/// <param name="strLibFile">A library file name like c:\somedir\uodbsvr.dll</param>
	static IntPtr AddALibrary(String^ strLibFile);

	/// <summary>
	/// Plug a library into a SocketPro server.
	/// The method returns a valid HINSTANCE. If failed, the return result will be zero.
	/// </summary>
	/// <param name="strLibFile">A library file name like c:\somedir\uodbsvr.dll.</param>
	/// <param name="nParam">A value passed into the library.</param>
	static IntPtr AddALibrary(String^ strLibFile, int nParam);
	
	/// <summary>
	/// A property indicating the total number of services registered inside the SocketPro server, including ones from other libraries.
	/// </summary>
	static property int CountOfAllServices
	{
		int get()
		{
			return g_SocketProLoader.GetCountOfServices();
		}
	}

	static property int LastSocketError
	{
		int get()
		{
			return g_SocketProLoader.GetLastSocketError();
		}
	}
	
	static property int CountOfLibraries
	{
		int get()
		{
			return g_SocketProLoader.GetCountOfLibraries();
		}
	}
	
	/// <summary>
	/// A property indicating the total number of services registered from this library.
	/// </summary>
	static property int CountOfServices
	{
		int get()
		{
			return g_SocketProLoader.GetCountOfServices();
		}
	}

protected:
	/// <summary>
	/// This virtual function is called within a main thread when a client is authenticated.
	/// </summary>
	virtual bool OnIsPermitted(int hSocket)
	{
		return true;
	}
	
	/// <summary>
	/// This virtual function is called within a main thread when the thread is just created.
	/// </summary>
	virtual void OnThreadCreated(int lThreadID, int hSocket)
	{
	}
	
	/// <summary>
	/// This virtual function is called within a worker thread when the thread is just started.
	/// </summary>
	virtual void OnThreadStarted()
	{

	}
	
	/// <summary>
	/// This virtual function is called within a worker thread when the thread is going to die.
	/// </summary>
	virtual void OnThreadShuttingDown()
	{

	}
	
	/// <summary>
	/// This virtual function is called within a worker thread before a message is dispatched. 
	/// For all of parameters, see the documnetation of window structiure MSG.
	/// If you does not want to continue processing a message, override the function with return value True.
	/// </summary>
	virtual bool OnPretranslateMessage(int hWnd, int nMessage, int wParam, int lParam, int uTime, int nPointX, int nPointY)
	{
		return false;
	}

	/// <summary>
	/// Associate a service with an instance of your CClientPeer derived class. 
	/// You must override the pure virtual function as shown in all of samples.
	/// </summary>
	virtual CClientPeer^ GetPeerSocket(int hSocket) abstract = 0;

protected:
	/// <summary>
	/// if m_bUsePool is true, all of CClientPeer derived objects are put into m_Pool, 
	/// and then deleted in a specified time (60 seconds) after a socket connection is closed.
	/// When the pool is enabled, it will be helpful for performance in case socket connections are repeatedly closed and connected.
	/// However, you may have to initialize a CClientPeer derived object by overriding CClientPeer::OnSwitchFrom
	/// because a CClientPeer derived object may be in a different state.
	/// If m_bUsePool is false, all of CClientPeer derived objects are deleted 
	/// immediately after a socket connection is closed or swicthed.
	/// </summary>
	property bool m_bUsePool
	{
		bool get()
		{
			return m_bPool;
		}
		void set(bool bUsePool)
		{
			m_bPool = bUsePool;
		}
	}

private:
	unsigned int			m_ulSvsID;
	CSvsContext				*m_pSvsContext;

	DOnClose				^m_OnClose;
	DOnIsPermitted			^m_OnIsPermitted;
	DOnSwitchTo				^m_OnSwitchTo;
	DOnBaseRequestCame		^m_OnBaseRequestCame;
	DOnCleanPool			^m_OnCleanPool;
	DOnFastRequestArrive	^m_OnFastRequestArrive;
	DSLOW_PROCESS			^m_SLOW_PROCESS;
	DOnRequestProcessed		^m_OnSlowRequestProcessed;
	
	//The following callbacks are not very often used
	DOnSend					^m_OnSend;
	DOnSendReturnData		^m_OnSendReturnData;
	DOnReceive				^m_OnReceive;
	DOnThreadCreated		^m_OnThreadCreated;
	DTHREAD_DYING			^m_Thread_DYING;
	DTHREAD_STARTED			^m_THREAD_STARTED;

	//Once a socket connection is closed, if m_bPool is true, all of CClientPeer derived objects are put into m_Pool, 
	//and then deleted after a period of time (60 seconds).
	//Pool is helpful for performance when socket connections are repeatedly closed.
	//However, you may have to initialize a CClientPeer derived object by overriding CClientPeer::OnSwitchFrom
	//because a CClientPeer derived object may be in a different state.

	//If m_bPool is false, all of CClientPeer derived objects are deleted 
	//immediately when a socket connection is closed or swicthed to another service.
	bool					m_bPool;
	List<CClientPeer^>		^m_aClientPeer;
	
	//
	List<CClientPeer^>		^m_Pool;

internal:
	virtual void OnBaseRequestCame(int hSocket, short sRequestID);
	virtual void OnRelease(int hSocket, bool bClose, long lInfo);
	virtual void OnFastRequestArrive(int hSocket, short sRequestID);
	virtual void OnSlowRequestArrive(int hSocket, short sRequestID);

private:
	void SetDelegates(unsigned int ulEvents);
	void OnFast(int hSocket, short usRequestID, int ulLen);
	bool OnPermitted(int hSocket, int ulSvsID);
	void OnSwitch(int hSocket, int ulPrevSvsID, int ulCurrSvsID);
	int OnSlow(short usRequestID, int ulLen, int hSocket);
	void OnClose(int hSocket, int nError);
	void OnBaseCame(int hSocket, short usRequestID);
	void OnReceive(int hSocket, int nError);
	void OnSend(int hSocket, int nError);
	void OnSlowRequestProcessed(int hSocket, short usRequestID);
	bool OnSendReturnData(int hSocket, short usRequestId, int ulLen, unsigned char *pBuffer);
	void OnCleanPool(int ulSvsID, int ulTickCount);
	void OnThreadCreated(int ulSvsID, int ulThreadID, int hSocket);
	CClientPeer^ GetClientPeerFromPool();
	void RemoveClientPeer(CClientPeer ^p);
	
internal:
	DOnPretranslateMessage			^m_OnPretranslateMessage;
	static List<CBaseService^>		^m_aService;
};

public ref class CNotifier : public CClientPeer
{
public:
	CNotifier()
	{
	}

protected:
	virtual void OnFastRequestArrive(short sRequestID, int nLen) override
	{
	}
	virtual int OnSlowRequestArrive(short sRequestID, int nLen) override
	{
		return 0;
	}
};

generic<typename TPeer> where TPeer : CClientPeer, gcnew()
public ref class CSocketProService : public CBaseService
{
public:
	CSocketProService()
	{
		m_bUsePool = true;
	}

protected:
	virtual CClientPeer^ GetPeerSocket(int hSocket) override
	{
		return gcnew TPeer();
	}
};

public ref class CNotificationService : public CSocketProService<CNotifier^>
{
};

};
#endif
};
#endif
