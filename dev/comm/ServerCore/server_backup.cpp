#include "stdafx.h"
#include "server.h"
#include <algorithm>

bool g_bRegistered = false;

CServerSession *GetSvrSession(USocket_Server_Handle h, unsigned int &index);

std::vector<CServerSession*>::iterator CServer::Seek(CServerSession *pSession) {
	std::vector<CServerSession*>::iterator it;
	std::vector<CServerSession*>::iterator end = m_aSession.end();
	if (pSession == NULL)
		return end;
	for (it = m_aSession.begin(); it != end; ++it) {
		if (*it == pSession)
			break;
	}
	return it;
}

void CServer::Recycle(CServerSession *pSession) {
	if (pSession) {
		USocket_Server_Handle h = pSession->MakeHandlerInternal();
		pSession->Initialize();
		if (CServerSession::m_pQLastIndex->IsDirty())
			CServerSession::m_pQLastIndex->Save();
		CAutoLock sl(m_mutex);

		//remove queue notification
		CMapQNotified::iterator s, end = m_mapQNotified.end();
		for (s = m_mapQNotified.begin(); s != end; ++s) {
			std::vector<CSNotified> &v = s->second;
			std::vector<CSNotified>::iterator b, send = v.end();
			for (b = v.begin(); b != send; ++b) {
				if (b->first == h) {
					v.erase(b);
					break;
				}
			}
		}

		//
		std::vector<CServerSession*>::iterator it = Seek(pSession);
		if (it != m_aSession.end()) {
			m_aSession.erase(it);
			m_aSessionDead.push_back(pSession);
		}
	}
}

CServer::CServer(int nParam)
: m_pSession(NULL),
m_pAcceptor(NULL),
m_pSslContext(NULL),
m_bZip(false),
m_nRequestCount(0),
m_ulTimerElapse(DefaultTimerElapse),
m_EncryptionMethod(MB::NoEncryption),
m_ulMinSwitchTime(DefaultMinSwitchTime),
m_ulRecycleGlobalMemoryInterval(DefaultRecycleGlobalMemoryInterval),
m_ulPingInterval(DefaultPingInterval),
m_ulSMInterval(DefaultSMInterval),
m_ulMaxConnectionsPerClient(DefaultMaxConnectionsPerClient),
m_ulMaxThreadIdleTimeBeforeSuicide(DefaultMaxThreadIdleTimeBeforeSuicide),
m_am(MB::amOwn),
m_bSharedAM(false),
m_pOnAccept(NULL),
m_nConnIndex(0),
m_bV6(false),
m_pThread(NULL),
m_Timer(m_IoService, boost::posix_time::microseconds(m_ulTimerElapse)),
m_TimerSM(m_IoService, boost::posix_time::microseconds(m_ulSMInterval)),
m_pOnIdle(NULL),
m_pOnSSLHandShakeCompleted(NULL),
m_pOnIsPermitted(NULL),
m_pOnClose(NULL),
m_nPort(0) {
	::memset(&m_ServerInfo, 0, sizeof (m_ServerInfo));
	m_ServerInfo.MajorVersion = 2;
}

void CServer::KillMainThread() {
	m_mutex.lock();
	if (m_pThread != NULL) {
		m_mutex.unlock();
		m_IoService.stop();
		m_pThread->join();
		m_mutex.lock();
		delete m_pThread;
		m_pThread = NULL;
	}
	m_mutex.unlock();
}

bool CServer::StartServerInternal(unsigned int port, unsigned int uiMaxBacklog) {
	m_mutex.lock();
	m_nPort = port;
	m_MainThreadId = boost::this_thread::get_id();
	bool bSuc = false;
	do {
		boost::asio::ssl::context_base::method method;
		if (m_pAcceptor || m_pSslContext)
			return false;
		if (IsSsl(method)) {
			m_pSslContext = new CSslContext(m_IoService, method);
			m_pSslContext->set_options(CSslContext::default_workarounds | CSslContext::no_sslv2, m_ec);
			if (m_ec)
				break;
			m_pSslContext->set_password_callback(boost::bind(&CServer::GetPassword, this), m_ec);
			if (m_ec)
				break;
			m_pSslContext->use_certificate_chain_file(m_strCertFile, m_ec);
			if (m_ec)
				break;
			if (m_strPfxFile.length() > 0) {
				//m_pSslContext->use_private_key_file(m_strPrivateKeyFile, CSslContext::pem, m_ec);
			} else
				m_pSslContext->use_private_key_file(m_strPrivateKeyFile, CSslContext::pem, m_ec);
			if (m_ec)
				break;
		}

		m_pAcceptor = new CAcceptor(m_IoService);
		boost::asio::ip::tcp::endpoint endpoint(m_bV6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), (unsigned short) port);
		m_pAcceptor->open(endpoint.protocol(), m_ec);
		if (m_ec)
			break;
		/*m_pAcceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), m_ec);
		if(m_ec)
										break;*/
		m_pAcceptor->bind(endpoint, m_ec);
		if (m_ec)
			break;
		m_pAcceptor->listen(uiMaxBacklog, m_ec);
		if (m_ec)
			break;
		m_pSession = new CServerSession();
		m_pAcceptor->async_accept(m_pSession->GetSocket().lowest_layer(), boost::bind(&CServer::OnAccepted, this, m_pSession, nsPlaceHolders::error));
		bSuc = true;
	} while (false);

	if (!bSuc) {
		delete m_pSession;
		delete m_pAcceptor;
		delete m_pSslContext;
		m_pSslContext = NULL;
		m_pAcceptor = NULL;
		m_pSession = NULL;
		m_mutex.unlock();
		PostQuit();
		m_mutex.lock();
	}
	m_mutex.unlock();
	return bSuc;
}

CServer::~CServer() {
	KillMainThread();
	CAutoLock sl(m_mutex);
	delete m_pAcceptor;
	delete m_pSslContext;
	delete m_pSession;
	Clean();
	DestroyThreadPool();
}

void CServer::CleanServiceContexts() {
	size_t n, size = m_vSC.size();
	for (n = 0; n < size; ++n) {
		CServiceContext *sc = m_vSC[n];
		delete sc;
	}
	m_vSC.clear();
}

