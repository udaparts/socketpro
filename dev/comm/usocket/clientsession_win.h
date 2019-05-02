
#ifndef __UMB_CLIENT_SESSION_H__
#define __UMB_CLIENT_SESSION_H__

#include "../../pinc/bf.h"
#include "../clientcore/clientthread.h"
#include "../../pinc/mqfile.h"
#include "ucertimpl_win.h"

/*
boost modification
windows inside boost/asio/detail/win_iocp_socket_recv_op.hpp or reactive_socket_recv_op.hpp for non-windows

static void do_complete(io_service_impl* owner, operation* base,
      const boost::system::error_code& result_ec,
      std::size_t bytes_transferred)
  {
    boost::system::error_code ec(result_ec);
    if (ec.value() == WSAEBADF)
        return;

    // Take ownership of the operation object.
    win_iocp_socket_recv_op* o(static_cast<win_iocp_socket_recv_op*>(base));
    ptr p = { boost::addressof(o->handler_), o, o };


both windows and linux inside boost/asio/ssl/detail/io.hpp
 std::size_t bytes_transferred = 0;
  do switch (op(core.engine_, ec, bytes_transferred))
  {
  case engine::want_input_and_retry:

    // If the input buffer is empty then we need to read some more data from
    // the underlying transport.
    if (boost::asio::buffer_size(core.input_) == 0)
      core.input_ = boost::asio::buffer(core.input_buffer_,
          next_layer.read_some(core.input_buffer_, ec));
    if (ec)
        break;
 */

class CClientSession : public SPA::ClientSide::UClientSocketBase {
public:
    CClientSession(CIoService &IoService, CClientThread *pClientThread);
    ~CClientSession();

