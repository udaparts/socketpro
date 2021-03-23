
#include "stdafx.h"
#include "server.h"
#include <algorithm>
#include "../core_shared/pinc/regkeys.h"
#include <boost/algorithm/string.hpp>

extern std::vector<PThreadEvent> g_vThreadEvent;

#ifdef WINCE

#elif defined(WIN32_64)
#include "../core_shared/pinc/WinCrashHandler.h"
#else

#endif

#ifndef USecretKey
#define USecretKey {'1', '2', '3', '4', '5', '6', '7', '8', 0}
#endif

bool g_bRegistered = false;

CServerSession *GetSvrSession(USocket_Server_Handle h, unsigned int &index);

std::string CServer::m_WorkingPath("");

std::vector<CServerSession*>::iterator CServer::Seek(CServerSession *pSession) {
    std::vector<CServerSession*>::iterator it;
    std::vector<CServerSession*>::iterator end = m_aSession.end();
    if (pSession == nullptr)
        return end;
    for (it = m_aSession.begin(); it != end; ++it) {
        if (*it == pSession)
            break;
    }
    return it;
}

void CServer::Recycle(CServerSession *pSession) {
    if (pSession) {
        if (pSession->m_pRoutingServiceContext != nullptr) {
            CRAutoLock al(pSession->m_mutex, pSession->m_bChatting);
            pSession->m_pRoutingServiceContext->RemoveRoutee(pSession);
            pSession->m_pServiceContext->NotifyRouteeChanged((unsigned int) (pSession->m_pRoutingServiceContext->GetRouteeSize()));
        }

        USocket_Server_Handle h = pSession->MakeHandlerInternal();
        pSession->Initialize();
        CRAutoLock al(pSession->m_mutex, pSession->m_bChatting);
        {
            //remove queue notification
            CAutoLock sl(m_mQ);
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
        }
        {
            CAutoLock sl(m_mutex);
            std::vector<CServerSession*>::iterator it = Seek(pSession);
            if (it != m_aSession.end()) {
                m_aSession.erase(it);
                m_aSessionDead.push_back(pSession);
            }
        }
    }
}

CServerRegistration CServer::m_reg;

CServer::CServer(int nParam)
: m_pSession(nullptr),
m_pAcceptor(nullptr),
m_bZip(false),
m_nRequestCount(0),
m_nRequestCountLast(0),
m_cvc(nullptr),
m_bStopped(true),
m_ulTimerElapse(DefaultTimerElapse),
m_EncryptionMethod(SPA::tagEncryptionMethod::NoEncryption),
m_ulMinSwitchTime(DefaultMinSwitchTime),
m_ulRecycleGlobalMemoryInterval(DefaultRecycleGlobalMemoryInterval),
m_ulPingInterval(DefaultPingInterval),
m_ulSMInterval(DefaultSMInterval),
m_ulMaxConnectionsPerClient(DefaultMaxConnectionsPerClient),
m_ulMaxThreadIdleTimeBeforeSuicide(DefaultMaxThreadIdleTimeBeforeSuicide),
m_am(SPA::ServerSide::tagAuthenticationMethod::amOwn),
m_bSharedAM(false),
m_pOnAccept(nullptr),
m_nConnIndex(0),
m_Timer(m_IoService, boost::posix_time::milliseconds(m_ulTimerElapse)),
m_TimerSM(m_IoService, boost::posix_time::milliseconds(m_ulSMInterval)),
m_pOnIdle(nullptr),
m_pOnSSLHandShakeCompleted(nullptr),
m_pOnIsPermitted(nullptr),
m_pOnClose(nullptr),
m_nPort(0),
m_nParam(nParam),
m_clientCertAuth(false),
m_cst(SPA::cstUnknown) {
    ::memset(&m_ServerInfo, 0, sizeof (m_ServerInfo));
    m_ServerInfo.MajorVersion = 2;
    ::memset(&m_hCreds, 0, sizeof (m_hCreds));
}

void CServer::KillMainThread() {
    if (m_vThread.size()) {
        std::shared_ptr<boost::thread> one = m_vThread.front();
        m_IoService.stop();
        while (!m_IoService.stopped()) {
            sleep(boost::posix_time::milliseconds(1));
        }
        one->timed_join(boost::posix_time::milliseconds(500));
    }
    m_vThread.clear();
}

bool CServer::StartServerInternal(unsigned int port, unsigned int uiMaxBacklog, bool v6) {
    CAutoLock al(m_mutex);
    std::string path = CServer::m_WorkingPath + SPA::GetAppName();
    m_nPort = port;
    bool bSuc = false;
    do {
        if (m_pAcceptor)
            return false;
        if (IsSsl()) {
            SPA::CScopeUQueue suPwd, suFile;
            std::string pwd = GetPassword();
            SPA::Utilities::ToWide(pwd.c_str(), pwd.size(), *suPwd);
            SPA::Utilities::ToWide(m_strCertFile.c_str(), m_strCertFile.size(), *suFile);
            bSuc = OpenPfx((const wchar_t *) suFile->GetBuffer(), (const wchar_t *) suPwd->GetBuffer());
            suPwd->CleanTrack();
            if (!bSuc)
                return false;
        }
        m_pAcceptor = new CAcceptor(m_IoService);
        boost::asio::ip::tcp::endpoint endpoint(v6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), (unsigned short) port);
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
        CServerSession::m_pQLastIndex.reset(new MQ_FILE::CQLastIndex(path.c_str(), false));
        m_pSession = new CServerSession();
        m_pSession->SetContext();
        m_pAcceptor->async_accept(m_pSession->GetSocket().lowest_layer(), boost::bind(&CServer::OnAccepted, this, m_pSession, nsPlaceHolders::error));
        bSuc = true;
    } while (false);

    if (!bSuc) {
        delete m_pSession;
        delete m_pAcceptor;
        DeleteSslContex();
        m_pAcceptor = nullptr;
        m_pSession = nullptr;
        //m_mutex.unlock();
        //PostQuitPump();
        //m_mutex.lock();
    }
    return bSuc;
}

void CServer::DeleteSslContex() {
    FreeCredHandle();
    m_pSelfCert.reset();
}

CServer::~CServer() {
    KillMainThread();
    {
        CAutoLock sl(m_mutex);
        delete m_pAcceptor;
        DeleteSslContex();
        delete m_pSession;
        Clean();
    }
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
        if ((*it)->GetSvsID() == nServiceId)
            return it;
    }
    return end;
}

bool CServer::SetRouting(unsigned int serviceId0, SPA::ServerSide::tagRoutingAlgorithm ra0, unsigned int serviceId1, SPA::ServerSide::tagRoutingAlgorithm ra1) {
    if (serviceId0 <= (unsigned int) SPA::tagServiceID::sidHTTP)
        return false;
    if (serviceId1 <= (unsigned int) SPA::tagServiceID::sidHTTP)
        return false;
    if (serviceId0 == serviceId1)
        return false;
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator end = m_vSC.end();
    std::vector<CServiceContext*>::iterator it0 = SeekSC(serviceId0);
    std::vector<CServiceContext*>::iterator it1 = SeekSC(serviceId1);
    if (it0 == end && it1 == end)
        return false; //not found
    else if (it0 == end) {
        unsigned int peerServiceId = (*it1)->GetRoutingSvsId();
        (*it1)->SetRoutingSvsId(0);
        it0 = SeekSC(peerServiceId);
        if (it0 != end)
            (*it0)->SetRoutingSvsId(0);
        return true;
    } else if (it1 == end) {
        unsigned int peerServiceId = (*it0)->GetRoutingSvsId();
        (*it0)->SetRoutingSvsId(0);
        it1 = SeekSC(peerServiceId);
        if (it1 != end)
            (*it1)->SetRoutingSvsId(0);
        return true;
    }
    (*it0)->SetRoutingSvsId((*it1)->GetSvsID());
    (*it0)->SetRoutingAlgorithm(ra0);
    (*it1)->SetRoutingSvsId((*it0)->GetSvsID());
    (*it1)->SetRoutingAlgorithm(ra1);
    return true;
}

unsigned int CServer::CheckRouting(unsigned int serviceId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(serviceId);
    if (it == m_vSC.end())
        return 0;
    return (*it)->GetRoutingSvsId();
}

CServiceContext* CServer::SeekServiceContext(unsigned int nServiceId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it != m_vSC.end())
        return *it;
    return nullptr;
}

void CServer::RemoveASvsContext(unsigned int nServiceId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it != m_vSC.end()) {
        delete(*it);
        m_vSC.erase(it);

    }
}

bool CServer::AddSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
    if (sReqId <= (unsigned short) SPA::tagChatRequestID::idSendUserMessageEx)
        return false;
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it != m_vSC.end())
        (*it)->AddSlowRequest(sReqId);
    return true;
}

bool CServer::AddAlphaRequest(unsigned int serviceId, unsigned short reqId) {
    if (reqId <= (unsigned short) SPA::tagBaseRequestID::idStopQueue)
        return false;
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(serviceId);
    if (it == m_vSC.end())
        return false;
    CServiceContext *sc = *it;
    if (sc->GetRoutingSvsId() == 0)
        return false;
    sc->AddAlpha(reqId);
    return true;
}

unsigned int CServer::GetAlphaRequestIds(unsigned int serviceId, unsigned short *reqIds, unsigned int count) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(serviceId);
    if (it == m_vSC.end())
        return 0;
    CServiceContext *sc = *it;
    if (sc->GetRoutingSvsId() == 0)
        return 0;
    return sc->GetAllAlpaRequestIds(reqIds, count);
}

