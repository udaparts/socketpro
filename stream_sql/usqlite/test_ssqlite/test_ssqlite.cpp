
#include "stdafx.h"
#include <iostream>

#include "../../../include/udatabase.h"
#include "../../../include/pexports.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ssqlite");
        if (m_h) {
            PSetSPluginGlobalOptions SetSPluginGlobalOptions = (PSetSPluginGlobalOptions) GetProcAddress(m_h, "SetSPluginGlobalOptions");
            SetSPluginGlobalOptions("{\"monitored_tables\":\"sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor\",\"global_connection_string\":\"usqlite.db\"}");
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
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press any key to stop the server ......\n";
    ::getchar();
}
