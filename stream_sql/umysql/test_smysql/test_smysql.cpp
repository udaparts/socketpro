#include "stdafx.h"
#include <iostream>
#include "../../../include/pexports.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        DoSPluginAuthentication = nullptr;
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("smysql");
        if (m_h) {
            DoSPluginAuthentication = (PDoSPluginAuthentication) GetProcAddress(m_h, "DoSPluginAuthentication");
        }
        return true;
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        if (DoSPluginAuthentication) {
            return DoSPluginAuthentication(h, userId, password, serviceId, L"host=localhost;port=3306;timeout=30");
        }
        return false;
    }

private:
    PDoSPluginAuthentication DoSPluginAuthentication;
    HINSTANCE m_h;
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
