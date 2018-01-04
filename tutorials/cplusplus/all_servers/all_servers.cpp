
#include "stdafx.h"
#include "../../../include/aserverw.h"
#include "../pub_sub/server/HWImpl.h"
#include "../Loading_balance/pi_i.h"
#include "../webdemo/httppeer.h"
#include "../../../include/sqlite/usqlite_server.h"

class CMySocketProServer : public CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(amOwn);

        //add service(s) into SocketPro server
        AddServices();

        //create four chat groups or topics
        bool ok = PushManager::AddAChatGroup(1, L"R&D Department");
        ok = PushManager::AddAChatGroup(2, L"Sales Department");
        ok = PushManager::AddAChatGroup(3, L"Management Department");
        ok = PushManager::AddAChatGroup(7, L"HR Department");

        //load socketpro async sqlite and queue server libraries located at the directory ../socketpro/bin
        auto h = CSocketProServer::DllManager::AddALibrary("ssqlite", SPA::ServerSide::Sqlite::ENABLE_GLOBAL_SQLITE_UPDATE_HOOK);
        if (h) {
            PSetSqliteDBGlobalConnectionString SetSqliteDBGlobalConnectionString = (PSetSqliteDBGlobalConnectionString) GetProcAddress(h, "SetSqliteDBGlobalConnectionString");
            SetSqliteDBGlobalConnectionString(L"usqlite.db");
        }
        //SocketPro asynchronous persistent message queue
        h = CSocketProServer::DllManager::AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes

        //exchange files by streaming files
        h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ustreamfile");

        return true; //true -- ok; false -- no listening server
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
        return true;
    }

    virtual void OnClose(USocket_Server_Handle h, int errCode) {
        CBaseService *bs = CBaseService::SeekService(h);
        if (bs != nullptr) {
            CSocketPeer *sp = bs->Seek(h);
            // ......
        }
    }

private:
    CSocketProService<HelloWorldPeer> m_HelloWorld;

    //SocketPro load balancing demo
    CSocketProService<SPA::ServerSide::CDummyPeer> m_Pi;
    CSocketProService<SPA::ServerSide::CDummyPeer> m_PiWorker;

    SPA::ServerSide::CSocketProService<CHttpPeer> m_myHttp;

private:
    void AddServices() {
        //load balancing
        bool ok = m_Pi.AddMe(sidPi);
        ok = m_PiWorker.AddMe(sidPiWorker);

        //Hello World
        ok = m_HelloWorld.AddMe(sidHelloWorld);
        ok = m_HelloWorld.AddSlowRequest(idSleepHelloWorld);

        //HTTP and WebSocket services
        //Copy all files inside directories ../socketpro/bin/js and ../socketpro/tutorials/webtests into the directory where the application is located
        ok = m_myHttp.AddMe(SPA::sidHTTP);
        ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idGet);
        ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idPost);
        ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idUserRequest);
    }
};

int main(int argc, char* argv[]) {
    CMySocketProServer MySocketProServer;
    //CSocketProServer::QueueManager::SetMessageQueuePassword("MyPasswordForMsgQueue");
#ifdef WIN32_64
    CSocketProServer::QueueManager::SetWorkDirectory("c:\\sp_test\\");
#else
    CSocketProServer::QueueManager::SetWorkDirectory("/home/yye/sp_test/");
#endif
    if (!MySocketProServer.Run(20901)) {
        int errCode = MySocketProServer.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    } else {
        CSocketProServer::Router::SetRouting(sidPi, sidPiWorker);
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
