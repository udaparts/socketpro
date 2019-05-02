#pragma once

#include "serverthread.h"
#include "../../pinc/bf.h"
#include "../../include/userver.h"
#include "../../apps/hparser/httpcontext.h"
#include <deque>
#include "../../apps/hparser/connectioncontext.h"

//
#ifdef WIN32_64
#define MAX_SESSION_INDEX (0xFFFFF)
#define INDEX_SHIFT_BITS 20
#else
#define MAX_SESSION_INDEX (0xFFFF)
#define INDEX_SHIFT_BITS 16
#endif

#define SOCKET_CLOSE_EVENT              (WM_CONTINUE_PROCESSING + 0x10)
#define MEMOREY_QUEUE_HEADER_REST_SIZE	(8*1460)

/*
#define DEFAULT_MAX_REQUEST_SIZE        ((unsigned int)(100*1460))
#define MAX_RECV_HEADER_SIZE            (5*1460)
#define BINARY_REQUEST_SIZE             (10*1460)
 */

class CServerSession : private boost::noncopyable {
public:
	CServerSession();
	virtual ~CServerSession();

	//no copy and assign operators
	CServerSession(const CServerSession &ss);
	CServerSession& operator=(const CServerSession &ss);

public:
	bool GetPeerOs(MB::tagOperationSystem *pOS);
	CSocket& GetSocket();
	void Start();
	unsigned int GetSocketNativeHandle();
	void Initialize();
	void Close();
	void PostClose(int errCode = 0);
	bool IsOpened();
	int GetErrorCode();
	std::string GetErrorMessage();
	unsigned int GetServiceId();
	unsigned int GetCurrentRequestLen();
	unsigned short GetCurrentRequestId();
	unsigned int GetSndBytesInQueue();
	unsigned int GetRcvBytesInQueue();
	unsigned int QueryRequestsQueued();
	unsigned int GetConnIndex();
	unsigned int RetrieveRequestBuffer(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek);
	unsigned int SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize);

	MB::U_UINT64 GetBytesReceived();
	MB::U_UINT64 GetBytesSent();
	bool IsBatching();
	unsigned int GetBytesBatched();
	bool StartBatching();
	bool CommitBatching();
	bool AbortBatching();
	bool Wait(unsigned int nTimeout = 100);
	void ExecuteSlowRequestFromThreadPool(unsigned short sReqId);
	void SetUserId(const wchar_t *strUserId);
	unsigned int GetUserId(wchar_t *strUserId, unsigned int chars);
	void SetPassword(const wchar_t *strPassword);
	unsigned int GetPassword(wchar_t *strPassword, unsigned int chars);
	bool Enter(const unsigned int *pChatGroupId, unsigned int nCount);
	void Exit();
	bool Speak(const MB::UVariant *pvtMessage, const MB::UVariant *pvtGroups);
	bool SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
	bool SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size);
	bool SendUserMessage(const wchar_t *userId, const MB::UVariant *pvtMessage);
	unsigned int GetCountOfJoinedChatGroups();
	unsigned int GetChatGroups(unsigned int *pChatGroup, unsigned int count);
	void GetPeerIpAddr(std::string &addr, unsigned short *port);
	unsigned int SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
	unsigned int SendExceptionResult(const char* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
	bool FakeAClientRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int nBufferSize);
	bool IsCanceled();

	//HTTP
	unsigned int GetHTTPRequestHeaders(MB::CHttpHeaderValue *HeaderValue, unsigned int count);
	const char* GetHTTPPath();
	MB::U_UINT64 GetHTTPContentLength();
	const char* GetHTTPQuery();
	bool DownloadFile(const char *filePath);
	MB::tagHttpMethod GetHTTPMethod();
	bool HTTPKeepAlive();
	bool IsWebSocket();
	bool IsCrossDomain();
	double GetHTTPVersion();
	bool HTTPGZipAccepted();
	const char* GetHTTPUrl();
	const char* GetHTTPHost();
	MB::tagTransport GetHTTPTransport();
	MB::tagTransferEncoding GetHTTPTransferEncoding();
	MB::tagContentMultiplax GetHTTPContentMultiplax();
	bool SetHTTPResponseCode(unsigned int errCode);
	bool SetHTTPResponseHeader(const char *uft8Header, const char *utf8Value);
	unsigned int SendHTTPReturnDataA(const char *str, unsigned int chars);
	USocket_Server_Handle MakeHandler();
	const char* GetHTTPId();
	unsigned int GetHTTPCurrentMultiplaxHeaders(MB::CHttpHeaderValue *HeaderValue, unsigned int count);
	unsigned int HTTPCallbackA(const char *name, const char *str);
	void ShrinkMemory();
	unsigned int StartChunkResponse();
	unsigned int SendChunk(const unsigned char *buffer, unsigned int len);
	unsigned int EndChunkResponse(const unsigned char *buffer, unsigned int len);
	bool IsFakeRequest();
	bool IsOld();
	void SetZip(bool bZip);
	bool GetZip();
	void SetZipLevel(MB::tagZipLevel zl);
	MB::tagZipLevel GetZipLevel();

