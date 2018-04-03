
#include "stdafx.h"
#include "SQueueImpl.h"

int main(int argc, char* argv[]) {
    CMySocketProServer MySocketProServer;

    //Optionally, set a work directory where server queue files will be created
    //CSocketProServer::QueueManager::SetWorkDirectory("sp_test");

    //pre-open a queue file, which may take long time if the existing queue file is very large
    CServerQueue sq = CSocketProServer::QueueManager::StartQueue("qperf", 24 * 3600);

    int param = 1;
    param <<= 24; //disable auto enqueue notification

    param += 32 * 1024; //32 * 1024 batch dequeuing size in bytes

    //load socketpro async queue server library located at the directory ../socketpro/bin
    HINSTANCE h = CSocketProServer::DllManager::AddALibrary("uasyncqueue", param);
    if (!h)
        std::cout << "Cannot load async persistent queue library" << std::endl;
    else if (!MySocketProServer.Run(20901)) {
        int errCode = MySocketProServer.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