HINSTANCE CServer::AddADll(const char *libFile, int nParam) {
    bool ok;
    PlugImpl pi;
#ifdef WIN32_64
    HINSTANCE h = ::LoadLibraryA(libFile);
#else
    HINSTANCE h = ::dlopen(libFile, RTLD_LAZY);
#endif
    if (!h)
        return h;
    pi.GetAServiceID = (PGetAServiceID)::GetProcAddress(h, "GetAServiceID");
    pi.GetNumOfServices = (PGetNumOfServices)::GetProcAddress(h, "GetNumOfServices");
    pi.GetNumOfSlowRequests = (PGetNumOfSlowRequests)::GetProcAddress(h, "GetNumOfSlowRequests");
    pi.GetOneSlowRequestID = (PGetOneSlowRequestID)::GetProcAddress(h, "GetOneSlowRequestID");
    pi.GetOneSvsContext = (PGetOneSvsContext)::GetProcAddress(h, "GetOneSvsContext");
    pi.InitServerLibrary = (PInitServerLibrary)::GetProcAddress(h, "InitServerLibrary");
    pi.UninitServerLibrary = (PUninitServerLibrary)::GetProcAddress(h, "UninitServerLibrary");

    if (!pi.IsOk() || !pi.InitServerLibrary(nParam)) {
        ::FreeLibrary(h);
        return nullptr;
    }
    bool suc = false;
    bool chatting = false;
    CAutoLock sl(m_mutex);
    unsigned short n, count = pi.GetNumOfServices();
    for (n = 0; n < count; ++n) {
        unsigned int svsId = pi.GetAServiceID(n);
        CSvsContext sc = pi.GetOneSvsContext(svsId);
        {
            CRAutoLock rsl(m_mutex, chatting);
            if (!AddSvsContext(svsId, sc))
                continue;
            suc = true;
            unsigned short s, slows = pi.GetNumOfSlowRequests(svsId);
            for (s = 0; s < slows; ++s) {
                unsigned short reqId = pi.GetOneSlowRequestID(svsId, s);
                ok = AddSlowRequest(svsId, reqId);
            }
        }
    }
    if (suc)
        m_mapLib[h] = pi;
    else {
        CRAutoLock sl(m_mutex, chatting);
        if (pi.UninitServerLibrary)
            pi.UninitServerLibrary();
        ::FreeLibrary(h);
        h = nullptr;
    }
    return h;
}

bool CServer::RemoveALibrary(HINSTANCE hLib) {
    bool chatting = false;
    CAutoLock sl(m_mutex);
    std::map<HINSTANCE, PlugImpl>::iterator it = m_mapLib.find(hLib);
    if (it == m_mapLib.end())
        return false;
    PlugImpl &pi = it->second;
    {
        CRAutoLock rsl(m_mutex, chatting);
        unsigned short n, count = pi.GetNumOfServices();
        for (n = 0; n < count; ++n) {
            unsigned int svsId = pi.GetAServiceID(n);
            RemoveASvsContext(svsId);
        }
    }
    {
        CRAutoLock rsl(m_mutex, chatting);
        pi.UninitServerLibrary();
        ::FreeLibrary(hLib);
    }
    m_mapLib.erase(it);
    return true;
}

bool CServer::AddSvsContext(unsigned int nServiceId, CSvsContext SvsContext) {
    if (nServiceId != (unsigned int) SPA::tagServiceID::sidChat &&
            nServiceId != (unsigned int) SPA::tagServiceID::sidHTTP &&
            nServiceId != (unsigned int) SPA::tagServiceID::sidFile &&
            nServiceId != (unsigned int) SPA::tagServiceID::sidODBC &&
            nServiceId < (unsigned int) SPA::tagServiceID::sidReserved)
        return false;
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it != m_vSC.end())
        return false;
    CServiceContext *p = new CServiceContext(nServiceId, SvsContext);
    m_vSC.push_back(p);
    return true;
}

void CServer::RemoveSlowRequest(unsigned int nServiceId, unsigned short sReqId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it != m_vSC.end())
        (*it)->RemoveSlowRequest(sReqId);
}

void CServer::RemoveAllSlowRequests(unsigned int serviceId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(serviceId);
    if (it != m_vSC.end())
        (*it)->RemoveAllSlowRequests();
}

size_t CServer::GetCountOfServices() {
    CAutoLock sl(m_mutexSC);
    return m_vSC.size();
}

unsigned int CServer::GetCountOfSlowRequests(unsigned int nServiceId) {
    CAutoLock sl(m_mutexSC);
    std::vector<CServiceContext*>::iterator it = SeekSC(nServiceId);
    if (it == m_vSC.end())
        return 0;
    return (unsigned int) ((*it)->GetCountOfSlowRequests());
}

unsigned int CServer::GetAllSlowRequestIds(unsigned int nServiceId, unsigned short *pReqId, unsigned int count) {
    unsigned int n;
    if (pReqId == nullptr || count == 0)
        return 0;
    std::vector<unsigned short> vReqId;
    {
        CAutoLock sl(m_mutexSC);
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
    if (pServiceId == nullptr || count == 0)
        return 0;
    CAutoLock sl(m_mutexSC);
    unsigned int size = (unsigned int) m_vSC.size();
    if (size < count)
        count = size;
    if (count > 0) {
        unsigned int n;
        for (n = 0; n < count; ++n) {
            pServiceId[n] = (m_vSC[n])->GetSvsID();
        }
    }
    return count;
}

void CServer::PutThreadBackIntoPool(CServerThread *pThread) {
    if (pThread == nullptr)
        return;
    if (pThread->GetThreadApartment() == SPA::tagThreadApartment::taApartment) {
        delete pThread;
        return;
    }
    SPA::CSpinAutoLock sl(m_mTP);
    m_vThreadPool.push_back(pThread);
}

void CServer::RemoveThread(CServerThread *pThread) {
    CServerThread *p = nullptr;
    std::vector<CServerThread*>::iterator it;
    SPA::CSpinAutoLock sl(m_mTP);
    std::vector<CServerThread*>::iterator end = m_vThreadPool.end();
    for (it = m_vThreadPool.begin(); it != end; ++it) {
        p = *it;
        if (p == pThread) {
            m_vThreadPool.erase(it);
            bool ok = p->Kill();
            delete p;
            return;
        }
    }
    assert(false);
    delete pThread;
}

CServerThread *CServer::GetOneThread(SPA::tagThreadApartment ta) {
    CServerThread *p = nullptr;
    if (ta != SPA::tagThreadApartment::taApartment) {
        int n, size;
        SPA::CSpinAutoLock sl(m_mTP);
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
    SPA::CSpinAutoLock sl(m_mTP);
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
    size_t size = m_aSession.size();
    while (size) {
        CServerSession *p = m_aSession.back();
        m_aSession.pop_back();
        delete p;
        size = m_aSession.size();
    }

    size = m_aSessionDead.size();
    while (size) {
        CServerSession *p = m_aSessionDead.back();
        m_aSessionDead.pop_back();
        delete p;
        size = m_aSessionDead.size();
    }
    if (m_pSession) {
        delete m_pSession;
        m_pSession = nullptr;
    }
}

bool CServer::HasUserId(const wchar_t *userId) {
    if (!userId)
        return false;
    size_t length = ::wcslen(userId);
    CAutoLock sl(m_mutex);
    std::vector<CServerSession*>::iterator it, end = m_aSession.end();
    for (it = m_aSession.begin(); it != end; ++it) {
        std::wstring &myUserId = (*it)->m_ccb.UserId;
        size_t len = myUserId.size();
        if (len != length)
            continue;
        if (UHTTP::iequals(myUserId.c_str(), userId))
            return true;
    }
    return false;
}

void CServer::OnQuit() {
    size_t size;
    {
        CAutoLock sl(m_mutex);
        size = m_aSession.size();
        if (size > 0)
            m_aSession.back()->PostClose();
    }
    if (size == 0)
        m_IoService.stop();
    else
        m_IoService.post(boost::bind(&CServer::OnQuit, this));
}

void CServer::StopSocketProServer() {
    try{
        KillMainThread();
        {
            CAutoLock al(m_mutexSC);
            CleanServiceContexts();
        }
        CAutoLock sl(m_mutex);
        DeleteSslContex();
        delete m_pAcceptor;
        m_pAcceptor = nullptr;
    }

    catch(...) {

    }
}

void CServer::PostQuitPump() {
    m_IoService.post(boost::bind(&CServer::OnQuit, this));
}

std::string CServer::GetPassword() const {
    SPA::UINT64 src = (SPA::UINT64)this;
    return MQ_FILE::CMyContainer::Container.Get(src);
}

std::string CServer::GetMessageQueuePassword() const {
    SPA::UINT64 src = (SPA::UINT64) & m_IoService;
    return MQ_FILE::CMyContainer::Container.Get(src);
}

bool CServer::StartIOPump() {
    if (m_vThread.size())
        return true;
    m_vThread.push_back(std::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CServer::StartIOPumpInternal, this))));
    sleep(boost::posix_time::milliseconds(100));
    return !m_bStopped;
}

