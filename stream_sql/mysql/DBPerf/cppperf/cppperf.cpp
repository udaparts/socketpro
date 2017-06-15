

#include "stdafx.h"
#include <iostream>
#include "../../../../include/mysql/umysql.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMyHandler;
typedef SPA::ClientSide::CSocketPool<CMyHandler> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20902;
    cc.UserId = L"root";
    cc.Password = L"Smash123";
	std::string s;
	std::cout << "Database name: " << std::endl;
	std::getline(std::cin, s);
    std::wstring dbName = SPA::Utilities::ToWide(s.c_str());
    std::cout << "Table name: " << std::endl;
    std::getline(std::cin, s);
    std::wstring tableName = SPA::Utilities::ToWide(s.c_str());
    std::cout << "sql filter: " << std::endl;
    std::getline(std::cin, s);
    std::wstring filter = SPA::Utilities::ToWide(s.c_str());
    std::cout << "Asynchronous execution (0) or synchronous execution (1) ?" << std::endl;
    std::getline(std::cin, s);
    int sync = std::atoi(s.c_str());

    CMyPool spMysql;
    bool ok = spMysql.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote async mysql server" << std::endl;
        std::cout << "Press any key to close the application ......" << std::endl;
        ::getchar();
        return 0;
    }

    unsigned int obtained = 0;
    CRowsetArray ra;

    std::cout << "Computing ......" << std::endl;
    auto pMysql = spMysql.Seek();

#ifndef WIN32_64
    //This may help performance on non-windows platforms if strings are ASCII ones!
    pMysql->Utf8ToW(false);
#endif

    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //ok = pMysql->GetAttachedClientSocket()->GetClientQueue().StartQueue("mysql_queue", 24 * 3600, false); //time-to-live 1 day and true for encryption

    CMyHandler::DResult dr = [](CMyHandler &handler, int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "res = " << res;
            std::wcout << L", errMsg: " << errMsg << std::endl;
        }
    };

    CMyHandler::DExecuteResult er = [&obtained, &ra](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        if (res) {
            std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
            std::wcout << errMsg;
            std::cout << std::endl;
        }
        ra.clear();
        ++obtained;
    };


    CMyHandler::DRows r = [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    ok = pMysql->Open(dbName.c_str(), dr);
    ok = pMysql->WaitAll();
    obtained = 0;
    std::wstring sql = L"select * from " + tableName;
    if (filter.size() > 0) {
        sql += L" where " + filter;
    }
    unsigned int count = 10000;
#ifdef WIN32_64
    DWORD dwStart = ::GetTickCount();
#else
    system_clock::time_point start = system_clock::now();
#endif
    for (unsigned int n = 0; n < count; ++n) {
        ok = pMysql->Execute(sql.c_str(), er, r, rh);
        if (sync && ok)
            ok = pMysql->WaitAll();
        if (!ok)
            break;
    }
    if (!sync && ok)
        ok = pMysql->WaitAll();
#ifdef WIN32_64
    DWORD diff = ::GetTickCount() - dwStart;
#else
    system_clock::time_point stop = system_clock::now();
    ms d = std::chrono::duration_cast<ms>(stop - start);
    unsigned int diff = d.count();
#endif
    std::cout << "Time required = " << diff << " millseconds for " << obtained << " requests" << std::endl;
    std::cout << "Press any key to close the application ......" << std::endl;
    ::getchar();
    return 0;
}
