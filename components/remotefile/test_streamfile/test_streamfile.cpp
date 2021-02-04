#include "stdafx.h"
#include "../../../include/file/ufile_server.h"
#include "../../../include/pexports.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ustreamfile");
        if (m_h) {
#ifdef WIN32_64
            PSetSPluginGlobalOptions SetSPluginGlobalOptions = (PSetSPluginGlobalOptions) GetProcAddress(m_h, "SetSPluginGlobalOptions");
            SetSPluginGlobalOptions("{\"root_directory\":\"C:\\\\cyetest\\\\nodejsdemos\\\\file_test\"}");
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
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press any key to stop the server ......\n";
    ::getchar();
    return 0;
}
