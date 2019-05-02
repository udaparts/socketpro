
#include "stdafx.h"
#include "session.h"
#include "server.h"
#include "serverthread.h"
#include <algorithm>
#include "ucommexception.h"
#include "../../apps/hparser/jsloader.h"
#include <assert.h>
#include <boost/scoped_ptr.hpp>
#include "../../apps/hparser/webresponseProcessor.h"
#include "../../pinc/uzip.h"

extern CServer *g_pServer;

std::vector<unsigned char*> CServerSession::m_aBuffer;
boost::mutex CServerSession::m_mutexBuffer;
boost::mutex CServerSession::m_qMutex;

unsigned char* CServerSession::GetIoBuffer() {
	unsigned char *s = NULL;
	{
		boost::mutex::scoped_lock sl(m_mutexBuffer);
		size_t size = m_aBuffer.size();
		if (size) {
			s = m_aBuffer[size - 1];
			m_aBuffer.pop_back();
		}
	}
	if (s == NULL)
		s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + 16);
	return s;
}

bool CServerSession::GetPeerOs(MB::tagOperationSystem *pOS) {
	CAutoLock sl(m_mutex);
	if (pOS)
		*pOS = m_ReqInfo.GetOS();
	return m_ReqInfo.IsBigEndian();
}

void CServerSession::ReleaseIoBuffer(unsigned char *&buffer) {
	if (buffer == NULL)
		return;
	m_mutexBuffer.lock();
	m_aBuffer.push_back(buffer);
	m_mutexBuffer.unlock();
	buffer = NULL;
}

boost::shared_ptr<MQ_FILE::CPMessage> CServerSession::m_pPIndex;
boost::shared_ptr<MQ_FILE::CQLastIndex> CServerSession::m_pQLastIndex;

CServerSession::CServerSession()
: m_pSocket(NULL),
m_pSslSocket(NULL),
ServiceId(MB::sidStartup),
m_pUThread(NULL),
m_ReadBuffer(NULL),
m_WriteBuffer(NULL),
m_bZip(g_pServer->m_bZip),
m_zl(MB::zlDefault),
m_pQBatch(NULL),
m_bDropSlowRequest(false),
m_pHttpContext(NULL), m_nHttpCallCount(0) {
	memset(&m_ReqInfo, 0, sizeof (m_ReqInfo));
	memset(&m_ClientInfo, 0, sizeof (m_ClientInfo));
	SetContext();
}

CServerSession::~CServerSession() {
	CAutoLock sl(m_mutex);
	Initialize();
}

void CServerSession::SetContext() {
	assert(m_pSslSocket == NULL && m_pSocket == NULL);
	boost::asio::ssl::context_base::method method;
	if (g_pServer->IsSsl(method))
		m_pSslSocket = new CSslSocket(g_pServer->m_IoService, *(g_pServer->m_pSslContext));
	else
		m_pSocket = new CSocket(g_pServer->m_IoService);
}

void CServerSession::Initialize() {
	m_qRead.SetSize(0);
	if (m_qRead.GetMaxSize() > IO_BUFFER_SIZE)
		m_qRead.ReallocBuffer(IO_BUFFER_SIZE);
	m_qWrite.SetSize(0);
	if (m_qWrite.GetMaxSize() > IO_BUFFER_SIZE)
		m_qWrite.ReallocBuffer(IO_BUFFER_SIZE);
	memset(&m_ReqInfo, 0, sizeof (m_ReqInfo));
	if (m_ccb.Id.size()) {
		Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
		if (sp) {
			Connection::CConnectionContextBase *p = sp.get();
			*p = m_ccb;
		}
	}
	m_ccb.Initialize();
	memset(&m_ClientInfo, 0, sizeof (m_ClientInfo));
	ServiceId = (unsigned int) MB::sidStartup;
	m_pUThread = NULL;

	ReleaseIoBuffer(m_ReadBuffer);
	ReleaseIoBuffer(m_WriteBuffer);
	m_bZip = g_pServer->m_bZip;
	m_zl = MB::zlDefault;
	MB::CScopeUQueue::Unlock(m_pQBatch);
	m_pQBatch = NULL;
	m_bDropSlowRequest = false;
	m_ec.clear();
	m_nHttpCallCount = 0;
	ConfirmFailed();
}

void CServerSession::ConfirmFailed() {
	CQueueMap::iterator it, end = m_mapDequeue.end();
	for (it = m_mapDequeue.begin(); it != end; ++it) {
		CQueueProperty &v = it->second;
		CQueueProperty::iterator vi, ve = v.end();
		for (vi = v.begin(); vi != ve; ++vi) {
			g_pServer->ConfirmQueue(it->first, vi->first, vi->second, false);
		}
	}
	m_mapDequeue.clear();
}

void CServerSession::SetUserId(const wchar_t *strUserId) {
	m_mutex.lock();
	m_ccb.UserId = strUserId;
	m_mutex.unlock();
}

unsigned int CServerSession::GetPassword(wchar_t *strPassword, unsigned int chars) {
	if (strPassword == NULL || chars == 0)
		return 0;
	--chars;
	m_mutex.lock();
	if (chars > (unsigned int) m_ccb.Password.size())
		chars = (unsigned int) m_ccb.Password.size();
	if (chars > 0)
		::memcpy(strPassword, m_ccb.Password.c_str(), sizeof (wchar_t) * chars);
	strPassword[chars] = 0;
	m_mutex.unlock();
	return chars;
}

unsigned int CServerSession::GetUserId(wchar_t *strUserId, unsigned int chars) {
	m_mutex.lock();
	chars = GetUserIdInternally(strUserId, chars);
	m_mutex.unlock();
	return chars;
}

unsigned int CServerSession::GetUserIdInternally(wchar_t *strUserId, unsigned int chars) {
	if (strUserId == NULL || chars == 0)
		return 0;
	--chars;
	if (chars > (unsigned int) m_ccb.UserId.size())
		chars = (unsigned int) m_ccb.UserId.size();
	if (chars > 0)
		::memcpy(strUserId, m_ccb.UserId.c_str(), sizeof (wchar_t) * chars);
	strUserId[chars] = 0;
	return chars;
}

void CServerSession::SetPassword(const wchar_t *strPassword) {
	m_mutex.lock();
	m_ccb.Password = strPassword;
	m_mutex.unlock();
}

MB::U_UINT64 CServerSession::GetBytesReceived() {
	CAutoLock sl(m_mutex);
	return m_ccb.m_ulRead;
}

MB::U_UINT64 CServerSession::GetBytesSent() {
	CAutoLock sl(m_mutex);
	return m_ccb.m_ulSent;
}

bool CServerSession::IsBatching() {
	CAutoLock sl(m_mutex);
	return (m_ccb.m_bBatching || m_pQBatch);
}

bool CServerSession::Enter(const unsigned int *pChatGroupId, unsigned int nCount) {
	if (pChatGroupId == NULL)
		nCount = 0;
	MB::CScopeUQueue sb;
	sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
	return FakeAClientRequest(MB::idEnter, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::ShrinkMemory() {
	CAutoLock sl(m_mutex);
	if (m_qRead.GetSize() == 0 && m_qRead.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE)
		m_qRead.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE);
	if (m_qWrite.GetSize() == 0 && m_qWrite.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE)
		m_qWrite.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE);
	if (ServiceId == MB::sidHTTP && m_pHttpContext && m_pHttpContext->IsWebSocket()) {
		UHTTP::CWebSocketMsg *p = m_pHttpContext->GetWebSocketMsg();
		if (p->Content.GetSize() == 0 && p->Content.GetMaxSize() > MEMOREY_QUEUE_HEADER_REST_SIZE / 2)
			p->Content.ReallocBuffer(MEMOREY_QUEUE_HEADER_REST_SIZE / 2);
		UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
		pWebRequestProcessor->ShrinkMemory();
	}
}

void CServerSession::Exit() {
	FakeAClientRequest(MB::idExit, NULL, 0);
}

