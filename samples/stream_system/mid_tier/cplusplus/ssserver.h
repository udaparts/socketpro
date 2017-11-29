#pragma once

#include "sspeer.h"

class CYourServer : public CSocketProServer {
public:
    CYourServer(int nParam);

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);

private:
    bool AddServices();
    void SetChatGroups();

public:
    static void CreateTestDB();
    static void StartMySQLPools();
    static std::shared_ptr<CMySQLMasterPool> Master;
    static std::shared_ptr<CMySQLSlavePool> Slave;

private:
    CSocketProService<CYourPeerOne> m_SSPeer;

private:
    CYourServer(const CYourServer &s);
    CYourServer& operator=(const CYourServer &s);
};

