
#include "stdafx.h"
#include "ssserver.h"
#include "config.h"

std::shared_ptr<CMySQLMasterPool> CSSServer::Master;
std::shared_ptr<CMySQLSlavePool> CSSServer::Slave;

void CSSServer::StartMySQLPools() {
    assert(g_config.m_vccSlave.size());
    assert(g_config.m_nSlaveSessions);
    assert((g_config.m_nSlaveSessions % g_config.m_vccSlave.size()) == 0);

    CSSServer::Master.reset(new CMySQLMasterPool);

    //start master pool for cache and update accessing
    bool ok = CSSServer::Master->StartSocketPool(g_config.m_ccMaster, (unsigned int) g_config.m_nMasterSessions, 1); //one thread enough

    //get threads and sockets_per_thread
    unsigned int threads = (unsigned int) (g_config.m_nSlaveSessions / g_config.m_vccSlave.size());
    unsigned int sockets_per_thread = (unsigned int) g_config.m_vccSlave.size();
    CSSServer::Slave.reset(new CMySQLSlavePool);

    typedef CConnectionContext* PCConnectionContext;
    //prepare connection contexts for slave pool
    PCConnectionContext *ppCCs = new PCConnectionContext[threads];
    for (unsigned int t = 0; t < threads; ++t) {
        CConnectionContext *pcc = new CConnectionContext[sockets_per_thread];
        ppCCs[t] = pcc;
        for (unsigned int s = 0; s < sockets_per_thread; ++s) {
            pcc[s] = g_config.m_vccSlave[s];
        }
    }
    //start slave pool for query accessing
    ok = CSSServer::Slave->StartSocketPool(ppCCs, threads, sockets_per_thread);

    //wait until all data of cached tables are brought from backend database server to this middle server application cache
    ok = CSSServer::Master->GetAsyncHandlers()[0]->WaitAll();

    for (unsigned int t = 0; t < threads; ++t) {
        CConnectionContext *pcc = ppCCs[t];
        delete[]pcc;
    }
    delete []ppCCs;
}

CSSServer::CSSServer(int nParam) : CSocketProServer(nParam) {

}

CSSServer::~CSSServer() {

}

bool CSSServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
    return true; //true -- ok; false -- no permission
}

bool CSSServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(amOwn);

    SetOnlineMessage();

    if (!AddServices()) {
        std::cout << "Unable to register a service" << std::endl;
        return false;
    }
    return true; //true -- ok; false -- no listening server
}

bool CSSServer::AddServices() {
    bool ok = m_SSPeer.AddMe(sidStreamSystem);
    if (!ok)
        return false;
    ok = m_SSPeer.AddSlowRequest(idSubscribeAndGetInitialCachedTablesData);
    ok = m_SSPeer.AddSlowRequest(idSetDefaultDatabaseName);
    ok = m_SSPeer.AddSlowRequest(idEndBatchProcessing);


    ok = m_SSPeer.AddSlowRequest(idQueryMaxMinAvgs);

    return true;
}

void CSSServer::SetOnlineMessage() {

}