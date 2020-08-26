#include "stdafx.h"
#include "HWImpl.h"

class CMySocketProServer : public CSocketProServer {

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(amOwn);

        //add service(s) into SocketPro server
        AddService();

        //create four chat groups
        PushManager::AddAChatGroup(1, L"R&D Department");
        PushManager::AddAChatGroup(2, L"Sales Department");
        PushManager::AddAChatGroup(3, L"Management Department");
        PushManager::AddAChatGroup(7, L"HR Department");

        return true; //true -- ok; false -- no listening server
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t* password, unsigned int serviceId) {
        std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;

        //true -- session permitted; and false -- connection denied and closed
        return true;
    }

private:
    //One server supports any number of services
    CSocketProService<HelloWorldPeer> m_HelloWorld;

private:
    void AddService() {
        //No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        bool ok = m_HelloWorld.AddMe(sidHelloWorld, taNone);
        ok = m_HelloWorld.AddSlowRequest(idSleep);
    }
};

int main(int argc, char* argv[]) {
    CMySocketProServer Server;

    //test certificate and private key files are located at ../SocketProRoot/bin
#ifdef WIN32_64 //windows platforms
    Server.UseSSL("intermediate.pfx", "", "mypassword");
    //or load cert and private key from windows system cert store
    //MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "");
#else //non-windows platforms
    Server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
#endif

    if (!Server.Run(20901)) {
        int errCode = Server.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
