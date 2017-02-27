
#include "stdafx.h"
#include <iostream>

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

public:
    CMySocketProServer(int nParam = 0) : SPA::ServerSide::CSocketProServer(nParam) {
    }

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes
        return true;
    }

private:
    HINSTANCE m_h;
};

int main(int argc, char* argv[]) {

#ifdef WIN32_64
    CMySocketProServer::QueueManager::SetWorkDirectory("c:\\sp_test");
#else

#endif
    CMySocketProServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}

