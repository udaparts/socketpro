#include "stdafx.h"
#include "../../../include/aserverw.h"
#include "../pub_sub/server/HWImpl.h"
#include "../Loading_balance/pi_i.h"
#include "../webdemo/httppeer.h"
#include "../../../include/udatabase.h"
#include "../../../include/pexports.h"

class CMyServer : public CSocketProServer {
protected:

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t* password, unsigned int serviceId) {
        std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << "\n";
        return true;
    }

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);

        //add service(s) into SocketPro server
        AddServices();

        bool ok = PushManager::AddAChatGroup(1, L"R&D Department");
        ok = PushManager::AddAChatGroup(2, L"Sales Department");
        ok = PushManager::AddAChatGroup(3, L"Management Department");
        ok = PushManager::AddAChatGroup(7, L"HR Department");
        ok = PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");

        //load socketpro async sqlite library located at the directory ../bin/free_services/sqlite
        auto h = DllManager::AddALibrary("ssqlite");
        if (h) {
            PSetSPluginGlobalOptions SetSPluginGlobalOptions = (PSetSPluginGlobalOptions)GetProcAddress(h, "SetSPluginGlobalOptions");
            //monitoring sakila.db table events (DELETE, INSERT and UPDATE) for tables actor, language, category and country
            SetSPluginGlobalOptions("{\"monitored_tables\":\"sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country\",\"global_connection_string\":\"usqlite.db\"}");
        }
        //load socketPro asynchronous persistent message queue library at the directory ../bin/free_services/queue
        h = DllManager::AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes

        //load socketPro file streaming library at the directory ../bin/free_services/file
        h = DllManager::AddALibrary("ustreamfile");

        //load MySQL/MariaDB socketPro server plugin library at the directory ../bin/free_services/mm_middle
        h = DllManager::AddALibrary("smysql");

        //load ODBC socketPro server plugin library at the directory ../bin/win or ../bin/linux
        h = DllManager::AddALibrary("sodbc");

        return true; //true -- ok; false -- no listening server
    }

private:
    CSocketProService<HelloWorldPeer> m_HelloWorld;

    //SocketPro server routing demo
    CSocketProService<CDummyPeer> m_Pi;
    CSocketProService<CDummyPeer> m_PiWorker;

    CSocketProService<CHttpPeer> m_myHttp;

private:

    void AddServices() {
        bool ok = m_Pi.AddMe(sidPi);
        ok = m_PiWorker.AddMe(sidPiWorker);
        ok = Router::SetRouting(sidPi, sidPiWorker);

        //Hello World
        ok = m_HelloWorld.AddMe(sidHelloWorld);
        ok = m_HelloWorld.AddSlowRequest(idSleep);

        //HTTP and WebSocket services
        //Copy uloader.js & uwebsocket.js at ../socketpro/bin/js and earthcity.jpg & ws0.htm ../socketpro/tutorials/webtests into the directory where the application is located
        ok = m_myHttp.AddMe((unsigned int)SPA::tagServiceID::sidHTTP);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idGet);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idPost);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idUserRequest);
    }
};

int main(int argc, char* argv[]) {
    CMyServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press a key to stop the server ......\n";
    ::getchar();
    return 0;
}
