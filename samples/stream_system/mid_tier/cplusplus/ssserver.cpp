#include "stdafx.h"
#include "ssserver.h"

CSQLMasterPool<true, CMysql>* CYourServer::Master = nullptr;
CSQLMasterPool<true, CMysql>::CSlavePool* CYourServer::Slave = nullptr;
vector<CDBString> CYourServer::FrontCachedTables;

CYourServer::CYourServer(int nParam) : CSocketProServer(nParam) {
}

bool CYourServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << endl;
    return true; //true -- ok; false -- no permission
}

bool CYourServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);

    SetChatGroups();

    if (!AddServices()) {
        cout << "Unable to register a service" << endl;
        return false;
    }
    return true; //true -- ok; false -- no listening server
}

bool CYourServer::AddServices() {
    bool ok = m_SSPeer.AddMe(sidStreamSystem);
    if (!ok)
        return false;
    ok = m_SSPeer.AddSlowRequest(UDB::idGetCachedTables);

    //tell this service that all results could be returned randomly (not in order)
    m_SSPeer.SetReturnRandom(true);
    return true;
}

void CYourServer::SetChatGroups() {
    bool ok = PushManager::AddAChatGroup(UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");
    ok = PushManager::AddAChatGroup(UDB::CACHE_UPDATE_CHAT_GROUP_ID, L"Cache update notification from middle tier to front");
}

void CYourServer::CreateTestDB() {
    auto handler = Master->Seek();
    if (handler) {
        CDBString sql = u"CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
        handler->Execute(sql.c_str());
    }
}