void CServerSession::Exit(const unsigned int *pChatGroupId, unsigned int nCount) {
	MB::CScopeUQueue sb;
	if (pChatGroupId == NULL)
		nCount = 0;
	if (nCount > 0) {
		sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
	}
	FakeAClientRequest(MB::idExit, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::Speak(const MB::UVariant *pvtMessage, const MB::UVariant *pvtGroups) {
	MB::CScopeUQueue sb;
	if (pvtMessage)
		sb << *pvtMessage;
	else {
		MB::UVariant vtMsg;
		sb << vtMsg;
	}
	if (pvtGroups == NULL)
		return false;
	unsigned short vt = MB::Map2VarintType(*pvtGroups);
	if (vt != (VT_ARRAY | VT_UINT) && vt != (VT_ARRAY | VT_UI4))
		return false;
	sb << *pvtGroups;
	return FakeAClientRequest(MB::idSpeak, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SpeakEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int nCount) {
	if (pChatGroupId == NULL || nCount == 0)
		return false;
	MB::CScopeUQueue sb;
	sb << nCount;
	sb->Push((const unsigned char*) pChatGroupId, nCount * sizeof (unsigned int));
	sb->Push(message, (unsigned int) size);
	return FakeAClientRequest(MB::idSpeakEx, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SendUserMessage(const wchar_t *userId, const MB::UVariant *pvtMessage) {
	if (userId == NULL)
		return false;
	MB::CScopeUQueue sb;
	sb << userId;
	if (pvtMessage)
		sb << *pvtMessage;
	else {
		MB::UVariant vt;
		sb << vt;
	}
	return FakeAClientRequest(MB::idSendUserMessage, sb->GetBuffer(), sb->GetSize());
}

bool CServerSession::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size) {
	if (userId == NULL)
		return false;
	MB::CScopeUQueue sb;
	sb << userId;
	sb->Push(message, size);
	return FakeAClientRequest(MB::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::ExecuteSlowRequestFromThreadPool(unsigned short sReqId) {
	m_mutex.lock();
	PSLOW_PROCESS p = m_ccb.SvsContext.m_SlowProcess;
	m_mutex.unlock();
	if (p != NULL) {
		assert(sReqId == GetCurrentRequestId());
		try {
			p(sReqId, GetCurrentRequestLen(), MakeHandler());
		} catch (MB::CMBException &err) {
			SendExceptionResult(err.what(), err.GetStack().c_str(), sReqId, err.GetErrCode());
		} catch (std::exception &err) {
			SendExceptionResult(err.what(), "Inside worker thread for processing slow request", sReqId, MB_STL_EXCEPTION);
		} catch (...) {
			SendExceptionResult(L"Unknown exception caught", "Inside worker thread for processing slow request", sReqId, MB_UNKNOWN_EXCEPTION);
		}
	}
}

bool CServerSession::IsCanceled() {
	CAutoLock al(m_mutex);
	return IsCanceledInternally();
}

bool CServerSession::IsCanceledInternally() {
	unsigned int pos = 0;
	unsigned int lenAll = m_qRead.GetSize();
	unsigned int total = (m_ReqInfo.RequestId == MB::idCancel) ? 1 : 0;
	if (!total) {
		MB::CStreamHeader *p = &m_ReqInfo;
		while (lenAll > p->Size) {
			pos += p->Size;
			lenAll -= p->Size;
			if (lenAll < sizeof (MB::CStreamHeader))
				break;
			p = (MB::CStreamHeader*)m_qRead.GetBuffer(pos);
			if (p->RequestId == MB::idCancel) {
				total = 1;
				break;
			}
			pos += sizeof (MB::CStreamHeader);
			lenAll -= sizeof (MB::CStreamHeader);
		}
	}

	if (total && pos) {
		m_qRead.Pop(pos); //remove previous requests queued
	}

	return (total > 0);
}

bool CServerSession::FakeAClientRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int nBufferSize) {
	MB::CStreamHeader reqInfo;
	reqInfo.MakeFake();
	reqInfo.RequestId = reqId;
	if (pBuffer == NULL)
		nBufferSize = 0;
	reqInfo.Size = nBufferSize;
	MB::CScopeUQueue sb;
	sb << reqInfo;
	if (nBufferSize > 0)
		sb->Push(pBuffer, nBufferSize);
	boost::mutex::scoped_lock sl(m_mutex);
	if (ServiceId == MB::sidHTTP && m_ccb.Id.size() == 0) {
		if (m_pHttpContext == NULL)
			return false;
		UHTTP::CWebRequestProcessor *p = m_pHttpContext->GetWebRequestProcessor();
		if (p == NULL)
			return false;
		if (p->GetUHttpRequest().SpRequest != UHTTP::srSwitchTo)
			return false;
	}
	if (ServiceId == MB::sidHTTP || m_pHttpContext)
		++m_nHttpCallCount;
	m_qRead.Insert(sb->GetBuffer(), sb->GetSize(), m_ReqInfo.Size);
	return true;
}

unsigned int CServerSession::GetBytesBatched() {
	CAutoLock sl(m_mutex);
	if (m_pQBatch == NULL)
		return 0;
	return m_pQBatch->GetSize();
}

bool CServerSession::StartBatching() {
	CAutoLock sl(m_mutex);
	if (m_pQBatch)
		return true;
	m_pQBatch = MB::CScopeUQueue::Lock();
	return (m_pQBatch != NULL);
}

bool CServerSession::CommitBatching() {
	CAutoLock sl(m_mutex);
	if (!m_pQBatch)
		return false;
	if (m_bZip) {
		if (m_pQBatch->GetSize() && CompressResultTo(IsOld(), MB::idBatchZipped, m_zl, m_pQBatch->GetBuffer(), m_pQBatch->GetSize(), m_qWrite)) {
			Write(NULL, 0);
		}
	} else
		Write(m_pQBatch->GetBuffer(), m_pQBatch->GetSize());
	MB::CScopeUQueue::Unlock(m_pQBatch);
	m_pQBatch = NULL;
	return true;
}

void CServerSession::GetPeerIpAddr(std::string &addr, unsigned short *port) {
	CErrorCode ec;
	addr = GetSocket().remote_endpoint(ec).address().to_string();
	if (port != NULL)
		*port = GetSocket().remote_endpoint(ec).port();
}

bool CServerSession::AbortBatching() {
	CAutoLock sl(m_mutex);
	MB::CScopeUQueue::Unlock(m_pQBatch);
	m_pQBatch = NULL;
	return true;
}

unsigned int CServerSession::GetSocketNativeHandle() {
	return (unsigned int) GetSocket().native();
}

bool CServerSession::IsFakeRequest() {
	CAutoLock sl(m_mutex);
	return m_ReqInfo.IsFake();
}

void CServerSession::OnSlowRequestProcessed(unsigned int res, unsigned short usRequestId) {
	CAutoLock sl(m_mutex);
	if (m_ulIndex == 0 || m_ReqInfo.RequestId != usRequestId) {
		g_pServer->PostSproMessage(this, WM_SOCKET_SVR_NOTIFY, SOCKET_CLOSE_EVENT, m_ec.value());
		return;
	}
	POnRequestProcessed p = m_ccb.SvsContext.m_OnRequestProcessed;
	m_ReqInfo.RequestId = 0;
	if (m_ReqInfo.Size > 0) {
		m_qRead.Pop(m_ReqInfo.Size);
		m_ReqInfo.Size = 0;
	}
	g_pServer->PutThreadBackIntoPool(m_pUThread);
	m_pUThread = NULL;
	if (p != NULL) {
		CRAutoLock sl(m_mutex);
		try {
			p(MakeHandler(), usRequestId);
		} catch (MB::CMBException &err) {
			SendExceptionResult(err.what(), err.GetStack().c_str(), usRequestId, err.GetErrCode());
		} catch (std::exception &err) {
			SendExceptionResult(err.what(), "Inside slow request processed notification", usRequestId, MB_STL_EXCEPTION);
		} catch (...) {
			SendExceptionResult(L"Unknown exception caught", "Inside slow request processed notification", usRequestId, MB_UNKNOWN_EXCEPTION);
		}
	}
	unsigned int start = m_qRead.GetSize();
	if (start >= sizeof (m_ReqInfo))
		Process(true);
	if (start == m_qRead.GetSize() || m_qRead.GetSize() < 2 * IO_BUFFER_SIZE)
		Read();
}

CSocket& CServerSession::GetSocket() {
	if (m_pSslSocket)
		return m_pSslSocket->next_layer();
	return *m_pSocket;
}

void CServerSession::Start() {

	m_ccb.RecvTime = boost::posix_time::microsec_clock::local_time();
	m_ccb.SendTime = m_ccb.RecvTime;

	if (m_pSslSocket)
		m_pSslSocket->async_handshake(nsSSL::stream_base::server, boost::bind(&CServerSession::OnSslHandShake, this, nsPlaceHolders::error));
	else {
		assert(m_ReadBuffer == NULL);
		m_ReadBuffer = GetIoBuffer();
		m_pSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), boost::bind(&CServerSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
	}
}

void CServerSession::OnSslHandShake(const CErrorCode& Error) {
	g_pServer->m_mutex.lock();
	POnSSLHandShakeCompleted p = g_pServer->m_pOnSSLHandShakeCompleted;
	g_pServer->m_mutex.unlock();
	CAutoLock sl(m_mutex);
	m_ec = Error;
	if (p != NULL) {
		try {
			CRAutoLock rsl(m_mutex);
			p(MakeHandler(), Error.value());
		} catch (...) {
		}
	}
	if (!Error) {
		assert(m_ReadBuffer == NULL);
		m_ReadBuffer = GetIoBuffer();
		m_pSslSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), boost::bind(&CServerSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
	} else {
		CloseInternal();
	}
}

void CServerSession::OnClose() {
	POnClose p = m_ccb.SvsContext.m_OnClose;
	int errCode = m_ec.value();
	m_cv.notify_all();
	if (p != NULL) {
		try {
			CRAutoLock sl(m_mutex);
			p(MakeHandler(), errCode);
		} catch (...) {
		}
	}
	g_pServer->m_mutex.lock();
	p = g_pServer->m_pOnClose;
	g_pServer->m_mutex.unlock();
	if (p != NULL) {
		try {
			CRAutoLock sl(m_mutex);
			p(MakeHandler(), errCode);
		} catch (...) {
		}
	}
}

USocket_Server_Handle CServerSession::MakeHandlerInternal() {
	USocket_Server_Handle h = (USocket_Server_Handle)this;
	h <<= INDEX_SHIFT_BITS;
	h += m_ulIndex;
	return h;
}

USocket_Server_Handle CServerSession::MakeHandler() {
	CAutoLock sl(m_mutex);
	return MakeHandlerInternal();
}

void CServerSession::PostCloseInternal(int errCode) {
	if (errCode != 0)
		m_ec.assign(errCode, boost::asio::error::get_system_category());
	GetSocket().get_io_service().post(boost::bind(&CServerSession::Close, this));
}

void CServerSession::PostClose(int errCode) {
	CAutoLock sl(m_mutex);
	if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
		m_qRead.SetSize(0);
		m_pHttpContext->PrepareWSResponseMessage(NULL, 0, UHTTP::ocConnectionClose, m_qWrite);
		Write(NULL, 0);
	} else
		PostCloseInternal(errCode);
}

bool CServerSession::IsOpened() {
	CAutoLock sl(m_mutex);
	return (m_ulIndex && GetSocket().is_open());
}

bool CServerSession::IsSameEndian() {
	return (m_ReqInfo.IsBigEndian() == MB::IsBigEndian());
}

void CServerSession::Close() {
	CAutoLock sl(m_mutex);
	CloseInternal();
}

void CServerSession::SetZip(bool bZip) {
	CAutoLock sl(m_mutex);
	m_bZip = bZip;
}

bool CServerSession::GetZip() {
	CAutoLock sl(m_mutex);
	return m_bZip;
}

void CServerSession::SetZipLevel(MB::tagZipLevel zl) {
	if (zl != MB::zlBestSpeed && zl != MB::zlDefault)
		return;
	CAutoLock sl(m_mutex);
	m_zl = zl;
}

MB::tagZipLevel CServerSession::GetZipLevel() {
	CAutoLock sl(m_mutex);
	return m_zl;
}

bool CServerSession::IsOld() {
	return (m_ClientInfo.MajorVersion < 2);
}

unsigned int CServerSession::DecompressRequestTo(unsigned short ratio, MB::tagZipLevel zl, const unsigned char *buffer, unsigned int size, MB::CUQueue &q) {
	unsigned int zSize = ratio * size;
	if (q.GetTailSize() < zSize) {
		unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
		q.ReallocBuffer(bufferSize);
	}
	zSize = q.GetTailSize();
	unsigned int start = q.GetSize();
	if (MB::Decompress(zl, buffer, size, (void*) q.GetBuffer(start), zSize))
		q.SetSize(start + zSize);
	return (q.GetSize() - start);
}

unsigned int CServerSession::CompressResultTo(bool old, unsigned short reqId, MB::tagZipLevel zl, const unsigned char *buffer, unsigned int size, MB::CUQueue &q) {
	unsigned int zSize;
	unsigned short ratio;
	bool ok = true;
	unsigned int start = q.GetSize();
	if (!buffer)
		size = 0;
	MB::CStreamHeader sh;
	sh.RequestId = reqId;
	zSize = size;
	sh.Size = size;
	q << sh;
	switch (zl) {
		case MB::zlDefault:
			if (size > 20) {
				zSize = (unsigned int) (1.1 * size + 16);
				if (q.GetTailSize() < zSize) {
					unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
					q.ReallocBuffer(bufferSize);
				}
				zSize = q.GetTailSize();
				ok = MB::Compress(zl, buffer, size, (void*) q.GetBuffer(q.GetSize()), zSize);
				if (ok) {
					ratio = (unsigned short) (size / zSize) + 1;
					sh.OrTreat((unsigned char) (ratio / 256), old, false);
					sh.SetRatio((unsigned char) (ratio % 256));
					assert(sh.GetZipRatio(old) == ratio);
					sh.Size = zSize;
					q.SetSize(q.GetSize() + zSize);
					q.Replace(start, sizeof (sh), (const unsigned char*) &sh, sizeof (sh));
				} else {
					assert(false);
				}
			} else {
				q.Push(buffer, size);
			}
			break;
		case MB::zlBestSpeed:
			if (size > 400) {
				zSize = (unsigned int) (size + 420);
				if (q.GetTailSize() < zSize) {
					unsigned int bufferSize = q.GetMaxSize() + (zSize - q.GetTailSize());
					q.ReallocBuffer(bufferSize);
				}
				zSize = q.GetTailSize();
				ok = MB::Compress(zl, buffer, size, (void*) q.GetBuffer(q.GetSize()), zSize);
				if (ok) {
					ratio = (unsigned short) (size / zSize) + 1;
					assert(ratio < 256);
					sh.SetRatio((unsigned char) ratio);
					assert(sh.GetZipRatio(old) == ratio);
					sh.Size = zSize;
					q.SetSize(q.GetSize() + zSize);
					q.Replace(start, sizeof (sh), (const unsigned char*) &sh, sizeof (sh));
				} else {
					assert(false);
				}
			} else {
				q.Push(buffer, size);
			}
			break;
		default:
			ok = false;
			assert(false); //not implemented
			break;
	}
	if (!ok)
		q.SetSize(start);
	return (q.GetSize() - start);
}

unsigned int CServerSession::SendExceptionResult(const char* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
	MB::CScopeUQueue su;
	MB::Utilities::ToWide(errMessage, *su);
	return SendExceptionResult((const wchar_t*)su->GetBuffer(), errWhere, requestId, errCode);
}

unsigned int CServerSession::SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
	{
		CAutoLock sl(m_mutex);
		if (ServiceId == MB::sidHTTP) {
			UHTTP::CWebResponseProcessor *pWebResponseProcessor = NULL;
			if (m_pHttpContext)
				pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
			if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake() && m_pHttpContext->GetResponseProgress().Status != UHTTP::hrsCompleted)) {
				unsigned int res = pWebResponseProcessor->SendExceptionResult(errMessage, errWhere, requestId, errCode, m_qWrite);
				Write(NULL, 0);
				return res;
			} else {
				return Connection::CConnectionContext::SendExceptionResult(m_ccb.Id, errMessage, errWhere, requestId, errCode);
			}
		}
	}
	if (requestId == 0)
		requestId = GetCurrentRequestId();
	if (errCode == 0)
		errCode = ERROR_EXCEPTION_CAUGHT;
	MB::CScopeUQueue sb;
	sb->Push((const unsigned char*) &requestId, sizeof (requestId));
	sb << errMessage << errCode << errWhere;
	return SendReturnData(MB::idServerException, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::CloseInternal() {
	if (m_ulIndex == 0)
		return;
	bool notify = m_ccb.ChatGroups.size() > 0;
	if (notify) {
		if (ServiceId != MB::sidHTTP || (m_pHttpContext && m_pHttpContext->IsWebSocket())) {
			g_pServer->Exit(this, m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
			m_ccb.ChatGroups.clear();
		} else if (ServiceId == MB::sidHTTP) {
			Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
			if (sp && !sp->IsGet && sp->IsOpera) {
				//Opera AJAX has problem in keep-alive
			} else {
				g_pServer->Exit(this, m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
				m_ccb.ChatGroups.clear();
				Connection::CConnectionContext::RemoveConnectionContext(m_ccb.Id.c_str());
			}
		}
		g_pServer->m_IoService.post(boost::bind(&CServerSession::Close, this));
		return;
	}
	OnClose();
	UHTTP::CHttpContext::Unlock(m_pHttpContext);
	m_pHttpContext = NULL;
	CErrorCode ec;
	if (m_pSslSocket)
		m_pSslSocket->lowest_layer().close(ec);
	else
		GetSocket().close(ec);
	g_pServer->Recycle(this);
	m_ulIndex = 0;
}

boost::posix_time::ptime& CServerSession::GetLatestTime() {
	return (m_ccb.RecvTime > m_ccb.SendTime) ? m_ccb.RecvTime : m_ccb.SendTime;
}

unsigned int CServerSession::GetCountOfJoinedChatGroups() {
	CAutoLock sl(m_mutex);
	return (unsigned int) m_ccb.ChatGroups.size();
}

unsigned int CServerSession::GetChatGroups(unsigned int *pChatGroup, unsigned int count) {
	unsigned int n;
	if (pChatGroup == NULL)
		count = 0;
	CAutoLock sl(m_mutex);
	if (count > (unsigned int) m_ccb.ChatGroups.size())
		count = (unsigned int) m_ccb.ChatGroups.size();
	for (n = 0; n < count; ++n) {
		pChatGroup[n] = m_ccb.ChatGroups[n];
	}
	return n;
}

void CServerSession::Write(const unsigned char *s, unsigned int nSize) {
	unsigned int ulLen;
	if (m_WriteBuffer) {
		m_qWrite.Push(s, nSize);
		return;
	}

	ulLen = m_qWrite.GetSize();
	if (ulLen == 0 && s && nSize > 0) {
		m_WriteBuffer = GetIoBuffer();
		if (nSize <= IO_BUFFER_SIZE) {
			::memcpy(m_WriteBuffer, s, nSize);
			ulLen = nSize;
		} else {
			::memcpy(m_WriteBuffer, s, IO_BUFFER_SIZE);
			ulLen = IO_BUFFER_SIZE;

			//remaining
			m_qWrite.Push(s + IO_BUFFER_SIZE, nSize - IO_BUFFER_SIZE);
		}
	} else {
		m_qWrite.Push(s, nSize);
		ulLen = m_qWrite.GetSize();
		if (ulLen == 0)
			return;
		if (ulLen > IO_BUFFER_SIZE)
			ulLen = IO_BUFFER_SIZE;
		m_WriteBuffer = GetIoBuffer();
		m_qWrite.Pop(m_WriteBuffer, ulLen);
	}
	m_ccb.m_ulSent += ulLen;
	if (m_pSslSocket)
		boost::asio::async_write(*m_pSslSocket,
			boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CServerSession::OnWriteCompleted, this, nsPlaceHolders::error));
	else
		boost::asio::async_write(*m_pSocket,
			boost::asio::buffer(m_WriteBuffer, ulLen), boost::bind(&CServerSession::OnWriteCompleted, this, nsPlaceHolders::error));
}

void CServerSession::Read() {
	if (m_ReadBuffer || ((m_pUThread || m_nHttpCallCount) && m_qRead.GetIdleSize() < IO_BUFFER_SIZE))
		return;
	if (m_qRead.GetSize() > IO_BUFFER_SIZE && (m_nHttpCallCount || QueryRequestsQueuedInternally() > 1))
		return;
	m_ReadBuffer = GetIoBuffer();
	if (m_pSslSocket)
		m_pSslSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE),
			boost::bind(&CServerSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
	else
		m_pSocket->async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE),
			boost::bind(&CServerSession::OnReadCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
}

unsigned int CServerSession::GetCurrentRequestLen() {
	m_mutex.lock();
	unsigned int len = m_ReqInfo.Size;
	m_mutex.unlock();
	return len;
}

unsigned short CServerSession::GetCurrentRequestId() {
	m_mutex.lock();
	unsigned short s = m_ReqInfo.RequestId;
	m_mutex.unlock();
	return s;
}

unsigned int CServerSession::GetSndBytesInQueue() {
	m_mutex.lock();
	unsigned int len = m_qWrite.GetSize();
	m_mutex.unlock();
	return len;
}

unsigned int CServerSession::GetRcvBytesInQueue() {
	m_mutex.lock();
	unsigned int len = m_qRead.GetSize();
	m_mutex.unlock();
	return len;
}

unsigned int CServerSession::GetConnIndex() {
	/*m_mutex.lock();
	unsigned int index = m_ulIndex;
	m_mutex.unlock();
	return index;*/
	return m_ulIndex;
}

unsigned int CServerSession::GetServiceId() {
	m_mutex.lock();
	unsigned int id = ServiceId;
	m_mutex.unlock();
	return id;
}

bool CServerSession::Wait(unsigned int nTimeout) {
	boost::posix_time::time_duration td = boost::posix_time::milliseconds(nTimeout);
	CAutoLock al(m_mutex);
	return m_cv.timed_wait(al, td);
}

int CServerSession::GetErrorCode() {
	m_mutex.lock();
	int ec = m_ec.value();
	m_mutex.unlock();
	return ec;
}

std::string CServerSession::GetErrorMessage() {
	CAutoLock sl(m_mutex);
	switch (m_ec.value()) {
		case ERROR_NO_ERROR:
			return "ok";
		case ERROR_WRONG_SWITCH:
			return "bad service switch";
		case ERROR_AUTHENTICATION_FAILED:
			return "authentication failed";
		case ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE:
			return "no service found at server side";
		case ERROR_BLOWFISH_KEY_WRONG:
			return "blowfish key";
		case ERROR_NOT_SWITCHED_YET:
			return "method SwitchTo not called yet";
		case ERROR_VERIFY_SALT:
			return "bad blowfish salt verification";
			break;
		case ERROR_BAD_HTTP_SIZE:
			return "bad http size";
			break;
		case ERROR_BAD_HTTP_REQUEST:
			return "bad http request";
			break;
		case ERROR_EXCEPTION_CAUGHT:
			return "exception caught";
			break;
		case ERROR_REQUEST_TOO_LARGE:
			return "request size too large";
			break;
		case ERROR_BAD_REQUEST:
			return "bad request";
			break;
		case ERROR_UNKNOWN_REQUEST:
			return "unknown request";
			break;
		default:
			break;
	}
	return m_ec.message();
}

unsigned int CServerSession::SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int ulBufferSize) {
	if (ServiceId == MB::sidHTTP) {
		g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
		return 0;
	}
	if (pBuffer == NULL)
		ulBufferSize = 0;
	CAutoLock sl(m_mutex);
	if (m_ulIndex == 0)
		return SOCKET_NOT_FOUND;
	if (usReqId != MB::idCancel && IsCanceledInternally())
		return REQUEST_CANCELED;
	if (m_pQBatch) {
		MB::CStreamHeader sh;
		sh.RequestId = usReqId;
		sh.Size = ulBufferSize;
		*m_pQBatch << sh;
		m_pQBatch->Push(pBuffer, ulBufferSize);
	} else if (!m_bZip) {
		MB::CStreamHeader sh;
		sh.RequestId = usReqId;
		sh.Size = ulBufferSize;
		MB::CScopeUQueue sb;
		sb << sh;
		sb->Push(pBuffer, ulBufferSize);
		Write(sb->GetBuffer(), sb->GetSize());
	} else { //zipped
		if (CompressResultTo(IsOld(), usReqId, m_zl, pBuffer, ulBufferSize, m_qWrite))
			Write(NULL, 0);
	}
	return ulBufferSize;
}

unsigned int CServerSession::RetrieveRequestBuffer(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek) {
	m_mutex.lock();
	unsigned int ret = RetrieveRequestBufferInternally(pBuffer, ulBufferSize, bPeek);
	m_mutex.unlock();
	return ret;
}

unsigned int CServerSession::RetrieveRequestBufferInternally(unsigned char *pBuffer, unsigned int ulBufferSize, bool bPeek) {
	unsigned int ulGet;
	if (pBuffer && ulBufferSize) {
		if (ulBufferSize > m_ReqInfo.Size)
			ulBufferSize = m_ReqInfo.Size;
		if (ulBufferSize == 0 || pBuffer == NULL)
			return 0;
		assert(ulBufferSize <= m_qRead.GetSize());
		if (m_ReqInfo.RequestId == MB::idSwitchTo) {
			bPeek = false;
			unsigned char *pHead = (unsigned char*) m_qRead.GetBuffer();
			ulGet = m_qRead.Pop((unsigned char*) pBuffer, ulBufferSize);
			::memset(pHead, 0, ulBufferSize);
		} else {
			if (bPeek) {
				::memcpy(pBuffer, m_qRead.GetBuffer(), ulBufferSize);
				ulGet = ulBufferSize;
			} else
				ulGet = m_qRead.Pop((unsigned char*) pBuffer, ulBufferSize);
		}
		assert(ulGet == ulBufferSize);
		if (!bPeek)
			m_ReqInfo.Size -= ulGet;
	}
	return ulGet;
}

unsigned int CServerSession::QueryRequestsQueued() {
	m_mutex.lock();
	unsigned int count = QueryRequestsQueuedInternally();
	m_mutex.unlock();
	return count;
}

unsigned int CServerSession::QueryRequestsQueuedInternally() {
	if (ServiceId == MB::sidStartup)
		return 1;
	if (m_pHttpContext && m_pHttpContext->GetPS() != UHTTP::psComplete)
		return 1;
	unsigned int ulRtns = 0;
	if (m_ReqInfo.RequestId && m_ReqInfo.Size > 0)
		ulRtns++;
	unsigned int ulStartPos = m_ReqInfo.Size;
	while (ulStartPos + sizeof (MB::CStreamHeader) <= m_qRead.GetSize()) {
		MB::CStreamHeader *pStreamHeader = (MB::CStreamHeader *)m_qRead.GetBuffer(ulStartPos);
		ulStartPos += (pStreamHeader->Size + sizeof (MB::CStreamHeader));
		ulRtns++;
	}
	return ulRtns;
}

void CServerSession::OnNonBaseRequestArrive() {
	m_bDropSlowRequest = false;
	CServiceContext *pSC = g_pServer->SeekServiceContext(ServiceId);
	if (pSC == NULL) {
		PostCloseInternal(ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE);
		return;
	}
	POnRequestArrive p = m_ccb.SvsContext.m_OnRequestArrive;
	if (p != NULL) {
		CRAutoLock sl(m_mutex);
		p(MakeHandler(), m_ReqInfo.RequestId, m_ReqInfo.Size);
	}

	if (pSC->IsSlowRequest(m_ReqInfo.RequestId)) {
		if (m_bDropSlowRequest)
			return;
		assert(m_pUThread == NULL);
		m_pUThread = g_pServer->GetOneThread(pSC->GetSvsContext().m_ta);
		m_pUThread->PostMessage(this, m_ReqInfo.RequestId, WM_ASK_FOR_PROCESSING, NULL, 0);
		return;
	}
	POnFastRequestArrive pF = m_ccb.SvsContext.m_OnFastRequestArrive;
	if (pF != NULL) {
		CRAutoLock sl(m_mutex);
		pF(MakeHandler(), m_ReqInfo.RequestId, m_ReqInfo.Size);
	}
}

void CServerSession::OnBaseRequestArrive() {
	switch (m_ReqInfo.RequestId) {
		case MB::idDequeueConfirmed:
		{
			unsigned int qHandle;
			MB::U_UINT64 mqPos;
			MB::U_UINT64 qIndex;
			assert(m_ReqInfo.Size == (sizeof (qHandle) + sizeof (mqPos) + sizeof (qIndex)));
			m_qRead >> qHandle >> mqPos >> qIndex;
			m_ReqInfo.Size = 0;
			CQueueMap::iterator it = m_mapDequeue.find(qHandle);
			if (it != m_mapDequeue.end()) {
				CQueueProperty &v = it->second;
				CQueueProperty::iterator b, end = v.end();
				for (b = v.begin(); b != end; ++b) {
					if (mqPos == b->first && qIndex == b->second) {
						v.erase(b);
						break;
					}
				}
			}
			g_pServer->ConfirmQueue(qHandle, mqPos, qIndex, true);
		}
			break;
		case MB::idStartBatching:
			{
				CRAutoLock sl(m_mutex);
				StartBatching();
				//SendReturnData(MB::idStartBatching, NULL, 0);
			}
			break;
		case MB::idCommitBatching:
			{
				CRAutoLock sl(m_mutex);
				//SendReturnData(MB::idCommitBatching, NULL, 0);
				CommitBatching();
			}
			break;
		case MB::idSetZipLevelAtSvr:
			assert(sizeof (int) == m_ReqInfo.Size);
		{
			int zl;
			m_qRead >> zl;
			m_ReqInfo.Size -= sizeof (zl);
			m_zl = (MB::tagZipLevel)zl;
			CRAutoLock sl(m_mutex);
			SendReturnData(MB::idSetZipLevelAtSvr, (const unsigned char*) &zl, sizeof (zl));
		}
			break;
		case MB::idTurnOnZipAtSvr:
			assert(sizeof (bool) == m_ReqInfo.Size);
		{
			m_qRead >> m_bZip;
			m_ReqInfo.Size -= sizeof (m_bZip);
			bool zip = m_bZip;
			CRAutoLock sl(m_mutex);
			SendReturnData(MB::idSetZipLevelAtSvr, (const unsigned char*) &zip, sizeof (zip));
		}
			break;
		case MB::idSetSockOptAtSvr:
			assert(3 * sizeof (int) == m_ReqInfo.Size);
		{
			int optName;
			int optValue;
			int level;
			int hr = 0;
			m_qRead >> optName >> optValue >> level;
			m_ReqInfo.Size = 0;
			hr = ::setsockopt(GetSocket().native(), level, optName, (const char*) &optValue, sizeof (optValue));
			CRAutoLock sl(m_mutex);
			SendReturnData(MB::idSetSockOptAtSvr, (const unsigned char*) &hr, sizeof (hr));
		}
			break;
		case MB::idGetSockOptAtSvr:
			assert(2 * sizeof (int) == m_ReqInfo.Size);
		{
			int optName;
			int level;
			int optValue = 0;
#ifdef WIN32_64
			static int len = sizeof (optValue);
#else
			static socklen_t len = sizeof (optValue);
#endif

			m_qRead >> optName >> level;
			m_ReqInfo.Size = 0;
			::getsockopt(GetSocket().native(), level, optName, (char*) &optValue, &len);
			CRAutoLock sl(m_mutex);
			SendReturnData(MB::idGetSockOptAtSvr, (const unsigned char*) &optValue, sizeof (optValue));
		}
			break;

		case MB::idStopQueue:
		case MB::idStartQueue:
			m_qRead >> m_ClientQFile;
			m_ReqInfo.Size = 0;
			break;
		case MB::idCancel:
			{
				CRAutoLock sl(m_mutex);
				AbortBatching();
			}
		case MB::idDoEcho:
		default:
			{
				unsigned short id;
				MB::CScopeUQueue sb;
				sb->Push(m_qRead.GetBuffer(), m_ReqInfo.Size);
				m_qRead.Pop(m_ReqInfo.Size);
				m_ReqInfo.Size = 0;
				id = m_ReqInfo.RequestId;
				CRAutoLock sl(m_mutex);
				SendReturnData(id, sb->GetBuffer(), sb->GetSize());
			}
			break;
	}

	POnBaseRequestCame p = m_ccb.SvsContext.m_OnBaseRequestCame;
	if (p != NULL) {
		CRAutoLock sl(m_mutex);
		p(MakeHandler(), m_ReqInfo.RequestId);
	}
}

bool CServerSession::DoAuthentication(unsigned int ServiceId) {
	g_pServer->m_mutex.lock();
	POnIsPermitted p = g_pServer->m_pOnIsPermitted;
	g_pServer->m_mutex.unlock();
	if (p != NULL) {
		CRAutoLock sl(m_mutex);
		return p(MakeHandler(), ServiceId);
	}
	return false;
}

void CServerSession::OnSwitchTo(unsigned int OldServiceId, unsigned int NewServiceId) {
	POnSwitchTo p = m_ccb.SvsContext.m_OnSwitchTo;
	unsigned int errCode = 0;
	MB::CScopeUQueue sb;
	sb << errCode;
	MB::CSwitchInfo si = g_pServer->m_ServerInfo;
	si.ServiceId = ServiceId;
	sb << si;
	CRAutoLock sl(m_mutex);
	if (NewServiceId != MB::sidHTTP)
		SendReturnData(MB::idSwitchTo, sb->GetBuffer(), sb->GetSize());
	if (p != NULL)
		p(MakeHandler(), OldServiceId, NewServiceId);
}

void CServerSession::OnRA() {
	if (m_ReqInfo.RequestId == MB::idSwitchTo) {
		MB::CScopeUQueue sb(m_ReqInfo.IsBigEndian(), m_ReqInfo.GetOS());
		if (sb->GetMaxSize() < m_ReqInfo.Size)
			sb->ReallocBuffer(m_ReqInfo.Size + 16);
		unsigned int len = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
		sb->SetSize(len);
		sb >> m_ClientInfo;
		if (m_ClientInfo.ServiceId == ServiceId) {
			PostCloseInternal(ERROR_WRONG_SWITCH);
			return;
		}

		CServiceContext *pServiceContext = g_pServer->SeekServiceContext(m_ClientInfo.ServiceId);
		if (pServiceContext == NULL) {
			PostCloseInternal(ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE);
			return;
		} else
			m_ccb.SvsContext = pServiceContext->GetSvsContext();

		assert(m_ccb.UserId.size() == 0);

		sb >> m_ccb.UserId;
		sb >> m_ccb.Password;

		unsigned int oldServiceId = ServiceId;
		unsigned int svsId = m_ClientInfo.ServiceId;
		bool ok = true;
		if (svsId == MB::sidStartup) {
			ServiceId = m_ClientInfo.ServiceId;
			OnSwitchTo(oldServiceId, svsId);
		} else if ((g_pServer->GetSharedAM() && ServiceId != MB::sidStartup) || DoAuthentication(svsId)) {
			ServiceId = m_ClientInfo.ServiceId;
			OnSwitchTo(oldServiceId, svsId);
		} else {
			ok = false;
		}
		m_ccb.Password.resize(m_ccb.Password.size(), ' ');
		m_ccb.Password.clear();
		if (!ok)
			PostCloseInternal(ERROR_AUTHENTICATION_FAILED);
		return;
	}
	if (ServiceId == MB::sidStartup) {
		PostCloseInternal(ERROR_NOT_SWITCHED_YET);
		return;
	}

	if (m_pHttpContext != NULL && !m_ReqInfo.IsFake()) {
		UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
		UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
		if (pWebRequestProcessor != NULL) {
			switch (m_ReqInfo.RequestId) {
				case MB::idEnter:
				case MB::idExit:
				case MB::idSendUserMessage:
				case MB::idSpeak:
				case MB::hrUserRequest:
					assert(m_ReqInfo.Size >= (sizeof (MB::U_INT64) + sizeof (UHTTP::tagSpError)));
					m_qRead >> pWebRequestProcessor->m_CurrentIndex;
					m_qRead >> pWebRequestProcessor->m_CurrentErrCode;
					assert(pWebRequestProcessor->m_CurrentErrCode <= UHTTP::seAuthenticationFailed);
					m_ReqInfo.Size -= (sizeof (MB::U_INT64) + sizeof (UHTTP::tagSpError));
					assert(pWebResponseProcessor->m_nReqCount > 0);
					assert(pWebRequestProcessor->m_CurrentIndex > 0);
					assert(pWebRequestProcessor->m_CurrentErrCode == UHTTP::seOk);
					--pWebResponseProcessor->m_nReqCount;
					if (pWebResponseProcessor->m_nReqCount == 0) {
						UHTTP::CWebSocketMsg *p = m_pHttpContext->GetWebSocketMsg();
						if (p)
							p->ParseStatus = UHTTP::psInitial;
					}
					break;
				default:
					break;
			}
		}
	}

	switch (m_ReqInfo.RequestId) {
		case MB::idTurnOnZipAtSvr:
		case MB::idPublicKeyFromSvr:
		case MB::idEncrypted:
		case MB::idBatchZipped:
		case MB::idCancel:
		case MB::idGetSockOptAtSvr:
		case MB::idSetSockOptAtSvr:
		case MB::idDoEcho:
		case MB::idStartBatching:
		case MB::idCommitBatching:
		case MB::idShrinkMemoryAtSvr:
		case MB::idEchoFromSvr:
		case MB::idPing:
		case MB::idSendingTooFast:
		case MB::idSendMySpecificBytes:
		case MB::idCleanTrack:
		case MB::idVerifySalt:
		case MB::idSetZipLevelAtSvr:
		case MB::idStartJob:
		case MB::idEndJob:
		case MB::idDropRequestResult:
		case MB::idDequeueConfirmed:
		case MB::idStartQueue:
		case MB::idStopQueue:
			OnBaseRequestArrive();
			break;
		case MB::idEnter:
		case MB::idSpeakEx:
		case MB::idSendUserMessageEx:
		case MB::idExit:
		{
			POnChatRequestComing p = m_ccb.SvsContext.m_OnChatRequestComing;
			if (p) {
				CRAutoLock rl(m_mutex);
				p(MakeHandler(), (MB::tagChatRequestID)m_ReqInfo.RequestId, m_ReqInfo.Size);
			}
		}
			OnChatRequestArrive();
			break;
		case MB::idSendUserMessage:
		case MB::idSpeak:
		{
			POnChatRequestComing p = m_ccb.SvsContext.m_OnChatRequestComing;
			if (p) {
				CRAutoLock rl(m_mutex);
				p(MakeHandler(), (MB::tagChatRequestID)m_ReqInfo.RequestId, m_ReqInfo.Size);
			}
		}
			OnChatVariantRequestArrive();
			break;
		default:
			OnNonBaseRequestArrive();
			break;
	}
}

void CServerSession::SendChatResult(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
	MB::CScopeUQueue sb;
	sb << senderAddr << (short) senderClientPort << sendUserId << senderServiceId;
	if (pBuffer == NULL)
		size = 0;
	if (size > 0)
		sb->Push(pBuffer, size);
	SendReturnData(usReqId, sb->GetBuffer(), sb->GetSize());
}

void CServerSession::BounceBackMessage(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
	unsigned short port;
	std::string addr;
	unsigned int svsId;
	GetPeerIpAddr(addr, &port);
	std::wstring userId = m_ccb.UserId;
	svsId = ServiceId;
	CRAutoLock sl(m_mutex);
	SendChatResult(addr.c_str(), port, userId.c_str(), svsId, usReqId, pBuffer, size);
}

void CServerSession::OnChatVariantRequestArrive() {
	unsigned int nCount;
	bool b = IsSameEndian();
	POnChatRequestCame crp = m_ccb.SvsContext.m_OnChatRequestCame;
	MB::CScopeUQueue sb(m_ReqInfo.IsBigEndian(), m_ReqInfo.GetOS());
	if (m_ReqInfo.Size > sb->GetMaxSize())
		sb->ReallocBuffer(m_ReqInfo.Size + 16);
	if (m_ReqInfo.Size > 0) {
		nCount = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
		sb->SetSize(nCount);
	}
	MB::UVariant vtMsg;
	switch (m_ReqInfo.RequestId) {
		case MB::idSpeak:
		{
			unsigned short vt = 0;
			unsigned int nCount = 0;
			sb >> vtMsg;
			sb >> vt;
			sb >> nCount;
			std::vector<unsigned int> vGroup, vFinal;
			if (nCount > 0) {
				unsigned int *p = (unsigned int*) sb->GetBuffer();
				if (!b && nCount)
					ChangeUInt32Endian(p, nCount);
				vGroup.assign(p, p + nCount);
				sb->Pop(nCount * sizeof (unsigned int));
			}
			assert(sb->GetSize() == 0);
			if (vGroup.size() > 0) {
				g_pServer->Speak(this, vGroup.data(), (unsigned int) vGroup.size(), vtMsg);
				if (crp) {
					CRAutoLock sl(m_mutex);
					crp(MakeHandler(), MB::idSpeak);
				}
			}
			sb->SetSize(0);
			sb << vtMsg;
			Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vGroup.data(), (unsigned int) vGroup.size(), vFinal);
			nCount = (unsigned int) vFinal.size();
			sb->Push((const unsigned char*) &nCount, sizeof (nCount));
			if (nCount > 0)
				sb->Push((const unsigned char*) vFinal.data(), nCount * sizeof (unsigned int));

			if (ServiceId == MB::sidHTTP) {
				UHTTP::CWebResponseProcessor *pWebResponseProcessor = NULL;
				if (m_pHttpContext)
					pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
				if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
					pWebResponseProcessor->BounceBackSpeak(vFinal.data(), nCount, m_qWrite);
					Write(NULL, 0);
				} else {
					Connection::CConnectionContext::Speak(m_ccb.Id, vtMsg, vFinal.data(), nCount);
				}
			} else
				BounceBackMessage(MB::idSpeak, sb->GetBuffer(), sb->GetSize());

			/*
			if (m_pHttpContext != NULL) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor != NULL) {
							pWebResponseProcessor->BounceBackSpeak(vFinal.data(), nCount, m_qWrite);
							Write(NULL, 0);
					}
			} else
					BounceBackMessage(MB::idSpeak, sb->GetBuffer(), sb->GetSize());*/
		}
			break;
		case MB::idSendUserMessage:
		{
			std::wstring userId;
			sb >> userId;
			sb >> vtMsg;
			g_pServer->SendUserMessage(this, userId.c_str(), vtMsg);
			if (crp) {
				CRAutoLock sl(m_mutex);
				crp(MakeHandler(), MB::idSendUserMessage);
			}
			assert(sb->GetSize() == 0);
			sb->SetSize(0);
			sb << vtMsg;
			if (ServiceId == MB::sidHTTP) {
				UHTTP::CWebResponseProcessor *pWebResponseProcessor = NULL;
				if (m_pHttpContext)
					pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
				if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
					pWebResponseProcessor->BounceBackSendUserMessage(m_qWrite);
					Write(NULL, 0);
				} else {
					Connection::CConnectionContext::SendUserMessage(m_ccb.Id, vtMsg, userId.c_str(), m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size());
				}
			} else
				BounceBackMessage(MB::idSendUserMessage, sb->GetBuffer(), sb->GetSize());

			/*
			if (m_pHttpContext != NULL) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor != NULL) {
							pWebResponseProcessor->BounceBackSendUserMessage(m_qWrite);
							Write(NULL, 0);
					}
			} else
					BounceBackMessage(MB::idSendUserMessage, sb->GetBuffer(), sb->GetSize());*/
		}
			break;
		default:
			//not implemented
			assert(false);
			break;
	}
}

