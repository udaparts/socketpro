#include "stdafx.h"
#include "ssserver.h"

int main(int argc, char* argv[]) {
    //set configuration settings
    CSpConfig &sc = SpManager::SetConfig(true, "sp_config.json");
    if (sc.GetErrMsg().size()) {
        std::cout << sc.GetErrMsg() << std::endl;
        std::cout << "Press any key to stop the server ......" << std::endl;
        ::getchar();
        return 1;
    }
    unsigned int svsId;
    try{
        CYourServer::Master = (CSQLMasterPool<true, CMysql>*)sc.GetPool("masterdb", &svsId);
        std::shared_ptr<CMysql> handler = CYourServer::Master->Seek();
        if (!handler) {
            std::cout << "No master DB available now" << std::endl;
        }
        CYourServer::Slave = (CSQLMasterPool<true, CMysql>::CSlavePool*)sc.GetPool("slavedb0", &svsId);
        handler = CYourServer::Slave->Seek();
        if (!handler) {
            std::cout << "No slave DB available now" << std::endl;
        }
	handler = CYourServer::Slave->SeekByQueue();
        if (!handler) {
            std::cout << "Slave DB queue not available. Some of requests will be not supported at server side!" << std::endl;
        }
    }

    catch(std::exception & err) {
        std::cout << err.what() << std::endl;
        std::cout << "Press any key to stop the server ......" << std::endl;
        ::getchar();
        return 1;
    }

#ifndef NATIVE_UTF16_SUPPORTED
    CYourServer::FrontCachedTables.push_back(L"sakila.actor");
    CYourServer::FrontCachedTables.push_back(L"sakila.country");
    CYourServer::FrontCachedTables.push_back(L"sakila.language");
#else
    CYourServer::FrontCachedTables.push_back(u"sakila.actor");
    CYourServer::FrontCachedTables.push_back(u"sakila.country");
    CYourServer::FrontCachedTables.push_back(u"sakila.language");
#endif

    //Cache is ready for use now
    auto v0 = CYourServer::Master->Cache.GetDBTablePair();
    if (!v0.size())
        std::cout << "There is no table cached" << std::endl;
    else
        std::cout << "Table cached:" << std::endl;
    for (auto it = v0.begin(), end = v0.end(); it != end; ++it) {
#ifndef NATIVE_UTF16_SUPPORTED
        std::wcout << "DB name = " << it->first.c_str() << ", table name = " << it->second.c_str() << std::endl;
#else
        std::wcout << "DB name = " << SPA::Utilities::ToWide(it->first) << ", table name = " << SPA::Utilities::ToWide(it->second) << std::endl;
#endif
    }
    std::cout << std::endl;
    if (v0.size()) {
#ifndef NATIVE_UTF16_SUPPORTED
        std::wcout << "Keys with " << v0.front().first << "." << v0.front().second << ":" << std::endl;
#else
        std::wcout << "Keys with " << SPA::Utilities::ToWide(v0.front().first) << "." << SPA::Utilities::ToWide(v0.front().second) << ":" << std::endl;
#endif
        auto v1 = CYourServer::Master->Cache.FindKeys(v0.front().first.c_str(), v0.front().second.c_str());
        for (auto it = v1.begin(), end = v1.end(); it != end; ++it) {
#ifndef NATIVE_UTF16_SUPPORTED
            std::wcout << L"Key ordinal = " << it->first << ", key column name = " << it->second.DisplayName << std::endl;
#else
            std::wcout << L"Key ordinal = " << it->first << ", key column name = " << SPA::Utilities::ToWide(it->second.DisplayName) << std::endl;
#endif
        }
        std::cout << std::endl;
    }
    auto v2 = CYourServer::Master->Cache.GetColumMeta(L"sakila", L"actor");
    for (auto it = v2.begin(), end = v2.end(); it != end; ++it) {
#ifndef NATIVE_UTF16_SUPPORTED
        std::wcout << L"DB name = " << it->DBPath << ", table name = " << it->TablePath << ", column name: " << it->DisplayName << std::endl;
#else
        std::wcout << L"DB name = " << SPA::Utilities::ToWide(it->DBPath) << ", table name = " << SPA::Utilities::ToWide(it->TablePath) << ", column name: " << SPA::Utilities::ToWide(it->DisplayName) << std::endl;
#endif
    }
    std::cout << std::endl;

    auto v3 = CYourServer::Master->Cache.GetColumnCount(L"sakila", L"actor");
    if (v3) {
        std::cout << "sakila.actor cached with " << v3 << " columns and rows = " << CYourServer::Master->Cache.GetRowCount(L"sakila", L"actor") << std::endl;
        std::cout << std::endl;
    }

    SPA::CTable table;
    int res = CYourServer::Master->Cache.Between(L"sakila", L"actor", 3, "2000-07-01", "2017-01-1", table);
    if (table.GetMeta().size()) {
        std::cout << "There are " << table.GetDataMatrix().size() << " records between 2000-07-01 and 2017-01-1" << std::endl;
        std::cout << std::endl;
    }
    CYourServer::CreateTestDB();

    CYourServer server(2);
#ifdef WIN32_64
    //or load cert and private key from windows system cert store
    //server.UseSSL("C:\\cyetest\\socketpro\\bin\\intermediate.pfx", "", "mypassword");
#else //non-windows platforms
    //server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
#endif
    //start listening socket with standard TLSv1.x security
    if (!server.Run(20911))
        std::cout << "Error happens with code = " << server.GetErrorCode() << std::endl;

    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
    return 0;
}
