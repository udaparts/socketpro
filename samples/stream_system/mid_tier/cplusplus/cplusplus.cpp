

#include "stdafx.h"
#include "config.h"
#include "ssserver.h"

int main(int argc, char* argv[]) {
    //set configuration settings
    g_config.SetConfig();
    if (!g_config.m_vccSlave.size() || !g_config.m_nMasterSessions || !g_config.m_nSlaveSessions) { //check requirements
        std::cout << "Wrong settings for remote MySQL master and slave servers, and press any key to stop the server ......" << std::endl;
        ::getchar();
        return 1;
    }

    CSSServer server(g_config.m_main_threads);

    //set configuration settings for persistent message queues that used by this middle tier server and master/slave requests backup
    if (g_config.m_working_directory.size()) {
        CClientSocket::QueueConfigure::SetWorkDirectory(g_config.m_working_directory.c_str());
        CSSServer::QueueManager::SetWorkDirectory(g_config.m_working_directory.c_str());
    }
    if (g_config.m_message_queue_password.size()) {
        CSSServer::QueueManager::SetMessageQueuePassword(g_config.m_message_queue_password.c_str());
        CClientSocket::QueueConfigure::SetMessageQueuePassword(g_config.m_message_queue_password.c_str());
    }

    //start two socket pools, master and slave
    CSSServer::StartMySQLPools();

    auto v0 = CMySQLMasterPool::Cache.GetDBTablePair();
    auto v1 = CMySQLMasterPool::Cache.FindKeys(v0.front().first.c_str(), v0.front().second.c_str());
    auto v2 = CMySQLMasterPool::Cache.FindARow(L"sakila", L"actor", 42);
    auto v3 = CMySQLMasterPool::Cache.GetColumMeta(L"sakila", L"actor");
    auto v4 = CMySQLMasterPool::Cache.GetColumnCount(L"sakila", L"actor");
    auto v5 = CMySQLMasterPool::Cache.GetRowCount(L"sakila", L"actor");

    if (!server.Run(g_config.m_nPort, 32, !g_config.m_bNoIpV6))
        std::cout << "Error happens with code = " << server.GetErrorCode() << std::endl;

    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();

    //shut down slave and master socket pools
    CSSServer::Slave.reset();
    CSSServer::Master.reset();
    return 0;
}
