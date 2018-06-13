
#include "stdafx.h"
#include "../../../include/file/ufile_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ustreamfile");
        if (m_h) {
#ifdef WIN32_64
            PSetRootDirectory SetRootDirectory = (PSetRootDirectory) GetProcAddress(m_h, "SetRootDirectory");
            SetRootDirectory(L"C:\\udaparts\\boost_1_60_0\\stage\\lib64");
#endif
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
    return 0;
}

