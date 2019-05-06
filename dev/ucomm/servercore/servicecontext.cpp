
#include "../include/definebase.h"
#if defined(OLD_IMPL)
#include "session.h"
#elif defined(WIN32_64)
#include "../uservercore_win/session.h"
#else
#include "../ServerCoreUnix/session.h"
#endif

#include "servicecontext.h"

std::mutex CServiceContext::m_mutex;

CServiceContext::CServiceContext(unsigned int nServiceId, const CSvsContext &sc)
: m_nServiceId(nServiceId), m_sc(sc), m_bRandom(false), m_nRoutingSvsId(0), m_ra(SPA::ServerSide::raDefault), m_bRegisterred(false) {

}

CServiceContext::~CServiceContext() {

}

const CSvsContext& CServiceContext::GetSvsContext() {
    //CAutoLock al(m_mutex);
    return m_sc;
}

bool CServiceContext::GetRandom() {
    return m_bRandom;
}

void CServiceContext::SetRandom(bool random) {
    m_bRandom = random;
}

void CServiceContext::SetRoutingAlgorithm(SPA::ServerSide::tagRoutingAlgorithm ra) {
    m_ra = ra;
}

SPA::ServerSide::tagRoutingAlgorithm CServiceContext::GetRoutingAlgorithm() {
    return m_ra;
}

std::vector<unsigned short>::iterator CServiceContext::Seek(unsigned short sReqId) {
    std::vector<unsigned short>::iterator it;
    std::vector<unsigned short>::iterator end = m_vSlowRequestId.end();
    for (it = m_vSlowRequestId.begin(); it != end; ++it) {
        if (*it == sReqId)
            return it;
    }
    return it;
}

bool CServiceContext::IsSlowRequest(unsigned short sReqId) {
    CAutoLock al(m_mutex);
    return (Seek(sReqId) != m_vSlowRequestId.end());
}

size_t CServiceContext::GetCountOfSlowRequests() {
    CAutoLock al(m_mutex);
    return m_vSlowRequestId.size();
}

void CServiceContext::AddSlowRequest(unsigned short sReqId) {
    CAutoLock al(m_mutex);
    if (Seek(sReqId) == m_vSlowRequestId.end())
        m_vSlowRequestId.push_back(sReqId);
}

unsigned int CServiceContext::GetSvsID() {
    return m_nServiceId;
}

void CServiceContext::RemoveSlowRequest(unsigned short sReqId) {
    CAutoLock al(m_mutex);
    std::vector<unsigned short>::iterator it = Seek(sReqId);
    if (it != m_vSlowRequestId.end())
        m_vSlowRequestId.erase(it);
}

void CServiceContext::GetAllSlowRequestId(std::vector<unsigned short> &vRequestId) {
    CAutoLock al(m_mutex);
    vRequestId = m_vSlowRequestId;
}

void CServiceContext::AddAlpha(unsigned short reqId) {
    CAutoLock al(m_mutex);
    if (std::find(m_vAlphaId.begin(), m_vAlphaId.end(), reqId) == m_vAlphaId.end())
        m_vAlphaId.push_back(reqId);
}

bool CServiceContext::IsAlpah(unsigned short reqId) {
    //CAutoLock al(m_mutex);
    return (std::find(m_vAlphaId.begin(), m_vAlphaId.end(), reqId) != m_vAlphaId.end());
}

unsigned int CServiceContext::GetAllAlpaRequestIds(unsigned short *ids, unsigned int count) {
    if (!ids)
        count = 0;
    unsigned int min = (m_vAlphaId.size() > count) ? count : (unsigned int) m_vAlphaId.size();
    if (min) {
        ::memcpy(ids, m_vAlphaId.data(), min * sizeof (unsigned short));
    }
    return min;
}

void CServiceContext::RemoveAllSlowRequests() {
    CAutoLock al(m_mutex);
    m_vSlowRequestId.clear();
}

void CServiceContext::AddRoutee(CServerSession* ss) {
    if (!ss)
        return;
    CAutoLock al(m_mutex);
    m_vRoutee.push_back(ss);
    std::vector<CServerSession*>::iterator it, end = m_vRoutee.end();
    for (it = m_vRoutee.begin(); it != end; ++it) {
        (*it)->ResetRoutingRequestCount();
    }
}

void CServiceContext::RemoveRoutee(CServerSession *ss) {
    if (!ss)
        return;
    CAutoLock al(m_mutex);
    std::vector<CServerSession*>::iterator it = std::find(m_vRoutee.begin(), m_vRoutee.end(), ss);
    if (it != m_vRoutee.end())
        m_vRoutee.erase(it);
}

bool CServiceContext::IsRoutee(CServerSession *ss) {
    if (!ss)
        return false;
    CAutoLock al(m_mutex);
    std::vector<CServerSession*>::iterator it = std::find(m_vRoutee.begin(), m_vRoutee.end(), ss);
    return (it != m_vRoutee.end());
}

