
#include "stdafx.h"
#include <iostream>

#include "../../../../include/odbc/uodbc_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("sodbc", 1234);
        if (m_h) {
            PSetOdbcDBGlobalConnectionString SetOdbcDBGlobalConnectionString = (PSetOdbcDBGlobalConnectionString) GetProcAddress(m_h, "SetOdbcDBGlobalConnectionString");
            SetOdbcDBGlobalConnectionString(L"dsn=ToMySQL;timeout=10;async=1");
        }
        return true;
    }

private:
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
