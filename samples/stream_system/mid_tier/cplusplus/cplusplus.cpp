#include "stdafx.h"
#include "ssserver.h"

int main(int argc, char* argv[]) {
    //set configuration settings
    CSpConfig &sc = SpManager::SetConfig(true, "sp_config.json");
    if (sc.GetErrMsg().size()) {
        cout << sc.GetErrMsg() << endl;
        cout << "Press any key to stop the server ......\n";
        ::getchar();
        return 1;
    }
    unsigned int svsId;
    try{
        CYourServer::Master = (CSQLMasterPool<true, CMysql>*)sc.GetPool("masterdb", &svsId);
        shared_ptr<CMysql> handler = CYourServer::Master->Seek();
        if (!handler) {
            cout << "No master DB available now\n";
        }
        CYourServer::Slave = (CSQLMasterPool<true, CMysql>::CSlavePool*)sc.GetPool("slavedb0", &svsId);
        handler = CYourServer::Slave->Seek();
        if (!handler) {
            cout << "No slave DB available now\n";
        }
        handler = CYourServer::Slave->SeekByQueue();
        if (!handler) {
            cout << "Slave DB queue not available. Some of requests will be not supported at server side!\n";
        }
    }

    catch(exception & err) {
        cout << err.what() << endl;
        cout << "Press any key to stop the server ......\n";
        ::getchar();
        return 1;
    }

    CYourServer::FrontCachedTables.push_back(u"sakila.actor");
    CYourServer::FrontCachedTables.push_back(u"sakila.country");
    CYourServer::FrontCachedTables.push_back(u"sakila.language");

    //Cache is ready for use now
    auto v0 = CYourServer::Master->Cache.GetDBTablePair();
    if (!v0.size())
        cout << "There is no table cached\n";
    else
        cout << "Table cached:\n";
    for (auto it = v0.begin(), end = v0.end(); it != end; ++it) {
        wcout << "DB name: " << Utilities::ToWide(it->first) << ", table name: " << Utilities::ToWide(it->second) << endl;
    }
    cout << endl;
    if (v0.size()) {
        wcout << "Keys with " << Utilities::ToWide(v0.front().first) << "." << Utilities::ToWide(v0.front().second) << ":\n";
        auto v1 = CYourServer::Master->Cache.FindKeys(v0.front().first.c_str(), v0.front().second.c_str());
        for (auto it = v1.begin(), end = v1.end(); it != end; ++it) {
            wcout << "Key ordinal: " << it->first << ", key column name: " << Utilities::ToWide(it->second.DisplayName) << endl;
        }
        cout << endl;
    }
    auto v2 = CYourServer::Master->Cache.GetColumMeta(u"sakila", u"actor");
    for (auto it = v2.begin(), end = v2.end(); it != end; ++it) {
        wcout << L"DB name: " << Utilities::ToWide(it->DBPath) << ", table name: " << Utilities::ToWide(it->TablePath) << ", column name: " << Utilities::ToWide(it->DisplayName) << endl;
    }
    cout << endl;

    auto v3 = CYourServer::Master->Cache.GetColumnCount(u"sakila", u"actor");
    if (v3) {
        cout << "sakila.actor cached with " << v3 << " columns and rows = " << CYourServer::Master->Cache.GetRowCount(u"sakila", u"actor") << "\n\n";
    }

    CTable table;
    int res = CYourServer::Master->Cache.Between(u"sakila", u"actor", 3, "2000-07-01", "2017-01-1", table);
    if (table.GetMeta().size()) {
        cout << "There are " << table.GetDataMatrix().size() << " records between 2000-07-01 and 2017-01-1\n\n";
    }
    CYourServer::CreateTestDB();

    CYourServer server(2);
#ifdef WIN32_64
    server.UseSSL("intermediate.pfx", "", "mypassword");
#else //non-windows platforms
    server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
#endif
    //start listening socket with standard TLSv1.x security
    if (!server.Run(20911))
        cout << "Error code: " << server.GetErrorCode() << endl;

    cout << "Press any key to stop the server ......\n";
    ::getchar();
    return 0;
}
