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

/*
#define	SOCKET_NOT_FOUND		(0xFFFFFFFF)
#define	LEN_NOT_AVAILABLE		(0xFFFFFFFF)
#define REQUEST_CANCELED		(0xFFFFFFFE)
#define RETURN_DATA_INTERCEPTED	(0xFFFFFFFD)
*/

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
/*
#define ERROR_UNO_ERROR							(0x000)
#define ERROR_UWRONG_SWITCH						(0x100)
#define ERROR_UAUTHENTICATION_FAILED			(0x101)
#define ERROR_USERVICE_NOT_FOUND				(0x102)
#define ERROR_UBLOWFISH_KEY_WRONG				(0x103)
#define ERROR_UNOT_SWITCHED_YET					(0x104)	
#define ERROR_VERIFY_SALT						(0x105)	
#define ERROR_BAD_HTTP_SIZE						(0x106)
#define ERROR_BAD_HTTP_REQUEST					(0x107)	*/
//SSL error codes		


//used with listening socket
typedef void (CALLBACK *POnChatRequestComing) (unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen);
typedef void (CALLBACK *POnChatRequestCame) (unsigned int hSocketSource, unsigned short usRequestID, unsigned long ulLen);
typedef void (CALLBACK *POnWinMessage) (HWND hWnd, UINT uMsg, WPARAM wParam,  LPARAM lParam);
typedef void (CALLBACK *POnAccept) (unsigned int hSocket, int nError);

typedef void (CALLBACK *POnCleanPool) (unsigned long ulSvsID, unsigned long ulTickCount);

//used by client-peer socket
typedef void (CALLBACK *POnClose) (unsigned int hSocket, int nError);
typedef void (CALLBACK *POnReceive) (unsigned int hSocket, int nError);
typedef void (CALLBACK *POnSend) (unsigned int hSocket, int nError);
typedef void (CALLBACK *POnSSLEvent) (unsigned int hSocket, int nWhere, int nRtn);
typedef void (CALLBACK *POnBaseRequestCame) (unsigned int hSocket, unsigned short usRequestID);
typedef void (CALLBACK *POnRequestProcessed) (unsigned int hSocket, unsigned short usRequestID);
typedef void (CALLBACK *POnFastRequestArrive) (unsigned int hSocket, unsigned short usRequestID, unsigned long ulLen);
typedef bool (CALLBACK *POnIsPermitted) (unsigned int hSocket, unsigned long ulSvsID);
typedef void (CALLBACK *POnSwitchTo) (unsigned int hSocket, unsigned long ulPrevSvsID, unsigned long ulCurrSvsID);

typedef bool (CALLBACK *POnSendReturnData) (unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char *pBuffer);

//used within a worker thread
typedef void (CALLBACK *PTHREAD_STARTED) ();
typedef HRESULT (CALLBACK *PSLOW_PROCESS) (unsigned short usRequestID, unsigned long ulLen, unsigned int hSocket);
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

//creating plugable and reusable standard window DLL
typedef bool (WINAPI *PInitServerLibrary) (int nParam);
typedef void (WINAPI *PUninitServerLibrary) ();
typedef unsigned short (WINAPI *PGetNumOfServices) ();
typedef unsigned long (WINAPI *PGetAServiceID) (unsigned short usIndex);
typedef CSvsContext (WINAPI *PGetOneSvsContext) (unsigned long ulSvsID);
typedef unsigned short (WINAPI *PGetNumOfSlowRequests) (unsigned long ulSvsID);
typedef unsigned short (WINAPI *PGetOneSlowRequestID) (unsigned long ulSvsID, unsigned long ulIndex);

/*
	Initialize the library. You must call the function first before calling others.
	nParam is ignored at this time.

	After calling the function, you need calling UninitSocketProServer at the very end.
*/
bool WINAPI InitSocketProServer(int nParam);
void WINAPI UninitSocketProServer();

/*
	start and stop a SocketPro server at a port (uiPort)

	Refer to the winsock function --

	int listen(	SOCKET s,
				int backlog
	);
*/
bool WINAPI StartSocketProServer(unsigned int uiPort, unsigned int uiMaxBacklog = 64); 
void WINAPI StopSocketProServer();

/*
	Check if the current request is already canceled from a client by calling IUSocket::Cancel
	You can call the method within a worker thread not main thread.
*/
bool WINAPI IsCanceled();

