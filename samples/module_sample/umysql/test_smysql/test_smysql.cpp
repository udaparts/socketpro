
#include "stdafx.h"
#include <iostream>

#include "../../../../include/mysql/umysql_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("smysql"/*, SPA::ServerSide::Mysql::DISABLE_REMOTE_MYSQL/*SPA::ServerSide::Mysql::DISABLE_EMBEDDED_MYSQL*/);
        if (m_h) {
            PSetMysqlDBGlobalConnectionString SetMysqlDBGlobalConnectionString = (PSetMysqlDBGlobalConnectionString) GetProcAddress(m_h, "SetMysqlDBGlobalConnectionString");
            PSetMysqlEmbeddedOptions SetMysqlEmbeddedOptions = (PSetMysqlEmbeddedOptions) GetProcAddress(m_h, "SetMysqlEmbeddedOptions");

            SetMysqlDBGlobalConnectionString(L"host=localhost;uid=root;database=sys;pwd=Smash123;port=3306;timeout=20", true);

            //make sure sys sub directory exists for embedded mysql
            SetMysqlDBGlobalConnectionString(L"sys", false);

            std::cout << "Embedded settings:" << std::endl << std::endl;
            std::cout << SetMysqlEmbeddedOptions(L"") << std::endl << std::endl;
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