void CServer::StartSubThread() {
#ifdef WIN32_64
    m_mutex.lock();
    UTHREAD_ID dwThreadId = ::GetCurrentThreadId();
    CCrashHandler::MAIN_THREADS.push_back(dwThreadId);
    m_mutex.unlock();
    bool bCOM = (::CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK);
#endif

#ifdef WINCE

#elif defined(WIN32_64)
    CCrashHandler::SetThreadExceptionHandlers();
#else

#endif
    {
        CAutoLock sl(m_mutex);
        m_vMainThread.push_back(::GetCurrentThreadId());
    }
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teStarted);
    }
    try{
        m_IoService.run(m_ec);
    }

    catch(boost::system::system_error & err) {
        //#ifndef NDEBUG
        std::cout << "boost::system_error " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(SPA::CUException & err) {
        //#ifndef NDEBUG
        std::cout << "SPA::CUException " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(std::exception & err) {
        //#ifndef NDEBUG
        std::cout << "std::exception " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(...) {
        //#ifndef NDEBUG
        std::cout << "Unknown exception " << ", " << __FUNCTION__ << std::endl;
        //#endif
    }
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teKilling);
    }
#ifdef WIN32_64

    if (bCOM)
        ::CoUninitialize();
    m_mutex.lock();
    auto it = std::find(CCrashHandler::MAIN_THREADS.cbegin(), CCrashHandler::MAIN_THREADS.cend(), dwThreadId);
    CCrashHandler::MAIN_THREADS.erase(it);
    m_mutex.unlock();
#endif
}

void CServer::StartIOPumpInternal() {
    char secret[10] = USecretKey;
    m_bStopped = false;
    m_vMainThread.clear();
    m_vMainThread.push_back(::GetCurrentThreadId());
#ifdef WIN32_64
    m_mutex.lock();
    UTHREAD_ID dwThreadId = ::GetCurrentThreadId();
    CCrashHandler::MAIN_THREADS.push_back(dwThreadId);
    m_mutex.unlock();
    bool bCOM = (::CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK);
#endif

#ifdef WINCE

#elif defined(WIN32_64)
    CCrashHandler::SetThreadExceptionHandlers();
#else

#endif
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teStarted);
    }
    {
        CAutoLock al(m_mutex);
        g_bRegistered = SPA::ServerSide::IsRegisterred(secret, m_reg);
        m_reg.SetOSs();
        if (m_pAcceptor == nullptr) {
            return;
        }
        if (g_bRegistered && m_reg.Services.size()) {
            for (auto it = m_reg.Services.cbegin(), end = m_reg.Services.cend(); it != end; ++it) {
                std::vector<CServiceContext*>::iterator sctx = SeekSC(*it);
                if (sctx != m_vSC.end()) {
                    (*sctx)->m_bRegisterred = true;
                }
            }
            g_bRegistered = false;
        }
        m_tStart = GetTimeTick();
        StartTimer();
    }
    int sub_threads = (m_nParam & 0xffff);
    if (sub_threads > (int) boost::thread::hardware_concurrency()) {
        sub_threads = (int) boost::thread::hardware_concurrency();
    }
    --sub_threads;
    for (int n = 0; n < sub_threads; ++n) {
        m_vThread.push_back(std::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CServer::StartSubThread, this))));
    }
    try{
        m_IoService.reset();
        m_IoService.run(m_ec);
    }

    catch(boost::system::system_error & err) {
        //#ifndef NDEBUG
        std::cout << "boost::system_error " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(SPA::CUException & err) {
        //#ifndef NDEBUG
        std::cout << "SPA::CUException " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(std::exception & err) {
        //#ifndef NDEBUG
        std::cout << "std::exception " << err.what() << ", " << __FUNCTION__ << std::endl;
        //#endif
    }

    catch(...) {
        //#ifndef NDEBUG
        std::cout << "Unknown exception " << ", " << __FUNCTION__ << std::endl;
        //#endif
    }
    m_bStopped = false;
    {
        CAutoLock al(m_mutex);
        delete m_pSession;
        delete m_pAcceptor;
        DeleteSslContex();
        m_pAcceptor = nullptr;
        m_pSession = nullptr;
        Clean();
    }
    if (CServerSession::m_pQLastIndex) {
        CServerSession::m_pQLastIndex->Stop();
        CServerSession::m_pQLastIndex.reset();
    }
    for (auto it = g_vThreadEvent.begin(), end = g_vThreadEvent.end(); it != end; ++it) {
        (*it)(SPA::ServerSide::tagThreadEvent::teKilling);
    }
    g_vThreadEvent.clear();
#ifdef WIN32_64
    if (bCOM)
        ::CoUninitialize();
    m_mutex.lock();
    auto it = std::find(CCrashHandler::MAIN_THREADS.cbegin(), CCrashHandler::MAIN_THREADS.cend(), dwThreadId);
    CCrashHandler::MAIN_THREADS.erase(it);
    m_mutex.unlock();
#endif
}

void CServer::HandleThreadPoolInternal() {
    bool b;
    size_t n, size;
    SPA::CSpinAutoLock sl(m_mTP);
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
                p->Kill();
                delete p;
                break;
            }
        }
    } while (b);
}

void CServer::HandleSlowSwitchInternal(SPA::UINT64 tNow, std::vector<CServerSession*> &aSession) {
    size_t n, size = aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = aSession[n];
        if (p->GetLatestTime() + m_ulMinSwitchTime < tNow) {
            unsigned int sid = p->GetSvsID();
            switch (sid) {
                case (unsigned int) SPA::tagServiceID::sidStartup:
                    p->PostClose(0); //????? set timeout
                    break;
                default:
                    break;
            }
        }
    }
}

void CServer::HandleServerPingInternal(SPA::UINT64 tNow, std::vector<CServerSession*> &aSession) {
    bool reg = false;
    //boost::posix_time::ptime tNow = t - boost::posix_time::milliseconds(m_ulPingInterval - m_ulTimerElapse - 1000);
    size_t n, size = aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = aSession[n];
        if ((p->GetLatestTime() + m_ulPingInterval - m_ulTimerElapse - 1000) < tNow) {
            if (!reg) {
                reg = true;
                m_reg.Initialize();
                if (m_reg.ShouldShutdown()) {
                    std::cout << "Server is going to shutdown because registration is not qualified. Please contact UDAParts for a new key." << std::endl;
                    sleep(boost::posix_time::seconds(std::rand() % 20));
                    PostQuitPump();
                }
            }
            unsigned int sid = p->GetSvsID();
            switch (sid) {
                case (unsigned int) SPA::tagServiceID::sidStartup:
                    break;
                case (unsigned int) SPA::tagServiceID::sidHTTP:
                    p->PostClose(0); //????? set timeout
                    break;
                default:
                    p->SendReturnData((unsigned short) SPA::tagBaseRequestID::idPing, nullptr, 0);
                    break;
            }
        }
    }
}

void CServer::OnTimerSM(const CErrorCode& Error) {
    {
        m_mutex.lock();
        std::vector<CServerSession*> aSession = m_aSession;
        m_mutex.unlock();
        size_t size = aSession.size();
        for (size_t n = 0; n < size; ++n) {
            CServerSession *p = aSession[n];
            unsigned int sid = p->GetSvsID();
            switch (sid) {
                case (unsigned int) SPA::tagServiceID::sidStartup:
                    break;
                default:
                    //p->ShrinkMemory(); //may deadlock
                    m_IoService.post(boost::bind(&CServerSession::ShrinkMemory, p));
                    break;
            }
        }
    }
    m_TimerSM.expires_at(m_TimerSM.expires_at() + boost::posix_time::milliseconds(m_ulSMInterval));
    m_TimerSM.async_wait(boost::bind(&CServer::OnTimerSM, this, nsPlaceHolders::error));
    UHTTP::CHttpHeaderScopeUQueue::ResetSize();
    SPA::CScopeUQueue::ResetSize();
    UHTTP::CHttpDownFileBuffer::ResetSize();
    UHTTP::CHttpSubRequestScopeUQueue::ResetSize();
}

void CServer::OnTimer(const CErrorCode& Error) {
    SPA::INT64 ms;
    if (m_bStopped) return;
    HandleThreadPoolInternal();
    SPA::UINT64 tNow = GetTimeTick();
    ms = (tNow - m_tStart);
    m_mutex.lock();
    std::vector<CServerSession*> aSession = m_aSession;
    m_mutex.unlock();
    HandleSlowSwitchInternal(ms, aSession);
    HandleServerPingInternal(ms, aSession);
    if (g_bRegistered && ((m_nRequestCount - m_nRequestCountLast) / m_ulTimerElapse > m_reg.MaxSpeed / 1000))
        g_bRegistered = false;
    m_Timer.expires_at(m_Timer.expires_at() + boost::posix_time::milliseconds(m_ulTimerElapse));
    m_Timer.async_wait(boost::bind(&CServer::OnTimer, this, nsPlaceHolders::error));
    m_nRequestCountLast = m_nRequestCount;
    //CAutoLock sl(m_mutex); //dead lock within multi-main-threads environment
    if (m_pOnIdle && !m_bStopped) {
        m_pOnIdle(ms);
    }
}

void CServer::StartTimer() {
    m_Timer.async_wait(boost::bind(&CServer::OnTimer, this, nsPlaceHolders::error));
    m_TimerSM.async_wait(boost::bind(&CServer::OnTimerSM, this, nsPlaceHolders::error));
}

void CServer::OnMessage() {
    size_t size;
    PSession pSession;
    SPA::CUThreadMessage message;
    {
        SPA::CSpinAutoLock sl(m_mTH);
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
                            //pSession->m_ec.assign(nData, boost::asio::error::get_system_category());
                            pSession->PostClose(nData);
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
                    RemoveThread(pServerThread);
                }
                    break;
                default:
                    break;
            }
        }

        SPA::CScopeUQueue::Unlock(message.m_pMessageBuffer);

        {
            SPA::CSpinAutoLock sl(m_mTH);
            size = m_qThreadMessage.size();
            if (size == 0)
                return;
            message = m_qThreadMessage.front();
            m_qThreadMessage.pop();
        }
    }
}