std::vector<CServiceContext*>::iterator CServer::SeekSC(unsigned int nServiceId) {
	std::vector<CServiceContext*>::iterator it;
	std::vector<CServiceContext*>::iterator end = m_vSC.end();
	for (it = m_vSC.begin(); it != end; ++it) {
		if ((*it)->GetServiceId() == nServiceId)
			return it;
	}
	return end;
}

CServiceContext* CServer::SeekServiceContext(unsigned int nServiceId) {
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it != m_vSC.end())
		return *it;
	return NULL;
}

void CServer::RemoveServiceContext(unsigned int nServiceId) {
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it != m_vSC.end())
		m_vSC.erase(it);
}

bool CServer::AddSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
	if (sReqId <= MB::idDropRequestResult)
		return false;
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it != m_vSC.end())
		(*it)->AddSlowRequest(sReqId);
	return true;
}

bool CServer::AddService(unsigned int nServiceId, CSvsContext SvsContext) {
	if (nServiceId != MB::sidChat &&
			nServiceId != MB::sidHTTP &&
			nServiceId <= MB::sidReserved2)
		return false;
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it != m_vSC.end())
		return false;
	CServiceContext *p = new CServiceContext(nServiceId, SvsContext);
	m_vSC.push_back(p);
	return true;
}

void CServer::RemoveSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it != m_vSC.end())
		(*it)->RemoveSlowRequest(sReqId);
}

size_t CServer::GetCountOfServices() {
	CAutoLock sl(m_mutex);
	return m_vSC.size();
}

unsigned int CServer::GetCountOfSlowRequests(unsigned int nServiceId) {
	CAutoLock sl(m_mutex);
	std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
	if (it == m_vSC.end())
		return 0;
	return (unsigned int) ((*it)->GetCountOfSlowRequests());
}

unsigned int CServer::GetAllSlowRequestIds(unsigned int nServiceId, unsigned short *pReqId, unsigned int count) {
	unsigned int n;
	if (pReqId == NULL || count == 0)
		return 0;
	std::vector<unsigned short> vReqId;
	{
		CAutoLock sl(m_mutex);
		std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
		if (it == m_vSC.end())
			return 0;
		(*it)->GetAllSlowRequestId(vReqId);
	}
	if (count > (unsigned int) (vReqId.size()))
		count = (unsigned int) (vReqId.size());
	for (n = 0; n < count; ++n) {
		pReqId[n] = vReqId[n];
	}
	return count;
}

unsigned int CServer::GetServices(unsigned int *pServiceId, unsigned int count) {
	if (pServiceId == NULL || count == 0)
		return 0;
	CAutoLock sl(m_mutex);
	unsigned int size = (unsigned int) m_vSC.size();
	if (size < count)
		count = size;
	if (count > 0) {
		unsigned int n;
		for (n = 0; n < count; ++n) {
			pServiceId[n] = (m_vSC[n])->GetServiceId();
		}
	}
	return count;
}

void CServer::PutThreadBackIntoPool(CServerThread *pThread) {
	if (pThread == NULL)
		return;
	if (pThread->GetThreadApartment() == MB::taApartment) {
		delete pThread;
		return;
	}
	CAutoLock sl(m_mutex);
	m_vThreadPool.push_back(pThread);
}

void CServer::RemoveThread(CServerThread *pThread) {
	CServerThread *p;
	std::vector<CServerThread*>::iterator it;
	std::vector<CServerThread*>::iterator end = m_vThreadPool.end();
	for (it = m_vThreadPool.begin(); it != end; ++it) {
		p = *it;
		if (p == pThread) {
			m_vThreadPool.erase(it);
			delete p;
			return;
		}
	}
	assert(false);
	delete pThread;
}

CServerThread *CServer::GetOneThread(MB::tagThreadApartment ta) {
	CServerThread *p = NULL;
	if (ta != MB::taApartment) {
		int n, size;
		CAutoLock sl(m_mutex);
		size = (int) m_vThreadPool.size();
		for (n = size - 1; n >= 0; --n) {
			p = m_vThreadPool[n];
			if (p->GetThreadApartment() != ta)
				continue;
			if (!p->IsStarted()) {
				if (p->Start()) {
					m_vThreadPool.erase(m_vThreadPool.begin() + n);
					return p;
				} else {
					assert(false);
				}
			}
			if (!p->IsBusy() && p->IsAliveSafe()) {
				m_vThreadPool.erase(m_vThreadPool.begin() + n);
				return p;
			}
		}
	}

	p = new CServerThread(m_ulMaxThreadIdleTimeBeforeSuicide, ta);
	if (!p->Start()) {
		assert(false);
	}
	return p;
}

void CServer::DestroyThreadPool() {
	bool b;
	size_t n, size = m_vThreadPool.size();
	for (n = 0; n < size; ++n) {
		CServerThread *p = m_vThreadPool[n];
		b = p->Kill();
		assert(b);
		delete p;
	}
	m_vThreadPool.clear();
}

void CServer::Clean() {
	CleanServiceContexts();
	size_t size = m_aSession.size();
	while (size) {
		CServerSession *p = m_aSession[size - 1];
		delete p;
		m_aSession.pop_back();
		size = m_aSession.size();
	}

	size = m_aSessionDead.size();
	while (size) {
		CServerSession *p = m_aSessionDead[size - 1];
		delete p;
		m_aSessionDead.pop_back();
		size = m_aSessionDead.size();
	}
	if (m_pSession) {
		delete m_pSession;
		m_pSession = NULL;
	}
}

void CServer::OnQuit() {
	size_t size = m_aSession.size();
	if (size > 0)
		m_aSession[size - 1]->PostClose();
	if (size == 0)
		m_IoService.stop();
	else
		m_IoService.post(boost::bind(&CServer::OnQuit, this));
}

void CServer::StopSocketProServer() {
	KillMainThread();
}

void CServer::PostQuit() {
	m_IoService.post(boost::bind(&CServer::OnQuit, this));
}

