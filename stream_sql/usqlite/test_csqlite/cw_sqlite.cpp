#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
static_assert(false, "No co_await support");
#endif
#include <iostream>
#include <deque>
#include "../../../include/async_sqlite.h"

using namespace SPA::UDB;
using namespace SPA::ClientSide;
using namespace std;

typedef CSocketPool<CSqlite> CMyPool;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;
using PSqlite = shared_ptr<CSqlite>;

wstring g_wstr;
string g_str;

void MakeLargeStrings() {
    while (g_wstr.size() < 128 * 1024) {
        g_wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }
    while (g_str.size() < 256 * 1024) {
        g_str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }
}

CSqlite::SqlWaiter TestPreparedStatements(PSqlite& sqlite, CRowsetArray& ra) {
    sqlite->Prepare(L"Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)");

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

    return sqlite->wait_execute(vData, [&ra](CSqlite& handler, CDBVariantArray& vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite& handler) {
            //rowset header comes here
            auto& vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }
    );
}

CSqlite::SqlWaiter TestBLOBByPreparedStatement(PSqlite& sqlite, CRowsetArray& ra) {
    sqlite->Prepare(u"insert or replace into employee(EMPLOYEEID,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?);select * from employee where employeeid=?");

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
    vData.push_back(1);
    vData.push_back(1); //google company id
    vData.push_back(u"Ted Cruz");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob << g_wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(g_wstr.c_str()); //large unicode string
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
    sbBlob << g_str;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(g_str.c_str()); //large ASCII string
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
    sbBlob << g_wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(g_wstr.c_str()); //large unicode string
    vData.push_back(6254000.0);
    vData.push_back(3);

    //execute multiple sets of parameter data in one call
    return sqlite->wait_execute(vData, [&ra](CSqlite& handler, CDBVariantArray& vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite& handler) {
            //rowset header comes here
            auto& vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }
    );
}

deque<CSqlite::SqlWaiter> TestBatch(PSqlite& sqlite, CRowsetArray& ra) {
    CDBVariantArray vParam;
    vParam.push_back(1); //ID
    vParam.push_back(2); //EMPLOYEEID
    //there is no manual transaction if isolation is tiUnspecified
    CSqlite::SqlWaiter w0 = sqlite->wait_executeBatch(tiUnspecified, u"Select datetime('now');select * from COMPANY where ID=?;select * from EMPLOYEE where EMPLOYEEID=?",
        vParam, [&ra](CSqlite& handler, CDBVariantArray& vData) {
            //rowset data come here
            assert((vData.size() % handler.GetColumnInfo().size()) == 0);
            CDBVariantArray& va = ra.back().second;
            if (va.empty())
                va = move(vData);
            else
                move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite& handler) {
            //rowset header comes here
            auto& vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }
    );

    vParam.clear();
    vParam.push_back(1); //ID
    vParam.push_back(2); //EMPLOYEEID
    vParam.push_back(2); //ID
    vParam.push_back(3); //EMPLOYEEID

    //Same as sqlite->BeginTrans();
    //Select datetime('now');
    //prepare for select * from COMPANY where ID=?
    //select * from COMPANY where ID=1;
    //select * from COMPANY where ID=2;
    //Select datetime('now');
    //prepare for select * from EMPLOYEE where EMPLOYEEID=?
    //select * from EMPLOYEE where EMPLOYEEID=2;
    //select * from EMPLOYEE where EMPLOYEEID=3
    //ok = sqlite->EndTrans();
    CSqlite::SqlWaiter w1 = sqlite->wait_executeBatch(tiReadCommited, u"Select datetime('now');select * from COMPANY where ID=?;Select datetime('now');select * from EMPLOYEE where EMPLOYEEID=?",
        vParam, [&ra](CSqlite& handler, CDBVariantArray& vData) {
            //rowset data come here
            assert((vData.size() % handler.GetColumnInfo().size()) == 0);
            CDBVariantArray& va = ra.back().second;
            if (va.empty())
                va = move(vData);
            else
                move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite& handler) {
            //rowset header comes here
            auto& vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }
    );
    return { w0, w1 };
}

deque<CSqlite::SqlWaiter> TestCreateTables(PSqlite& sqlite) {
    const char16_t* ct0 = u"CREATE TABLE IF NOT EXISTS COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)";
    const char16_t* ct1 = u"CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
    return { sqlite->wait_execute(ct0), sqlite->wait_execute(ct1) };
}

CAwTask MyTest(PSqlite& sqlite, CRowsetArray& rowset_array) {
    try {
        //stream all DB and SQL requests with in-line batching
        wcout << (co_await sqlite->wait_open(u"")).ToString() << "\n";
        auto vC = TestCreateTables(sqlite);
        auto wbt = sqlite->wait_beginTrans();
        auto wps = TestPreparedStatements(sqlite, rowset_array);
        auto wbs = TestBLOBByPreparedStatement(sqlite, rowset_array);
        auto wet = sqlite->wait_endTrans();
        auto vB = TestBatch(sqlite, rowset_array);

        //co_await all results streamed from SocketPro server
        while (vC.size()) {
            wcout << (co_await vC.front()).ToString() << "\n";
            vC.pop_front();
        }
        wcout << (co_await wbt).ToString() << "\n";
        wcout << (co_await wps).ToString() << "\n";
        wcout << (co_await wbs).ToString() << "\n";
        wcout << (co_await wet).ToString() << "\n";

        while (vB.size()) {
            wcout << (co_await vB.front()).ToString() << "\n";
            vB.pop_front();
        }
    }

    catch (CServerError& ex) {
        wcout << ex.ToString() << "\n";
    }

    catch (CSocketError& ex) {
        wcout << ex.ToString() << "\n";
    }

    catch (exception& ex) {
        wcout << "Unexpected error: " << ex.what() << "\n";
    }
    //print out all received rowsets
    int index = 0;
    cout << "\n+++++ Start rowsets +++\n";
    for (auto& pair : rowset_array) {
        cout << "Statement index: " << index;
        if (pair.first.size()) {
            cout << ", rowset with columns: " << pair.first.size() << ", records: " << pair.second.size() / pair.first.size() << ".\n";
        }
        else {
            cout << ", no rowset received.\n";
        }
        ++index;
    }
    cout << "+++++ End rowsets +++\n\n";
}

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -pthread

int main(int argc, char* argv[]) {
    MakeLargeStrings();
    CConnectionContext cc;
    cout << "Remote host: " << endl;
    getline(cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CRowsetArray rowset_array;
    CMyPool spSqlite;
    //spSqlite.SetQueueName("qsqlite");
    if (!spSqlite.StartSocketPool(cc, 1)) {
        cout << "No connection to remote async sqlite server\n";
    }
    else {
        auto sqlite = spSqlite.Seek();
        MyTest(sqlite, rowset_array);
    }
    cout << "Press any key to close the demo ......\n";
    ::getchar();
    return 0;
}