private:
	static unsigned int CompressResultTo(bool old, unsigned short reqId, MB::tagZipLevel zl, const unsigned char *buffer, unsigned int size, MB::CUQueue &q);
	static unsigned int DecompressRequestTo(unsigned short ratio, MB::tagZipLevel zl, const unsigned char *buffer, unsigned int size, MB::CUQueue &q);
	USocket_Server_Handle MakeHandlerInternal();
	void ConfirmFailed();
	bool Process(bool bSlowProcessed = false);
	bool ProcessWebSocketRequest();
	bool ProcessAjaxRequest();
	bool ProcessJavaScriptRequest();
	bool PreocessWebRequest(MB::CUQueue &q);
	bool IsSecure();
	bool ProcessHttpRequest();
	inline bool IsSameEndian();
	bool ProcessWithLock();
	void PostCloseInternal(int errCode);
	unsigned int GetUserIdInternally(wchar_t *strUserId, unsigned int chars);
	void CloseInternal();
	unsigned int RetrieveRequestBufferInternally(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek);
	unsigned int QueryRequestsQueuedInternally();
	bool IsCanceledInternally();
	void Exit(const unsigned int *pChatGroupId, unsigned int nCount);
	void BounceBackMessage(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size);
	void SendChatResult(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, unsigned short usReqId, const unsigned char *pBuffer, unsigned int size);
	boost::posix_time::ptime& GetLatestTime();
	void OnRA();
	void SetContext();
	void OnSslHandShake(const CErrorCode& Error);
	void OnReadCompleted(const CErrorCode& Error, size_t bytes_transferred);
	void OnWriteCompleted(const CErrorCode& Error);
	void OnClose();
	void Write(const unsigned char *s, unsigned int nSize);
	void Read();
	void OnSlowRequestProcessed(unsigned int res, unsigned short usRequestId);
	void OnBaseRequestArrive();
	void OnNonBaseRequestArrive();
	void OnChatRequestArrive();
	void OnChatVariantRequestArrive();
	static unsigned char* GetIoBuffer();
	static void ReleaseIoBuffer(unsigned char *buffer);
	bool DoAuthentication(unsigned int ServiceId);
	void OnSwitchTo(unsigned int OldServiceId, unsigned int NewServiceId);
	bool Decompress();

private:
	CServerThread *m_pUThread;
	MB::CSwitchInfo m_ClientInfo;
	MB::CStreamHeader m_ReqInfo;
	unsigned int ServiceId;
	CSslSocket *m_pSslSocket;
	CSocket *m_pSocket;
	MB::CUQueue m_qRead;
	MB::CUQueue m_qWrite;
	unsigned char *m_ReadBuffer;
	unsigned char *m_WriteBuffer;
	boost::mutex m_mutex;
	//boost::recursive_mutex		m_mutex;
	unsigned int m_ulIndex;
	bool m_bZip;
	MB::tagZipLevel m_zl;
	MB::CUQueue *m_pQBatch;
	CErrorCode m_ec;
	CConditionVariable m_cv;
	bool m_bDropSlowRequest;
	UHTTP::CHttpContext *m_pHttpContext;
	Connection::CConnectionContextBase m_ccb;
	unsigned int m_nHttpCallCount;
	typedef std::map<unsigned int, std::vector<MB::U_UINT64> > CQueueMap;
	CQueueMap m_mapDequeue;
	static std::vector<unsigned char*> m_aBuffer;
	static boost::mutex m_mutexBuffer;
	static const unsigned int MULTIPLE_CONTEXT_LENGTH = 30 * 1024;
	friend class CServer;
};

typedef CServerSession* PSession;