void CServerSession::OnChatRequestArrive() {
	unsigned int nCount;
	bool b = IsSameEndian();
	POnChatRequestCame crp = m_ccb.SvsContext.m_OnChatRequestCame;
	MB::CScopeUQueue sb(m_ReqInfo.IsBigEndian(), m_ReqInfo.GetOS());
	if (m_ReqInfo.Size > sb->GetMaxSize())
		sb->ReallocBuffer(m_ReqInfo.Size + 16);
	if (m_ReqInfo.Size > 0) {
		nCount = RetrieveRequestBufferInternally((unsigned char*) sb->GetBuffer(), m_ReqInfo.Size, false);
		sb->SetSize(nCount);
	}
	switch (m_ReqInfo.RequestId) {
		case MB::idEnter:
		{
			unsigned int n;
			std::vector<unsigned int> vExit, vNew;

			nCount = sb->GetSize() / sizeof (unsigned int);
			std::vector<unsigned int> vChatGroup, vAll;
			g_pServer->GetChatGroups(vAll);
			if (!b && nCount)
				ChangeUInt32Endian((unsigned int*) sb->GetBuffer(), nCount);
			Connection::CConnectionContextBase::ChatGroupsAnd((const unsigned int*) sb->GetBuffer(), nCount, vAll.data(), (unsigned int) vAll.size(), vChatGroup);
			Connection::CConnectionContextBase::ChatGroupsNew(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vChatGroup.data(), (unsigned int) vChatGroup.size(), vNew, vExit);

			//fake request exit
			nCount = (unsigned int) vExit.size();
			if (nCount > 0) {
				CRAutoLock sl(m_mutex);
				Exit(vExit.data(), nCount);
			}

			//notify other clients
			nCount = (unsigned int) vNew.size();
			if (nCount > 0) {
				g_pServer->Enter(this, vNew.data(), nCount);
				if (crp) {
					CRAutoLock sl(m_mutex);
					crp(MakeHandler(), MB::idEnter);
				}
			}
			for (n = 0; n < nCount; ++n) {
				m_ccb.ChatGroups.push_back(vNew[n]);
			}
			Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
			if (sp)
				sp->ChatGroups = m_ccb.ChatGroups;

			//bounce back enter
			sb->SetSize(0);
			nCount = (unsigned int) vNew.size();
			sb->Push((const unsigned char*) vNew.data(), nCount * sizeof (unsigned int));
			if (ServiceId == MB::sidHTTP) {
				UHTTP::CWebResponseProcessor *pWebResponseProcessor = NULL;
				if (m_pHttpContext)
					pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
				if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
					pWebResponseProcessor->BounceBackEnter(vNew.data(), nCount, m_qWrite);
					Write(NULL, 0);
				} else
					Connection::CConnectionContext::Enter(m_ccb.Id, vNew.data(), nCount);
			} else
				BounceBackMessage(MB::idEnter, sb->GetBuffer(), sb->GetSize());
		}
			break;
		case MB::idSpeakEx:
			assert(sb->GetSize() >= sizeof (unsigned int));
		{
			sb >> nCount;
			assert(sb->GetSize() >= nCount * sizeof (unsigned int));
			std::vector<unsigned int> vGroup, vFinal;
			if (nCount > 0) {
				unsigned int *p = (unsigned int*) sb->GetBuffer();
				if (!b && nCount)
					ChangeUInt32Endian(p, nCount);
				vGroup.assign(p, p + nCount);
				sb->Pop(nCount * sizeof (unsigned int));
			}
			if (vGroup.size() > 0) {
				g_pServer->SpeakEx(this, sb->GetBuffer(), sb->GetSize(), vGroup.data(), (unsigned int) vGroup.size());
				if (crp) {
					CRAutoLock sl(m_mutex);
					crp(MakeHandler(), MB::idSpeakEx);
				}
			}
			Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), vGroup.data(), (unsigned int) vGroup.size(), vFinal);
			nCount = (unsigned int) vFinal.size();
			if (nCount > 0)
				sb->Insert((const unsigned char*) vFinal.data(), nCount * sizeof (unsigned int));
			sb->Insert((const unsigned char*) &nCount, sizeof (nCount));
			BounceBackMessage(MB::idSpeakEx, sb->GetBuffer(), sb->GetSize());
		}
			break;
		case MB::idSendUserMessageEx:
			assert(sb->GetSize() >= sizeof (unsigned int));
		{
			std::wstring userId;
			sb >> userId;
			g_pServer->SendUserMessage(this, userId.c_str(), sb->GetBuffer(), sb->GetSize());
			if (crp) {
				CRAutoLock sl(m_mutex);
				crp(MakeHandler(), MB::idSendUserMessageEx);
			}
			BounceBackMessage(MB::idSendUserMessageEx, sb->GetBuffer(), sb->GetSize());
		}
			break;
		case MB::idExit:
		{
			std::vector<unsigned int> v0;
			if (sb->GetSize() == 0) {
				v0 = m_ccb.ChatGroups;
				m_ccb.ChatGroups.clear();
			} else {
				assert(sb->GetSize() >= sizeof (unsigned int));
				unsigned int *p = NULL;
				nCount = sb->GetSize() / sizeof (unsigned int);
				if (nCount > 0) {
					p = (unsigned int*) sb->GetBuffer();
					if (!b)
						ChangeUInt32Endian(p, nCount);
				}
				std::vector<unsigned int> v(p, p + nCount), v1;
				Connection::CConnectionContextBase::ChatGroupsAnd(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), v.data(), (unsigned int) v.size(), v0);
				Connection::CConnectionContextBase::ChatGroupsNew(m_ccb.ChatGroups.data(), (unsigned int) m_ccb.ChatGroups.size(), v0.data(), (unsigned int) v0.size(), v, v1);
				m_ccb.ChatGroups = v1;
				Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
				if (sp)
					sp->ChatGroups = v1;
			}
			sb->SetSize(0);
			nCount = (unsigned int) v0.size();
			if (nCount > 0) {
				g_pServer->Exit(this, v0.data(), nCount);
				if (crp) {
					CRAutoLock sl(m_mutex);
					crp(MakeHandler(), MB::idExit);
				}
				sb->Push((const unsigned char*) v0.data(), sizeof (unsigned int) *nCount);
			}
			if (m_ulIndex) {
				if (ServiceId == MB::sidHTTP) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = NULL;
					if (m_pHttpContext)
						pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
					if ((m_pHttpContext && m_pHttpContext->IsWebSocket()) || (pWebResponseProcessor && !m_ReqInfo.IsFake())) {
						pWebResponseProcessor->BounceBackExit(v0.data(), nCount, m_qWrite);
						Write(NULL, 0);
					} else {
						Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
						if (sp) {
							Connection::CConnectionContext::Exit(m_ccb.Id.c_str(), v0.data(), nCount);
						}
					}
				} else
					BounceBackMessage(MB::idExit, sb->GetBuffer(), sb->GetSize());
			}
		}
			break;
		default:
			//not implemented
			assert(false);
			break;
	}
}