bool CServer::PostSproMessage(CServerSession *pSession, unsigned int nMsgId, int nEvent, int nData) {
    SPA::CScopeUQueue sb;
    sb << nEvent << nData;
    return PostSproMessage(pSession, nMsgId, sb->GetBuffer(), sb->GetSize());
}

bool CServer::PostSproMessage(CServerSession *pSession, unsigned int nMsgId, const void *pBuffer, unsigned int nSize) {
    SPA::CUThreadMessage message(nMsgId, SPA::CScopeUQueue::Lock(), 0);
    *(message.m_pMessageBuffer) << pSession;
    if (pBuffer && nSize) {
        message.m_pMessageBuffer->Push((const unsigned char*) pBuffer, (unsigned int) nSize);
    }
    m_mTH.lock();
    m_qThreadMessage.push(message);
    m_mTH.unlock();
    boost::asio::post(m_IoService, boost::bind(&CServer::OnMessage, this));
    return true;
}

void CServer::PostSproMessage(SPA::CUThreadMessage message) {
    m_mTH.lock();
    m_qThreadMessage.push(message);
    m_mTH.unlock();
    boost::asio::post(m_IoService, boost::bind(&CServer::OnMessage, this));
}

bool CServer::IsSsl() {
    switch (m_EncryptionMethod) {
        case SPA::tagEncryptionMethod::TLSv1:
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
    if (keyFile == nullptr)
        keyFile = "";
    //CAutoLock sl(m_mutex);
    m_strPrivateKeyFile = keyFile;
}

void CServer::SetOnAccept(POnAccept p) {
    //CAutoLock sl(m_mutex);
    m_pOnAccept = p;
}

void CServer::SetOnIdle(POnIdle p) {
    CAutoLock sl(m_mutex);
    m_pOnIdle = p;
}

void CServer::SetCertFile(const char *certFile) {
    if (certFile == nullptr)
        certFile = "";
    //CAutoLock sl(m_mutex);
    m_strCertFile = certFile;
}

void CServer::SetDhFile(const char* file) {
    if (file == nullptr)
        file = "";
    //CAutoLock sl(m_mutex);
    m_strDhFile = file;
}

void CServer::SetMessageQueuePassword(const char *pwd) {
    std::string str;
    if (pwd)
        str = pwd;
    SPA::UINT64 src = (SPA::UINT64) & m_IoService;
    MQ_FILE::CMyContainer::Container.Set(src, str.c_str());
}

void CServer::SetPKFPassword(const char *pwd) {
    SPA::UINT64 src = (SPA::UINT64)this;
    std::string str;
    if (pwd)
        str = pwd;
    MQ_FILE::CMyContainer::Container.Set(src, str.c_str());
}

void CServer::SetEncryptionMethod(SPA::tagEncryptionMethod em) {
    //CAutoLock sl(m_mutex);
    m_EncryptionMethod = em;
}

SPA::tagEncryptionMethod CServer::GetEncryptionMethod() {
    //CAutoLock sl(m_mutex);
    return m_EncryptionMethod;
}

void CServer::SetPfxFile(const char *pfxFile) {
    //CAutoLock sl(m_mutex);
    m_strPfxFile = pfxFile;
}

void CServer::SetAuthenticationMethod(SPA::ServerSide::tagAuthenticationMethod am) {
    //CAutoLock sl(m_mutex);
    m_am = am;
}

SPA::ServerSide::tagAuthenticationMethod CServer::GetAuthenticationMethod() {
    //CAutoLock sl(m_mutex);
    return m_am;
}

void CServer::SetDefaultZip(bool bZip) {
    //CAutoLock sl(m_mutex);
    m_bZip = bZip;
}

bool CServer::GetDefaultZip() {
    //CAutoLock sl(m_mutex);
    return m_bZip;
}

void CServer::SetMaxConnectionsPerClient(unsigned int ulMaxConnectionsPerClient) {
    //CAutoLock sl(m_mutex);
    m_ulMaxConnectionsPerClient = ulMaxConnectionsPerClient;
}

unsigned int CServer::GetMaxConnectionsPerClient() {
    //CAutoLock sl(m_mutex);
    return m_ulMaxConnectionsPerClient;
}

void CServer::SetMaxThreadIdleTimeBeforeSuicide(unsigned int ulMaxThreadIdleTimeBeforeSuicide) {
    //CAutoLock sl(m_mutex);
    m_ulMaxThreadIdleTimeBeforeSuicide = ulMaxThreadIdleTimeBeforeSuicide;
}

unsigned int CServer::GetMaxThreadIdleTimeBeforeSuicide() {
    //CAutoLock sl(m_mutex);
    return m_ulMaxThreadIdleTimeBeforeSuicide;
}

unsigned int CServer::GetCountOfClients() {
    CAutoLock sl(m_mutex);
    return (unsigned int) m_aSession.size();
}

void CServer::SetTimerElapse(unsigned int ulTimerElapse) {
    //CAutoLock sl(m_mutex);
    m_ulTimerElapse = ulTimerElapse;
}

unsigned int CServer::GetTimerElapse() {
    //CAutoLock sl(m_mutex);
    return m_ulTimerElapse;
}

unsigned int CServer::GetSMInterval() {
    //CAutoLock sl(m_mutex);
    return m_ulSMInterval;
}

void CServer::SetSMInterval(unsigned int ulSMInterval) {
    //CAutoLock sl(m_mutex);
    m_ulSMInterval = ulSMInterval;
}

void CServer::SetPingInterval(unsigned int pingInterval) {
    //CAutoLock sl(m_mutex);
    if (pingInterval < m_ulTimerElapse)
        pingInterval = m_ulTimerElapse;
    if (pingInterval > (0xFFFF * 1000))
        pingInterval = (0xFFFF * 1000);
    m_ulPingInterval = pingInterval;
}

unsigned int CServer::GetPingInterval() {
    //CAutoLock sl(m_mutex);
    return m_ulPingInterval;
}

void CServer::SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval) {
    //CAutoLock sl(m_mutex);
    m_ulRecycleGlobalMemoryInterval = recycleGlobalMemoryInterval;
}

unsigned int CServer::GetRecycleGlobalMemoryInterval() {
    //CAutoLock sl(m_mutex);
    return m_ulRecycleGlobalMemoryInterval;
}

SPA::UINT64 CServer::GetRequestCount() {
    //CAutoLock sl(m_mutex);
    return m_nRequestCount;
}

unsigned int CServer::GetSwitchTime() {
    //CAutoLock sl(m_mutex);
    return m_ulMinSwitchTime;
}

void CServer::SetSwitchTime(unsigned int switchTime) {
    //CAutoLock sl(m_mutex);
    m_ulMinSwitchTime = switchTime;
}

USocket_Server_Handle CServer::GetClient(unsigned int index) {
    CAutoLock sl(m_mutex);
    if (index >= (unsigned int) m_aSession.size())
        return 0;
    return m_aSession[index]->MakeHandlerInternal();
}

void CServer::SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p) {
    //CAutoLock sl(m_mutex);
    m_pOnSSLHandShakeCompleted = p;
}

void CServer::SetOnClose(POnClose p) {
    //CAutoLock sl(m_mutex);
    m_pOnClose = p;
}

void CServer::SetOnIsPermitted(POnIsPermitted p) {
    //CAutoLock sl(m_mutex);
    m_pOnIsPermitted = p;
}

void CServer::SetSharedAM(bool b) {
    //CAutoLock sl(m_mutex);
    m_bSharedAM = b;
}

bool CServer::GetSharedAM() {
    //CAutoLock sl(m_mutex);
    return m_bSharedAM;
}

int CServer::GetErrorCode() {
    CAutoLock sl(m_mutex);
    return m_ec.value();
}

bool CServer::IsMainThread() {
    //CAutoLock sl(m_mutex);
    return std::find(m_vMainThread.begin(), m_vMainThread.end(), ::GetCurrentThreadId()) != m_vMainThread.end();
}

unsigned int CServer::GetMainThreads() {
    return (unsigned int) m_vMainThread.size();
}

std::string CServer::GetErrorMessage() {
    CAutoLock sl(m_mutex);
    return m_ec.message();
}

bool CServer::IsRunning() {
    //CAutoLock sl(m_mutex);
    return (m_pAcceptor != nullptr);
}

bool CServer::IsSSLEnabled() {
    //CAutoLock sl(m_mutex);
    return IsSsl();
}

void CServer::AddAChatGroup(unsigned int chatGroupId, const wchar_t *description) {
    if (description == nullptr)
        description = L"";
    //CAutoLock sl(m_mutex);
    m_mapChatGroup[chatGroupId] = description;
}

void CServer::GetJoinedGroupIds(std::vector<unsigned int> &vChatGroup) {
    vChatGroup.clear();
    std::map<unsigned int, std::wstring>::iterator it;
    //CAutoLock sl(m_mutex);
    std::map<unsigned int, std::wstring>::iterator end = m_mapChatGroup.end();
    for (it = m_mapChatGroup.begin(); it != end; ++it) {
        vChatGroup.push_back(it->first);
    }
}

unsigned int CServer::GetCountOfChatGroups() {
    //CAutoLock sl(m_mutex);
    return (unsigned int) (m_mapChatGroup.size());
}

