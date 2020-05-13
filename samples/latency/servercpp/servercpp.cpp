#include <iostream>
#include "latencyserver.h"

int main(int argc, char* argv[])
{
	CLatencyServer latency_Server;
	if (!latency_Server.Run(20901)) {
		int errCode = latency_Server.GetErrorCode();
		std::cout << "Error happens with code = " << errCode << std::endl;
	}
	std::cout << "Press any key to stop the server ......" << std::endl;
	::getchar();
	return 0;
}