bool CServerSession::ProcessWithLock() {
	CAutoLock sl(m_mutex);
	Read();
	bool b = Process();
	Write(NULL, 0);
	return b;
}

bool CServerSession::IsSecure() {
	return (m_pSslSocket != NULL);
}

bool CServerSession::PreocessWebRequest(MB::CUQueue &q) {
	bool Continue;
	assert(boost::this_thread::get_id() == g_pServer->GetMainThreadId());
	UHTTP::CWebRequestProcessor *pWebRequestProcessor = m_pHttpContext->GetWebRequestProcessor();
	const UHTTP::UHttpRequest &ur = pWebRequestProcessor->GetUHttpRequest();
	UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
	if (ur.ErrCode != UHTTP::seOk) {
		assert(pWebResponseProcessor->m_nReqCount > 0);
		pWebResponseProcessor->ProcessBadRequest(q, ur.ErrCode);
		return false;
	} else if (ur.SpRequest != UHTTP::srSwitchTo) {
		if (!ur.Id || (!m_pHttpContext->IsWebSocket() && !Connection::CConnectionContext::SeekConnectionContext(ur.Id))) {
			assert(pWebResponseProcessor->m_nReqCount > 0);
			pWebResponseProcessor->ProcessBadRequest(q, UHTTP::seAuthenticationFailed);
			return false;
		}
	}

	if (ur.SpRequest != UHTTP::srSwitchTo && !m_pHttpContext->IsWebSocket() && m_ccb.Id.size() == 0) {
		Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(ur.Id);
		if (sp) {
			m_ccb = *sp;
			m_ccb.RecvTime = boost::posix_time::microsec_clock::local_time();
		}
	}

	switch (ur.SpRequest) {
		case UHTTP::srSwitchTo:
		{
			bool ok = true;
			std::string ipAddr;
			unsigned short port;
			m_ccb.UserId = ur.GetUserIdW();
			m_ccb.Password = ur.GetPwdW();
			UHTTP::UHttpRequest &bad = (UHTTP::UHttpRequest&)ur;
			bad.CleanPwd();
			assert(pWebResponseProcessor->m_nReqCount == 1);
			POnHttpAuthentication p = m_ccb.SvsContext.m_OnHttpAuthentication;
			if (p) {
				CRAutoLock s(m_mutex);
				ok = p(MakeHandler(), m_ccb.UserId.c_str(), m_ccb.Password.c_str());
			}
			m_ccb.Password.resize(m_ccb.Password.size(), ' ');
			m_ccb.Password.clear();
			GetPeerIpAddr(ipAddr, &port);
			--pWebResponseProcessor->m_nReqCount;
			m_ccb.Id = pWebResponseProcessor->ProcessHttpSwitch(ok, ipAddr.c_str(), port, q);
			if (!m_pHttpContext->IsWebSocket()) {
				Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(m_ccb.Id.c_str());
				sp->SvsContext = m_ccb.SvsContext;
				sp->m_ulRead += m_ccb.m_ulRead;
				sp->m_ulSent += m_ccb.m_ulSent;
				m_ccb.Pt = sp->Pt;
				m_ccb.IsGet = sp->IsGet;
				m_ccb.IsOpera = sp->IsOpera;
			}
			Continue = (m_nHttpCallCount > 0);
		}
			break;
		case UHTTP::srPing:
			assert(pWebResponseProcessor->m_nReqCount == 1);
			--pWebResponseProcessor->m_nReqCount;
			pWebResponseProcessor->ProcessPing(q);
		{
			POnBaseRequestCame p = m_ccb.SvsContext.m_OnBaseRequestCame;
			if (p) {
				CRAutoLock s(m_mutex);
				p(MakeHandler(), MB::idPing);
			}
		}

			Continue = false;
			break;
		case UHTTP::srClose:
			assert(pWebResponseProcessor->m_nReqCount == 1);
			--pWebResponseProcessor->m_nReqCount;
			pWebResponseProcessor->ProcessClose(q);
			Continue = false;
			break;
		default:
		{
			const MB::CUQueue &req = pWebRequestProcessor->GetBinaryRequests();
			m_qRead.Insert(req.GetBuffer(), req.GetSize(), m_ReqInfo.Size);
			m_nHttpCallCount += ur.GetReqCount();
			Continue = true;
		}
			break;
	}
	return Continue;
}

