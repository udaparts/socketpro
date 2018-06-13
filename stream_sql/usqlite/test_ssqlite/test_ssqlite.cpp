
#include "stdafx.h"
#include <iostream>

#include "../../../include/sqlite/usqlite_server.h"
#include "../../../include/udatabase.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ssqlite");
        if (m_h) {
            PSetSqliteDBGlobalConnectionString SetSqliteDBGlobalConnectionString = (PSetSqliteDBGlobalConnectionString) GetProcAddress(m_h, "SetSqliteDBGlobalConnectionString");
            SetSqliteDBGlobalConnectionString(L"usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor");
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

