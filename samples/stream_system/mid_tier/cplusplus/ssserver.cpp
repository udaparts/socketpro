
#include "stdafx.h"
#include "ssserver.h"
#include "config.h"

std::shared_ptr<CMySQLPool> CSSServer::Master;
std::shared_ptr<CMySQLPool> CSSServer::Slave;

void CSSServer::SetMySQLPools() {
    CSSServer::Master.reset(new CMySQLPool);
    bool ok = CSSServer::Master->StartSocketPool(g_config.m_Master, (unsigned int) g_config.m_nMasterSessions, 1); //one thread enough
    CMySQLPool::PHandler a0 = CSSServer::Master->GetAsyncHandlers()[0];


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