std::string CServer::GetPassword() const {
	return m_strPassword;
}

bool CServer::StartIOPump() {
	CAutoLock sl(m_mutex);
	if (m_pThread != NULL)
		return true;
	CServerSession::m_pPIndex.reset(new MQ_FILE::CPMessage(MB::GetAppName().c_str(), false));
	CServerSession::m_pQLastIndex.reset(new MQ_FILE::CQLastIndex(MB::GetAppName().c_str(), false));
	m_pThread = new boost::thread(boost::bind(&CServer::StartIOPumpInternal, this));
	return true;
}

void CServer::StartIOPumpInternal() {
	m_mutex.lock();
	if (m_pAcceptor == NULL) {
		m_mutex.unlock();
		return;
	}
	m_tStart = boost::posix_time::microsec_clock::local_time();
	m_MainThreadId = boost::this_thread::get_id();
	StartTimer();
	m_mutex.unlock();
	try {
		m_IoService.run(m_ec);
	} catch (boost::system::system_error &err) {
		std::cout << "boost::system_error " << err.what() << std::endl;
	}
	m_mutex.lock();
	delete m_pSession;
	delete m_pAcceptor;
	delete m_pSslContext;
	m_pSslContext = NULL;
	m_pAcceptor = NULL;
	m_pSession = NULL;
	Clean();
	m_mutex.unlock();
}

void CServer::HandleThreadPoolInternal() {
	bool b;
	size_t n, size;
	do {
		b = false;
		size = m_vThreadPool.size();
		for (n = 0; n < size; ++n) {
			CServerThread *p = m_vThreadPool[n];
			if (p->IsBusy())
				continue;
			if (!p->IsAliveSafe()) {
				b = true;
				m_vThreadPool.erase(m_vThreadPool.begin() + n);
				delete p;
				break;
			}
		}
	} while (b);
}

void CServer::HandleSlowSwitchInternal(const boost::posix_time::ptime &t) {
	size_t n, size = m_aSession.size();
	boost::posix_time::ptime tNow = t - boost::posix_time::milliseconds(m_ulMinSwitchTime);
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p->GetLatestTime() < tNow) {
			unsigned int sid = p->GetServiceId();
			switch (sid) {
				case MB::sidStartup:
					p->PostClose(0); //????? set timeout
					break;
				default:
					break;
			}
		}
	}
}

void CServer::HandleServerPingInternal(const boost::posix_time::ptime &t) {
	boost::posix_time::ptime tNow = t - boost::posix_time::milliseconds(m_ulPingInterval);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p->GetLatestTime() < tNow && !p->m_pUThread) {
			unsigned int sid = p->GetServiceId();
			switch (sid) {
				case MB::sidStartup:
					break;
				case MB::sidHTTP:
					p->PostClose(0); //????? set timeout
					break;
				default:
					p->SendReturnData(MB::idPing, NULL, 0);
					break;
			}
		}
	}
}

void CServer::OnTimerSM(const CErrorCode& Error) {
	size_t n;
	CAutoLock sl(m_mutex);
	size_t size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		unsigned int sid = p->GetServiceId();
		switch (sid) {
			case MB::sidStartup:
				break;
			default:
				p->ShrinkMemory();
				break;
		}
	}
	m_TimerSM.expires_at(m_TimerSM.expires_at() + boost::posix_time::milliseconds(m_ulSMInterval));
	m_TimerSM.async_wait(boost::bind(&CServer::OnTimerSM, this, nsPlaceHolders::error));

	UHTTP::CHttpHeaderScopeUQueue::ResetSize();
	MB::CScopeUQueue::ResetSize();
	UHTTP::CHttpDownFileBuffer::ResetSize();
	UHTTP::CHttpSubRequestScopeUQueue::ResetSize();

	//size_t allocatedSize = rapidjson::CrtAllocator::GetAllocatedSize();
	//rapidjson::CrtAllocator::Release();
}

void CServer::OnTimer(const CErrorCode& Error) {
	MB::U_INT64 ms;
	POnIdle pOnIdle;
	boost::posix_time::ptime tNow = boost::posix_time::microsec_clock::local_time();
	{
		CAutoLock sl(m_mutex);
		ms = (tNow - m_tStart).total_milliseconds();
		pOnIdle = m_pOnIdle;

		HandleSlowSwitchInternal(tNow);
		HandleServerPingInternal(tNow);
		HandleThreadPoolInternal();

		m_Timer.expires_at(m_Timer.expires_at() + boost::posix_time::milliseconds(m_ulTimerElapse));
		m_Timer.async_wait(boost::bind(&CServer::OnTimer, this, nsPlaceHolders::error));
	}
	if (pOnIdle) {
		pOnIdle(ms);
	}
}

void CServer::StartTimer() {
	m_Timer.async_wait(boost::bind(&CServer::OnTimer, this, nsPlaceHolders::error));
	m_TimerSM.async_wait(boost::bind(&CServer::OnTimerSM, this, nsPlaceHolders::error));
}

void CServer::OnMessage() {
	size_t size;
	PSession pSession;
	MB::CUThreadMessage message;
	{
		CAutoLock sl(m_mutex);
		size = m_qThreadMessage.size();
		if (size == 0)
			return;
		message = m_qThreadMessage.front();
		m_qThreadMessage.pop();
	}
	while (size) {
		unsigned int nMsgId = message.m_nMsgId;
		*(message.m_pMessageBuffer) >> pSession;
		if (pSession) {
			switch (nMsgId) {
				case WM_SOCKET_SVR_NOTIFY:
				{
					int nEvent, nData;
					*(message.m_pMessageBuffer) >> nEvent >> nData;
					switch (nEvent) {
						case SOCKET_CLOSE_EVENT:
						{
							pSession->m_ec.assign(nData, boost::asio::error::get_system_category());
							pSession->Close();
						}
							break;
						default:
							break;
					}
				}
					break;
				case WM_REQUEST_PROCESSED:
				{
					unsigned int res;
					*(message.m_pMessageBuffer) >> res;
					pSession->OnSlowRequestProcessed(res, message.m_uRequestId);
				}
					break;
				default:
					break;
			}
		} else {
			switch (nMsgId) {
				case WM_WORKER_THREAD_DYING:
				{
					PServerThread pServerThread;
					*(message.m_pMessageBuffer) >> pServerThread;
					CAutoLock sl(m_mutex);
					RemoveThread(pServerThread);
				}
					break;
				default:
					break;
			}
		}

		MB::CScopeUQueue::Unlock(message.m_pMessageBuffer);

		{
			CAutoLock sl(m_mutex);
			size = m_qThreadMessage.size();
			if (size == 0)
				return;
			message = m_qThreadMessage.front();
			m_qThreadMessage.pop();
		}
	}
}

