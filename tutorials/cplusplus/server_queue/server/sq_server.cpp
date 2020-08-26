#include "stdafx.h"

class CMySocketProServer : public CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(amOwn);

        //load socketpro async queue server libraries located at the directory ../socketpro/bin
        HINSTANCE h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("uasyncqueue", 16 * 1024); //16 * 1024 batch dequeuing size in bytes
        return (h != nullptr); //true -- ok; false -- no listening server
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
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
