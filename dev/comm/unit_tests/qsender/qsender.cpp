
#include "stdafx.h"

#include "hwsender.h"

int main(int argc, char* argv[])
{
	int data;
	unsigned int count;
	unsigned int index = 0;
	SPA::ClientSide::CConnectionContext cc;
    std::cout << "Remote host ip address ?" << std::endl;
    std::cin >> cc.Host;
    std::cout << "Remote host port ?" << std::endl;
    std::cin >> cc.Port;

#ifdef WIN32_64
    cc.UserId = L"Win_SocketPro";
#else
    cc.UserId = L"Nix_SocketPro";
#endif
    cc.Password = L"MyPassword";

	SPA::ClientSide::CSocketPool<HWSender> poolShw;

	bool ok = poolShw.StartSocketPool(cc, 1, 1);
	auto p = poolShw.Lock();
    if (!p) {
        std::cout << "No socket connection for queue" << std::endl;
        return -1;
    }

	do
	{
		count = p->SayHelloWord(L"UDAParts", index);
		std::cout << "Requests in queue " << count << std::endl;
		std::cout << "Input a non-zero number and ENTER to keep on equeing ......" <<std::endl;
		std::cin >> data;
		++index;
	}while(data != 0);

	std::cout << "Press a number and ENTER to stop the sender ......"<<std::endl;
	std::cin >> data;
	return 0;
}

