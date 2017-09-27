
#include "stdafx.h"

int main(int argc, char* argv[]) {

    CConnectionContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20902;
    cc.UserId = L"root";
    cc.Password = L"Smash123";


    return 0;
}

