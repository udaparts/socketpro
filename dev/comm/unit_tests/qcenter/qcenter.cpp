#include "stdafx.h"
#include "qhwImpl.h"

CMessageQueue HWReceiverPeer::m_mq(0);

int main(int argc, char* argv[])
{
	CMySocketProServer	MySocketProServer;
	if (!MySocketProServer.Run(20901)) {
		int errCode = MySocketProServer.GetErrorCode();
		std::cout<< "Error happens with code = " << errCode << std::endl;
	}
	HWReceiverPeer::m_mq = CMySocketProServer::QueueManager::StartQueue("hwQueue");
	if (HWReceiverPeer::m_mq.IsAvailable()) {
		std::cout << "The queue for 'Hello World' is created successfully with queued requests = " 
			<< HWReceiverPeer::m_mq.GetMessageCount() 
			<< std::endl;
	}
	else
		std::cout << "The queue for 'Hello World' is not started!" << std::endl;
	std::cout << "Press any key to stop the server ......" << std::endl;
	::getchar();
	return 0;
}
