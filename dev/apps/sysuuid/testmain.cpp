// SysUUID.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../pinc/getsysid.h"

int main(int argc, char* argv[]) {
	SPA::ServerSide::URegistration reg;
    std::string id = SPA::GetSysId();
    std::cout << "System Id = " << id << std::endl;
    std::string appName = SPA::GetAppName();
    std::cout << "AppName = " << appName << std::endl;
    bool ok = SPA::ServerSide::IsRegisterred("Mytest", reg);
    std::cout << "Press a key to end the application ......" << std::endl;
    int res = ::getchar();
    return 0;
}

