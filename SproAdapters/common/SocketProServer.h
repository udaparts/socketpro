#pragma once

using namespace System::Threading;

#ifndef __SOCKETPRO_ADAPTER_SERVERSIDE_SOCKETPRO_SERVER_H__
#define __SOCKETPRO_ADAPTER_SERVERSIDE_SOCKETPRO_SERVER_H__

namespace SocketProAdapter
{

#ifndef _WIN32_WCE
namespace ServerSide
{
	public enum struct tagThreadApartment
	{
		taNone = 0, //no COM objet is involved
		taApartment, //thread will enter a STA apartment
		taFree, //thread will enter MTA apartment
	};

	public enum struct tagEvent
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

	public enum struct tagHTTPRequest
	{
		Unknown = 0,
		Get,
		Post,
		Put,
		Delete,
		Options,
		Trace,
		Head,
		Connect,
	};

	private delegate void DOnChatRequestComing(int hSocketSource, short sRequestID, int nLen);
	private delegate void DOnChatRequestCame(int hSocketSource, short sRequestID, int nLen);
	private delegate void DOnWinMessage(int hWnd, int uMsg, int wParam,  int lParam);
	private delegate void DOnAccept(int hSocket, int nError);
	private delegate void DOnCleanPool(int ulSvsID, int ulTickCount);
	private delegate void DOnClose(int hSocket, int nError);
	private delegate void DOnReceive(int hSocket, int nError);
	private delegate void DOnSend(int hSocket, int nError);
	private delegate void DOnSSLEvent(int hSocket, int nWhere, int nRtn);
	private delegate void DOnBaseRequestCame(int hSocket, short usRequestID);
	private delegate void DOnRequestProcessed(int hSocket, short usRequestID);
	private delegate void DOnFastRequestArrive(int hSocket, short usRequestID, int ulLen);
	private delegate bool DOnIsPermitted(int hSocket, int ulSvsID);
	private delegate void DOnSwitchTo(int hSocket, int ulPrevSvsID, int ulCurrSvsID);
	private delegate bool DOnSendReturnData(int hSocket, short usRequestId, int ulLen, unsigned char *pBuffer);
	private delegate void DTHREAD_STARTED();
	private delegate int DSLOW_PROCESS(short usRequestID, int ulLen, int hSocket);
	private delegate void DTHREAD_DYING();
	private delegate bool DOnPretranslateMessage(int hWnd, int nMessage, int wParam, int lParam, int uTime, int nPointX, int nPointY);
	private delegate void DOnThreadCreated(int ulSvsID, int ulThreadID, int hSocket);


[CLSCompliantAttribute(true)] 
public ref class CSocketProServer abstract
{
public:
	static const int ERROR_UNO_ERROR	= (0x000);
	static const int ERROR_UWRONG_SWICTH	= (0x100);
	static const int ERROR_UAUTHENTICATION_FAILED = (0x101);
	static const int ERROR_USERVICE_NOT_FOUND = (0x102);
	static const int ERROR_UBLOWFISH_KEY_WRONG = (0x103);
	static const int ERROR_UNOT_SWITCHED_YET	= (0x104); 
	static const int ERROR_VERIFY_SALT = (0x105);
	static const int ERROR_BAD_HTTP_SIZE = (0x106);
	static const int ERROR_BAD_HTTP_REQUEST = (0x107);
	static const int ERROR_UEXCEPTION_CAUGHT = (0x108);
	static const int ERROR_REQUEST_TOO_LARGE = (0x109);
	static const int ERROR_UNKNOWN_REQUEST = (0x10A);

public:
	/// <summary>
	/// Create an instance of CSocketProServer. 
	/// Note that a SocketPro server can have ONE instance ONLY.
	/// Once an instance of CSocketProServer is created, the underlying usktpror.dll is initialized.
	/// </summary>
	CSocketProServer();

	/// <summary>
	/// Create an instance of CSocketProServer. 
	/// Note that a SocketPro server can have ONE instance ONLY.
	/// Once an instance of CSocketProServer is created, the underlying usktpror.dll is initialized.
	/// </summary>
	/// <param name="nParam"> A data passed into usktpror.dll. At this time, it is reserved for the future use.</param>
	CSocketProServer(int nParam);
	
	virtual ~CSocketProServer();

protected:
	virtual bool OnSettingServer() = 0;

private:
	/*/// <summary>
	/// Start a SocketPro server on a given port with the default blacklog size 64.
	/// The method returns true or false. If false is returned, see the static property LastSocketError.
	/// </summary>
	/// <param name="nPort">A given port number like 25, 80, 17000, 17001, 22222, or ..... </param>
	bool StartSocketProServer(int nPort);
	
	/// <summary>
	/// Start a SocketPro server on a given port. You can call this method ONE time ONLY.
	/// </summary>
	/// <returns>
	/// True or false. If false is returned, see the static property LastSocketError.
	/// </returns>
	/// <param name="nPort">A given port number like 25, 80, 17000, 17001, 22222, or ..... </param>
	/// <param name="nMaxBlacklog">blacklog size like 5, 10, 16, ....... </param>
	bool StartSocketProServer(int nPort, int nMaxBlacklog);*/

	/// <summary>
	/// Start a window message pump. Call ths method right after calling StartSocketProServer if there is no message pump available.
	/// However, don't call this method if a window message pump is already available.
	/// </summary>
	void StartMessagePump();

public:
	/// <summary>
	/// Starts a SocketPro server running on a given port (nPort) with default events OnClose, OnAccept, OnIsPermited and OnChatRequestComing.
	/// The default max backlog is 64.
	/// </summary>
	bool Run(int nPort);

	/// <summary>
	/// Starts a SocketPro server running on a given port (nPort) with extra events (lEvents) plus default events OnClose, OnAccept, OnIsPermited and OnChatRequestComing.
	/// The default max backlog is 64.
	/// </summary>
	bool Run(int nPort, long lEvents);

	/// <summary>
	/// Starts a SocketPro server running on a given port (nPort) with extra events (lEvents) plus default events OnClose, OnAccept, OnIsPermited and OnChatRequestComing.
	/// </summary>
	bool Run(int nPort, long lEvents, int nMaxBacklog);

	/// <summary>
	/// Request one or more global events. Call ths method before starting a SocketPro server for monitoring a set of selected global events by overriding virtual functions.
	/// </summary>
	/// <param name="nEvents">Events could be eOnSwitchTo, eOnIsPermitted, eOnClose, eOnAccept, eOnSSLEvent .....</param>
	void AskForEvents(int nEvents);
	
	/// <summary>
	/// Stop a SocketPro server.
	/// Note that this function does not do anything if you call it from main thread.
	/// </summary>
	virtual void StopSocketProServer();
	
	/// <summary>
	/// Start a secured SocketPro server with SSL23 or TLSv1 using OpenSSL
	/// Call this method before calling the method StartSocketProServer. The parameter EncryptionMethod must be either SSL23 or TLSv1.
	/// </summary>
	static void UseSSL(USOCKETLib::tagEncryptionMethod EncryptionMethod, String^ strCertFile, String^ strPrivateKeyFile);
	
	/// <summary>
	/// Start a secured SocketPro server with MSSSL or MSTLSv1 using MS SSPI
	/// Call this method before calling the method StartSocketProServer. The parameter EncryptionMethod must be either MSSSL or MSTLSv1.
	/// The parameters strSubject, bMachine and bRoot indicate the certificate (strSubject) in a store (bMachine and bRoot).
	/// </summary>
	static void UseSSL(USOCKETLib::tagEncryptionMethod EncryptionMethod, String ^strSubject, bool bMachine, bool bRoot);
	
	/// <summary>
	/// Start a secured SocketPro server with either OpenSSL or MS SSPI with certificate and key file (strPfxFile) protected by a password (strPassword).
	/// Call this method before calling the method StartSocketProServer. 
	/// If the parameter EncryptionMethod is either MSSSL or MSTLSv1, MS SSPI is used. Also, a certificate subject (strSubject) must be specified.
	/// If the parameter EncryptionMethod is either SSL23 or TLSv1, OpenSSL is used. No certificate subject (strSubject) is required.
	/// </summary>
	static void UseSSL(String ^strPfxFile, String ^strPassword, String ^strSubject, USOCKETLib::tagEncryptionMethod EncryptionMethod);

	/// <summary>
	/// Check whether a socket is closed.
	/// The method returns 0, -1, or valid socket handle. 0 or -1 means that the socket hSocket is not available now.
	/// </summary>
	/// <returns>
	/// 0, -1, or valid socket handle. 0 or -1 means that the socket hSocket is not available now.
	/// </returns>
	static int FindClient(int hSocket);
	
	/// <summary>
	/// Get a socket handle from a given zero-based index.
	/// The method returns 0, -1, or valid socket handle. 0 or -1 means that the socket hSocket is not available.
	/// </summary>
	static int GetClient(int nIndex);

	/// <summary>
	/// Post-quit IO completion port or window message pump.
	/// </summary>
	static bool PostQuit();

	static String^ GetPassword(int hSocket);
	static String^ GetUserID(int hSocket);

	/// <summary>
	/// Set a password inside the call CSocketProServer::OnIsPermitted for BlowFish. 
	/// </summary>
	static bool SetPassword(int hSocket, String^ strPassword);
	
	/// <summary>
	/// A collection of methods for accessing SocketPro server chat groups.
	/// </summary>
	ref class PushManager abstract sealed
	{
	public:
		/// <summary>
		/// Add a chat group into a SocketPro server.
		/// </summary>
		/// <returns>
		/// True or false.
		/// </returns>
		/// <param name="nGroupID">A unique non-zero number indicating a group.</param>
		/// <param name="strDescription">An optional text string describing the group.</param>
		static bool AddAChatGroup(int nGroupID, String ^strDescription);

		/// <summary>
		/// Get a description about a given chat group (nGroupID).
		/// </summary>
		static String^ GetAChatGroupDiscription (int nGroupID);
		
		/// <summary>
		/// Find a socket handle within a chat group (nGroupID) from a given zero-based index. 
		/// The method returns a valid socket handle.
		/// </summary>
		static int GetChatterSocket(int nGroupID, int nIndex);
		
		/// <summary>
		/// Get the number of clients joined into a given chat group (nGroupID).
		/// </summary>
		static int GetCountOfChatters(int nGroupID);

		/// <summary>
		/// Get a chat group id from a given zero-based index (nIndex).
		/// </summary>
		static int GetGroupID(int nIndex);

		static property long CountOfChatGroups
		{
			long get()
			{
				if(g_SocketProLoader.XGetCountOfChatGroups)
					return (long)g_SocketProLoader.XGetCountOfChatGroups();
				return (long)g_SocketProLoader.GetCountOfChatGroups();
			}
		}
	};
	
/*
	/// <summary>
	/// Query how many HTTP chatters join one group (ulGroupId)
	/// </summary>
	static int GetCountOfHTTPChatters(int nGroupId);
	
	/// <summary>
	/// Query a HTTP chat id for a zero-based index.
	/// </summary>
	static String^ GetHTTPChatId(int nGroupIds, int nIndex);*/
	ref class HttpPush abstract sealed
	{
	public:
		/// <summary>
		/// Query an array of HTTP chat id strings for a given group id (nGroupId).
		/// </summary>
		static array<String^>^ GetHTTPChatIds(int nGroupId);

		/// <summary>
		/// Query a HTTP chat context
		/// </summary>
		static bool GetHTTPChatContext(String ^strChatId, String ^%strUserID, String ^%strIpAddr, int %nLeaseTime, array<long> ^%GroupIds, int %nTimeout, int %nCountOfMessages);

		static property long CountOfChatGroups
		{
			long get()
			{
				return PushManager::CountOfChatGroups;
			}
		}
	};

public:
	property int Events
	{
		int get()
		{
			int nEvents = 0;
			if(m_pOnAccept != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnAccept;

			if(m_pOnClose != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnClose;

			if(m_pOnSend != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnSend;

			if(m_pOnChatRequestCame != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnChatRequestCame;

			if(m_pOnChatRequestComing != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnChatRequestComing;

			if(m_pOnIsPermitted != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnIsPermitted;

			if(m_pOnSSLEvent != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnSSLEvent;

			if(m_pOnSwitchTo != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnSwitchTo;

			if(m_pOnWinMessage != IntPtr::Zero)
				nEvents += (int)tagEvent::eOnWinMessage;

			return nEvents;
		}
	}
	
	/// <summary>
	/// A set of static attributes to control SocketPro server running enviroment attributes.
	/// </summary>
	ref class Config abstract sealed
	{
	public:
		static property USOCKETLib::tagAuthenticationMethod AuthenticationMethod 
		{
			USOCKETLib::tagAuthenticationMethod get()
			{
				return (USOCKETLib::tagAuthenticationMethod)g_SocketProLoader.GetAuthenticationMethod();
			}
			void set(USOCKETLib::tagAuthenticationMethod am)
			{
				g_SocketProLoader.SetAuthenticationMethod((tagAuthenticationMethod)am);
			}
		}
		
		/// <summary>
		/// Interval time for cleaning a pool of sockets. It is default to 60,000 ms.
		/// </summary>
		static property int CleanPoolInterval
		{
			int get()
			{
				return g_SocketProLoader.GetCleanPoolInterval();
			}
			void set(int nNewInterval)
			{
				g_SocketProLoader.SetCleanPoolInterval(nNewInterval);
			}
		}

		static property USOCKETLib::tagEncryptionMethod DefaultEncryptionMethod
		{
			USOCKETLib::tagEncryptionMethod get()
			{
				return (USOCKETLib::tagEncryptionMethod)g_SocketProLoader.GetDefaultEncryptionMethod();
			}
			void set(USOCKETLib::tagEncryptionMethod em)
			{
				g_SocketProLoader.SetDefaultEncryptionMethod((tagEncryptionMethod)em);
			}
		}
		
		static property bool DefaultZip
		{
			bool get()
			{
				return g_SocketProLoader.GetDefaultZip();
			}
			void set(bool bZip)
			{
				g_SocketProLoader.SetDefaultZip(bZip);
			}
		}

		static property int MaxConnectionsPerClient
		{
			int get()
			{
				return g_SocketProLoader.GetMaxConnectionsPerClient();
			}
			void set(int nMaxConnectionsPerClient)
			{
				g_SocketProLoader.SetMaxConnectionsPerClient(nMaxConnectionsPerClient);
			}
		}
		
		static property int MaxThreadIdleTimeBeforeSuicide
		{
			int get()
			{
				return g_SocketProLoader.GetMaxThreadIdleTimeBeforeSuicide();
			}

			void set(int nMaxThreadIdleTimeBeforeSuicide)
			{
				g_SocketProLoader.SetMaxThreadIdleTimeBeforeSuicide(nMaxThreadIdleTimeBeforeSuicide);
			}
		}

		static property int PingInterval
		{
			int get()
			{
				return g_SocketProLoader.GetPingInterval();
			}
			void set(int nPingInterval)
			{
				g_SocketProLoader.SetPingInterval(nPingInterval);
			}
		}

		static property int RecycleGlobalMemoryInterval
		{
			int get()
			{
				return g_SocketProLoader.GetRecycleGlobalMemoryInterval();
			}
			void set(int nRecycleGlobalMemoryInterval)
			{
				g_SocketProLoader.SetRecycleGlobalMemoryInterval(nRecycleGlobalMemoryInterval);
			}
		}
		/// <summary>
		/// True or falase. 
		/// If the property is true, a client can switch freely among different services after the first authentication.
		/// If the property is false, a client has to provide its user id and password for each of switches.
		/// </summary>
		static property bool SharedAM
		{
			bool get()
			{
				return g_SocketProLoader.GetSharedAM();
			}
			void set(bool bSharedAM)
			{
				g_SocketProLoader.SetSharedAM(bSharedAM);
			}
		}

		static property bool HTTPServerPush
		{
			bool get()
			{
				return g_SocketProLoader.GetHTTPServerPush();
			}
			void set(bool bHTTPServerPush)
			{
				g_SocketProLoader.SetHTTPServerPush(bHTTPServerPush);
			}
		}

		static property int SMInterval
		{
			int get()
			{
				return g_SocketProLoader.GetSMInterval();
			}
			void set(int nSMInterval)
			{
				g_SocketProLoader.SetSMInterval(nSMInterval);
			}
		}
		
		/// <summary>
		/// The max allowed time in ms between a client socket connection and calling IUSocket::SwitchTo.
		/// If a client calls the method IUSocket::SwitchTo is too late, 
		/// a SocketPro server will shut down the socket connection with error ERROR_UNOT_SWITCHED_YET
		/// </summary>
		static property int SwitchTime
		{
			int get()
			{
				return g_SocketProLoader.GetSwitchTime();
			}
			void set(int nSwitchTime)
			{
				g_SocketProLoader.SetSwitchTime(nSwitchTime);
			}
		}
		
		static property int TimerElapse
		{
			int get()
			{
				return g_SocketProLoader.GetTimerElapse();
			}
			void set(int nTimerElapse)
			{
				g_SocketProLoader.SetTimerElapse(nTimerElapse);
			}
		}

		static property bool UseWindowMessagePump
		{
			bool get()
			{
				if(g_SocketProLoader.GetUseWindowMessagePump == NULL)
					return true;
				return g_SocketProLoader.GetUseWindowMessagePump();
			}
			void set(bool bUseWindowMessagePump)
			{
				if(g_SocketProLoader.SetUseWindowMessagePump != NULL)
				{
					g_SocketProLoader.SetUseWindowMessagePump(bUseWindowMessagePump);
				}
			}
		}

	};

	static property int CountOfClients
	{
		int get()
		{
			return g_SocketProLoader.GetCountOfClients();
		}
	}

	static property int ListeningSocketHandle
	{
		int get()
		{
			return g_SocketProLoader.GetListeningSocket();
		}
	}

	static property String^ LocalName
	{
		String^ get()
		{
			WCHAR strData[1025] = {0};
			g_SocketProLoader.GetLocalName(strData, sizeof(strData)/sizeof(WCHAR));
			return gcnew String(strData);
		}
	}
	
	static property int MainThreadID
	{
		int get()
		{
			return g_SocketProLoader.GetMainThreadID();
		}
	}

	static property int Win
	{
		int get()
		{
			return (int)g_SocketProLoader.GetWin();
		}
	}

	static property int LastSocketError
	{
		int get()
		{
			return g_SocketProLoader.GetLastSocketError();
		}
	}

protected:
	//track all of window messages by overriding this function
	virtual bool OnWinMessage(int hWnd, int nMessage, int wParam, int lParam, int dwTime, int nPointX, int nPointY);
	virtual void OnWinMessage(int hWnd, int nMessage, int wParam, int lParam);
	virtual void OnAccept(int hSocket, int nError);
	virtual void OnClose(int hSocket, int nError);
	virtual void OnSend(int hSocket, int nError);
	virtual void OnSwitchTo(int hSocket, int nPrevSvsID, int nCurrSvsID);
	virtual bool OnIsPermitted(int hSocket, int nSvsID);
	virtual void OnSSLEvent(int hSocket, int nWhere, int nRtn);
	virtual void OnChatRequestComing(int hSocketSource, USOCKETLib::tagChatRequestID ChatRequestId, Object ^Param0, Object ^Param1);
	virtual void OnChatRequestCame(int hSocketSource, USOCKETLib::tagChatRequestID ChatRequestId);
	
private:
	void Init();
	void RemoveDelegates();
	void OnCRComing(int hSocketSource, short sRequestID, int nLen);
	void OnCRCame(int hSocketSource, short sRequestID, int nLen);
	static void ThreadProc();

private:
	Thread			^m_Thread;
	HANDLE			m_hEvent;
	UINT			m_nPort;
	UINT			m_unMaxBacklog;
	unsigned long	m_ulEvents;
	static CComAutoCriticalSection	*m_pCS = new CComAutoCriticalSection();

	static CSocketProServer		^m_pSocketProServer;
	DOnAccept					^m_OnAccept;
	IntPtr						m_pOnAccept;
	DOnChatRequestComing		^m_OnChatRequestComing;
	IntPtr						m_pOnChatRequestComing;
	DOnChatRequestCame			^m_OnChatRequestCame;
	IntPtr						m_pOnChatRequestCame;
	DOnClose					^m_OnClose;
	IntPtr						m_pOnClose;
	DOnIsPermitted				^m_OnIsPermitted;
	IntPtr						m_pOnIsPermitted;
	DOnSwitchTo					^m_OnSwitchTo;
	IntPtr						m_pOnSwitchTo;
	DOnSend						^m_OnSend;
	IntPtr						m_pOnSend;
	DOnSSLEvent					^m_OnSSLEvent;
	IntPtr						m_pOnSSLEvent;
	DOnWinMessage				^m_OnWinMessage;
	IntPtr						m_pOnWinMessage;
};

}; //namespace SocketProAdapter
#endif
}; //namespace SocketProAdapter

#endif