bool CServer::PostSproMessage(CServerSession *pSession, unsigned int nMsgId, int nEvent, int nData) {
	MB::CScopeUQueue su;
	su << nEvent << nData;
	return PostSproMessage(pSession, nMsgId, su->GetBuffer(), su->GetSize());
}

bool CServer::PostSproMessage(CServerSession *pSession, unsigned int nMsgId, const void *pBuffer, unsigned int nSize) {
	MB::CUThreadMessage message;
	message.m_nMsgId = nMsgId;
	message.m_uRequestId = 0;
	message.m_pMessageBuffer = MB::CScopeUQueue::Lock();

	PSession session = pSession;
	*(message.m_pMessageBuffer) << session;

	if (pBuffer && nSize)
		message.m_pMessageBuffer->Push((const unsigned char*) pBuffer, (unsigned int) nSize);
	{
		CAutoLock sl(m_mutex);
		m_qThreadMessage.push(message);
	}

	m_IoService.post(boost::bind(&CServer::OnMessage, this));

	return true;
}

void CServer::PostSproMessage(MB::CUThreadMessage message) {
	{
		CAutoLock sl(m_mutex);
		m_qThreadMessage.push(message);
	}
	m_IoService.post(boost::bind(&CServer::OnMessage, this));
}

bool CServer::IsSsl(boost::asio::ssl::context_base::method &method) {
	switch (m_EncryptionMethod) {
		case MB::TLSv1:
			method = CSslContext::tlsv1_server;
			return true;
			break;
		default:
			break;
	}
	return false;
}

bool CServer::IsTooMany(CServerSession *pSession) {
	CErrorCode ec;
	size_t count = 0;
	size_t n, size = m_aSession.size();
	boost::asio::ip::address myaddr = pSession->GetSocket().remote_endpoint().address();
	for (n = 0; n < size; n++) {
		CServerSession *p = m_aSession[n];
		CSocket &s = p->GetSocket();
		const nsIP::tcp::endpoint &end = s.remote_endpoint(ec);
		if (ec)
			continue;
		boost::asio::ip::address addr = end.address();
		if (addr == myaddr)
			count++;
		if (count > m_ulMaxConnectionsPerClient)
			return true;
	}
	return false;
}

void CServer::SetPrivateKeyFile(const char *keyFile) {
	CAutoLock sl(m_mutex);
	m_strPrivateKeyFile = keyFile;
}

void CServer::SetOnAccept(POnAccept p) {
	CAutoLock sl(m_mutex);
	m_pOnAccept = p;
}

void CServer::SetOnIdle(POnIdle p) {
	CAutoLock sl(m_mutex);
	m_pOnIdle = p;
}

void CServer::SetCertFile(const char *certFile) {
	CAutoLock sl(m_mutex);
	m_strCertFile = certFile;
}

void CServer::SetPKFPassword(const char *pwd) {
	CAutoLock sl(m_mutex);
	if (pwd == NULL)
		m_strPassword.clear();
	else
		m_strPassword = pwd;
	boost::trim(m_strPassword);
}

void CServer::SetEncryptionMethod(MB::tagEncryptionMethod em) {
	CAutoLock sl(m_mutex);
	m_EncryptionMethod = em;
}

MB::tagEncryptionMethod CServer::GetEncryptionMethod() {
	CAutoLock sl(m_mutex);
	return m_EncryptionMethod;
}

void CServer::SetPfxFile(const char *pfxFile) {
	CAutoLock sl(m_mutex);
	m_strPfxFile = pfxFile;
}

void CServer::SetAuthenticationMethod(MB::tagAuthenticationMethod am) {
	CAutoLock sl(m_mutex);
	m_am = am;
}

MB::tagAuthenticationMethod CServer::GetAuthenticationMethod() {
	CAutoLock sl(m_mutex);
	return m_am;
}

void CServer::SetDefaultZip(bool bZip) {
	CAutoLock sl(m_mutex);
	m_bZip = bZip;
}

bool CServer::GetDefaultZip() {
	CAutoLock sl(m_mutex);
	return m_bZip;
}

void CServer::SetMaxConnectionsPerClient(unsigned int ulMaxConnectionsPerClient) {
	CAutoLock sl(m_mutex);
	m_ulMaxConnectionsPerClient = ulMaxConnectionsPerClient;
}

unsigned int CServer::GetMaxConnectionsPerClient() {
	CAutoLock sl(m_mutex);
	return m_ulMaxConnectionsPerClient;
}

void CServer::SetMaxThreadIdleTimeBeforeSuicide(unsigned int ulMaxThreadIdleTimeBeforeSuicide) {
	CAutoLock sl(m_mutex);
	m_ulMaxThreadIdleTimeBeforeSuicide = ulMaxThreadIdleTimeBeforeSuicide;
}

unsigned int CServer::GetMaxThreadIdleTimeBeforeSuicide() {
	CAutoLock sl(m_mutex);
	return m_ulMaxThreadIdleTimeBeforeSuicide;
}

unsigned int CServer::GetCountOfClients() {
	CAutoLock sl(m_mutex);
	return (unsigned int) m_aSession.size();
}

void CServer::SetTimerElapse(unsigned int ulTimerElapse) {
	CAutoLock sl(m_mutex);
	m_ulTimerElapse = ulTimerElapse;
}

unsigned int CServer::GetTimerElapse() {
	CAutoLock sl(m_mutex);
	return m_ulTimerElapse;
}

