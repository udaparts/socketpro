

#include "stdafx.h"
#include "config.h"
#include "ssserver.h"

int main(int argc, char* argv[])
{
	CConfig::GetConfig(g_config);

	CSSServer server(g_config.m_main_threads);
	if (g_config.m_working_directory.size())
		CSSServer::QueueManager::SetWorkDirectory(g_config.m_working_directory.c_str());
	if (g_config.m_message_queue_password.size())
		CSSServer::QueueManager::SetMessageQueuePassword(g_config.m_message_queue_password.c_str());
	if (!server.Run(g_config.m_nPort, 32, !g_config.m_bNoIpV6))
		std::cout << "Error happens with code = " << server.GetErrorCode() << std::endl;
	std::cout << "Press any key to stop the server ......" << std::endl;
	::getchar();
	return 0;
}

