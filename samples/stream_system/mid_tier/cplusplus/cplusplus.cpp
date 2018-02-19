
#include "stdafx.h"
#include "config.h"
#include "ssserver.h"

int main(int argc, char* argv[]) {
    //get configuration settings
    g_config.GetConfig();
    if (!g_config.m_vccSlave.size() || !g_config.m_nMasterSessions || !g_config.m_slave_threads || !g_config.m_slave_threads) { //check requirements
        std::cout << "Wrong settings for remote MySQL master and slave servers, and press any key to stop the server ......" << std::endl;
        ::getchar();
        return 1;
    }

    CYourServer server(g_config.m_main_threads);

    //start two socket pools, master and slave
    CYourServer::StartMySQLPools();

    //Cache is ready for use now
    auto v0 = server.Master->Cache.GetDBTablePair();
    if (!v0.size())
        std::cout << "There is no table cached" << std::endl;
    else
        std::cout << "Table cached:" << std::endl;
    for (auto it = v0.begin(), end = v0.end(); it != end; ++it) {
        std::wcout << L"DB name = " << it->first << ", table name = " << it->second << std::endl;
    }
    std::cout << std::endl;
    if (v0.size()) {
        std::wcout << "Keys with " << v0.front().first << "." << v0.front().second << ":" << std::endl;
        auto v1 = server.Master->Cache.FindKeys(v0.front().first.c_str(), v0.front().second.c_str());
        for (auto it = v1.begin(), end = v1.end(); it != end; ++it) {
            std::wcout << L"Key ordinal = " << it->first << ", key column name = " << it->second.DisplayName << std::endl;
        }
        std::cout << std::endl;
    }
#if defined(_UMYSQL_SOCKETPRO_H_)
    auto v2 = server.Master->Cache.GetColumMeta(L"sakila", L"actor");
    for (auto it = v2.begin(), end = v2.end(); it != end; ++it) {
        std::wcout << L"DB name = " << it->DBPath << ", table name = " << it->TablePath << ", column name: " << it->DisplayName << std::endl;
    }
    std::cout << std::endl;

    auto v3 = server.Master->Cache.GetColumnCount(L"sakila", L"actor");
    if (v3) {
        std::cout << "sakila.actor cached with " << v3 << " columns and rows = " << server.Master->Cache.GetRowCount(L"sakila", L"actor") << std::endl;
        std::cout << std::endl;
    }

    SPA::CTable table;
    int res = server.Master->Cache.Between(L"sakila", L"actor", 3, "2000-07-01", "2017-01-1", table);
    if (table.GetMeta().size()) {
        std::cout << "There are " << table.GetDataMatrix().size() << " records between 2000-07-01 and 2017-01-1" << std::endl;
        std::cout << std::endl;
    }
#else
    auto v2 = server.Master->Cache.GetColumMeta(L"main", L"actor");
    for (auto it = v2.begin(), end = v2.end(); it != end; ++it) {
        std::wcout << L"DB name = " << it->DBPath << ", table name = " << it->TablePath << ", column name: " << it->DisplayName << std::endl;
    }
    std::cout << std::endl;

    auto v3 = server.Master->Cache.GetColumnCount(L"main", L"actor");
    if (v3) {
        std::cout << "main.actor cached with " << v3 << " columns and rows = " << server.Master->Cache.GetRowCount(L"main", L"actor") << std::endl;
        std::cout << std::endl;
    }

    SPA::CTable table;
    int res = server.Master->Cache.Between(L"maIn", L"actor", 3, "2000-07-01", "2017-01-1", table);
    if (table.GetMeta().size()) {
        std::cout << "There are " << table.GetDataMatrix().size() << " records between 2000-07-01 and 2017-01-1" << std::endl;
        std::cout << std::endl;
    }
#endif

    CYourServer::CreateTestDB();

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