bool CServerSession::ProcessWebSocketRequest() {
	const unsigned char *end = m_pHttpContext->ParseWSMsg(m_qRead.GetBuffer(), m_qRead.GetSize());
	unsigned int len = (unsigned int) (end - m_qRead.GetBuffer());
	UHTTP::CWebSocketMsg *pWebSocketMsg = m_pHttpContext->GetWebSocketMsg();
	if ((m_pHttpContext->GetContentLength() + 64) > m_qRead.GetMaxSize())
		m_qRead.ReallocBuffer((unsigned int) m_pHttpContext->GetContentLength() + 64);
	if (pWebSocketMsg->ParseStatus == UHTTP::psComplete && len > 0 && pWebSocketMsg->IsFin()) {
		m_qRead.Pop(len);
		UHTTP::tagWSOpCode wsOpCode = pWebSocketMsg->GetOpCode();
		switch (wsOpCode) {
			case UHTTP::ocTextMsg:
			{
				const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
				MB::CScopeUQueue su;
				bool ok = PreocessWebRequest(*su);
				pWebSocketMsg->Content.SetSize(0);
				Write(su->GetBuffer(), su->GetSize());
				UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
				if (pWebResponseProcessor->m_nReqCount == 0)
					pWebSocketMsg->ParseStatus = UHTTP::psInitial;
				return ok;
			}
				break;
			case UHTTP::ocMsgContinuation:
				assert(false);
				return false;
				break;
			case UHTTP::ocConnectionClose:
				m_qRead.SetSize(0);
				m_pHttpContext->PrepareWSResponseMessage(NULL, 0, wsOpCode, m_qWrite);
				return true;
				break;
			case UHTTP::ocPing:
			case UHTTP::ocPong:
				assert(false);
				return false;
				break;
			case UHTTP::ocBinaryMsg:
			default:
				return false;
				break;
		}
	} else {
		return false;
	}
	return true;
}

