
#include "stdafx.h"
#include "ssserver.h"
#include "config.h"

std::shared_ptr<CMySQLMasterPool> CYourServer::Master;
std::shared_ptr<CMySQLSlavePool> CYourServer::Slave;

void CYourServer::StartMySQLPools() {
    assert(g_config.m_vccSlave.size());
    assert(g_config.m_nSlaveSessions);
    assert((g_config.m_nSlaveSessions % g_config.m_vccSlave.size()) == 0);

    CYourServer::Master.reset(new CMySQLMasterPool(g_config.m_master_default_db.c_str()));

    //These case-sensitivities depends on your DB running platform and sensitivity settings.
    //All of them are false or case-insensitive by default
    CYourServer::Master->Cache.SetFieldNameCaseSensitive(false);
    CYourServer::Master->Cache.SetTableNameCaseSensitive(false);
    CYourServer::Master->Cache.SetDBNameCaseSensitive(false);

    if (g_config.m_master_queue_name.size())
        CYourServer::Master->SetQueueName(g_config.m_master_queue_name.c_str());
    //start master pool for cache and update accessing
    bool ok = CYourServer::Master->StartSocketPool(g_config.m_ccMaster, (unsigned int) g_config.m_nMasterSessions, 1); //one thread enough

    //compute threads and sockets_per_thread
    unsigned int threads = (unsigned int) (g_config.m_nSlaveSessions / g_config.m_vccSlave.size());
    unsigned int sockets_per_thread = (unsigned int) g_config.m_vccSlave.size();
    CYourServer::Slave.reset(new CMySQLSlavePool(g_config.m_slave_default_db.c_str()));

    typedef SPA::ClientSide::CConnectionContext* PCConnectionContext;

    //prepare connection contexts for slave pool
    PCConnectionContext *ppCCs = new PCConnectionContext[threads];
    for (unsigned int t = 0; t < threads; ++t) {
        SPA::ClientSide::CConnectionContext *pcc = new SPA::ClientSide::CConnectionContext[sockets_per_thread];
        ppCCs[t] = pcc;
        for (unsigned int s = 0; s < sockets_per_thread; ++s) {
            pcc[s] = g_config.m_vccSlave[s];
        }
    }

    if (g_config.m_slave_queue_name.size())
        CYourServer::Slave->SetQueueName(g_config.m_slave_queue_name.c_str());
    //start slave pool for query accessing
    ok = CYourServer::Slave->StartSocketPool(ppCCs, threads, sockets_per_thread);

    //wait until all data of cached tables are brought from backend database server to this middle server application cache
    ok = CYourServer::Master->GetAsyncHandlers()[0]->WaitAll();

    for (unsigned int t = 0; t < threads; ++t) {
        SPA::ClientSide::CConnectionContext *pcc = ppCCs[t];
        delete[]pcc;
    }
    delete []ppCCs;
}

CYourServer::CYourServer(int nParam) : CSocketProServer(nParam) {

}

bool CYourServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
    return true; //true -- ok; false -- no permission
}

bool CYourServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(amOwn);

    SetChatGroups();

    if (!AddServices()) {
        std::cout << "Unable to register a service" << std::endl;
        return false;
    }
    return true; //true -- ok; false -- no listening server
}

bool CYourServer::AddServices() {
    bool ok = m_SSPeer.AddMe(sidStreamSystem);
    if (!ok)
        return false;
    ok = m_SSPeer.AddSlowRequest(SPA::UDB::idGetCachedTables);

    //tell this service that all results could be returned randomly (not in order)
    m_SSPeer.SetReturnRandom(true);
    return true;
}

void CYourServer::SetChatGroups() {
    bool ok = PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");
    ok = PushManager::AddAChatGroup(SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, L"Cache update notification from middle tier to front");
}

void CYourServer::CreateTestDB() {
    bool ok;
#if defined(_UMYSQL_SOCKETPRO_H_)
    std::wstring sql = L"CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila";
    auto handler = Master->Seek();
    if (handler) {
        ok = handler->Execute(sql.c_str());
        sql = L"INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
        ok = handler->Execute(sql.c_str());
    }
#else
    std::wstring sql = L"CREATE TABLE mysample.COMPANY(ID INT8 PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE mysample.EMPLOYEE(EMPLOYEEID INTEGER PRIMARY KEY AUTOINCREMENT,CompanyId INT8 not null,Name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
    auto v = Master->GetAsyncHandlers();
    for (auto it = v.begin(), end = v.end(); it != end; ++it) {
        ok = (*it)->Execute(L"ATTACH DATABASE 'mysample.db' as mysample", nullptr);
        if (it == v.begin()) {
            ok = (*it)->Execute(sql.c_str());
            sql = L"INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.');INSERT INTO mysample.COMPANY(ID,Name)VALUES(2,'Microsoft Inc.');INSERT INTO mysample.COMPANY(ID,Name)VALUES(3,'Amazon Inc.')";
            ok = (*it)->Execute(sql.c_str());
        }
    }
#endif
}
