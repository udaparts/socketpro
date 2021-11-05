
#include "stdafx.h"
#include <iostream>
#include "../../../include/pexports.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t* password, unsigned int serviceId) {
        assert(DoSPluginAuthentication);
        //A cheap but quick way to do authentication against a database server by use of ODBC plugin
        int res = DoSPluginAuthentication(h, userId, password, serviceId, L"dsn=ToMySQL;database=sakila;maxsqlsbatched=8");
        switch (res) {
            case SP_PLUGIN_AUTH_OK:
                return true;
            case SP_PLUGIN_AUTH_FAILED:
                std::cout << "Authentication failed\n";
                return false;
            case SP_PLUGIN_AUTH_INTERNAL_ERROR:
                std::cout << "Internal error!\n";
                return false;
            case SP_PLUGIN_AUTH_NOT_IMPLEMENTED:
                std::cout << "Authentication not implemented yet\n";
                break;
            default:
                assert(false);
                break;
        }
        return false;
    }

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        DoSPluginAuthentication = nullptr;
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("sodbc");
        if (m_h) {
            DoSPluginAuthentication = (PDoSPluginAuthentication) GetProcAddress(m_h, "DoSPluginAuthentication");
        }
        return true;
    }

private:
    HINSTANCE m_h;
    PDoSPluginAuthentication DoSPluginAuthentication;
};

int main(int argc, char* argv[]) {
    CMySocketProServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press any key to stop the server ......\n";
    ::getchar();
}