bool CServerSession::ProcessAjaxRequest() {
	MB::CScopeUQueue su;
	m_pHttpContext->SetContent(m_qRead.GetBuffer(), (unsigned int) m_pHttpContext->GetContentLength());
	m_qRead.Pop((unsigned int) m_pHttpContext->GetContentLength());
	const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
	bool b = PreocessWebRequest(*su);
	switch (ur.SpRequest) {
		case UHTTP::srClose:
		{
			MB::CStreamHeader reqInfo;
			reqInfo.MakeFake();
			reqInfo.RequestId = MB::idExit;
			m_qRead << reqInfo;
			++m_nHttpCallCount;
			b = true;
		}
			assert(ur.GetReqCount() == 1);
			Write(su->GetBuffer(), su->GetSize());
			break;
		case UHTTP::srSwitchTo:
		case UHTTP::srPing:
			assert(ur.GetReqCount() == 1);
			Write(su->GetBuffer(), su->GetSize());
			break;
		default:
			if (!b) {
				Write(su->GetBuffer(), su->GetSize());
			}
			break;
	}
	return b;
}

bool CServerSession::ProcessJavaScriptRequest() {
	MB::CScopeUQueue su;
	const UHTTP::UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
	bool b = PreocessWebRequest(*su);
	switch (ur.SpRequest) {
		case UHTTP::srClose:
		{
			MB::CStreamHeader reqInfo;
			reqInfo.RequestId = MB::idExit;
			reqInfo.MakeFake();
			m_qRead << reqInfo;
			++m_nHttpCallCount;
			b = true;
		}
			assert(ur.GetReqCount() == 1);
			Write(su->GetBuffer(), su->GetSize());
			break;
		case UHTTP::srSwitchTo:
		case UHTTP::srPing:
			assert(ur.GetReqCount() == 1);
			Write(su->GetBuffer(), su->GetSize());
			break;
		default:
			if (m_pHttpContext->IsSpRequest() && !m_pHttpContext->GetResponseProgress().Chunked && ur.GetReqCount() > 1) {
				m_pHttpContext->StartChunkedResponse(m_qWrite);
				Write(NULL, 0);
			}
			break;
	}
	return b;
}

