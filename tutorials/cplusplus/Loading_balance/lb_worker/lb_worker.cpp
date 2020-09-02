#include "stdafx.h"
#include "pi.h"
#include <thread>

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    std::cout << "This is worker. Input router server ip address ......" << std::endl;
    std::getline(std::cin, cc.Host);

    cc.Port = 20901;
    cc.UserId = L"My_LB_UserId";
    cc.Password = L"My_LB_Password";

    typedef CSocketPool<Pi, CClientSocket> CMyPool;
    CMyPool spPi;
    bool ok = spPi.StartSocketPool(cc, 1, std::thread::hardware_concurrency());
    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