SPA::UINT64 CServiceContext::GetBestRouteeByDefault(unsigned int &routeeSize) {
    routeeSize = 0;
    CServerSession *routee = nullptr;
    SPA::UINT64 temp = 0;
    SPA::UINT64 routeeTime = 0;
    {
        CAutoLock al(m_mutex);
        size_t count = m_vRoutee.size();
        if (count == 0)
            return 0;
        boost::random::uniform_int_distribution<> dist(0, (int) (count - 1));
        size_t pos = dist(m_gen);
        routee = m_vRoutee[pos];
        routeeSize = routee->GetWritingBufferSizeAndSendTime(routeeTime);
        std::vector<CServerSession*>::iterator it, end = m_vRoutee.end();
        for (it = m_vRoutee.begin(); it != end; ++it) {
            CServerSession *ss = *it;
            if (ss == routee)
                continue;
            unsigned int size = ss->GetWritingBufferSizeAndSendTime(temp);
            if (routeeSize == size) {
                if (routeeTime > temp) {
                    //select an earlier routee
                    routeeTime = temp;
                    routee = ss;
                } else if (routeeTime == temp && ss->m_ccb.m_ulSent < routee->m_ccb.m_ulSent) {
                    routee = ss;
                }
            } else if (routeeSize > size) {
                routeeSize = size;
                routeeTime = temp;
                routee = ss;
            }
        }
        if (!routee)
            return 0;
    }
    return routee->MakeHandler();
}

SPA::UINT64 CServiceContext::GetBestRouteeByRandom(unsigned int &routeeSize) {
    routeeSize = 0;
    SPA::UINT64 routeeTime = 0;
    CServerSession *routee = nullptr;
    {
        CAutoLock al(m_mutex);
        size_t count = m_vRoutee.size();
        if (count == 0)
            return 0;
        boost::random::uniform_int_distribution<> dist(0, (int) (count - 1));
        size_t pos = dist(m_gen);
        routee = m_vRoutee[pos];
        routeeSize = routee->GetWritingBufferSizeAndSendTime(routeeTime);
    }
    return routee->MakeHandler();
}

SPA::UINT64 CServiceContext::GetBestRouteeByAverage(unsigned int &routeeSize) {
    SPA::UINT64 rcount;
    CServerSession *routee = nullptr;
    SPA::UINT64 temp = 0;
    {
        CAutoLock al(m_mutex);
        std::vector<CServerSession*>::iterator it, end = m_vRoutee.end();
        for (it = m_vRoutee.begin(); it != end; ++it) {
            CServerSession *ss = *it;
            if (!routee) {
                routee = ss;
                rcount = ss->GetRoutingRequestCount();
                continue;
            }
            SPA::UINT64 size = ss->GetRoutingRequestCount();
            if (rcount > size) {
                rcount = size;
                routee = ss;
            }
        }
        if (!routee) {
            routeeSize = 0;
            return 0;
        }
        routeeSize = routee->GetWritingBufferSizeAndSendTime(temp);
    }
    return routee->MakeHandler();
}

SPA::UINT64 CServiceContext::GetBestRouteeByFirst(unsigned int &routeeSize) {
    routeeSize = 0;
    CServerSession *routee = nullptr;
    SPA::UINT64 temp = 0;
    {
        CAutoLock al(m_mutex);
        std::vector<CServerSession*>::iterator it, end = m_vRoutee.end();
        for (it = m_vRoutee.begin(); it != end; ++it) {
            CServerSession *ss = *it;
            if (!routee) {
                routee = ss;
                routeeSize = ss->GetWritingBufferSizeAndSendTime(temp);
                continue;
            }
            unsigned int size = ss->GetWritingBufferSizeAndSendTime(temp);
            if (routeeSize > size) {
                routeeSize = size;
                routee = ss;
            }
        }
        if (!routee)
            return 0;
    }
    return routee->MakeHandler();
}

SPA::UINT64 CServiceContext::GetBestRoutee(unsigned int &routeeSize) {
    switch (m_ra) {
        case SPA::ServerSide::raDefault:
            return GetBestRouteeByDefault(routeeSize);
            break;
        case SPA::ServerSide::raRandom:
            return GetBestRouteeByRandom(routeeSize);
            break;
        case SPA::ServerSide::raAverage:
            return GetBestRouteeByAverage(routeeSize);
            break;
            /*
                case SPA::ServerSide::raFirst:
                    return GetBestRouteeByFirst(routeeSize);
                    break;
             */
        default:
            break;
    }
    assert(false);
    return 0;
}

unsigned int CServiceContext::GetRoutingSvsId() {
    //CAutoLock al(m_mutex);
    return m_nRoutingSvsId;
}

void CServiceContext::SetRoutingSvsId(unsigned int routingSvsId) {
    CAutoLock al(m_mutex);
    m_nRoutingSvsId = routingSvsId;
}

size_t CServiceContext::GetRouteeSize() {
    CAutoLock al(m_mutex);
    return m_vRoutee.size();
}

void CServiceContext::NotifyRouteeChanged(unsigned int count) {
    SPA::CScopeUQueue su;
    SPA::CUQueue &q = *su;
    SPA::CStreamHeader sh;
    sh.Size = sizeof (count);
    sh.RequestId = SPA::idRouteeChanged;
    q << sh << count;
    CAutoLock al(m_mutex);
    std::vector<CServerSession*>::iterator it, end = m_vRoutee.end();
    for (it = m_vRoutee.begin(); it != end; ++it) {
        CServerSession *session = *it;
        session->m_mutex.lock();
        if (session->m_cs >= csConnected) {
            session->Write(q.GetBuffer(), q.GetSize());
        }
        session->m_mutex.unlock();
    }
}