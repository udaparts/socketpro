#include "stdafx.h"

class CMySocketProServer : public CSocketProServer
{

protected:

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(amOwn);

        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ustreamfile");

        //create four chat groups or topics
        PushManager::AddAChatGroup(1, L"R&D Department");
        PushManager::AddAChatGroup(2, L"Sales Department");
        PushManager::AddAChatGroup(3, L"Management Department");

        return true; //true -- ok; false -- no listening server
    }

private:
    HINSTANCE m_h;
};

int main(int argc, char* argv[]) {
    CMySocketProServer MySocketProServer;
    if (!MySocketProServer.Run(20901)) {
        int errCode = MySocketProServer.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
