// ws_cplusplus.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "httppeer.h"

class CMySocketProServer : public CSocketProServer {
protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);

        //add service(s) into SocketPro server
        AddServices();

        //create four chat groups or topics
        PushManager::AddAChatGroup(1, L"R&D Department");
        PushManager::AddAChatGroup(2, L"Sales Department");
        PushManager::AddAChatGroup(3, L"Management Department");
        PushManager::AddAChatGroup(7, L"HR Department");

        return true; //true -- ok; false -- no listening server
    }

private:
    SPA::ServerSide::CSocketProService<CHttpPeer> m_myHttp;

private:
    void AddServices() {
        //HTTP and WebSocket services
        bool ok = m_myHttp.AddMe((unsigned int)SPA::tagServiceID::sidHTTP);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idGet);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idPost);
        ok = m_myHttp.AddSlowRequest((unsigned short)tagHttpRequestID::idUserRequest);
    }
};

int main(int argc, char* argv[])
{
	CMySocketProServer MySocketProServer;
    if (!MySocketProServer.Run(20901)) {
        int errCode = MySocketProServer.GetErrorCode();
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press any key to stop the server ......\n";
    ::getchar();
	return 0;
}
