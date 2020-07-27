
#include "stdafx.h"
#include <iostream>

#include "../../../include/odbc/uodbc_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("sodbc");
        if (m_h) {
            PSetOdbcDBGlobalConnectionString SetOdbcDBGlobalConnectionString = (PSetOdbcDBGlobalConnectionString) GetProcAddress(m_h, "SetOdbcDBGlobalConnectionString");
#ifdef WIN32_64
            SetOdbcDBGlobalConnectionString(L"dsn=ToSqlServer64;uid=sa;pwd=Smash123");
#else
            SetOdbcDBGlobalConnectionString(L"dsn=ToMySQL;uid=root;pwd=Smash123");
#endif
        }
        return true;
    }

private:
    HINSTANCE m_h;
};

int main(int argc, char* argv[]) {
    const char* str = "1234.567";
    DECIMAL dec;
    SPA::ParseDec_long(str, dec);
    std::string sdec = SPA::ToString_long(dec);
    CMySocketProServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
}
