#ifndef _UMB_SERVICE_CONTEXT_H__
#define _UMB_SERVICE_CONTEXT_H__

#include "../core_shared/shared/includes.h"
#include "../include/userver.h"
#include <boost/random.hpp>
#include <boost/atomic.hpp>

class CServerSession;

class CServiceContext : private boost::noncopyable {
public:
    CServiceContext(unsigned int nServiceId, const CSvsContext &sc);
    virtual ~CServiceContext();

public:
    bool IsSlowRequest(unsigned short sReqId);
    size_t GetCountOfSlowRequests();
    void AddSlowRequest(unsigned short sReqId);
    void RemoveSlowRequest(unsigned short sReqId);
    void GetAllSlowRequestId(std::vector<unsigned short> &vRequestId);
    unsigned int GetSvsID();
    const CSvsContext& GetSvsContext();
    void RemoveAllSlowRequests();
    bool GetRandom();
    void SetRandom(bool random);
    bool IsRoutee(CServerSession *ss);

    void AddRoutee(CServerSession *ss);
    void RemoveRoutee(CServerSession *ss);
    unsigned int GetRoutingSvsId();
    void SetRoutingSvsId(unsigned int routingSvsId);
    SPA::UINT64 GetBestRoutee(unsigned int &routeeSize);
    size_t GetRouteeSize();
    void NotifyRouteeChanged(unsigned int count);
    void AddAlpha(unsigned short reqId);
    unsigned int GetAllAlpaRequestIds(unsigned short *ids, unsigned int count);
    bool IsAlpah(unsigned short reqId);
    void SetRoutingAlgorithm(SPA::ServerSide::tagRoutingAlgorithm ra);
    SPA::ServerSide::tagRoutingAlgorithm GetRoutingAlgorithm();

private:
    std::vector<unsigned short>::iterator Seek(unsigned short sReqId);
    SPA::UINT64 GetBestRouteeByDefault(unsigned int &routeeSize);
    SPA::UINT64 GetBestRouteeByRandom(unsigned int &routeeSize);
    SPA::UINT64 GetBestRouteeByAverage(unsigned int &routeeSize);
    SPA::UINT64 GetBestRouteeByFirst(unsigned int &routeeSize);

private:
	std::mutex m_mutex;
    unsigned int m_nServiceId;
    std::vector<unsigned short> m_vSlowRequestId;
    CSvsContext m_sc;
    bool m_bRandom;
    unsigned int m_nRoutingSvsId;
    boost::random::mt19937 m_gen;
    std::vector<CServerSession*> m_vRoutee; //protected by m_mutex
    std::vector<unsigned short> m_vAlphaId;
    SPA::ServerSide::tagRoutingAlgorithm m_ra;

public:
    bool m_bRegisterred;
};

#endif