unsigned int CServer::GetJoinedGroupIds(unsigned int *pChatGroupId, unsigned int count) {
    std::map<unsigned int, std::wstring>::iterator it;
    if (count == 0 || pChatGroupId == nullptr)
        return 0;
    unsigned int n = 0;
    //CAutoLock sl(m_mutex);
    std::map<unsigned int, std::wstring>::iterator end = m_mapChatGroup.end();
    for (it = m_mapChatGroup.begin(); n < count && it != end; ++it) {
        pChatGroupId[n] = it->first;
        ++n;
    }
    return n;
}

unsigned int CServer::GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars) {
    if (description == nullptr || chars == 0)
        return 0;
    --chars;
    //CAutoLock sl(m_mutex);
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
    //CAutoLock sl(m_mutex);
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
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    Connection::CConnectionContext::Enter(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, pChatGroupId, nCount);
    CAutoLock al(m_mutex);
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
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->Enter(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->Enter(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else
                p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idEnter, (const unsigned char*) vChatGroup.data(), count * sizeof (unsigned int));
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
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    Connection::CConnectionContext::Exit(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, pChatGroupId, nCount);
    CAutoLock al(m_mutex);
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
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->Exit(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->Exit(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vChatGroup.data(), count, p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else
                p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idExit, (const unsigned char*) vChatGroup.data(), count * sizeof (unsigned int));
        }
    }
}

bool CServer::Speak(CServerSession *pSession, const unsigned int *pChatGroupId, unsigned int nCount, const SPA::UVariant &vtMsg) {
    unsigned short portSender;
    SPA::CScopeUQueue sb;
    std::vector<unsigned int> vChatGroup;
    wchar_t strUserID[MAX_USERID_CHARS + 1];
    std::string ipAddrSender;
    unsigned int ServiceId = pSession->ServiceId;
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    Connection::CConnectionContext::Speak(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMsg, pChatGroupId, nCount);
    CAutoLock al(m_mutex);
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
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->Speak(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMsg, vChatGroup.data(), count, *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->Speak(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMsg, vChatGroup.data(), count, p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else {
                sb << vtMsg;
                sb << count;
                sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
                p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idSpeak, sb->GetBuffer(), sb->GetSize());
                sb->SetSize(0);
            }
        }
    }
    return true;
}

bool CServer::Speak(const unsigned int *pChatGroupId, unsigned int nCount, const SPA::UVariant &vtMsg) {
    SPA::CScopeUQueue sb;
    std::vector<unsigned int> vChatGroup;
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
            continue;
        }
        p->m_mutex.lock();
        Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
        p->m_mutex.unlock();
        unsigned int count = (unsigned int) vChatGroup.size();
        if (count > 0) {
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->Speak("", 0, L"", 0, vtMsg, vChatGroup.data(), count, *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->Speak("", 0, L"", 0, vtMsg, vChatGroup.data(), count, p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else {
                sb << vtMsg;
                sb << count;
                sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
                p->SendChatResult("", 0, L"", 0, (unsigned short) SPA::tagChatRequestID::idSpeak, sb->GetBuffer(), sb->GetSize());
                sb->SetSize(0);
            }
        }
    }
    return true;
}

bool CServer::SpeakEx(CServerSession *pSession, const unsigned char *message, unsigned int len, const unsigned int *pChatGroupId, unsigned int nCount) {
    unsigned short portSender;
    SPA::CScopeUQueue sb;
    std::vector<unsigned int> vChatGroup;
    wchar_t strUserID[MAX_USERID_CHARS + 1];
    std::string ipAddrSender;
    unsigned int ServiceId = pSession->ServiceId;
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p == pSession || p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP)
            continue;
        p->m_mutex.lock();
        Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
        p->m_mutex.unlock();
        unsigned int count = (unsigned int) vChatGroup.size();
        if (count > 0) {
            sb << len;
            if (len)
                sb->Push(message, len);
            sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
            p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idSpeakEx, sb->GetBuffer(), sb->GetSize());
            sb->SetSize(0);
        }
    }
    return true;
}

bool CServer::SpeakEx(const unsigned char *message, unsigned int len, const unsigned int *pChatGroupId, unsigned int nCount) {
    SPA::CScopeUQueue sb;
    std::vector<unsigned int> vChatGroup;
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP)
            continue;
        p->m_mutex.lock();
        Connection::CConnectionContextBase::ChatGroupsAnd(pChatGroupId, nCount, p->m_ccb.ChatGroups.data(), (unsigned int) p->m_ccb.ChatGroups.size(), vChatGroup);
        p->m_mutex.unlock();
        unsigned int count = (unsigned int) vChatGroup.size();
        if (count > 0) {
            sb << len;
            if (len)
                sb->Push(message, len);
            sb->Push((const unsigned char*) vChatGroup.data(), sizeof (unsigned int) *count);
            p->SendChatResult("", 0, L"", 0, (unsigned short) SPA::tagChatRequestID::idSpeakEx, sb->GetBuffer(), sb->GetSize());
            sb->SetSize(0);
        }
    }
    return true;
}

bool CServer::SendUserMessage(CServerSession *pSession, const wchar_t *userId, const SPA::UVariant &vtMessage) {
    unsigned short portSender;
    wchar_t strUserID[MAX_USERID_CHARS + 1], strMe[MAX_USERID_CHARS + 1];
    std::string ipAddrSender;
    unsigned int ServiceId = pSession->ServiceId;
    SPA::CScopeUQueue us;
    us << vtMessage;
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    pSession->m_mutex.lock();
    Connection::CConnectionContext::SendUserMessage(pSession->m_ccb.Id, ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMessage, userId, pSession->m_ccb.ChatGroups.data(), (unsigned int) pSession->m_ccb.ChatGroups.size());
    pSession->m_mutex.unlock();
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p == pSession)
            continue;
        if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
            continue;
        }
        p->GetUID(strMe, sizeof (strMe) / sizeof (wchar_t));
#ifdef WIN32_64
        if (_wcsicmp(strMe, userId) == 0)
#else
        if (wcscasecmp(strMe, userId) == 0)
#endif
        {
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->SendUserMessage(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMessage, pSession->m_ccb.ChatGroups.data(), (unsigned int) pSession->m_ccb.ChatGroups.size(), *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->SendUserMessage(ipAddrSender.c_str(), portSender, strUserID, ServiceId, vtMessage, pSession->m_ccb.ChatGroups.data(), (unsigned int) pSession->m_ccb.ChatGroups.size(), p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else
                p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idSendUserMessage, us->GetBuffer(), us->GetSize());
        }
    }
    return true;
}

bool CServer::SendUserMessage(const wchar_t *userId, const SPA::UVariant &vtMessage) {
    wchar_t strMe[MAX_USERID_CHARS + 1];
    SPA::CScopeUQueue us;
    us << vtMessage;
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p->m_pHttpContext && !p->m_pHttpContext->IsWebSocket()) {
            continue;
        }
        p->GetUID(strMe, sizeof (strMe) / sizeof (wchar_t));
#ifdef WIN32_64
        if (_wcsicmp(strMe, userId) == 0)
#else
        if (wcscasecmp(strMe, userId) == 0)
#endif
        {
            if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP) {
                CAutoLock al(p->m_mutex);
                if (p->m_pHttpContext) {
                    UHTTP::CWebResponseProcessor *pWebResponseProcessor = p->m_pHttpContext->GetWebResponseProcessor();
                    if (pWebResponseProcessor) {
                        if (p->m_pSspi) {
                            SPA::CScopeUQueue sb;
                            pWebResponseProcessor->SendUserMessage("", 0, L"", 0, vtMessage, nullptr, 0, *sb);
                            p->Write(sb->GetBuffer(), sb->GetSize());
                        } else {
                            pWebResponseProcessor->SendUserMessage("", 0, L"", 0, vtMessage, nullptr, 0, p->m_qWrite);
                            p->Write(nullptr, 0);
                        }
                    }
                }
            } else
                p->SendChatResult("", 0, L"", 0, (unsigned short) SPA::tagChatRequestID::idSendUserMessage, us->GetBuffer(), us->GetSize());
        }
    }
    return true;
}

bool CServer::SendUserMessage(CServerSession *pSession, const wchar_t *userId, const unsigned char *message, unsigned int len) {
    unsigned short portSender;
    wchar_t strUserID[MAX_USERID_CHARS + 1], strMe[MAX_USERID_CHARS + 1];
    std::string ipAddrSender;
    unsigned int ServiceId = pSession->ServiceId;
    pSession->GetUserIdInternally(strUserID, sizeof (strUserID) / sizeof (wchar_t));
    pSession->GetPeerName(ipAddrSender, &portSender);
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p == pSession || p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP)
            continue;
        p->GetUID(strMe, sizeof (strMe) / sizeof (wchar_t));
#ifdef WIN32_64
        if (_wcsicmp(strMe, userId) == 0)
#else
        if (wcscasecmp(strMe, userId) == 0)
#endif
        {
            p->SendChatResult(ipAddrSender.c_str(), portSender, strUserID, ServiceId, (unsigned short) SPA::tagChatRequestID::idSendUserMessageEx, message, len);
        }
    }
    return true;
}

bool CServer::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int len) {
    wchar_t strMe[MAX_USERID_CHARS + 1];
    CAutoLock al(m_mutex);
    size_t n, size = m_aSession.size();
    for (n = 0; n < size; ++n) {
        CServerSession *p = m_aSession[n];
        if (p->GetSvsID() == (unsigned int) SPA::tagServiceID::sidHTTP)
            continue;
        p->GetUID(strMe, sizeof (strMe) / sizeof (wchar_t));
#ifdef WIN32_64
        if (_wcsicmp(strMe, userId) == 0)
#else
        if (wcscasecmp(strMe, userId) == 0)
#endif
        {
            p->SendChatResult("", 0, L"", 0, (unsigned short) SPA::tagChatRequestID::idSendUserMessageEx, message, len);
        }
    }
    return true;
}

