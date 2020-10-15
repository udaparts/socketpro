#include <iostream>
#include "latencyserver.h"

int main(int argc, char* argv[]) {
    CLatencyServer latency_Server;
    if (!latency_Server.Run(20901)) {
        int errCode = latency_Server.GetErrorCode();
        std::cout << "Error code: " << errCode << "\n";
    }
    std::cout << "Press any key to stop the server ......\n";
    ::getchar();
    return 0;
}
