
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

    CYourServer server(g_config.m_main_threads);

    //set configuration settings for persistent message queues that used by this middle tier server and master/slave requests backup
    if (g_config.m_working_directory.size()) {
        SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(g_config.m_working_directory.c_str());
        CYourServer::QueueManager::SetWorkDirectory(g_config.m_working_directory.c_str());
    }
    if (g_config.m_message_queue_password.size()) {
        CYourServer::QueueManager::SetMessageQueuePassword(g_config.m_message_queue_password.c_str());
        SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(g_config.m_message_queue_password.c_str());
    }

    //start two socket pools, master and slave
    CYourServer::StartMySQLPools();
    //Cache is ready for use now

    auto v0 = CMySQLMasterPool::Cache.GetDBTablePair();
    auto v1 = CMySQLMasterPool::Cache.FindKeys(v0.front().first.c_str(), v0.front().second.c_str());
    auto v2 = CMySQLMasterPool::Cache.GetColumMeta(L"sakila", L"actor");
    auto v3 = CMySQLMasterPool::Cache.GetColumnCount(L"sakila", L"actor");
    auto v4 = CMySQLMasterPool::Cache.GetRowCount(L"sakila", L"actor");
    SPA::CTable table;
    int res = CMySQLMasterPool::Cache.Between(L"sakila", L"actor", 3, "2017-07-01", "2017-01-1", table);

    //test certificate and private key files are located at the directory ../socketpro/bin
#ifdef WIN32_64 //windows platforms
    if (g_config.m_store_or_pfx.rfind(".pfx") != std::string::npos) {
        server.UseSSL(g_config.m_store_or_pfx.c_str(), "", g_config.m_password_or_subject.c_str());
    } else {
        //or load cert and private key from windows system cert store
        server.UseSSL(g_config.m_store_or_pfx.c_str()/*"my"*/, g_config.m_password_or_subject.c_str(), "");
    }
#else //non-windows platforms
    server.UseSSL(g_config.m_cert.c_str(), g_config.m_key.c_str(), g_config.m_password_or_subject.c_str());
#endif

    //start listening socket with standard TLSv1.x security
    if (!server.Run(g_config.m_nPort, 32, !g_config.m_bNoIpV6))
        std::cout << "Error happens with code = " << server.GetErrorCode() << std::endl;

    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();

    //shut down slave and master socket pools
    CYourServer::Slave.reset();
    CYourServer::Master.reset();
    return 0;
}