void CServer::OnAccepted(CServerSession* pSession, const CErrorCode& Error) {
    bool chatting = false;
    CAutoLock al(m_mutex);
    ++m_nConnIndex;
    if (m_nConnIndex > MAX_SESSION_INDEX)
        m_nConnIndex = 1;
    if (!Error) {
        if (IsTooMany(pSession)) {
            //pSession->m_ec.assign(boost::asio::error::connection_refused, boost::asio::error::get_system_category());
            pSession->m_ulIndex = 0;
            pSession->PostClose(boost::asio::error::connection_refused);
            //pSession->Close();
        } else {
            m_ec.clear();
            pSession->m_ulIndex = m_nConnIndex;
            m_aSession.push_back(pSession);
            pSession->Start();
            if (m_aSession.size() > m_reg.ManyClients)
                g_bRegistered = false;
        }

        if (m_pOnAccept != nullptr) {
            boost::uint64_t handler = pSession->MakeHandlerInternal();
            try{
                CRAutoLock ral(m_mutex, chatting);
                m_pOnAccept(handler, m_ec.value());
            }

            catch(...) {
            }
        }

        size_t size = m_aSessionDead.size();
        if (size > 20) {
            m_pSession = m_aSessionDead.front();
            m_aSessionDead.pop_front();
        } else {
            m_pSession = new CServerSession();
        }
        m_pSession->SetContext();
        m_pAcceptor->async_accept(m_pSession->GetSocket().lowest_layer(), boost::bind(&CServer::OnAccepted, this, m_pSession, nsPlaceHolders::error));

    } else if (pSession) {
        m_ec = Error;
        pSession->m_ulIndex = 0;
        if (m_pOnAccept != nullptr) {
            boost::uint64_t handler = pSession->MakeHandlerInternal();
            try{
                CRAutoLock ral(m_mutex, chatting);
                m_pOnAccept(handler, m_ec.value());
            }

            catch(...) {
            }
        }
    }
}

unsigned int CServer::StartQueue(const char *qFileName, bool dequeueShared, unsigned int ttl) {
    if (!qFileName)
        return INVALID_QUEUE_HANDLE;
    std::string fn = qFileName;
    boost::trim(fn);
    if (fn.size() == 0)
        return INVALID_QUEUE_HANDLE;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute())
        fn = m_WorkingPath + fn;
#ifdef WIN32_64
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
#endif
    CMapQueue::iterator it;
    CAutoLock sl(m_mQ);
    CMapQueue::iterator end = m_mapQueue.end();
    for (it = m_mapQueue.begin(); it != end; ++it) {
        if (it->second->GetMQFileName().find(fn) == 0)
            return it->first;
    }

    unsigned int key = (unsigned int) m_mapQueue.size() + 1;
    while (m_mapQueue.find(key) != end) {
        ++key;
    }

    std::shared_ptr<MQ_FILE::CMqFile > q;

    std::string pwd = GetMessageQueuePassword();

    if (pwd.size()) {
        SPA::CScopeUQueue suId;
        SPA::CScopeUQueue suPwd;
        std::string id = SPA::GetAppName();
#ifdef WIN32_64
        std::transform(id.begin(), id.end(), id.begin(), ::tolower);
#endif
        SPA::Utilities::ToWide(id.c_str(), id.size(), *suId);
        SPA::Utilities::ToWide(pwd.c_str(), pwd.size(), *suPwd);
        q = std::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFileEx(fn.c_str(), ttl, SPA::tagOptimistic::oSystemMemoryCached,
                (const wchar_t*) suId->GetBuffer(),
                (const wchar_t*) suPwd->GetBuffer(), CServerSession::m_pQLastIndex.get(), false, dequeueShared));

    } else {
        q = std::shared_ptr<MQ_FILE::CMqFile > (new MQ_FILE::CMqFile(fn.c_str(), ttl, SPA::tagOptimistic::oSystemMemoryCached, false, false, dequeueShared));
    }

    if (q)
        q->SetOptimistic(SPA::tagOptimistic::oMemoryCached);

    if (!q->IsAvailable())
        return INVALID_QUEUE_HANDLE;
    else
        m_mapQueue[key] = q;

    return key;
}

const char* CServer::GetQueueFileName(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return nullptr;
    return it->second->GetMQFileName().c_str();
}

unsigned int CServer::PeekQueuedRequests(unsigned int qHandle, SPA::CQueuedRequestInfo *qri, unsigned int count) {
    bool chatting = false;
    CAutoLock al(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    {
        CRAutoLock ral(m_mQ, chatting);
        return it->second->PeekRequests(qri, count);
    }
}

SPA::UINT64 CServer::CancelQueuedRequests(unsigned int qHandle, SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
    CAutoLock al(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    std::shared_ptr<MQ_FILE::CMqFile> &qRequest = it->second;
    if (qRequest && qRequest->IsAvailable()) {
        return qRequest->CancelQueuedRequests(startIndex, endIndex);
    }
    return 0;
}

unsigned int CServer::CancelQueuedRequests(unsigned int qHandle, const unsigned short *ids, unsigned int count) {
    if (!ids || !count)
        return 0;
    CAutoLock al(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    std::shared_ptr<MQ_FILE::CMqFile> &qRequest = it->second;
    if (qRequest && qRequest->IsAvailable()) {
        return qRequest->CancelQueuedRequests(ids, count);
    }
    return 0;
}

unsigned int CServer::GetMessagesInDequeuing(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    return it->second->GetMessagesInDequeuing();
}

SPA::UINT64 CServer::BatchEnqueue(unsigned int qHandle, unsigned int count, const unsigned char *messages) {
    SPA::UINT64 qIndex = 0;
    if (!messages || !count)
        count = 0;
    std::shared_ptr<MQ_FILE::CMqFile> queue;
    {
        CAutoLock sl(m_mQ);
        CMapQueue::iterator it = m_mapQueue.find(qHandle);
        if (it == m_mapQueue.end())
            return qIndex;
        queue = it->second;
    }
    bool ok;
    {
        qIndex = queue->BatchEnqueue(count, messages);
        ok = (qIndex != INVALID_NUMBER);
    }
    if (!ok)
        return 0;
    //notify clients that a message is available now
    if (queue->JustOne(count)) {
        CAutoLock sl(m_mQ);
        CMapQNotified::iterator mi = m_mapQNotified.find(qHandle);
        if (mi != m_mapQNotified.end()) {
            SPA::CStreamHeader sh((unsigned short) SPA::tagBaseRequestID::idMessageQueued);
            std::vector<CSNotified> &v = mi->second;
            std::vector<CSNotified>::iterator s, end = v.end();
            for (s = v.begin(); s != end; ++s) {
                if (s->second) {
                    unsigned int index;
                    USocket_Server_Handle h = s->first;
                    CServerSession *pSession = GetSvrSession(h, index);
                    CAutoLock lock(pSession->m_mutex);
                    if (pSession->m_cs >= csConnected)
                        pSession->Write((const unsigned char*) &sh, sizeof (sh));
                }
            }
        }
    }
    return qIndex;
}

SPA::UINT64 CServer::Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size) {
    SPA::UINT64 qIndex = 0;
    if (reqId < (unsigned short) SPA::tagBaseRequestID::idReservedTwo)
        return qIndex;
    if (!buffer)
        size = 0;
    SPA::CStreamHeader ReqInfo(reqId, size);
    std::shared_ptr<MQ_FILE::CMqFile> queue;
    {
        CAutoLock sl(m_mQ);
        CMapQueue::iterator it = m_mapQueue.find(qHandle);
        if (it == m_mapQueue.end())
            return qIndex;
        queue = it->second;
    }
    bool ok;
    bool oneonly;
    {
        qIndex = queue->Enqueue(ReqInfo, buffer, size, oneonly);
        ok = (qIndex != INVALID_NUMBER);
    }
    if (!ok)
        return 0;

    //notify clients that a message is available now
    if (oneonly) {
        CAutoLock sl(m_mQ);
        CMapQNotified::iterator mi = m_mapQNotified.find(qHandle);
        if (mi != m_mapQNotified.end()) {
            SPA::CStreamHeader sh((unsigned short) SPA::tagBaseRequestID::idMessageQueued);
            std::vector<CSNotified> &v = mi->second;
            std::vector<CSNotified>::iterator s, end = v.end();
            for (s = v.begin(); s != end; ++s) {
                if (s->second) {
                    unsigned int index;
                    USocket_Server_Handle h = s->first;
                    CServerSession *pSession = GetSvrSession(h, index);
                    CAutoLock lock(pSession->m_mutex);
                    if (pSession->m_cs >= csConnected)
                        pSession->Write((const unsigned char*) &sh, sizeof (sh));
                }
            }
        }
    }
    return qIndex;
}

SPA::UINT64 CServer::GetMessageCount(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    return it->second->GetMessageCount();
}

SPA::UINT64 CServer::GetQueueSize(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it != m_mapQueue.end())
        return it->second->GetMQSize();
    return 0;
}

SPA::UINT64 CServer::GetLastQueueMessageTime(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it != m_mapQueue.end())
        return it->second->GetLastTime();
    return 0;
}