//Set a set of global callbacks
void WINAPI SetOnSSLEvent(POnSSLEvent pOnSSLEvent);
void WINAPI SetOnWinMessage(POnWinMessage pOnWinMessage);
void WINAPI SetOnAccept(POnAccept pOnAccept);
void WINAPI SetOnSwitchTo(POnSwitchTo pOnSwitchTo);
void WINAPI SetOnIsPermitted(POnIsPermitted pOnIsPermitted);
void WINAPI SetOnClose(POnClose pOnClose);
void WINAPI SetOnSend(POnSend pOnSend);

//amOwn, amMix, and amIntergrated, 
unsigned long WINAPI GetAuthenticationMethod();
void WINAPI SetAuthenticationMethod(unsigned long ulAuthMethod);

/*
	A socket connection may support multiple switchable services.
	To avoid authentication for every switching, you can set SharedAM to true.
*/
void WINAPI SetSharedAM(bool bShared);
bool WINAPI GetSharedAM();

/*
	Query socket handle of a listening socket.
	Don't confuse with socket handles of client peer sockets.
	One SocketPro server just has one listening socket only
	but it manages many of client peer sockets.
	Each of client peer sockets represents a socket connection/session.
*/
unsigned int WINAPI GetListeningSocket();

/*
	Switch time determining the maximun number of time 
	that a client can stay with the startup service (sidStartup)
	Usually it is 60,000 ms
	If a socket stays with the service longer than it, 
	the socket connection will automatically closed with an error code uecClientRejected
*/
unsigned long WINAPI GetSwitchTime();
void WINAPI SetSwitchTime(unsigned long ulSwitchTime);

//Query the number of socket connections
unsigned int WINAPI GetCountOfClients();

//Get a socket handle by a zero-based index
unsigned int WINAPI GetClient(unsigned int uiIndex);

/*
	Get a hidden window handle that is used to monitor socket events.
	A Socketpro server just has one hidden window only.
*/
HWND WINAPI GetWin();

//Get main thread id
unsigned long WINAPI GetMainThreadID();

//Same as WSAGetLastError
int WINAPI GetLastSocketError();

/*
	Encryption method
	NoEncryption	= 0,
	BlowFish	= 1,
	SSL23	= 2,
	MSSSL = 3,
	TLSv1	= 4,
	MSTLSv1 = 5,

	Note UDAParts may add more encryption methods in the future.
*/
void WINAPI SetDefaultEncryptionMethod(tagEncryptionMethod EncryptionMethod);
tagEncryptionMethod WINAPI GetDefaultEncryptionMethod();

/*
	Turn on/off default online compressiong
	Note that you can overwrite it at any time at either client or server side.
*/
void WINAPI SetDefaultZip(bool bZip);
bool WINAPI GetDefaultZip();

/*
	All of worker threads except STA threads can suicide after they are idle over a limited time.
	By default, the time limit is 60,000 ms.
*/
void WINAPI SetMaxThreadIdleTimeBeforeSuicide(unsigned long ulMaxThreadIdleTimeBeforeSuicide);
unsigned long WINAPI GetMaxThreadIdleTimeBeforeSuicide();

/*
	The maximun of socket connections can be made from a machine
	By default, it is 32.
	It is designed for preventing Denial of Sevice attack
*/
void WINAPI SetMaxConnectionsPerClient(unsigned long ulMaxConnectionsPerClient);
unsigned long WINAPI GetMaxConnectionsPerClient();

/*
	A value indicating the elapse time of an internal timer. See the window function --

  UINT SetTimer(HWND hWnd,              // handle of window for timer messages
				UINT nIDEvent,          // timer identifier
				UINT uElapse,           // time-out value
				TIMERPROC lpTimerFunc   // address of timer procedure
				);

	By default, it is 1,000 ms or 1 second.
*/
void WINAPI SetTimerElapse(unsigned long ulTimerElapse);
unsigned long WINAPI GetTimerElapse(); 

/*
	A property indicating a time interval for shrinking memory 
	if a socket session doesn't detect any data movement.
*/
unsigned long WINAPI GetSMInterval();
void WINAPI SetSMInterval(unsigned long ulSMInterval); 

