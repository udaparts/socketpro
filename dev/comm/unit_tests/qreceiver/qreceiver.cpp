

#include "stdafx.h"
#include "hwreceiver.h"

int main(int argc, char* argv[])
{
	int data;
	SPA::UINT64 res;
	const unsigned int BATCH_COUNT = 20;
	unsigned int messages;
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

	SPA::ClientSide::CSocketPool<HWReceiver> poolConsumer;

	bool ok = poolConsumer.StartSocketPool(cc, 1, 1);
	auto p = poolConsumer.Lock();
    if (!p) {
        std::cout << "No socket connection for dequeue" << std::endl;
        return -1;
    }
	
	do
	{
		res = p->DoDequeue(BATCH_COUNT, true);
		messages = (res & 0xFFFFFFFF);
		unsigned int bytes = (unsigned int)(res >> 32);
		std::cout << "Messages "<< messages << " with bytes = " << bytes << " dequed successfully." << std::endl;
	}while(messages >= BATCH_COUNT);

	std::cout << "Press a number and ENTER to stop the sender ......"<<std::endl;
	std::cin >> data;
	return 0;
}