bool CServer::StopQueueByHandle(unsigned int qHandle, bool permanent) {
    int res = 0;
    CAutoLock sl(m_mQ);
    m_mapQNotified.erase(qHandle);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it != m_mapQueue.end()) {
        if (permanent) {
            it->second->StopQueue();
            res = ::remove(it->second->GetMQFileName().c_str());
        }
        m_mapQueue.erase(it);
        return (res == 0);
    }
    return false;
}

SPA::UINT64 CServer::Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime) {
    unsigned int index;
    if (!messageCount)
        return 0;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    SPA::CScopeUQueue sb;
    SPA::CScopeUQueue temp;
    SPA::CUQueue &qTemp = *temp;
    SPA::CUQueue &buffer = *sb;
    std::shared_ptr<MQ_FILE::CMqFile> queue;
    MQ_FILE::CQueueInitialInfo qii;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;
    unsigned int count = 0;
    {
        CAutoLock sl(m_mQ);
        CMapQueue::iterator it = m_mapQueue.find(qHandle);
        if (it == m_mapQueue.end())
            return INVALID_QUEUE_HANDLE;
        queue = it->second;
        if (!queue->IsAvailable())
            return INVALID_QUEUE_HANDLE;
        qii = queue->GetMQInitInfo();
    }
    std::vector<unsigned int> vSize = queue->DoBatchDequeue(messageCount, *qAttr, *qRequests, waitTime);
    unsigned int records = (unsigned int) vSize.size();
    const MQ_FILE::QAttr *qattr = (const MQ_FILE::QAttr *)qAttr->GetBuffer();
    {
        CAutoLock al(pSession->m_mutex);
        if (pSession->m_cs >= csConnected && pSession->GetConnIndex() == index) {
            if (pSession->m_mapDequeue.find(qHandle) == pSession->m_mapDequeue.end()) {
                buffer << qii;
                SPA::CStreamHeader sh((unsigned short) SPA::tagBaseRequestID::idStartQueue, buffer.GetSize());
                buffer.Insert((const unsigned char*) &sh, sizeof (sh));
                pSession->Write(buffer.GetBuffer(), buffer.GetSize());
            }
        } else {
            for (unsigned int it = 0; it < records; ++it) {
                queue->ConfirmDequeue(qattr[it].MessagePos, qattr[it].MessageIndex, true);
            }
            return SOCKET_NOT_FOUND;
        }
    }
    {
        CAutoLock sl(m_mQ);
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
    }
    buffer.SetSize(0);
    unsigned int bytes = 0;
    unsigned int pos = 0;
    CAutoLock lock(pSession->m_mutex);
    if (pSession->m_cs < csConnected || pSession->GetConnIndex() != index) {
        for (unsigned int it = 0; it < records; ++it) {
            queue->ConfirmDequeue(qattr[it].MessagePos, qattr[it].MessageIndex, true);
        }
        return SOCKET_NOT_FOUND;
    }
    CServerSession::CQueueMap::iterator qp = pSession->m_mapDequeue.find(qHandle);
    if (qp == pSession->m_mapDequeue.end()) {
        CQueueProperty v;
        v.second.reserve(records);
        pSession->m_mapDequeue[qHandle] = v;
        qp = pSession->m_mapDequeue.find(qHandle);
    }
    for (auto it = vSize.begin(), end = vSize.end(); it != end; ++it) {
        unsigned int total = *it;
        SPA::CStreamHeader *sh = (SPA::CStreamHeader *)qRequests->GetBuffer(pos);
        unsigned int size = sh->Size;
        assert((size + sizeof (SPA::CStreamHeader)) <= total);
        sh->Size += (sizeof (MQ_FILE::QAttr) + sizeof (qHandle));
        sh->SetQueued(true);
        buffer << *sh;
        buffer << qHandle;
        if (records == count + 1) {
            MQ_FILE::QAttr qa = qattr[count];
            qa.MessagePos |= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
            buffer << qa;
        } else {
            buffer << qattr[count];
        }
        buffer.Push(qRequests->GetBuffer(pos + sizeof (SPA::CStreamHeader)), size);
        const MQ_FILE::QAttr &p = qattr[count];
        qp->second.second.push_back(p);
        pSession->Write(buffer.GetBuffer(), buffer.GetSize());
        bytes += buffer.GetSize();
        pos += total;
        ++count;
        buffer.SetSize(0);
    }
    SPA::UINT64 res = bytes;
    res <<= 32;
    return (res + count);
}

SPA::UINT64 CServer::Dequeue2(unsigned int qHandle, USocket_Server_Handle h, unsigned int maxBytes, bool beNotifiedWhenAvailable, unsigned int waitTime) {
    unsigned int index;
    if (!maxBytes)
        return 0;
    MQ_FILE::CQueueInitialInfo qii;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    SPA::CScopeUQueue sb;
    SPA::CScopeUQueue temp;
    SPA::CUQueue &qTemp = *temp;
    SPA::CUQueue &buffer = *sb;
    std::shared_ptr<MQ_FILE::CMqFile> queue;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;
    unsigned int count = 0;
    {
        CAutoLock sl(m_mQ);
        CMapQueue::iterator it = m_mapQueue.find(qHandle);
        if (it == m_mapQueue.end())
            return INVALID_QUEUE_HANDLE;
        queue = it->second;
        if (!queue->IsAvailable())
            return INVALID_QUEUE_HANDLE;
        qii = queue->GetMQInitInfo();
    }
    std::vector<unsigned int> vSize = queue->DoBatchDequeue(*qAttr, *qRequests, maxBytes, waitTime);
    unsigned int records = (unsigned int) vSize.size();
    const MQ_FILE::QAttr *qattr = (const MQ_FILE::QAttr *)qAttr->GetBuffer();
    {
        CAutoLock al(pSession->m_mutex);
        if (pSession->m_cs >= csConnected && pSession->GetConnIndex() == index) {
            if (pSession->m_mapDequeue.find(qHandle) == pSession->m_mapDequeue.end()) {
                buffer << qii;
                SPA::CStreamHeader sh((unsigned short) SPA::tagBaseRequestID::idStartQueue, buffer.GetSize());
                buffer.Insert((const unsigned char*) &sh, sizeof (sh));
                pSession->Write(buffer.GetBuffer(), buffer.GetSize());
            }
        } else {
            for (unsigned int it = 0; it < records; ++it) {
                queue->ConfirmDequeue(qattr[it].MessagePos, qattr[it].MessageIndex, true);
            }
            return SOCKET_NOT_FOUND;
        }
    }

    {
        CAutoLock sl(m_mQ);
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
    }
    buffer.SetSize(0);
    unsigned int bytes = 0;
    unsigned int pos = 0;
    CAutoLock lock(pSession->m_mutex);
    if (pSession->m_cs < csConnected || pSession->GetConnIndex() != index) {
        for (unsigned int it = 0; it < records; ++it) {
            queue->ConfirmDequeue(qattr[it].MessagePos, qattr[it].MessageIndex, true);
        }
        return SOCKET_NOT_FOUND;
    }
    CServerSession::CQueueMap::iterator qp = pSession->m_mapDequeue.find(qHandle);
    if (qp == pSession->m_mapDequeue.end()) {
        CQueueProperty v;
        v.second.reserve(records);
        pSession->m_mapDequeue[qHandle] = v;
        qp = pSession->m_mapDequeue.find(qHandle);
    }
    for (auto it = vSize.begin(), end = vSize.end(); it != end; ++it) {
        unsigned int total = *it;
        SPA::CStreamHeader *sh = (SPA::CStreamHeader *)qRequests->GetBuffer(pos);
        unsigned int size = sh->Size;
        assert((size + sizeof (SPA::CStreamHeader)) <= total);
        sh->Size += (sizeof (MQ_FILE::QAttr) + sizeof (qHandle));
        sh->SetQueued(true);
        buffer << *sh;
        buffer << qHandle;
        if (records == count + 1) {
            MQ_FILE::QAttr qa = qattr[count];
            qa.MessagePos |= MQ_FILE::QAttr::RANGE_DEQUEUED_POSITION_END;
            buffer << qa;
        } else {
            buffer << qattr[count];
        }
        buffer.Push(qRequests->GetBuffer(pos + sizeof (SPA::CStreamHeader)), size);
        const MQ_FILE::QAttr &p = qattr[count];
        qp->second.second.push_back(p);
        pSession->Write(buffer.GetBuffer(), buffer.GetSize());
        bytes += buffer.GetSize();
        pos += total;
        ++count;
        buffer.SetSize(0);
    }
    SPA::UINT64 res = bytes;
    res <<= 32;
    return (res + count);
}

void CServer::FreeCredHandle() {
    if (m_hCreds.dwLower || m_hCreds.dwUpper) {
        ::FreeCredentialsHandle(&m_hCreds);
        ::memset(&m_hCreds, 0, sizeof (m_hCreds));
    }
}

PCredHandle CServer::GetCredHandle() {
    return &m_hCreds;
}