/*
	Ping interval time.
	It is 60,000 ms by default.
	Note that if a socket connection has data movement, it will no be pinged
*/
void WINAPI SetPingInterval(unsigned long ulPingInterval);
unsigned long WINAPI GetPingInterval();

/*
	Interval time for recycling global memory
	It is 1,200,000 ms by default
*/
void WINAPI SetRecycleGlobalMemoryInterval(unsigned long ulRecycleGlobalMemoryInterval);
unsigned long WINAPI GetRecycleGlobalMemoryInterval();

//Interval time for cleaning pool. See the callback POnCleanPool
void WINAPI SetCleanPoolInterval(unsigned long ulCleanPoolInterval);
unsigned long WINAPI GetCleanPoolInterval();

/*
	used by OpenSSL. 
	When using SSL23 or TLSv1 to secure SocketPro, 
	you must call the two methods before starting SocketPro server
*/
void WINAPI SetCertFile(const wchar_t *strCertFile);	
void WINAPI SetPrivateKeyFile(const wchar_t *strPrivateKeyFile);

//use the method to query an OpenSSL method according to a given name
FARPROC GetOpenSSLProcAddress(const wchar_t *strOpenSSLFuncName);
//void* WINAPI GetSSL(unsigned int hSocket);

//plug and unplug a dll that may contain one or more services
HINSTANCE WINAPI AddADll(const wchar_t *strLibFile, int nParam);
bool WINAPI RemoveADll(const wchar_t *strLibFile);
unsigned long WINAPI GetCountOfLibraries();
HINSTANCE WINAPI GetADll(unsigned long ulIndex);
bool WINAPI RemoveADllByHandle(HINSTANCE hInstance);

//APIs for managing services
bool WINAPI AddSvsContext(unsigned long ulSvsID, CSvsContext SvsContext);
CSvsContext WINAPI GetSvsContext(unsigned long ulSvsID);
bool WINAPI RemoveASvsContext(unsigned long ulSvsID);
bool WINAPI ReplaceSvsContext(unsigned long ulSvsID, CSvsContext SvsContext);
unsigned long WINAPI GetCountOfServices();
unsigned long WINAPI GetServiceID(unsigned long ulIndex);
bool WINAPI AddSlowRequest(unsigned long ulSvsID, unsigned short usRequestID);
bool WINAPI RemoveSlowRequest(unsigned long ulSvsID, unsigned short usRequestID);
unsigned short WINAPI GetCountOfSlowRequests(unsigned long ulSvsID);
void WINAPI RemoveAllSlowRequests(unsigned long ulSvsID);

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

//set callback for monitoring chatting
void WINAPI SetOnChatRequestComing(POnChatRequestComing pOnChatRequestComing);
void WINAPI SetOnChatRequestCame(POnChatRequestCame pOnChatRequestCame);

/*
	Note that chat group id must be 1, 2, 4, 8, 16, .....
	A SocketPro can support 32 chat groups at most.
	Each of chat groups has a simple discription.
*/
bool WINAPI AddAChatGroup(unsigned long ulGroupID, const wchar_t *strDescription);

/*
	Get a chat group discription according to a given buffer size (strDescription & ulChars) for a given group ulGroupID. 
	It returns the numbers of chars.
*/
unsigned long WINAPI GetAChatGroup(unsigned long ulGroupID, wchar_t *strDescription, unsigned long ulChars);
unsigned char WINAPI GetCountOfChatGroups();

/*
	Get a group id from a given zero-based index.
*/
unsigned long WINAPI GetGroupID(unsigned char bIndex);

//Query how many chatters join the group ulGroupID
unsigned long WINAPI GetCountOfChatters(unsigned long ulGroupID);

//Query a chat socket handle for a zero-based index.
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
bool WINAPI Speak(unsigned int hSocket, const VARIANT *pvtMsg, unsigned long ulGroups = odAllGroups);
bool WINAPI SpeakEx(unsigned int hSocket, const unsigned char *pMessage, unsigned long ulLen, unsigned long ulGroups = odAllGroups);

/*
	Join or leave chat groups.
	A client can join multiple groups at one time.
*/
bool WINAPI Enter(unsigned int hSocket, unsigned long ulGroups);
bool WINAPI Exit(unsigned int hSocket);

//Get an OpenSSL handle
void* WINAPI GetSSL(unsigned int hSocket);