unsigned int CServer::GetSMInterval() {
	CAutoLock sl(m_mutex);
	return m_ulSMInterval;
}

void CServer::SetSMInterval(unsigned int ulSMInterval) {
	CAutoLock sl(m_mutex);
	m_ulSMInterval = ulSMInterval;
}

void CServer::SetPingInterval(unsigned int pingInterval) {
	CAutoLock sl(m_mutex);
	m_ulPingInterval = pingInterval;
}

unsigned int CServer::GetPingInterval() {
	CAutoLock sl(m_mutex);
	return m_ulPingInterval;
}

void CServer::SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval) {
	CAutoLock sl(m_mutex);
	m_ulRecycleGlobalMemoryInterval = recycleGlobalMemoryInterval;
}

unsigned int CServer::GetRecycleGlobalMemoryInterval() {
	CAutoLock sl(m_mutex);
	return m_ulRecycleGlobalMemoryInterval;
}

MB::U_UINT64 CServer::GetRequestCount() {
	CAutoLock sl(m_mutex);
	return m_nRequestCount;
}

unsigned int CServer::GetSwitchTime() {
	CAutoLock sl(m_mutex);
	return m_ulMinSwitchTime;
}

void CServer::SetSwitchTime(unsigned int switchTime) {
	CAutoLock sl(m_mutex);
	m_ulMinSwitchTime = switchTime;
}

USocket_Server_Handle CServer::GetClient(unsigned int index) {
	CAutoLock sl(m_mutex);
	if (index >= (unsigned int) m_aSession.size())
		return 0;
	return m_aSession[index]->MakeHandler();
}

void CServer::SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p) {
	CAutoLock sl(m_mutex);
	m_pOnSSLHandShakeCompleted = p;
}

void CServer::SetOnClose(POnClose p) {
	CAutoLock sl(m_mutex);
	m_pOnClose = p;
}

void CServer::SetOnIsPermitted(POnIsPermitted p) {
	CAutoLock sl(m_mutex);
	m_pOnIsPermitted = p;
}

void CServer::SetSharedAM(bool b) {
	CAutoLock sl(m_mutex);
	m_bSharedAM = b;
}

bool CServer::GetSharedAM() {
	CAutoLock sl(m_mutex);
	return m_bSharedAM;
}

int CServer::GetErrorCode() {
	CAutoLock sl(m_mutex);
	return m_ec.value();
}

boost::thread::id& CServer::GetMainThreadId() {
	return m_MainThreadId;
}

std::string CServer::GetErrorMessage() {
	CAutoLock sl(m_mutex);
	return m_ec.message();
}

bool CServer::IsRunning() {
	CAutoLock sl(m_mutex);
	return (m_pAcceptor != NULL);
}

bool CServer::IsSSLEnabled() {
	boost::asio::ssl::context_base::method method;
	CAutoLock sl(m_mutex);
	return IsSsl(method);
}

void CServer::AddAChatGroup(unsigned int chatGroupId, const wchar_t *description) {
	if (description == NULL)
		description = L"";
	CAutoLock sl(m_mutex);
	m_mapChatGroup[chatGroupId] = description;
}

void CServer::GetChatGroups(std::vector<unsigned int> &vChatGroup) {
	vChatGroup.clear();
	std::map<unsigned int, std::wstring>::iterator it;
	CAutoLock sl(m_mutex);
	std::map<unsigned int, std::wstring>::iterator end = m_mapChatGroup.end();
	for (it = m_mapChatGroup.begin(); it != end; ++it) {
		vChatGroup.push_back(it->first);
	}
}

unsigned int CServer::GetCountOfChatGroups() {
	CAutoLock sl(m_mutex);
	return (unsigned int) (m_mapChatGroup.size());
}

unsigned int CServer::GetChatGroups(unsigned int *pChatGroupId, unsigned int count) {
	std::map<unsigned int, std::wstring>::iterator it;
	if (count == 0 || pChatGroupId == NULL)
		return 0;
	unsigned int n = 0;
	CAutoLock sl(m_mutex);
	std::map<unsigned int, std::wstring>::iterator end = m_mapChatGroup.end();
	for (it = m_mapChatGroup.begin(); n < count && it != end; ++it) {
		pChatGroupId[n] = it->first;
		++n;
	}
	return n;
}

unsigned int CServer::GetAChatGroupDiscription(unsigned int chatGroupId, wchar_t *description, unsigned int chars) {
	if (description == NULL || chars == 0)
		return 0;
	--chars;
	CAutoLock sl(m_mutex);
	std::map<unsigned int, std::wstring>::iterator it = m_mapChatGroup.find(chatGroupId);
	if (it == m_mapChatGroup.end())
		return 0;
	unsigned int size = (unsigned int) it->second.size();
	if (chars > size)
		chars = size;
	if (chars > 0) {
		::memcpy(description, it->second.c_str(), sizeof (wchar_t) * chars);
		description[chars] = 0;
	}
	return chars;
}

void CServer::RemoveChatGroup(unsigned int chatGroupId) {
	CAutoLock sl(m_mutex);
	std::map<unsigned int, std::wstring>::iterator it = m_mapChatGroup.find(chatGroupId);
	if (it != m_mapChatGroup.end())
		m_mapChatGroup.erase(it);
}

bool CServer::Enter(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount) {
	unsigned short portSender;
	std::vector<unsigned int> vChatGroup;
	wchar_t strUserID[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	unsigned int ServiceId = pSession->ServiceId;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	Connection::CConnectionContext::Enter(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, pChatGroupId, nCount);
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession)
			continue;
		if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
			continue;
		}
		p->m_mutex.lock();
		Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
		p->m_mutex.unlock();
		unsigned int count = (unsigned int) vChatGroup.size();
		if (count > 0) {
			if (p->GetServiceId() == MB::sidHTTP) {
				CAutoLock al(p->m_mutex);
				if (p->m_pHttpContext) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor) {
						pWebResponseProcessor->Enter(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, p->m_qWrite);
						p->Write(NULL, 0);
					}
				}
			} else
				p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idEnter, (const unsigned char*) vChatGroup.data(), count * sizeof (unsigned int));
		}
	}
	return true;
}