bool CServer::OpenPfx(const wchar_t *filePfx, const wchar_t *password) {
    bool ok = true;

    //if filePfx not set, we don't use secure channel
    if (!filePfx || !std::wcslen(filePfx)) {
        return true;
    }

    CString s(filePfx);
    bool bPfx = (s.Right(4).CompareNoCase(_T(".pfx")) == 0);

    FreeCredHandle();
    do {
        PCCERT_CONTEXT context = nullptr;
        if (bPfx) {
            ok = SPA::CCertificateImpl::CertStore.OpenFromPfxFile(filePfx, password);
            if (!ok) {
                m_ec.assign(::GetLastError(), boost::asio::error::get_system_category());
                break;
            }
            context = ::CertFindCertificateInStore(SPA::CCertificateImpl::CertStore.GetCertStore(), X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, nullptr, nullptr);
        } else {
            if (boost::iequals(L"root", filePfx))
                m_cst = SPA::cstRoot;
            else
                m_cst = SPA::cstMy;
            ok = SPA::CCertificateImpl::CertStore.OpenStore(m_cst);
            if (!ok) {
                m_ec.assign(::GetLastError(), boost::asio::error::get_system_category());
                break;
            }
            context = ::CertFindCertificateInStore(SPA::CCertificateImpl::CertStore.GetCertStore(), X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_STR_A, m_strPrivateKeyFile.c_str(), nullptr);
        }
        if (!context) {
            ok = false;
            m_ec.assign(::GetLastError(), boost::asio::error::get_system_category());
            break;
        }
        m_pSelfCert.reset(new SPA::CCertificateImpl(context, ""));
        if (!m_pSelfCert && !m_pSelfCert->GetCertContext()) {
            ok = false;
            m_ec.assign(::GetLastError(), boost::asio::error::get_system_category());
            break;
        }
        PCCERT_CONTEXT pCertContext = m_pSelfCert->GetCertContext();
        m_cst = SPA::CCertificateImpl::CertStore.GetCertStoreType();
        SCHANNEL_CRED SchannelCred;
        ::memset(&SchannelCred, 0, sizeof (SchannelCred));
        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;
        SchannelCred.grbitEnabledProtocols = SP_PROT_TLS1_X_SERVER; //TLSv1.0, 1.1 and 1.2 only 
        //SchannelCred.hRootStore = CCertificateImpl::CertStore.GetCertStore();
        SchannelCred.dwFlags = (SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_REVOCATION_CHECK_CHAIN/* | SCH_SEND_ROOT_CERT*/);
        SECURITY_STATUS status = ::AcquireCredentialsHandle(nullptr,
                (LPWSTR) UNISP_NAME,
                SECPKG_CRED_INBOUND,
                nullptr,
                &SchannelCred,
                nullptr,
                nullptr,
                &m_hCreds,
                nullptr);
        if (status != SEC_E_OK) {
            ::memset(&m_hCreds, 0, sizeof (m_hCreds));
            m_ec.assign(status, boost::asio::error::get_system_category());
            ok = false;
        }
    } while (false);
    return ok;
}

void CServer::ConfirmQueueJob(unsigned int qHandle, const MQ_FILE::QAttr *qa, size_t count, bool successful) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return;
    std::shared_ptr<MQ_FILE::CMqFile> &queue = it->second;
    if (!queue->IsAvailable())
        return;
    queue->ConfirmDequeueJob(qa, count, !successful);
}

void CServer::ConfirmQueue(unsigned int qHandle, const MQ_FILE::QAttr *qa, size_t count) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return;
    std::shared_ptr<MQ_FILE::CMqFile> &queue = it->second;
    if (!queue->IsAvailable())
        return;
    queue->DoConfirmDequeue(qa, count);
}

void CServer::ConfirmQueue(unsigned int qHandle, SPA::UINT64 mqPos, SPA::UINT64 qIndex, bool successful) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return;
    std::shared_ptr<MQ_FILE::CMqFile> &queue = it->second;
    if (!queue->IsAvailable())
        return;
    queue->ConfirmDequeue(mqPos, qIndex, !successful);
}

bool CServer::IsQueueStarted(const char *qName) {
    std::string fn = qName;
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute()) {
#ifdef WIN32_64
        fn = m_WorkingPath + fn;
#else
        fn = m_WorkingPath + fn;
#endif
    }
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it, end = m_mapQueue.end();
    for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
        if (boost::istarts_with(it->second->GetMQFileName(), fn))
#else
        if (boost::starts_with(it->second->GetMQFileName(), fn))
#endif
        {
            return it->second->IsAvailable();
        }
    }
    return false;
}

bool CServer::IsQueueStarted(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    return (it != m_mapQueue.end() && it->second && it->second->IsAvailable());
}

bool CServer::AbortJob(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    return (it != m_mapQueue.end() && it->second && it->second->AbortJob());
}

bool CServer::StartJob(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    return (it != m_mapQueue.end() && it->second && it->second->StartJob() != INVALID_NUMBER);
}

bool CServer::EndJob(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end() || !it->second)
        return false;
    SPA::UINT64 jobSize = it->second->GetJobSize();
    SPA::UINT64 msgCount = it->second->GetMessageCount();
    unsigned int pending = it->second->GetMessagesInDequeuing();
    bool ok = (it->second->EndJob() != INVALID_NUMBER);
    //notify clients that a message is available now
    if (ok && msgCount == (jobSize + pending) && jobSize > 1) {
        CMapQNotified::iterator mi = m_mapQNotified.find(qHandle);
        if (mi != m_mapQNotified.end()) {
            SPA::CStreamHeader sh((unsigned short) SPA::tagBaseRequestID::idMessageQueued);
            std::vector<CSNotified> &v = mi->second;
            std::vector<CSNotified>::iterator s, end = v.end();
            for (s = v.begin(); s != end; ++s) {
                if (s->second) {
                    unsigned int index;
                    USocket_Server_Handle h = s->first;
                    CServerSession *pSession = GetSvrSession(h, index);
                    CAutoLock lock(pSession->m_mutex);
                    if (pSession->m_cs >= csConnected)
                        pSession->Write((const unsigned char*) &sh, sizeof (sh));
                }
            }
        }
    }
    return ok;
}

SPA::UINT64 CServer::AppendQueue(unsigned int qHandle, unsigned int qSrc) {
    if (qHandle == qSrc)
        return 0;
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    CMapQueue::iterator itSrc = m_mapQueue.find(qSrc);
    if (it == m_mapQueue.end() || itSrc == m_mapQueue.end() || !it->second || !itSrc->second)
        return 0;
    return it->second->Append(*(itSrc->second.get()));
}

SPA::UINT64 CServer::GetJobSize(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    return it->second->GetJobSize();
}

SPA::UINT64 CServer::GetQueueLastIndex(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    return it->second->GetLastMessageIndex();
}

SPA::tagOptimistic CServer::GetOptimistic(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return SPA::tagOptimistic::oSystemMemoryCached;
    return it->second->IsOptimistic();
}

void CServer::SetOptimistic(unsigned int qHandle, SPA::tagOptimistic bOptimistic) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return;
    it->second->SetOptimistic(bOptimistic);
}

SPA::tagQueueStatus CServer::GetServerQueueStatus(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return SPA::tagQueueStatus::qsNormal;
    return it->second->GetQueueOpenStatus();
}

std::shared_ptr<MQ_FILE::CMqFile> CServer::GetQueue(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return std::shared_ptr<MQ_FILE::CMqFile > ();
    return it->second;
}

bool CServer::IsDequeueShared(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return false;
    return it->second->IsDequeueShared();
}

void CServer::ResetQueue(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return;
    it->second->Empty();
}

unsigned int CServer::GetTTL(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return 0;
    return it->second->GetTTL();
}

SPA::UINT64 CServer::RemoveQueuedRequestsByTTL(unsigned int qHandle) {
    CMapQueue::iterator it;
    {
        CAutoLock sl(m_mQ);
        it = m_mapQueue.find(qHandle);
        if (it == m_mapQueue.end())
            return 0;
    }
    return it->second->RemoveByTTL();
}

bool CServer::IsQueueSecured(const char *qName) {
    std::string fn = qName;
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute()) {
#ifdef WIN32_64
        fn = m_WorkingPath + fn;
#else
        fn = m_WorkingPath + fn;
#endif
    }
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it, end = m_mapQueue.end();
    for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
        if (boost::istarts_with(it->second->GetMQFileName(), fn))
#else
        if (boost::starts_with(it->second->GetMQFileName(), fn))
#endif
        {
            return it->second->IsSecure();
        }
    }
    return false;
}

const char* CServer::GetQueueName(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    if (it == m_mapQueue.end())
        return nullptr;
    return it->second->GetRawName().c_str();
}

bool CServer::IsQueueSecured(unsigned int qHandle) {
    CAutoLock sl(m_mQ);
    CMapQueue::iterator it = m_mapQueue.find(qHandle);
    return (it != m_mapQueue.end() && it->second && it->second->IsSecure());
}

bool CServer::StopQueueByName(const char *qName, bool permanent) {
    int res = 0;
    std::string fn = qName;
    boost::trim(fn);
    if (fn.size() == 0)
        return false;
    boost::filesystem::path bpath(fn);
    if (!bpath.is_absolute()) {
#ifdef WIN32_64
        fn = m_WorkingPath + fn;
#else
        fn = m_WorkingPath + fn;
#endif
    }
    //it doesn't matter no matter if it is false or true here
    unsigned int handle = StartQueue(qName, true, 0);
    CAutoLock sl(m_mQ);
    m_mapQNotified.erase(handle);
    CMapQueue::iterator it, end = m_mapQueue.end();
    for (it = m_mapQueue.begin(); it != end; ++it) {
#ifdef WIN32_64
        if (boost::istarts_with(it->second->GetMQFileName(), fn))
#else
        if (boost::starts_with(it->second->GetMQFileName(), fn))
#endif
        {
            if (permanent) {
                it->second->StopQueue();
                res = ::remove(it->second->GetMQFileName().c_str());
            }
            m_mapQueue.erase(it);
            return (res == 0);
        }
    }
    return false;
}

