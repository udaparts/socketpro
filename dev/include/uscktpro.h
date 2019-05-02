// This is a part of the SocketPro package.
// Copyright (C) 2000-2005 UDAParts 
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

#ifndef ___U_SOCKET_PRO_H____
#define ___U_SOCKET_PRO_H____

#ifdef __cplusplus
extern "C" {
#endif

#define	SOCKET_NOT_FOUND		(0xFFFFFFFF)
#define	LEN_NOT_AVAILABLE		(0xFFFFFFFF)
#define REQUEST_CANCELED		(0xFFFFFFFE)
#define RETURN_DATA_INTERCEPTED	(0xFFFFFFFD)
#define RETURN_RESULT_RANDOM	((unsigned short)0x4000)

//your window message can NOT be in the following range
#define WM_SOCKETPRO_RESERVED_MIN		(WM_USER + 0x0401)
#define WM_SOCKETPRO_RESERVED_MAX		(WM_USER + 0x06FF)

#define WM_SOCKET_SVR_NOTIFY			(WM_USER + 0x0480)
#define	WM_ASK_FOR_PROCESSING			(WM_USER + 0x0500)
#define	WM_REQUEST_PROCESSED			(WM_USER + 0x0501)
#define	WM_WORKER_THREAD_DYING			(WM_USER + 0x0502)
#define	WM_SET_PROCESSING_FUNC			(WM_USER + 0x0503)
#define WM_SET_PRETRANS_FUNC			(WM_USER + 0x0504)
#define	WM_CANCEL_REQUEST				(WM_USER + 0x0505)
#define	WM_CONTINUE_PROCESSING			(WM_USER + 0x0506)

//Error codes 
#define ERROR_UNO_ERROR							(0x000)
#define ERROR_UWRONG_SWITCH						(0x100)
#define ERROR_UAUTHENTICATION_FAILED			(0x101)
#define ERROR_USERVICE_NOT_FOUND				(0x102)
#define ERROR_UBLOWFISH_KEY_WRONG				(0x103)
#define ERROR_UNOT_SWITCHED_YET					(0x104)	
#define ERROR_VERIFY_SALT						(0x105)	
#define ERROR_BAD_HTTP_SIZE						(0x106)
#define ERROR_BAD_HTTP_REQUEST					(0x107)
#define ERROR_UEXCEPTION_CAUGHT					(0x108)
#define ERROR_REQUEST_TOO_LARGE					(0x109)
#define ERROR_UNKNOWN_REQUEST					(0x10A)

//SSL error codes		


//used with listening socket
typedef void (CALLBACK *POnWinMessage) (HWND hWnd, UINT uMsg, WPARAM wParam,  LPARAM lParam);
typedef void (CALLBACK *POnCleanPool) (unsigned long ulSvsID, unsigned long ulTickCount);
typedef void (CALLBACK *POnReceive) (unsigned int hSocket, int nError);
typedef void (CALLBACK *POnSend) (unsigned int hSocket, int nError);
typedef void (CALLBACK *POnSSLEvent) (unsigned int hSocket, int nWhere, int nRtn);
typedef bool (CALLBACK *POnSendReturnData) (unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer);
typedef void (CALLBACK *PTHREAD_STARTED) ();
typedef void (CALLBACK *PTHREAD_DYING) ();
typedef BOOL (CALLBACK *PPRETRANSLATEMESSAGE) (MSG* pMsg);
typedef void (CALLBACK *POnThreadCreated) (unsigned long ulSvsID, unsigned long ulThreadID, unsigned int hSocket);


enum enumThreadApartment
{
	taNone = 0, //no COM objet is involved
	taApartment, //thread will enter a STA apartment
	taFree, //thread will enter MTA apartment
};

enum enumHTTPRequest
{
	hrUnknown = 0,
	hrGet,
	hrPost,
	hrPut,
	hrDelete,
	hrOptions,
	hrTrace,
	hrHead,
	hrConnect,
};

struct CSvsContext
{
	//called within main thread
	POnIsPermitted			m_OnIsPermitted;		//for authentication
	POnSwitchTo				m_OnSwitchTo;			//called when a service is switched
	POnFastRequestArrive	m_OnFastRequestArrive;  //request processed within main thread
	POnBaseRequestCame		m_OnBaseRequestCame;	//SocketPro defines a set of base requests
	POnCleanPool			m_OnCleanPool;			//Clean objects in pool
	POnRequestProcessed		m_OnRequestProcessed;	//called when a slow request processed

	POnClose				m_OnClose;				//native socket event
	POnReceive				m_OnReceive;			//native socket event
	POnSend					m_OnSend;				//native socket event
	
	//called within worker thread
	enumThreadApartment		m_enumTA;				//required with a worker thread							
	PSLOW_PROCESS			m_SlowProcess;			//required with a worker thread	
	PPRETRANSLATEMESSAGE	m_PreTranslateMessage;	//Optional
	PTHREAD_STARTED			m_ThreadStarted;		//Optional
	PTHREAD_DYING			m_ThreadDying;			//Optional

	POnSendReturnData		m_OnSendReturnData;		//
	POnThreadCreated		m_OnThreadCreated;		//Called within main thread
};

//Set a set of global callbacks
void WINAPI SetOnSSLEvent(POnSSLEvent pOnSSLEvent);
void WINAPI SetOnWinMessage(POnWinMessage pOnWinMessage);
void WINAPI SetOnSend(POnSend pOnSend);

/*
	Get a hidden window handle that is used to monitor socket events.
	A Socketpro server just has one hidden window only.
*/
HWND WINAPI GetWin();

//Get main thread id
unsigned long WINAPI GetMainThreadID();

//Same as WSAGetLastError
int WINAPI GetLastSocketError();

//Interval time for cleaning pool. See the callback POnCleanPool
void WINAPI SetCleanPoolInterval(unsigned long ulCleanPoolInterval);
unsigned long WINAPI GetCleanPoolInterval();

//use the method to query an OpenSSL method according to a given name
FARPROC GetOpenSSLProcAddress(const wchar_t *strOpenSSLFuncName);
//void* WINAPI GetSSL(unsigned int hSocket);

//plug and unplug a dll that may contain one or more services
bool WINAPI RemoveADll(const wchar_t *strLibFile);
unsigned long WINAPI GetCountOfLibraries();
HINSTANCE WINAPI GetADll(unsigned long ulIndex);


//APIs for managing services
bool WINAPI ReplaceSvsContext(unsigned long ulSvsID, CSvsContext SvsContext);
unsigned long WINAPI GetServiceID(unsigned long ulIndex);

/*
	Query a slow request id for a service ulSvsID.
	usIndex is a zero-based index.
*/
unsigned short WINAPI GetSlowRequest(unsigned long ulSvsID, unsigned short usIndex);

//The following nineteen methods are used for chat service
void WINAPI SetGroupsNotifiedWhenEntering(unsigned long ulGroupsNotifiedWhenEntering);
void WINAPI SetGroupsNotifiedWhenExiting(unsigned long ulGroupsNotifiedWhenExiting);
unsigned long WINAPI GetGroupsNotifiedWhenEntering();
unsigned long WINAPI GetGroupsNotifiedWhenExiting();
unsigned long WINAPI XGetCountOfChatGroups();

/*
	Get a group id from a given zero-based index.
*/
unsigned long WINAPI GetGroupID(unsigned char bIndex);
unsigned long WINAPI XGetGroupID(unsigned long ulIndex);

//Query how many non-HTTP chatters join the group ulGroupID
unsigned long WINAPI GetCountOfChatters(unsigned long ulGroupID);

//Query a non-HTTP chat socket handle for a zero-based index.
unsigned int WINAPI GetChatterSocket(unsigned long ulGroupID, unsigned long ulIndex);

/*
	hSocket always cooresponds to a client socket connection, and is unique at any time
	SocketPro uses hSocket as a key index.
*/
unsigned long WINAPI GetJoinedGroup(unsigned int hSocket);

//Send a private message to a client identified by hSocketTarget
bool WINAPI SpeakTo(unsigned int hSocket, const VARIANT *pvtMsg, unsigned int hSocketTarget);
bool WINAPI SpeakToEx(unsigned int hSocket, const unsigned char *pMessage, unsigned long ulLen, unsigned int hSocketTarget);

//Send a message to one or more groups of clients
bool WINAPI XSpeak(unsigned int hSocket, const VARIANT *pvtMsg, const VARIANT *pvtGroups);
bool WINAPI XSpeakEx(unsigned int hSocket, const unsigned char *pMessage, unsigned long ulLen, unsigned long ulGroupCount, const unsigned long *pGroups);

/*
	Join or leave chat groups.
	A client can join multiple groups at one time.
*/
bool WINAPI XEnter(unsigned int hSocket, const VARIANT *pvtGroups);

/*
	Find a client. If the client exists, it will return a socket handle equal to hSocket.
	If it doesn't, it will return an invalid handle 0 or -1.
*/
unsigned int WINAPI FindClient(unsigned int hSocket);


bool WINAPI ContinueProcessing(unsigned int hSocket, unsigned short usRequestID);
void WINAPI ResetBytesIn(unsigned int hSocket, unsigned long ulStart = 0, unsigned long *pulHigh = 0);
void WINAPI ResetBytesOut(unsigned int hSocket, unsigned long ulStart = 0, unsigned long *pulHigh = 0);
unsigned long WINAPI GetBytesIn(unsigned int hSocket, unsigned long *pulHigh = 0);
unsigned long WINAPI GetBytesOut(unsigned int hSocket, unsigned long *pulHigh = 0);
unsigned long WINAPI GetLastSndTime(unsigned int hSocket);
unsigned long WINAPI GetLastRcvTime(unsigned int hSocket);
unsigned long WINAPI GetSndBufferSize(unsigned int hSocket);
unsigned long WINAPI GetRcvBufferSize(unsigned int hSocket);
unsigned long WINAPI GetTotalMemory(unsigned int hSocket);
bool WINAPI GetInterfaceAttributes(unsigned int hSocket, unsigned long *pulMTU, unsigned long *pulMaxSpeed, unsigned long *pulType, unsigned long *pulMask);
void WINAPI ShrinkMemory(unsigned int hSocket);
unsigned long WINAPI GetCountOfMySpecificBytes(unsigned int hSocket);
unsigned long WINAPI GetMySpecificBytes(unsigned int hSocket, unsigned char* pBuffer, unsigned long ulLen);
bool WINAPI SetBlowFish(unsigned int hSocket, unsigned char bKeyLen, unsigned char *strKey);

bool WINAPI SetEncryptionMethod(unsigned int hSocket, tagEncryptionMethod EncryptionMethod);
tagEncryptionMethod WINAPI GetEncryptionMethod(unsigned int hSocket);

/*
	A property indicating if both client and server share the same endian (byte order).
	Note that this property is NOT implemented yet.
*/
bool WINAPI IsSameEndian(unsigned int hSocket);

/*
	Get the current thread id associated with a socket session.
*/
unsigned long WINAPI GetAssociatedThreadID(unsigned int hSocket);

/*
	Query an associated socket handler within a worker thread. Don't call the function in main thread.
*/
unsigned int WINAPI GetAssociatedSocket();

/*
	Retrieve an array of request ids in a pre-allocated buffer (pusRequestID) with a given size ulSize to be processed.

	The function returns the actual number of request ids
*/
unsigned long WINAPI GetRequestIDsInQueue(unsigned int hSocket, unsigned short *pusRequestID, unsigned long ulSize);

/*
	?????
*/
void WINAPI Register(unsigned int hSocket, unsigned char *pbLock, unsigned char *pbKey);

/*
	Clean the track of all of meory queues on both client and server sides	
*/
void WINAPI CleanTrack(unsigned int hSocket);

/*
	Tells SocketProServer what and where certificate and its private key will be used with MS SSPI.
*/
void WINAPI UseMSCert(bool bMachine, bool bRoot, const wchar_t *strCertSubject);


/*
	Use or disable window message pump to drive socket events.
*/
bool WINAPI SetUseWindowMessagePump(bool bUseWindowMessagePump);

/*
	Check if window message pump is used to drive socket events
*/
bool WINAPI GetUseWindowMessagePump();

/*
	Start IO completion port pump
*/
bool WINAPI StartIOPump();

bool WINAPI IsClosing(unsigned int hSocket);

bool WINAPI SetHTTPResponseHeader(unsigned int hSocket, const char *strUTF8Header, const char *strUTF8HeaderValue);
void WINAPI SetHTTPResponseCode(unsigned int hSocket, unsigned int nHTTPResponseCode);

//The following two functions work for both HTTP and non-HTTP services
void WINAPI SetHTTPMaxMessageSize(unsigned int hSocket, unsigned int nNewMax);
unsigned int WINAPI GetHTTPMaxMessageSize(unsigned int hSocket);

void WINAPI SetHTTPAutoPartition(unsigned int hSocket, bool bAutoPartition);
bool WINAPI GetHTTPAutoPartition(unsigned int hSocket);

void WINAPI SetHTTPServerPush(bool bEnableHTTPServerPush);
bool WINAPI GetHTTPServerPush();
const wchar_t* WINAPI HTTPEnter(unsigned int hSocket, unsigned long ulGroups, unsigned long dwLeaseTime, const wchar_t *strIpAddr);
bool WINAPI HTTPExit(unsigned int hSocket, const wchar_t* strPushSessionId);
bool WINAPI HTTPSendUserMessage(unsigned int hSocket, const wchar_t* strPushSessionId, const wchar_t* strUserID, const VARIANT *pvtMsg);
bool WINAPI HTTPSpeak(unsigned int hSocket, const wchar_t* strPushSessionId, const VARIANT *pvtMsg, unsigned long ulGroups);
bool WINAPI HTTPSubscribe(unsigned int hSocket, const wchar_t* strPushSessionId, unsigned long ulTimeout, const wchar_t *strCrossSiteJSCallback);

//Query how many HTTP chatters join one group (ulGroupId)
unsigned long WINAPI GetCountOfHTTPChatters(unsigned long ulGroupId);

//Query a HTTP chat id for a zero-based index.
const wchar_t* WINAPI GetHTTPChatId(unsigned long ulGroupId, unsigned long ulIndex);

//Query a HTTP chat context
bool WINAPI GetHTTPChatContext(const wchar_t *strChatId, wchar_t **pstrUserID, wchar_t **pstrIpAddr, unsigned long *pLeaseTime, VARIANT *pvtGroups, unsigned long *pTimeout, unsigned long *pCountOfMessages);

void WINAPI DropRequestResult(unsigned int hSocket, unsigned short usRequestId);

bool WINAPI XHTTPSpeak(unsigned int hSocket, const wchar_t* strPushSessionId, const VARIANT *pvtMsg, const VARIANT *pvtGroups);

const wchar_t* WINAPI XHTTPEnter(unsigned int hSocket, const VARIANT *pvtGroups, unsigned long dwLeaseTime, const wchar_t *strIpAddr);

const VARIANT* WINAPI GetHTTPJoinedGroups(unsigned int hSocket, const wchar_t *strChatSession);


#ifdef __cplusplus
}
#endif

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2005 UDAParts 
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