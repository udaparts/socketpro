#include "stdafx.h"
#include "piImpl.h"

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
