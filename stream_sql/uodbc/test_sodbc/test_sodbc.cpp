
#include "stdafx.h"
#include <iostream>

#include "../../../include/odbc/uodbc_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("sodbc");
        if (m_h) {
            DoODBCAuthentication = nullptr;
            PSetOdbcDBGlobalConnectionString SetOdbcDBGlobalConnectionString = (PSetOdbcDBGlobalConnectionString) GetProcAddress(m_h, "SetOdbcDBGlobalConnectionString");
#ifdef WIN32_64
            SetOdbcDBGlobalConnectionString(L"dsn=ToSqlServer64;uid=sa;pwd=Smash123");
#else
            SetOdbcDBGlobalConnectionString(L"dsn=ToMySQL;uid=root;pwd=Smash123");
#endif
            DoODBCAuthentication = (PDoODBCAuthentication) GetProcAddress(m_h, "DoODBCAuthentication");
        }
        return true;
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        if (DoODBCAuthentication) {
#ifdef WIN32_64
            return DoODBCAuthentication(h, userId, password, serviceId, nullptr, L"dsn=ToSqlServer64");
#else
            return DoODBCAuthentication(h, userId, password, serviceId, nullptr, L"dsn=ToMySQL");
#endif
        }
        return false;
    }

private:
    PDoODBCAuthentication DoODBCAuthentication;
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