/*
	Find a client. If the client exists, it will return a socket handle equal to hSocket.
	If it doesn't, it will return an invalid handle 0 or -1.
*/
unsigned int WINAPI FindClient(unsigned int hSocket);

//Close a socket connection lazily
bool WINAPI PostClose(unsigned int hSocket, unsigned short usError);

/*
	Retrieve a request. 
	If bPeek is false, the internal buffer is cleaned. Otherwise, it is not cleaned.
*/
unsigned long WINAPI RetrieveBuffer(unsigned int hSocket, unsigned long ulLen, unsigned char* pBuffer, bool bPeek = false);


/*
	Send a return result to a client for a request usRequestId.

	Possble returned value could be 
		1.	ulLen -- data are put into a sending queue normally,
		2.	SOCKET_NOT_FOUND -- hSocket is closed or closing,
		3.	REQUEST_CANCELED -- current request is canceled by calling IUSocket::Cancel from client
		4.	RETURN_DATA_INTERCEPTED -- data is intercepted. 

	Note that you should check the return value when sending a large size of data to a remote client.
	When finding the return value is either SOCKET_NOT_FOUND or REQUEST_CANCELED, you should stop sending data.
*/
unsigned long WINAPI SendReturnData(unsigned int hSocket, unsigned short usRequestId, unsigned long ulLen, const unsigned char* pBuffer);

/*
	Batching return results at server side.
	If successful, they all return true. Otherwise, they return false.
*/
bool WINAPI StartBatching(unsigned int hSocket);
bool WINAPI CommitBatching(unsigned int hSocket);
bool WINAPI AbortBatching(unsigned int hSocket);

//check if SocketPro is batching return results for the socket coonection hSocket.
bool WINAPI IsBatching(unsigned int hSocket);

/*
	Query the batched return results queued in byte.
*/
unsigned long WINAPI GetBytesBatched(unsigned int hSocket);

/*
	?????
*/
bool WINAPI ContinueProcessing(unsigned int hSocket, unsigned short usRequestID);

/*
	??????
*/
void WINAPI ResetBytesIn(unsigned int hSocket, unsigned long ulStart = 0, unsigned long *pulHigh = 0);
void WINAPI ResetBytesOut(unsigned int hSocket, unsigned long ulStart = 0, unsigned long *pulHigh = 0);

/*
	Query the numbers of bytes sent and received since a socket connection is establisehed.
*/
unsigned long WINAPI GetBytesIn(unsigned int hSocket, unsigned long *pulHigh = 0);
unsigned long WINAPI GetBytesOut(unsigned int hSocket, unsigned long *pulHigh = 0);

/*
	Get the input parameters data length of current request in byte.
*/
unsigned long WINAPI GetCurrentRequestLen(unsigned int hSocket);

/*
	Get the current request id.
*/
unsigned short WINAPI GetCurrentRequestID(unsigned int hSocket);

/*
	Get the last time tick that a SocketPro server sends data to a client.
*/
unsigned long WINAPI GetLastSndTime(unsigned int hSocket);

/*
	Get the last time tick that a SocketPro server receives data from a client.
*/
unsigned long WINAPI GetLastRcvTime(unsigned int hSocket);

/*
	Get the queue buffer size for sending data.
*/
unsigned long WINAPI GetSndBufferSize(unsigned int hSocket);

/*
	Get the queue buffer size for receiving data.
*/
unsigned long WINAPI GetRcvBufferSize(unsigned int hSocket);

/*
	Get the data size in byte remaining in receiving queue.
*/
unsigned long WINAPI GetSndBytesInQueue(unsigned int hSocket);

/*
	Get the data size in byte remaining in sending queue.
*/
unsigned long WINAPI GetRcvBytesInQueue(unsigned int hSocket);

/*
	Get a service id for the socket connection hSocket.
*/
unsigned long WINAPI GetSvsID(unsigned int hSocket);

/*
	Get the memory consumed by the socket connection hSocket in byte.
	If the memory is consumed too much, you can call ShrinkMemory to decrease it.
*/
unsigned long WINAPI GetTotalMemory(unsigned int hSocket);

/*
	Get the number of requests to be processed in queue.
*/
unsigned long WINAPI QueryRequestsInQueue(unsigned int hSocket);