    CClientSession(const CClientSession &cs);
    CClientSession& operator=(const CClientSession &cs);

public:
    SPA::tagOperationSystem GetPeerOs(bool *endian);
    bool IsOpened();
    tagConnectionState GetConnectionState();
    unsigned short GetCurrentRequestID();
    unsigned int GetCurrentResultSize();
    bool IsSslEnabled();
    int GetErrorCode();
    std::string GetErrorMessage();
    CSocket* GetSocket();
    void SetEncryptionMethod(SPA::tagEncryptionMethod em);
    SPA::tagEncryptionMethod GetEncryptionMethod();
    bool Connect(const char *strHost, unsigned int nPort, bool bSync = false, bool b6 = false);
    void Close();
    void Shutdown(nsIP::tcp::socket::shutdown_type nHow = nsIP::tcp::socket::shutdown_both);
    unsigned int RetrieveResult(unsigned char *pBuffer, unsigned int size);
    void SetOnSocketClosed(POnSocketClosed p);
    void SetOnHandShakeCompleted(POnHandShakeCompleted p);
    void SetOnSocketConnected(POnSocketConnected p);
    void SetOnRequestProcessed(POnRequestProcessed p);
    unsigned int GetCountOfRequestsInQueue();
    CClientThread* GetClientThread();
    bool WaitAll(unsigned int nTimeout = (~0));
    bool SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int len);
    CSocketPool* GetSocketPool();
    bool Cancel(unsigned int requestsQueued = (~0));
    bool IsRandom();
    bool IsRouting();
    const SPA::CSwitchInfo* GetServerInfo();
    const SPA::CSwitchInfo* GetClientInfo();
    void SetClientInfo(SPA::CSwitchInfo si);
    unsigned int GetBytesInSendingBuffer();
    unsigned int GetBytesInReceivingBuffer();
    bool IsBatching();
    unsigned int GetBytesBatched();
    bool StartBatching();
    bool CommitBatching(bool bBatchingAtServerSide);
    bool AbortBatching();
    SPA::UINT64 GetBytesReceived();
    SPA::UINT64 GetBytesSent();
    void SetUserID(const wchar_t *strUserId);
    unsigned int GetUID(wchar_t *strUserId, unsigned int chars);
    void SetPassword(const wchar_t *strPassword);
    bool SwitchTo(unsigned int serviceId);
    bool Enter(const unsigned int *pChatGroupId, unsigned int nCount);
    void Exit();
    bool Speak(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount);
    bool SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size);
    bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size);
    SPA::UINT64 GetSocketNativeHandle();
    void SetOnEnter(POnEnter p);
    void SetOnExit(POnExit p);
    void SetOnSpeakEx(POnSpeakEx p);
    void SetOnSpeak(POnSpeak p);
    void SetOnSendUserMessageEx(POnSendUserMessageEx p);
    void SetOnSendUserMessage(POnSendUserMessage p);
    void SetOnServerException(POnServerException p);
    void SetOnBaseRequestProcessed(POnBaseRequestProcessed p);
    void SetOnAllRequestsProcessed(POnAllRequestsProcessed p);
    void SetZip(bool zip);
    bool GetZip();
    void SetZipLevel(SPA::tagZipLevel zl);
    SPA::tagZipLevel GetZipLevel();
    bool StartQueue(const char *qName, bool secure, bool dequeueShared, unsigned int ttl);

    void StopQueue(bool permanent);
    bool DequeuedResult();
    bool IsQueueSecured();
    bool IsQueueStarted();
    unsigned int GetMessagesInDequeuing();
    SPA::UINT64 GetMessageCount();
    SPA::UINT64 GetQueueSize();
    SPA::UINT64 GetQueueLastIndex();
    const char* GetQueueName();
    const char* GetQueueFileName();
    unsigned int CancelQueuedRequests(const unsigned short *ids, unsigned int count);
    SPA::UINT64 CancelQueuedRequests(SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    SPA::UINT64 GetLastQueueMessageTime();

    bool AbortJob();
    bool StartJob();
    bool EndJob();
    SPA::UINT64 GetJobSize();

    boost::posix_time::ptime& GetLatestTime();

    void PostCloseInternal(int error);
    bool DoEcho();
    bool SetSockOpt(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    bool SetSockOptAtSvr(SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    bool TurnOnZipAtSvr(bool enableZip);
    bool SetZipLevelAtSvr(SPA::tagZipLevel zipLevel);
    bool GetSockAddr(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars);
    bool GetPeerName(unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars);

    unsigned int GetConnTimeout();
    unsigned int GetRecvTimeout();

    void SetRecvTimeout(unsigned int timeout);
    void SetConnTimeout(unsigned int timeout);
    void SetAutoConn(bool autoConnecting);
    bool GetAutoConn();
    unsigned short GetServerPingTime();
    CUCertImpl* GetUCert();
    void* GetSSL();
    bool IgnoreLastRequest(unsigned short reqId);
    void TimerHandler();
    unsigned int PeekQueuedRequests(SPA::CQueuedRequestInfo *qri, unsigned int count);
    bool IsDequeueEnabled();
    unsigned int GetCurrentServiceId();
    void SetPeerDequeueFailed(bool fail);
    bool GetPeerDequeueFailed();
    bool IsRouteeRequest();
    bool SendRouteeResult(unsigned short reqId, const unsigned char *buffer, unsigned int len);
    unsigned int GetRouteeCount();
    SPA::UINT64 AppendQueue(boost::shared_ptr<MQ_FILE::CMqFile> q);
    bool IsDequeueShared();
    boost::shared_ptr<MQ_FILE::CMqFile> GetQueue();
    unsigned int GetTTL();
    SPA::UINT64 RemoveQueuedRequestsByTTL();
    void ResetQueue();
    bool PushQueueTo(const std::vector<CClientSession*> &vClients);
    void SendFromPersistantQueue();

private:
    static unsigned int CompressRequestTo(unsigned short reqId, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q);
    static unsigned int DecompressResultTo(unsigned short ratio, SPA::tagZipLevel zl, const unsigned char *buffer, unsigned int size, SPA::CUQueue &q);
    bool IsRoutingInternal();
    void WriteIntoBatch(const void *pBuffer, unsigned int len);
    void OnClosed(int errCode);
    void OnChatRequest(unsigned short nRequestId, SPA::CScopeUQueue &sb);
    void OnRequestProcessed(unsigned short nRequestId, unsigned int nLen);
    void OnBaseRequestProcessed(unsigned short nRequestId, unsigned int nLen);
    void OnConnectedInternal(int errCode);
    void OnHandleShakeCompleted(int errCode);
    bool IsContextSet();
    void SetContext();
    void OnConnected(const CErrorCode &ec, CResolver::iterator ep);
    void OnSslHandShake(const CErrorCode& Error);
    void OnReadCompleted(const CErrorCode& Error, size_t len);
    void OnWriteCompleted(const CErrorCode& Error);
    void Read();
    void Write(const unsigned char *s, unsigned int nSize);
    void WriteFromQueueFile();
    void OnResolve(const CErrorCode &ec, CResolver::iterator ep);
    bool SendRequestInternal(unsigned short reqId, const void *pBuffer, unsigned int len);
    bool IsSameThread();
    bool RemoveRequestId(unsigned short nRequestId);
    void ConenctInternally();
    inline bool IsSameEndian();
    static unsigned char* GetIoBuffer();
    static void ReleaseIoBuffer(unsigned char *buffer);
    bool Decompress();
    void CloseInternal(int nError = 0);
    bool WaitConnected(CAutoLock &sl, unsigned int nTimeout);
    bool WaitAllInternal(CAutoLock &sl, unsigned int nTimeout);
    unsigned int RetrieveResultInternal(unsigned char *pBuffer, unsigned int size);
    bool StartQueueInternal(const char *qName, bool secure, bool dequeueShared, unsigned int ttl);
    void StopQueueInternal(bool permanent);
    void SendStartQueueMessage();
    void SendStopQueueMessage();
    bool CheckQueueAvailable();
    static bool Find(const std::string &rawName);
    bool SwitchToIntenal(unsigned int serviceId);
    unsigned short GetServerPingTimeInternal();
    void SetVQtrans();
    void NotifyDequeued(unsigned int qHandle);
    void NotifyDequeuedStartQueueTrans(unsigned int qHandle);
    void NotifyDequeuedCommitQueueTrans(unsigned int qHandle);
    bool SendRoutingResultInternal(unsigned short reqId, const unsigned char *buffer, unsigned int len);
    std::wstring GetPwd();

public:
    static std::string m_WorkingPath;

private:
    SPA::CSwitchInfo m_ServerInfo;
    SPA::CMsSsl *m_pSsl;
    boost::shared_ptr<CSocket> m_pSocket;
    unsigned char *m_ReadBuffer;
    bool m_bRBLocked;
    unsigned char *m_WriteBuffer;
    bool m_bWBLocked;
    SPA::tagZipLevel m_zl;
    SPA::CUQueue m_qRead;
    SPA::CUQueue m_qWrite;
    boost::mutex m_mutex;
    boost::uint64_t m_ulRead;
    boost::uint64_t m_ulSent;
    boost::posix_time::ptime m_tRecv;
    boost::posix_time::ptime m_tSend;
    SPA::CUQueue *m_pQBatch;
    bool m_bZip;
    CErrorCode m_ec;
    SPA::CSwitchInfo m_ClientInfo;
    CIoService *m_pIoService;
    SPA::tagEncryptionMethod m_EncryptionMethod;
    static boost::posix_time::ptime m_tStart;
    CResolver m_Resolver;
    unsigned int m_nPort;
    std::string m_strhost;
    SPA::CStreamHeader m_ResultInfo;
    POnSocketClosed m_OnSocketClosed;
    POnHandShakeCompleted m_OnHandShakeCompleted;
    POnSocketConnected m_OnSocketConnected;
    POnRequestProcessed m_OnRequestProcessed;
    POnEnter m_OnSubscribe;
    POnExit m_OnUnsubscribe;
    POnSpeakEx m_OnBroadcastEx;
    POnSpeak m_OnBroadcast;
    POnSendUserMessageEx m_OnPostUserMessageEx;
    POnSendUserMessage m_OnPostUserMessage;
    POnServerException m_OnServerException;
    POnBaseRequestProcessed m_OnBaseRequestProcessed;
    POnAllRequestsProcessed m_OnAllRequestsProcessed;
    tagConnectionState m_ConnState;
    CConditionVariable m_cv;
    CClientThread *m_pThread;
    SPA::CUQueue m_qReqIdCancel;
    SPA::CUQueue m_qReqIdWait;
    unsigned int m_nConnTimeout;
    unsigned int m_nRecvTimeout;
    unsigned int m_nCancel;
    boost::shared_ptr<CUCertImpl> m_pCert;

    std::wstring m_strUserId;
    boost::shared_ptr<MQ_FILE::CMqFile> m_qRequest;
    MQ_FILE::CQueueInitialInfo m_ServerQFile;
    bool m_bAutoConn;
    unsigned int m_nPoolId;
    bool m_bFail;
    bool m_bDequeueTrans;
    MQ_FILE::QAttr m_qa;
    std::vector<MQ_FILE::QAttr> m_vQTrans;
    bool m_bConfirmTrans;
    bool m_bConfirmFail;
    SPA::UINT64 m_RouterHandle;
    unsigned int m_nRouteeCount;

    static boost::mutex m_mutexQLI;
    static std::vector<boost::shared_ptr<MQ_FILE::CMqFile> > m_vQRequest;

    static boost::mutex m_mutexBuffer;
    static std::vector<unsigned char*> m_aBuffer;

public:
    static boost::shared_ptr<MQ_FILE::CQLastIndex> m_pQLastIndex;
};


#endif