void CServer::Exit(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount) {
	unsigned short portSender;
	std::vector<unsigned int> vChatGroup;
	wchar_t strUserID[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	unsigned int ServiceId = pSession->ServiceId;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	Connection::CConnectionContext::Exit(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, pChatGroupId, nCount);
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession)
			continue;
		if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
			continue;
		}
		p->m_mutex.lock();
		Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
		p->m_mutex.unlock();
		unsigned int count = (unsigned int) vChatGroup.size();
		if (count > 0) {
			if (p->GetServiceId() == MB::sidHTTP) {
				CAutoLock al(p->m_mutex);
				if (p->m_pHttpContext) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor) {
						pWebResponseProcessor->Exit(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, p->m_qWrite);
						p->Write(NULL, 0);
					}
				}
			} else
				p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idExit, (const unsigned char*) vChatGroup.data(), count * sizeof (unsigned int));
		}
	}
}

bool CServer::Speak(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount, const MB::UVariant &vtMsg) {
	unsigned short portSender;
	MB::CScopeUQueue sb;
	std::vector<unsigned int> vChatGroup(pChatGroupId, pChatGroupId + nCount);
	wchar_t strUserID[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	unsigned int ServiceId = pSession->ServiceId;
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	Connection::CConnectionContext::Speak(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMsg, pChatGroupId, nCount);
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession)
			continue;
		if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
			continue;
		}
		p->m_mutex.lock();
		Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
		p->m_mutex.unlock();
		unsigned int count = (unsigned int) vChatGroup.size();
		if (count > 0) {
			if (p->GetServiceId() == MB::sidHTTP) {
				CAutoLock al(p->m_mutex);
				if (p->m_pHttpContext) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor) {
						pWebResponseProcessor->Speak(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMsg, vChatGroup.data(), count, p->m_qWrite);
						p->Write(NULL, 0);
					}
				}
			} else {
				sb << vtMsg;
				sb << count;
				sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
				p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idSpeak, sb->GetBuffer(), sb->GetSize());
				sb->SetSize(0);
			}
		}
	}
	return true;
}

bool CServer::SpeakEx(CServerSession *pSession, const unsigned char *message, unsigned int len, const unsigned int *pChatGroupId, unsigned int nCount) {
	unsigned short portSender;
	MB::CScopeUQueue sb;
	std::vector<unsigned int> vChatGroup(pChatGroupId, pChatGroupId + nCount);
	wchar_t strUserID[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	unsigned int ServiceId = pSession->ServiceId;
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession || p->GetServiceId() == MB::sidHTTP)
			continue;
		p->m_mutex.lock();
		Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
		p->m_mutex.unlock();
		unsigned int count = (unsigned int) vChatGroup.size();
		if (count > 0) {
			sb << count;
			sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
			sb->Push(message, len);
			p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idSpeakEx, sb->GetBuffer(), sb->GetSize());
			sb->SetSize(0);
		}
	}
	return true;
}

bool CServer::SendUserMessage(CServerSession *pSession, const wchar_t *userId, const MB::UVariant &vtMessage) {
	unsigned short portSender;
	wchar_t strUserID[MAX_USERID_CHARS + 1], strMe[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	unsigned int ServiceId = pSession->ServiceId;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	MB::CScopeUQueue us;
	us << vtMessage;
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	Connection::CConnectionContext::SendUserMessage(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMessage, userId, pSession->m_ccb.ChatGroups.data(), (unsigned int) pSession->m_ccb.ChatGroups.size());
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession)
			continue;
		if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
			continue;
		}
		p->GetUserId(strMe, sizeof (strMe));
#ifdef WIN32_64
		if (_wcsicmp(strMe, userId) == 0)
#else
		if (wcscasecmp(strMe, userId) == 0)
#endif
		{
			if (p->GetServiceId() == MB::sidHTTP) {
				CAutoLock al(p->m_mutex);
				if (p->m_pHttpContext) {
					UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
					if (pWebResponseProcessor) {
						pWebResponseProcessor->SendUserMessage(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMessage, pSession->m_ccb.ChatGroups.data(), (unsigned int) pSession->m_ccb.ChatGroups.size(), p->m_qWrite);
						p->Write(NULL, 0);
					}
				}
			} else
				p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idSendUserMessage, us->GetBuffer(), us->GetSize());
		}
	}
	return true;
}

bool CServer::SendUserMessage(CServerSession *pSession, const wchar_t *userId, const unsigned char *message, unsigned int len) {
	unsigned short portSender;
	wchar_t strUserID[MAX_USERID_CHARS + 1], strMe[MAX_USERID_CHARS + 1];
	std::string ipAddrSender;
	unsigned int ServiceId = pSession->ServiceId;
	assert(boost::this_thread::get_id() == GetMainThreadId());
	pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
	pSession->GetPeerIpAddr(ipAddrSender, &portSender);
	CAutoLock sl(m_mutex);
	size_t n, size = m_aSession.size();
	for (n = 0; n < size; ++n) {
		CServerSession *p = m_aSession[n];
		if (p == pSession || p->GetServiceId() == MB::sidHTTP)
			continue;
		p->GetUserId(strMe, sizeof (strMe));
#ifdef WIN32_64
		if (_wcsicmp(strMe, userId) == 0)
#else
		if (wcscasecmp(strMe, userId) == 0)
#endif
		{
			p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, MB::idSendUserMessageEx, message, len);
		}
	}
	return true;
}