/*
	Get network card or modem (interface) attributes.
			
	pulMTU -- receives Maximum Transmission Unit (MTU) like 1500
	pulMaxSpeed -- receives the speed of the interface in bits per second (bandwidth) like 10,000,000, 100,000,000 mbps.
					Note that actual bandwidth is determined by the slowest device on the path, 
					and is always less than or equal to the value.
	pulIType -- receives the type of device. It can be one of valuses in the enum tagInterfaceType.
	pulMask -- receives network mask string like "255.255.255.0".
*/
bool WINAPI GetInterfaceAttributes(unsigned int hSocket, unsigned long *pulMTU, unsigned long *pulMaxSpeed, unsigned long *pulType, unsigned long *pulMask);

/*
	Decrease the memory required by the socket session hSocket.
*/
void WINAPI ShrinkMemory(unsigned int hSocket);

/*
	Retrieve the specific bytes.

	A client can send an array of specific bytes data to a remote SocketPro server by calling IUSocket::SendMySpecificBytes.
*/
unsigned long WINAPI GetCountOfMySpecificBytes(unsigned int hSocket);
unsigned long WINAPI GetMySpecificBytes(unsigned int hSocket, unsigned char* pBuffer, unsigned long ulLen);

/*
	Set a key for Blowfish. The length is not over 56.
*/
bool WINAPI SetBlowFish(unsigned int hSocket, unsigned char bKeyLen, unsigned char *strKey);

/*
	Encryption method:

	enum tagEncryptionMethod
	{
		NoEncryption=0,
		BlowFish = 1,
		SSL23 = 2,
		MSSSL = 3,
		TLSv1 = 4,
		MSTLSv1 = 5,
	};
*/
bool WINAPI SetEncryptionMethod(unsigned int hSocket, tagEncryptionMethod EncryptionMethod);
tagEncryptionMethod WINAPI GetEncryptionMethod(unsigned int hSocket);

/*
	Turn on/off online compressing
*/
bool WINAPI SetZip(unsigned int hSocket, bool bZip);
bool WINAPI GetZip(unsigned int hSocket);

/*
	Get a user id and return the length of user id in char. 
	Note that user id will not be over 256 chars.

	ulCharLen indicating the buffer size of strUID in char.
*/
unsigned long WINAPI GetUID(unsigned int hSocket, wchar_t *strUID, unsigned long ulCharLen);

/*
	Get a password and return the length of password in char. 
	Note that password will not be over 256 chars.

	ulCharLen indicating the buffer size of strPassword in char.

	Note that you can call the function to get a password 
	ONLY within the global callback OnIsPermitted when Authentication method is set to either amMixed and amOwn.
*/
unsigned long WINAPI GetPassword(unsigned int hSocket, wchar_t *strPassword, unsigned long ulCharLen);

/*
	?????
*/
unsigned long WINAPI GetTimeOut(unsigned int hSocket);
bool WINAPI SetTimeOut(unsigned int hSocket, unsigned long ulTimeOut);