bool CServerSession::ProcessHttpRequest() {
	if (m_pHttpContext == NULL) {
		m_pHttpContext = UHTTP::CHttpContext::Lock();
	}
	if (m_pHttpContext->IsWebSocket())
		return ProcessWebSocketRequest();
	UHTTP::tagParseStatus ps = m_pHttpContext->GetPS();
	do {
		if (ps >= UHTTP::psHeaders)
			break;
		const char *start = (const char*) m_qRead.GetBuffer();
		const char *end = m_pHttpContext->ParseHeaders(start);
		ps = m_pHttpContext->GetPS();
		if (ps >= UHTTP::psHeaders) {
			unsigned int parsed = (unsigned int) (end - start);
			m_qRead.Pop(parsed);
			m_qRead.SetHeadPosition();
			m_qRead.SetNull();
			if (ServiceId == MB::sidStartup) {
				MB::CScopeUQueue sb;
				MB::CSwitchInfo SwitchInfo;
				::memset(&SwitchInfo, 0, sizeof (SwitchInfo));
				SwitchInfo.ServiceId = MB::sidHTTP;
				SwitchInfo.MajorVersion = 2;
				sb << SwitchInfo;
				sb << m_ccb.UserId;
				sb << m_ccb.Password;
				{
					CRAutoLock sl(m_mutex);
					FakeAClientRequest(MB::idSwitchTo, sb->GetBuffer(), sb->GetSize());
				}

				if (m_pHttpContext->IsWebSocket()) {
					m_pHttpContext->PrepareResponse(NULL, 0, m_qWrite, UHTTP::hrfpAll);
					Write(NULL, 0);
				} else {
					m_pHttpContext->SetPostPS(UHTTP::psHeaders);
					g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
				}
				return true;
			}
		} else if ((ps == UHTTP::psFailed && ServiceId == MB::sidHTTP) || (ps >= UHTTP::psMethod && m_pHttpContext->GetResponseCode() >= 400)) {
			PostCloseInternal(0);
			return false;
		} else
			return false; //if headers are not available, stop here!
	} while (false);

	MB::U_UINT64 content_length = m_pHttpContext->GetContentLength();
	if (content_length != UHTTP::CONTENT_LEN_UNKNOWN) {
		unsigned int ms = m_qRead.GetMaxSize();
		if (content_length > ms && MULTIPLE_CONTEXT_LENGTH > ms)
			m_qRead.ReallocBuffer(MULTIPLE_CONTEXT_LENGTH + 128);
		if (m_qRead.GetSize() < content_length && m_qRead.GetSize() < MULTIPLE_CONTEXT_LENGTH) {
			return false;
		}
	} else {

	}

	MB::tagHttpMethod hm = m_pHttpContext->GetMethod();
	switch (hm) {
		case MB::hmGet:
		{
			assert(m_qRead.GetSize() == 0);
			m_pHttpContext->SetPostPS();
			UHTTP::tagHttpRequestType request_type = m_pHttpContext->GetHttpRequestType();
			if (request_type == UHTTP::hrtDownloadAdapter) {//download spadapter.js
				unsigned int len;
				assert(m_qRead.GetSize() == 0);
				UHTTP::CUJsLoader loader(m_pHttpContext->GetUserAgent(),
						m_pHttpContext->GetParams().Start,
						m_pHttpContext->GetHost(),
						m_pHttpContext->IsCrossDomain(),
						IsSecure());
				const char *code = loader.GetSPACode(len);
				m_pHttpContext->PrepareResponse((const unsigned char*) code, len, m_qWrite, UHTTP::hrfpAll);
				Write(NULL, 0);
				return false;
			} else if (request_type == UHTTP::hrtDownloadLoader) { //download uloader.js
				assert(m_qRead.GetSize() == 0);
				m_pHttpContext->StartDownloadFile(m_pHttpContext->GetUrl().Start + 1, m_qWrite);
				Write(NULL, 0);
				return false;
			} else if (request_type == UHTTP::hrtJsRequest) {
				assert(m_qRead.GetSize() == 0);
				return ProcessJavaScriptRequest();
			} else { //Non-SocketPro JavaScript request
				assert(m_qRead.GetSize() == 0);
				MB::CStreamHeader reqInfo;
				reqInfo.RequestId = MB::hrGet;
				m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
				++m_nHttpCallCount;
				return true;
			}
		}
			break;
		case MB::hmPost:
			if (m_pHttpContext->GetCM() != MB::cmUnknown || m_pHttpContext->GetTE() != MB::teUnknown) {
				m_pHttpContext->SetResponseCode(501); //Not Implemented
				m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
				m_pHttpContext->PrepareResponse(NULL, 0, m_qWrite, UHTTP::hrfpAll);
				Write(NULL, 0);
				return false;
			} else {
				if (m_pHttpContext->GetContentLength() <= m_qRead.GetSize()) {
					m_pHttpContext->SetPostPS();
					if (m_pHttpContext->GetHttpRequestType() == UHTTP::hrtJsRequest) {
						bool ok = ProcessAjaxRequest();
						if (!ok)
							return false;
					} else //Non-SocketPro JavaScript request
					{
						MB::CStreamHeader reqInfo;
						reqInfo.RequestId = MB::hrPost;
						reqInfo.Size = (unsigned int) m_pHttpContext->GetContentLength();
						m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
						++m_nHttpCallCount;
					}
				} else {
					return false;
				}
			}
			break;
		case MB::hmPut:
		case MB::hmOptions:
			if (m_pHttpContext->GetContentLength() != UHTTP::CONTENT_LEN_UNKNOWN) {
				MB::CStreamHeader reqInfo;
				reqInfo.Size = (unsigned int) m_pHttpContext->GetContentLength();
				switch (hm) {
					case MB::hmPut:
						reqInfo.RequestId = MB::hrPut;
						break;
					case MB::hmOptions:
						reqInfo.RequestId = MB::hrOptions;
						break;
				}
				m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
				++m_nHttpCallCount;
			} else {
				m_pHttpContext->SetResponseCode(501); //Not Implemented
				m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
				m_pHttpContext->PrepareResponse(NULL, 0, m_qWrite, UHTTP::hrfpAll);
				Write(NULL, 0);
				return false;
			}
			break;
		case MB::hmHead:
		case MB::hmDelete:
		case MB::hmTrace:
			if (m_pHttpContext->GetContentLength() == UHTTP::CONTENT_LEN_UNKNOWN || m_pHttpContext->GetContentLength() == 0) {
				MB::CStreamHeader reqInfo;
				switch (hm) {
					case MB::hmHead:
						reqInfo.RequestId = MB::hrHead;
						break;
					case MB::hmDelete:
						reqInfo.RequestId = MB::hrDelete;
						break;
					case MB::hmTrace:
						reqInfo.RequestId = MB::hrTrace;
						break;
				}
				m_qRead.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), m_ReqInfo.Size);
				++m_nHttpCallCount;
				break; //break only for the supported HTTP request
			}
		default:
			m_pHttpContext->SetResponseCode(501); //Not Implemented
			m_pHttpContext->AddResponseHeader(UHTTP::CHttpContext::Connection.c_str(), UHTTP::CHttpContext::SP_CONNECTION_CLOSE.c_str());
			m_pHttpContext->PrepareResponse(NULL, 0, m_qWrite, UHTTP::hrfpAll);
			Write(NULL, 0);
			return false;
			break;
	}
	return true;
}

bool CServerSession::Decompress() {
	unsigned int res;
	bool reset = false;
	bool defaultZipped = m_ReqInfo.IsDefaultZipped(IsOld());
	bool fastZip = m_ReqInfo.IsFastZipped(IsOld());

	if (m_ReqInfo.RequestId == MB::idBatchZipped) {
		if (defaultZipped || fastZip) {
			MB::CScopeUQueue sb;
			unsigned short ratio = m_ReqInfo.GetZipRatio();
			assert(ratio > 0);
			res = DecompressRequestTo(ratio, defaultZipped ? MB::zlDefault : MB::zlBestSpeed, m_qRead.GetBuffer(), m_ReqInfo.Size, *sb);
			assert(res > 0);
			m_qRead.Replace(0, m_ReqInfo.Size, sb->GetBuffer(), res);
		}
		m_ReqInfo.Size = 0;
		m_ReqInfo.RequestId = 0;
		reset = true;
	} else if (defaultZipped || fastZip) {
		MB::CScopeUQueue sb;
		unsigned short ratio = m_ReqInfo.GetZipRatio();
		res = DecompressRequestTo(ratio, defaultZipped ? MB::zlDefault : MB::zlBestSpeed, m_qRead.GetBuffer(), m_ReqInfo.Size, *sb);
		assert(res > 0);
		m_qRead.Replace(0, m_ReqInfo.Size, sb->GetBuffer(), res);
		m_ReqInfo.Size = res;
	}
	return reset;
}

bool CServerSession::Process(bool bSlowProcessed) {
	MB::U_UINT64 qClientPos;
	MB::U_UINT64 qIndex;
	bool bOk = false;
	if (m_pUThread)
		return false;
	if (ServiceId == MB::sidStartup) {
		m_qRead.SetNull();
		UHTTP::CHttpContext *p = UHTTP::CHttpContext::Lock();
		const char *end = p->ParseHeaders((const char*) m_qRead.GetBuffer(), true);
		UHTTP::tagParseStatus ps = p->GetPS();
		UHTTP::CHttpContext::Unlock(p);
		if (ps >= UHTTP::psMethod) {
			if (!ProcessHttpRequest())
				return false;
		}
	} else if (ServiceId == MB::sidHTTP) {
		if (m_nHttpCallCount == 0) {
			if (!ProcessHttpRequest())
				return false;
		}
	}

	//binary requests processing ......

	if (m_qRead.GetSize() < sizeof (m_ReqInfo))
		return bOk;
	while(true) {
		if (ServiceId == MB::sidHTTP && m_nHttpCallCount == 0) {
			if (m_qWrite.GetSize() < IO_BUFFER_SIZE) //this check reduces CPU and avoid extra buffer for m_qWrite
				g_pServer->m_IoService.post(boost::bind(&CServerSession::ProcessWithLock, this));
			return bOk;
		}

		if (m_ReqInfo.RequestId == 0 && m_qRead.GetSize() >= sizeof (m_ReqInfo))
		{
			m_qRead >> m_ReqInfo;
			if (ServiceId != MB::sidHTTP && m_ReqInfo.RequestId != MB::idSwitchTo) {
				m_qRead.SetEndian(m_ReqInfo.IsBigEndian());
				m_qRead.SetOS(m_ReqInfo.GetOS());
			}
		}

		if (m_ReqInfo.RequestId == 0 || m_qRead.GetSize() < m_ReqInfo.Size)
			return bOk;

		if (m_ReqInfo.GetQueued()) {
			MB::CScopeUQueue sb;
			m_qRead >> qClientPos >> qIndex;
			MQ_FILE::QAttr qa(qClientPos, qIndex);
			m_ReqInfo.Size -= (sizeof (qClientPos) + sizeof (qIndex));
			MB::CStreamHeader sh;
			sh.RequestId = MB::idDequeueConfirmed;
			sh.Size = (sizeof (qClientPos) + sizeof (qIndex));
			sb << sh << qClientPos << qIndex;
			m_qWrite.Push(sb->GetBuffer(), sb->GetSize());
			m_ReqInfo.SetQueued(false);
			m_pQLastIndex->Set(m_ClientQFile, qa);
		}

		if (Decompress())
			continue;

		g_pServer->m_mutex.lock();
		++g_pServer->m_nRequestCount;
		g_pServer->m_mutex.unlock();

		if (!IsSameEndian()) {
			MB::CUQueue::ChangeEndian((unsigned char*) &m_ReqInfo.RequestId, sizeof (m_ReqInfo.RequestId));
			if (m_ReqInfo.Size)
				MB::CUQueue::ChangeEndian((unsigned char*) &m_ReqInfo.Size, sizeof (m_ReqInfo.Size));
		}

		if (m_ReqInfo.RequestId == MB::idSwitchTo && m_ReqInfo.Size > (MAX_USERID_CHARS + MAX_PASSWORD_CHARS + sizeof (m_ClientInfo))) //2*(510) + 2*sizeof(unsigned short) + sizeof(m_ClientInfo)
		{
			PostCloseInternal(ERROR_BAD_REQUEST);
			bOk = false;
			break;
		}

		if (ServiceId == MB::sidStartup && (m_ReqInfo.Size > 1460 || m_ReqInfo.RequestId != MB::idSwitchTo || m_ReqInfo.Size < (4 + sizeof (m_ClientInfo)))) {
			PostCloseInternal(ERROR_BAD_REQUEST);
			bOk = false;
			break;
		}

		//preprocess unziping, debatching or both

		try {
			OnRA();
		} catch (MB::CMBException &err) {
			CRAutoLock sl(m_mutex);
			SendExceptionResult(err.what(), err.GetStack().c_str(), m_ReqInfo.RequestId, err.GetErrCode());
		} catch (std::exception &err) {
			CRAutoLock sl(m_mutex);
			SendExceptionResult(err.what(), "Inside request processing loop", m_ReqInfo.RequestId, MB_STL_EXCEPTION);
			break;
		} catch (...) {
			CRAutoLock sl(m_mutex);
			SendExceptionResult(L"Unknown exception caught", "Inside request processing loop", m_ReqInfo.RequestId, MB_UNKNOWN_EXCEPTION);
			break;
		}

		if (ServiceId == MB::sidHTTP)
			--m_nHttpCallCount;

		if (m_pUThread) {
			bOk = false;
			break;
		}

		bOk = true;
		m_ReqInfo.RequestId = 0;
		if (m_ReqInfo.Size > 0) {
			m_qRead.Pop(m_ReqInfo.Size);
			m_ReqInfo.Size = 0;
		}
	}
	return bOk;
}

void CServerSession::OnReadCompleted(const CErrorCode& Error, size_t nLen) {
	CAutoLock sl(m_mutex);
	if (!Error) {
		unsigned int len = (unsigned int) nLen;
		m_ccb.m_ulRead += len;
		m_ec.clear();
		m_ccb.RecvTime = boost::posix_time::microsec_clock::local_time();
		if (len && m_ReadBuffer) {
			if (m_qRead.GetTailSize() <= IO_BUFFER_SIZE && m_qRead.GetIdleSize() >= IO_BUFFER_SIZE)
				m_qRead.SetHeadPosition();
			m_qRead.Push(m_ReadBuffer, len);
		}
		m_qRead.SetNull();

		//clean password into memory
		if ((ServiceId == MB::sidStartup || (ServiceId == MB::sidHTTP && m_ccb.Id.empty())) && m_ReadBuffer)
			::memset(m_ReadBuffer, 0, IO_BUFFER_SIZE);
		ReleaseIoBuffer(m_ReadBuffer);
		Process();
		Read();
		Write(NULL, 0);
	} else {
		m_ec = Error;
		CloseInternal();
	}
}

