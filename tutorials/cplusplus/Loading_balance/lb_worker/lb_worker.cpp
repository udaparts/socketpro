// hw_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "pi.h"

int main(int argc, char* argv[]) {
	CConnectionContext cc;
	std::cout << "This is worker. Input router server ip address ......" << std::endl;
	std::getline(std::cin, cc.Host);

	cc.Port = 20901;
	cc.UserId = L"MyUserId";
	cc.Password = L"MyPassword";

	typedef CSocketPool<Pi, CClientSocket> CMyPool;
	CMyPool spPi;
	bool ok = spPi.StartSocketPool(cc, 1);
	std::cout << "Press a key to shutdown the demo application ......" << std::endl;
	::getchar();
	return 0;
}