/*
	A property contains five parameters.
	Note that you can get five parameters, but set the last three parameters only.
	Also, see the structure CSwitchInfo.
*/
bool WINAPI GetClientInfo(unsigned int hSocket, CSwitchInfo *pClientInfo);
bool WINAPI GetServerInfo(unsigned int hSocket, CSwitchInfo *pServerInfo);
bool WINAPI SetServerInfo(unsigned int hSocket, CSwitchInfo *pServerInfo);

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
	Retrieve the local name like 111.222.101.211 for a socket. Refer to winsock function --
	int getsockname(	SOCKET s,                    
						struct sockaddr FAR*  name,  
						int FAR*  namelen            
					);

	Note that you should allocate a large enough buffer strIPAddrBuffer (shouldn't less than 16 chars) to receive the IP address
*/
bool WINAPI GetSockAddr(unsigned int hSocket, unsigned int *pnSockPort, wchar_t *strIPAddrBuffer, unsigned short usChars);

/*
	Retrieve the name of the peer like 111.222.101.211 to which a socket is connected. Refer to winsock function --
	int getpeername(SOCKET s,                    
					struct sockaddr FAR*  name,  
					int FAR*  namelen            
					);

	Note that you should allocate a large enough buffer strIPAddrBuffer (shouldn't less than 16 chars) to receive the IP address
*/
bool WINAPI GetPeerName(unsigned int hSocket, unsigned int *pnPeerPort, wchar_t *strPeerAddr, unsigned short usChars);

/*
	Retrieve an array of request ids in a pre-allocated buffer (pusRequestID) with a given size ulSize to be processed.

	The function returns the actual number of request ids
*/
unsigned long WINAPI GetRequestIDsInQueue(unsigned int hSocket, unsigned short *pusRequestID, unsigned long ulSize);

/*
	Get the standard host name (strLocalName) for the local machine with a given size (usChars) in char. Refer to winsock function --
	int gethostname(	char FAR * name,  
					int namelen       
				);

	Note that you should pre-allocate a large size enough of buffer to hold the name.
*/
bool WINAPI GetLocalName(wchar_t *strLocalName, unsigned short usChars);

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
	Secure SocketProServer with MS SSPI or OpenSSL using ceriticate and private key stored in a pfx file,
*/
void WINAPI SetPfxFile(const wchar_t *strPfxFile, const wchar_t *strPassword);

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

/*
	Post-quit SocketPro internal message pump or window message pump
*/
bool WINAPI PostQuitPump(unsigned long ulThreadID);

void WINAPI DropCurrentSlowRequest(unsigned int hSocket);

bool WINAPI IsClosing(unsigned int hSocket);

bool WINAPI SendUserMessage(unsigned int hSocket, const wchar_t *strUserID, const VARIANT *pvtMsg);

bool WINAPI SendUserMessageEx(unsigned int hSocket, const wchar_t *strUserID, const unsigned char *pMessage, unsigned long ulLen);

bool WINAPI SetPassword(unsigned int hSocket, const wchar_t *strPassword);

void WINAPI SetZipLevel(unsigned int hSocket, tagZipLevel zl);

tagZipLevel WINAPI GetZipLevel(unsigned int hSocket);

bool WINAPI SetHTTPResponseHeader(unsigned int hSocket, const char *strUTF8Header, const char *strUTF8HeaderValue);
void WINAPI SetHTTPResponseCode(unsigned int hSocket, unsigned int nHTTPResponseCode);

void WINAPI SetHTTPMaxMessageSize(unsigned int hSocket, unsigned int nNewMax);
unsigned int WINAPI GetHTTPMaxMessageSize(unsigned int hSocket);

void WINAPI SetHTTPAutoPartition(unsigned int hSocket, bool bAutoPartition);
bool WINAPI GetHTTPAutoPartition(unsigned int hSocket);

void WINAPI SetHTTPServerPush(bool bEnableHTTPServerPush);
bool WINAPI GetHTTPServerPush();
bool WINAPI	SetUserID(unsigned int hSocket, const wchar_t *strUserID);
const wchar_t* WINAPI HTTPEnter(unsigned int hSocket, unsigned long ulGroups, unsigned long dwLeaseTime, const wchar_t *strIpAddr);
bool WINAPI HTTPExit(unsigned int hSocket, const wchar_t* strPushSessionId);
bool WINAPI HTTPSendUserMessage(unsigned int hSocket, const wchar_t* strPushSessionId, const wchar_t* strUserID, const VARIANT *pvtMsg);
bool WINAPI HTTPSpeak(unsigned int hSocket, const wchar_t* strPushSessionId, const VARIANT *pvtMsg, unsigned long ulGroups);
bool WINAPI HTTPSubscribe(unsigned int hSocket, const wchar_t* strPushSessionId, unsigned long ulTimeout, const wchar_t *strCrossSiteJSCallback);

//Query how many HTTP chatters join one group (ulGroupId)
unsigned long WINAPI GetCountOfHTTPChatters(unsigned long ulGroupId);

//Query a HTTP chat id for a zero-based index.
const wchar_t* WINAPI GetHTTPChatId(unsigned long ulGroupIDs, unsigned long ulIndex);

//Query a HTTP chat context
bool WINAPI GetHTTPChatContext(const wchar_t *strChatId, wchar_t **pstrUserID, wchar_t **pstrIpAddr, unsigned long *pLeaseTime, unsigned long *pGroups, unsigned long *pTimeout, unsigned long *pCountOfMessages);

void WINAPI SetReturnRandom(unsigned long ulSvsId, bool bRandom);

bool WINAPI GetReturnRandom(unsigned long ulSvsId);

void WINAPI DropRequestResult(unsigned int hSocket, unsigned short usRequestId);

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