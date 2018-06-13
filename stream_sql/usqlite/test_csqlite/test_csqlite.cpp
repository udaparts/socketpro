
#include "stdafx.h"
#include <iostream>
#include "../../../include/async_sqlite.h"
using namespace SPA::UDB;

typedef SPA::ClientSide::CSqliteBase CMyHandler;
typedef SPA::ClientSide::CSocketPool<SPA::ClientSide::CSqlite> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

void TestCreateTables(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite);
void InsertBLOBByPreparedStatement(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite, CRowsetArray &ra);
void TestPreparedStatements(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite, CRowsetArray &ra);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spSqlite;
    bool ok = spSqlite.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote async sqlite server" << std::endl;
        std::cout << "Press any key to close the application ......" << std::endl;
        ::getchar();
        return 0;
    }
    auto pSqlite = spSqlite.Seek();

    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //ok = pSqlite->GetAttachedClientSocket()->GetClientQueue().StartQueue("sqlite", 24 * 3600, false); //time-to-live 1 day and true for encryption

    ok = pSqlite->Open(nullptr, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
        std::cout << "res = " << res;
        std::wcout << L", errMsg: " << errMsg << std::endl;
    });
    TestCreateTables(pSqlite);
    CRowsetArray rowset_array;
    ok = pSqlite->BeginTrans();
    TestPreparedStatements(pSqlite, rowset_array);
    InsertBLOBByPreparedStatement(pSqlite, rowset_array);
    ok = pSqlite->EndTrans();
    ok = pSqlite->WaitAll();

    //print out all received rowsets
    int index = 0;
    std::cout << std::endl;
    std::cout << "+++++ Start rowsets +++" << std::endl;
    for (auto it = rowset_array.begin(), end = rowset_array.end(); it != end; ++it) {
        std::cout << "Statement index = " << index;
        if (it->first.size()) {
            std::cout << ", rowset with columns = " << it->first.size() << ", records = " << it->second.size() / it->first.size() << "." << std::endl;
        } else {
            std::cout << ", no rowset received." << std::endl;
        }
        ++index;
    }
    std::cout << "+++++ End rowsets +++" << std::endl;
    std::cout << std::endl;
    std::cout << "Press any key to close the application ......" << std::endl;
    ::getchar();
    return 0;
}

void TestPreparedStatements(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite, CRowsetArray &ra) {
    static const wchar_t *sql_insert_parameter = L"Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";

    bool ok = pSqlite->Prepare(sql_insert_parameter, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
        std::cout << "res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });

    CDBVariantArray vData;
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000000000.0);

    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600000000.0);

    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000000000.0);

    ok = pSqlite->Execute(vData, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg;
        if (!res) {
            std::cout << ", last insert id = ";
                    std::cout << vtId.llVal;
        }
        std::cout << std::endl;
    }, [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
                    row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    }, [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
                column_rowset_pair.first = vColInfo;
                ra.push_back(column_rowset_pair);
    });
}

void TestCreateTables(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite) {
    const wchar_t *create_table = L"CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income float not null)";
    bool ok = pSqlite->Execute(create_table, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });

    create_table = L"CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique, CompanyId INT8 not null, name NCHAR(64) NOT NULL, JoinDate DATETIME not null default(datetime('now')), IMAGE BLOB, DESCRIPTION NTEXT, Salary real, FOREIGN KEY(CompanyId) REFERENCES COMPANY(id))";
    ok = pSqlite->Execute(create_table, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });
}

void InsertBLOBByPreparedStatement(std::shared_ptr<SPA::ClientSide::CSqlite> pSqlite, CRowsetArray &ra) {
    std::wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    std::string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t *sqlInsert = L"insert or replace into employee(EMPLOYEEID, CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?, ?);select * from employee where employeeid = ?";

    bool ok = pSqlite->Prepare(sqlInsert, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
        std::cout << "res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
    vData.push_back(1);
    vData.push_back(1); //google company id
    vData.push_back(L"Ted Cruz");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(wstr.c_str());
    vData.push_back(254000.0);
    vData.push_back(1);

    //second set of data
    vData.push_back(2);
    vData.push_back(1); //google company id
    vData.push_back("Donald Trump");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob->SetSize(0);
    sbBlob << str;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(str.c_str());
    vData.push_back(20254000.0);
    vData.push_back(2);

    //third set of data
    vData.push_back(3);
    vData.push_back(2); //Microsoft company id
    vData.push_back("Hillary Clinton");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(wstr.c_str());
    vData.push_back(6254000.0);
    vData.push_back(3);

    //execute multiple sets of parameter data in one short
    ok = pSqlite->Execute(vData, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg;
        if (!res) {
            std::cout << ", last insert id = ";
                    std::cout << vtId.llVal;
        }
        std::cout << std::endl;
    }, [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            CDBVariant &d = vData[n];
                    row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    }, [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
                column_rowset_pair.first = vColInfo;
                ra.push_back(column_rowset_pair);
    });
}