void CServer::OnAccepted(CServerSession* pSession, const CErrorCode& Error) {
	m_mutex.lock();
	++m_nConnIndex;
	if (m_nConnIndex > MAX_SESSION_INDEX)
		m_nConnIndex = 1;
	if (!Error) {
		if (IsTooMany(pSession)) {
			pSession->m_ec.assign(boost::asio::error::connection_refused, boost::asio::error::get_system_category());
			pSession->m_ulIndex = 0;
			pSession->Close();
		} else {
			m_ec.clear();
			pSession->m_ulIndex = m_nConnIndex;
			pSession->Start();
			m_aSession.push_back(pSession);
		}

		if (m_pOnAccept != NULL) {
			boost::uint64_t handler = pSession->MakeHandler();
			m_mutex.unlock();
			try {
				m_pOnAccept(handler, m_ec.value());
			} catch (...) {
			}
			m_mutex.lock();
		}

		size_t size = m_aSessionDead.size();
		if (size > 0) {
			m_pSession = m_aSessionDead[--size];
			m_aSessionDead.pop_back();
		} else {
			m_pSession = new CServerSession();
		}
		m_pAcceptor->async_accept(m_pSession->GetSocket().lowest_layer(), boost::bind(&CServer::OnAccepted, this, m_pSession, nsPlaceHolders::error));

	} else if (pSession) {
		m_ec = Error;
		pSession->m_ulIndex = 0;
		if (m_pOnAccept != NULL) {
			boost::uint64_t handler = pSession->MakeHandler();
			m_mutex.unlock();
			try {
				m_pOnAccept(handler, m_ec.value());
			} catch (...) {
			}
			m_mutex.lock();
		}
	}
	m_mutex.unlock();
}

unsigned int CServer::StartQueue(const char *qFileName) {
	if (!qFileName)
		return INVALID_QUEUE_HANDLE;
	std::string fn = qFileName;
	boost::trim(fn);
	if (fn.size() == 0)
		return INVALID_QUEUE_HANDLE;
#ifdef WIN32_64
	std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
#endif
	CMapQueue::iterator it;
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return INVALID_QUEUE_HANDLE;
	CMapQueue::iterator end = m_mapQueue.end();
	for (it = m_mapQueue.begin(); it != end; ++it) {
		if (fn == it->second->GetRawName())
			return it->first;
	}

	unsigned int key = (unsigned int) m_mapQueue.size() + 1;
	while (m_mapQueue.find(key) != end) {
		++key;
	}

	boost::shared_ptr<MQ_FILE::CMqFile > q;
	if (m_strPassword.size()) {
		MB::CScopeUQueue suId;
		MB::CScopeUQueue suPwd;
		std::string machineId = MB::GetSysId();
		std::string id = MB::GetAppName();
#ifdef WIN32_64
		std::transform(id.begin(), id.end(), id.begin(), ::tolower);
#endif
		MB::Utilities::ToWide(id.c_str(), *suId);
		suId->SetNull();
		MB::Utilities::ToWide(m_strPassword.c_str(), *suPwd);
		suPwd->SetNull();
		q = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFileEx(fn.c_str(), m_nPort,
				(const wchar_t*)suId->GetBuffer(),
				(const wchar_t*)suPwd->GetBuffer(), machineId.c_str()));

	} else {
		q = boost::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFile(fn.c_str(), m_nPort, false));
	}

	if (!q->IsAvailable())
		return INVALID_QUEUE_HANDLE;
	else
		m_mapQueue[key] = q;

	CServerSession::m_pPIndex->SetPM(q.get());


	return key;
}

const char* CServer::GetQueueFileName(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return NULL;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it == m_mapQueue.end())
		return NULL;
	return it->second->GetMQFileNmae();
}

unsigned int CServer::GetMessagesInDequeuing(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return INVALID_MESSAGE_COUNT;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it == m_mapQueue.end())
		return INVALID_MESSAGE_COUNT;
	return it->second->GetMessagesInDequeuing();
}

bool CServer::Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size) {
	if (reqId < MB::idReservedTwo)
		return false;
	MB::CScopeUQueue sb;
	MB::CStreamHeader ReqInfo;
	if (!buffer)
		size = 0;
	ReqInfo.Size = size;
	ReqInfo.RequestId = reqId;
	sb << ReqInfo;
	if (size)
		sb->Push(buffer, size);
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return false;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it == m_mapQueue.end())
		return false;

	boost::shared_ptr<MQ_FILE::CMqFile> &queue = it->second;

	if (!queue->IsAvailable())
		return false;

	bool ok = (queue->Enqueue(*sb) > 0);

	//notify clients that a message is available now
	MB::U_UINT64 msgCount = queue->GetMessageCount();
	unsigned int pending = queue->GetMessagesInDequeuing();
	if (msgCount == (pending + 1)) {
		CMapQNotified::iterator mi = m_mapQNotified.find(qHandle);
		if (mi != m_mapQNotified.end()) {
			MB::CStreamHeader sh;
			sh.RequestId = MB::idMessageQueued;
			std::vector<CSNotified> &v = mi->second;
			std::vector<CSNotified>::iterator s, end = v.end();
			for (s = v.begin(); s != end; ++s) {
				if (s->second) {
					unsigned int index;
					USocket_Server_Handle h = s->first;
					CServerSession *pSession = GetSvrSession(h, index);
					pSession->m_mutex.lock();
					pSession->Write((const unsigned char*) &sh, sizeof (sh));
					pSession->m_mutex.unlock();
				}
			}
		}
	}
	return ok;
}

MB::U_UINT64 CServer::GetMessageCount(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return INVALID_MESSAGE_COUNT;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it == m_mapQueue.end())
		return INVALID_MESSAGE_COUNT;
	return it->second->GetMessageCount();
}

MB::U_UINT64 CServer::GetQueueSize(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return INVALID_QUEUE_SIZE;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it != m_mapQueue.end())
		return it->second->GetMQSize();
	return INVALID_QUEUE_SIZE;
}

void CServer::StopQueue(boost::shared_ptr<MQ_FILE::CMqFile> qFile, bool permanent) {
	bool bQueue = (qFile && qFile->IsAvailable());
	if (permanent && bQueue) {
		CServerSession::m_pPIndex->Remove(qFile->GetMQFileNmae());
	}
	else if (bQueue) {
		unsigned int n;
		unsigned int pendings = qFile->GetMessagesInDequeuing();
		const MQ_FILE::QAttr *qa = qFile->GetMessageAttributesInDequeuing();
		for (n=0; n<pendings; ++n) {
			CServerSession::m_pPIndex->Add(qFile->GetMQFileNmae(), qa[n]);
		}
	}
	if (CServerSession::m_pPIndex->IsDirty())
		CServerSession::m_pPIndex->Save();
}