void CServerSession::OnWriteCompleted(const CErrorCode& Error) {
	CAutoLock sl(m_mutex);
	m_ec = Error;
	ReleaseIoBuffer(m_WriteBuffer);
	unsigned int len = m_qWrite.GetSize();
	if (len > 5 * IO_BUFFER_SIZE && len < 8 * IO_BUFFER_SIZE)
		m_cv.notify_all();
	if (Error) {
		CloseInternal();
		return;
	}

	if (ServiceId == MB::sidHTTP) {
		do {
			if (m_pHttpContext == NULL) {
				if (m_qRead.GetSize())
					ProcessHttpRequest();
				break;
			}
			if (m_pHttpContext->IsWebSocket()) {
				if (m_qRead.GetSize())
					Process();
				break;
			}
			if (m_pHttpContext->GetResponseProgress().Status == UHTTP::hrsCompleted) {
				UHTTP::CHttpContext::Unlock(m_pHttpContext);
				m_pHttpContext = NULL;
				break;
			}
			if (m_qWrite.GetSize() < IO_BUFFER_SIZE) {
				m_qWrite.SetHeadPosition();
				m_pHttpContext->DownloadFile(m_qWrite);
			}
		} while (false);
	} else {
		Process();
	}
	if (m_qWrite.GetTailSize() < IO_BUFFER_SIZE && m_qWrite.GetHeadPosition() > IO_BUFFER_SIZE)
		m_qWrite.SetHeadPosition();

	m_ccb.SendTime = boost::posix_time::microsec_clock::local_time();

	Write(NULL, 0);
	Read();
}

const char* CServerSession::GetHTTPId() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return NULL;
	return m_ccb.Id.c_str();
}

unsigned int CServerSession::GetHTTPCurrentMultiplaxHeaders(MB::CHttpHeaderValue *HeaderValue, unsigned int count) {
	if (HeaderValue == NULL || count == 0)
		return 0;
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext || m_pHttpContext->GetCM() == MB::cmUnknown)
		return 0;
	unsigned int n, size = 0;
	const UHTTP::CHeaderValue *pHV = m_pHttpContext->GetMultiplaxContext()->GetHeaderValue(size);
	for (n = 0; n < size && count > 0; ++n, --count, ++pHV) {
		HeaderValue[n].Header = pHV->Header.Start;
		HeaderValue[n].Value = pHV->Value.Start;
	}
	return n;
}

unsigned int CServerSession::GetHTTPRequestHeaders(MB::CHttpHeaderValue *HeaderValue, unsigned int count) {
	if (HeaderValue == NULL || count == 0)
		return 0;
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return 0;
	unsigned int n, size = 0;
	const UHTTP::CHeaderValue *pHV = m_pHttpContext->GetRequestHeaders(size);
	for (n = 0; n < size && count > 0; ++n, --count, ++pHV) {
		HeaderValue[n].Header = pHV->Header.Start;
		HeaderValue[n].Value = pHV->Value.Start;
	}
	return n;
}

const char* CServerSession::GetHTTPPath() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return NULL;
	return m_pHttpContext->GetUrl().Start;
}

MB::U_UINT64 CServerSession::GetHTTPContentLength() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return 0;
	if (m_pHttpContext->GetMethod() != MB::hmPost)
		return 0;
	if (m_pHttpContext->GetTE() != MB::teUnknown || m_pHttpContext->GetCM() != MB::cmUnknown)
		return 0;
	return m_pHttpContext->GetContentLength();
}

const char* CServerSession::GetHTTPQuery() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return NULL;
	return m_pHttpContext->GetParams().Start;
}

bool CServerSession::DownloadFile(const char *filePath) {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return false;
	if (m_pHttpContext->GetMethod() != MB::hmGet)
		return false;
	if (m_pHttpContext->GetPS() != UHTTP::psComplete)
		return false;
	bool b = m_pHttpContext->StartDownloadFile(filePath, m_qWrite);
	if (m_pHttpContext->GetResponseProgress().Status == UHTTP::hrsCompleted) {
		UHTTP::CHttpContext::Unlock(m_pHttpContext);
		m_pHttpContext = NULL;
	}
	m_qWrite.SetNull();
	Write(NULL, 0);
	return b;
}

MB::tagHttpMethod CServerSession::GetHTTPMethod() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return MB::hmUnknown;
	return m_pHttpContext->GetMethod();
}

bool CServerSession::HTTPKeepAlive() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return true;
	return m_pHttpContext->IsKeepAlive();
}

bool CServerSession::IsWebSocket() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return false;
	return m_pHttpContext->IsWebSocket();
}

unsigned int CServerSession::StartChunkResponse() {
	CAutoLock sl(m_mutex);
	if (ServiceId != MB::sidHTTP || !m_pHttpContext)
		return BAD_OPERATION;
	if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
		return BAD_OPERATION;
	UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
	if (rs != UHTTP::hrsInitial)
		return BAD_OPERATION;
	if (!m_pHttpContext->AddResponseHeader(UHTTP::TRANSFER_ENCODING.c_str(), UHTTP::CHUNKED.c_str()))
		return RESULT_SENDING_FAILED;
	unsigned int start = m_qWrite.GetSize();
	m_pHttpContext->StartChunkedResponse(m_qWrite);
	start = (m_qWrite.GetSize() - start);
	Write(NULL, 0);
	return start;
}

unsigned int CServerSession::SendChunk(const unsigned char *buffer, unsigned int len) {
	if (!buffer)
		len = 0;
	if (!len)
		return 0;
	CAutoLock sl(m_mutex);
	if (ServiceId != MB::sidHTTP || !m_pHttpContext)
		return BAD_OPERATION;
	if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
		return BAD_OPERATION;
	UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
	if (rs < UHTTP::hrsHeadersOnly || rs > UHTTP::hrsContent)
		return BAD_OPERATION;
	const char *chunk = m_pHttpContext->SeekResponseHeaderValue(UHTTP::TRANSFER_ENCODING.c_str());
	if (!chunk || !boost::iequals(UHTTP::CHUNKED, chunk))
		return BAD_OPERATION;
	unsigned int start = m_qWrite.GetSize();
	m_pHttpContext->SendChunkedData(buffer, len, m_qWrite);
	m_qWrite.SetNull();
	start = (m_qWrite.GetSize() - start);
	Write(NULL, 0);
	return start;
}

unsigned int CServerSession::EndChunkResponse(const unsigned char *buffer, unsigned int len) {
	if (!buffer)
		len = 0;
	CAutoLock sl(m_mutex);
	if (ServiceId != MB::sidHTTP || !m_pHttpContext)
		return BAD_OPERATION;
	if (m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
		return BAD_OPERATION;
	UHTTP::tagHttpResponseStatus rs = m_pHttpContext->GetResponseProgress().Status;
	if (rs < UHTTP::hrsHeadersOnly || rs > UHTTP::hrsContent)
		return BAD_OPERATION;
	const char *chunk = m_pHttpContext->SeekResponseHeaderValue(UHTTP::TRANSFER_ENCODING.c_str());
	if (!chunk || !boost::iequals(UHTTP::CHUNKED, chunk))
		return BAD_OPERATION;
	unsigned int start = m_qWrite.GetSize();
	if (len)
		m_pHttpContext->SendChunkedData(buffer, len, m_qWrite);
	m_pHttpContext->SendChunkedData(NULL, 0, m_qWrite);
	m_qWrite.SetNull();
	start = (m_qWrite.GetSize() - start);
	Write(NULL, 0);
	return start;
}

bool CServerSession::IsCrossDomain() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return false;
	return m_pHttpContext->IsCrossDomain();
}

double CServerSession::GetHTTPVersion() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return 0.0;
	return m_pHttpContext->GetVersion();
}

bool CServerSession::HTTPGZipAccepted() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return false;
	return m_pHttpContext->IsGZipAccepted();
}

const char* CServerSession::GetHTTPUrl() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return NULL;
	return m_pHttpContext->GetUrl().Start;
}

const char* CServerSession::GetHTTPHost() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return NULL;
	return m_pHttpContext->GetHost();
}

MB::tagTransport CServerSession::GetHTTPTransport() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return MB::tUnknown;
	return m_pHttpContext->GetTransport();
}

MB::tagTransferEncoding CServerSession::GetHTTPTransferEncoding() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return MB::teUnknown;
	return m_pHttpContext->GetTE();

}

MB::tagContentMultiplax CServerSession::GetHTTPContentMultiplax() {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext)
		return MB::cmUnknown;
	return m_pHttpContext->GetCM();
}

bool CServerSession::SetHTTPResponseCode(unsigned int errCode) {
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext || m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
		return false;
	m_pHttpContext->SetResponseCode(errCode);
	return true;
}

bool CServerSession::SetHTTPResponseHeader(const char *uft8Header, const char *utf8Value) {
	if (uft8Header == NULL || ::strlen(uft8Header) == 0)
		return false;
	if (utf8Value == NULL)
		utf8Value = "";
	CAutoLock sl(m_mutex);
	if (!m_pHttpContext || m_pHttpContext->IsWebSocket() || m_pHttpContext->IsSpRequest())
		return false;
	return m_pHttpContext->AddResponseHeader(uft8Header, utf8Value);
}

unsigned int CServerSession::HTTPCallbackA(const char *name, const char *str) {
	unsigned int res = 0;
	CAutoLock sl(m_mutex);
	if (ServiceId != MB::sidHTTP || (m_pHttpContext && !m_pHttpContext->IsSpRequest()))
		return BAD_OPERATION;
	if (!m_pHttpContext || !m_pHttpContext->IsWebSocket())
		return Connection::CConnectionContext::SendResult(m_ccb.Id, m_ReqInfo.RequestId, str, name);
	else if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
		MB::CScopeUQueue su;
		Connection::CConnectionContext::SendWSResult(m_ReqInfo.RequestId, str, *su, name);
		unsigned int start = m_qWrite.GetSize();
		m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, m_qWrite);
		res = m_qWrite.GetSize() - start;
	} else
		return BAD_OPERATION;
	return res;
}

unsigned int CServerSession::SendHTTPReturnDataA(const char *str, unsigned int chars) {
	unsigned int res = 0;
	static const char *empty = "";
	if (str == NULL) {
		chars = 0;
		str = empty;
	} else if (chars == (~0))
		chars = (unsigned int) ::strlen(str);
	CAutoLock sl(m_mutex);
	if (ServiceId != MB::sidHTTP)
		return BAD_OPERATION;
	if (m_ReqInfo.RequestId > MB::idReservedTwo) {
		if (!m_pHttpContext || !m_pHttpContext->IsWebSocket())
			return Connection::CConnectionContext::SendResult(m_ccb.Id, m_ReqInfo.RequestId, str, (const char*) NULL);
		else if (m_pHttpContext && m_pHttpContext->IsWebSocket()) {
			MB::CScopeUQueue su;
			Connection::CConnectionContext::SendWSResult(m_ReqInfo.RequestId, str, *su, (const char*) NULL);
			unsigned int start = m_qWrite.GetSize();
			m_pHttpContext->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), UHTTP::ocTextMsg, m_qWrite);
			res = m_qWrite.GetSize() - start;
		} else
			return BAD_OPERATION;
	} else {
		if (!m_pHttpContext)
			return BAD_OPERATION;
		UHTTP::CWebResponseProcessor *pWebResponseProcessor = m_pHttpContext->GetWebResponseProcessor();
		if (pWebResponseProcessor)
			res = pWebResponseProcessor->ProcessUserRequest(str, m_qWrite);
		else {
			unsigned int start = m_qWrite.GetSize();
			m_pHttpContext->PrepareResponse((const unsigned char*) str, chars, m_qWrite);
			res = m_qWrite.GetSize() - start;
		}
	}
	Write(NULL, 0);
	return res;
}
