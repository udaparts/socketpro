
#include "stdafx.h"
#include <iostream>

#include "../../../include/mysql/umysql_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        DoMySQLAuthentication = nullptr;
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("smysql");
        if (m_h) {
            PSetMysqlDBGlobalConnectionString SetMysqlDBGlobalConnectionString = (PSetMysqlDBGlobalConnectionString) GetProcAddress(m_h, "SetMysqlDBGlobalConnectionString");
            SetMysqlDBGlobalConnectionString(L"host=localhost;uid=root;database=sakila;pwd=Smash123;port=3306;timeout=20", true);
            DoMySQLAuthentication = (PDoMySQLAuthentication) GetProcAddress(m_h, "DoMySQLAuthentication");
        }
        return true;
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        if (DoMySQLAuthentication) {
            return DoMySQLAuthentication(h, userId, password, serviceId, L"host=localhost;port=3306;timeout=30");
        }
        return false;
    }

private:
    PDoMySQLAuthentication DoMySQLAuthentication;
    HINSTANCE m_h;
};

int main(int argc, char* argv[]) {
    CMySocketProServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
}