bool CServer::StopQueueByHandle(unsigned int qHandle, bool permanent) {
	int res;
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return false;
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it != m_mapQueue.end()) {
		StopQueue(it->second, permanent);
		std::string fileName = it->second->GetMQFileNmae();
		m_mapQueue.erase(it);
		if (permanent)
			res = ::remove(fileName.c_str());
		return true;
	}
	return false;
}

unsigned int CServer::Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable) {
	unsigned int index;
	MB::U_UINT64 mqIndex;
	MB::U_UINT64 pos;
	unsigned int count = 0;
	MB::CScopeUQueue su;
	MB::CScopeUQueue temp;
	MB::CUQueue &qTemp = *temp;
	MB::CUQueue &buffer = *su;
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	if (it == m_mapQueue.end())
		return INVALID_QUEUE_HANDLE;
	boost::shared_ptr<MQ_FILE::CMqFile> queue = it->second;
	if (!queue->IsAvailable())
		return INVALID_QUEUE_HANDLE;
	CServerSession *pSession = GetSvrSession(h, index);
	if (index == 0 || index != pSession->GetConnIndex())
		return SOCKET_NOT_FOUND;

	{
		MB::CScopeUQueue su;
		MB::CStreamHeader sh;
		su << queue->GetMQFileNmae();
		sh.RequestId = MB::idStartQueue;
		sh.Size = su->GetSize();
		su->Insert((const unsigned char*)&sh, sizeof(sh));
		pSession->Write(su->GetBuffer(), su->GetSize());
	}

	//set queue notification
	CMapQNotified::iterator mi = m_mapQNotified.find(qHandle);
	if (mi == m_mapQNotified.end()) {
		CSNotified dn(h, beNotifiedWhenAvailable);
		std::vector<CSNotified> v;
		v.push_back(dn);
		m_mapQNotified[qHandle] = v;
	} else {
		bool found = false;
		std::vector<CSNotified> &v = mi->second;
		std::vector<CSNotified>::iterator jend = v.end();
		for (auto i = v.begin(); i != jend; ++i) {
			if (i->first == h) {
				i->second = beNotifiedWhenAvailable;
				found = true;
				break;
			}
		}
		if (!found) {
			CSNotified dn(h, beNotifiedWhenAvailable);
			v.push_back(dn);
		}
	}

	while (count < messageCount) {
		pos = queue->Dequeue(buffer, mqIndex);
		if (pos == INVALID_QUEUE_INDEX)
			break;
		MB::CStreamHeader *sh = (MB::CStreamHeader*)buffer.GetBuffer();
		sh->SetQueued(true);
		qTemp << qHandle << pos << mqIndex;
		sh->Size += qTemp.GetSize();
		buffer.Insert(qTemp.GetBuffer(), qTemp.GetSize(), sizeof (MB::CStreamHeader));

		//may remove tail because of decryption
		buffer.SetSize(sh->Size + sizeof (MB::CStreamHeader));

		CServerSession::CQueueMap::iterator it = pSession->m_mapDequeue.find(qHandle);
		std::pair<MB::U_UINT64, MB::U_UINT64> p;
		p.first = pos;
		p.second = mqIndex;
		pSession->m_mutex.lock();
		if (it == pSession->m_mapDequeue.end()) {
			CQueueProperty v;
			v.push_back(p);
			pSession->m_mapDequeue[qHandle] = v;
		} else {
			it->second.push_back(p);
		}

		pSession->Write(buffer.GetBuffer(), buffer.GetSize());
		pSession->m_mutex.unlock();
		qTemp.SetSize(0);
		++count;
	}
	return count;
}

void CServer::ConfirmQueue(unsigned int qHandle, MB::U_UINT64 mqPos, MB::U_UINT64 qIndex, bool successful) {
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	boost::shared_ptr<MQ_FILE::CMqFile> &queue = it->second;
	if (it == m_mapQueue.end() || !queue->IsAvailable())
		return;
	if (successful)
		queue->ConfirmDequeue(mqPos, qIndex, false);
	else {
		MQ_FILE::QAttr qa(mqPos, qIndex);
		CServerSession::m_pPIndex->Add(queue->GetMQFileNmae(), qa);
		if(CServerSession::m_pPIndex->IsDirty())
			CServerSession::m_pPIndex->Save();
	}
}

bool CServer::IsQueueStarted(const char *qName) {
	std::string fn = qName;
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it, end = m_mapQueue.end();
	for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
		if (boost::iequals(fn, it->second->GetRawName()))
#else
		if (fn == it->second->GetRawName())
#endif
		{
			return it->second->IsAvailable();
		}
	}
	return false;
}

bool CServer::IsQueueStarted(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	return (it != m_mapQueue.end() && it->second && it->second->IsAvailable());
}

bool CServer::IsQueueSecured(const char *qName) {
	std::string fn = qName;
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it, end = m_mapQueue.end();
	for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
		if (boost::iequals(fn, it->second->GetRawName()))
#else
		if (fn == it->second->GetRawName())
#endif
		{
			return it->second->IsSecure();
		}
	}
	return false;
}

bool CServer::IsQueueSecured(unsigned int qHandle) {
	CAutoLock sl(m_mutex);
	CMapQueue::iterator it = m_mapQueue.find(qHandle);
	return (it != m_mapQueue.end() && it->second && it->second->IsSecure());
}

bool CServer::StopQueueByName(const char *qName, bool permanent) {
	int res;
	std::string fn = qName;
	unsigned int handle = StartQueue(qName);
	CAutoLock sl(m_mutex);
	if (!m_nPort)
		return false;
	CMapQueue::iterator it, end = m_mapQueue.end();
	for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
		if (boost::iequals(fn, it->second->GetRawName()))
#else
		if (fn == it->second->GetRawName())
#endif
		{
			StopQueue(it->second, permanent);
			std::string fileName = it->second->GetMQFileNmae();
			m_mapQueue.erase(it);
			if (permanent)
				res = ::remove(fileName.c_str());
			return true;
		}
	}
	return false;
}

