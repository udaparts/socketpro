#include "stdafx.h"
#include <iostream>
#include "../../../include/async_sqlite.h"

using namespace SPA::UDB;
using namespace SPA::ClientSide;
using namespace std;

typedef future<CSqlite::SQLExeInfo> CSqlFuture;
typedef CSocketPool<CSqlite> CMyPool;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;

vector<CSqlFuture> TestCreateTables(shared_ptr<CSqlite> sqlite);
CSqlFuture InsertBLOBByPreparedStatement(shared_ptr<CSqlite> sqlite, CRowsetArray &ra);
CSqlFuture TestPreparedStatements(shared_ptr<CSqlite> sqlite, CRowsetArray &ra);
vector<CSqlFuture> TestBatch(shared_ptr<CSqlite> sqlite, CRowsetArray &ra);

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

int main(int argc, char* argv[]) {
    MakeLargeStrings();
    CConnectionContext cc;
    cout << "Remote host: " << endl;
    getline(cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spSqlite;
    //spSqlite.SetQueueName("sqlitequeue");
    if (!spSqlite.StartSocketPool(cc, 1)) {
        cout << "No connection to remote async sqlite server\n";
        cout << "Press any key to close the application ......\n";
        ::getchar();
        return 0;
    }
    auto sqlite = spSqlite.Seek();
    CRowsetArray rowset_array;
    try{
        //stream all SQL requests with in-line batching
        auto fopen = sqlite->open(u"");
        auto vF = TestCreateTables(sqlite);
        auto fbt = sqlite->beginTrans();
        auto fp0 = TestPreparedStatements(sqlite, rowset_array);
        auto fp1 = InsertBLOBByPreparedStatement(sqlite, rowset_array);
        auto fet = sqlite->endTrans();
        auto vFb = TestBatch(sqlite, rowset_array);

        cout << "All SQL requests are streamed,";
        cout << " and waiting for results ......\n";
        wcout << fopen.get().ToString() << endl;
        for (auto &e : vF) {
            wcout << e.get().ToString() << endl;
        }
        wcout << fbt.get().ToString() << endl;
        wcout << fp0.get().ToString() << endl;
        wcout << fp1.get().ToString() << endl;
        wcout << fet.get().ToString() << endl;
        for (auto& e : vFb) {
            wcout << e.get().ToString() << endl;
        }
    }

    catch(CServerError & ex) {
        wcout << ex.ToString() << "\n";
    }

    catch(CSocketError & ex) {
        wcout << ex.ToString() << "\n";
    }

    catch(exception & ex) {
        wcout << "Unexpected error: " << ex.what() << "\n";
    }
    //print out all received rowsets
    int index = 0;
    cout << "\n+++++ Start rowsets +++\n";
    for (auto &pair : rowset_array) {
        cout << "Statement index: " << index;
        if (pair.first.size()) {
            cout << ", rowset with columns: " << pair.first.size() << ", records: " << pair.second.size() / pair.first.size() << ".\n";
        } else {
            cout << ", no rowset received.\n";
        }
        ++index;
    }
    cout << "+++++ End rowsets +++\n\n";
    cout << "Press any key to close the demo ......\n";
    ::getchar();
    return 0;
}

vector<CSqlFuture> TestBatch(shared_ptr<CSqlite> sqlite, CRowsetArray &ra) {
    vector<CSqlFuture> vF;
    CDBVariantArray vParam;
    vParam.push_back(1); //ID
    vParam.push_back(2); //EMPLOYEEID
    //there is no manual transaction if isolation is tiUnspecified
    vF.push_back(sqlite->executeBatch(tiUnspecified, u"Select datetime('now');select * from COMPANY where ID=?;select * from EMPLOYEE where EMPLOYEEID=?",
        vParam, [&ra](CSqlite &handler, CDBVariantArray & vData) {
            //rowset data come here
            assert((vData.size() % handler.GetColumnInfo().size()) == 0);
            CDBVariantArray &va = ra.back().second;
            if (va.empty())
                va = move(vData);
            else
                move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite & handler) {
            //rowset header comes here
            auto &vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }));

    vParam.clear();
    vParam.push_back(1); //ID
    vParam.push_back(2); //EMPLOYEEID
    vParam.push_back(2); //ID
    vParam.push_back(3); //EMPLOYEEID
    //Same as sqlite->BeginTrans();
    //Select datetime('now');select * from COMPANY where ID=1;select * from COMPANY where ID=2;Select datetime('now');
    //select * from EMPLOYEE where EMPLOYEEID=2;select * from EMPLOYEE where EMPLOYEEID=3
    //ok = sqlite->EndTrans();
    vF.push_back(sqlite->executeBatch(tiReadUncommited, u"Select datetime('now');select * from COMPANY where ID=?;Select datetime('now');select * from EMPLOYEE where EMPLOYEEID=?",
        vParam, [&ra](CSqlite &handler, CDBVariantArray & vData) {
            //rowset data come here
            assert((vData.size() % handler.GetColumnInfo().size()) == 0);
            CDBVariantArray &va = ra.back().second;
            if (va.empty())
                va = move(vData);
            else
                move(vData.begin(), vData.end(), back_inserter(va));
        }, [&ra](CSqlite & handler) {
            //rowset header comes here
            auto &vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        }));
    return vF;
}

CSqlFuture TestPreparedStatements(shared_ptr<CSqlite> sqlite, CRowsetArray &ra) {
    sqlite->Prepare(u"Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)");

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

    return sqlite->execute(vData, [&ra](CSqlite &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    }, [&ra](CSqlite & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    });
}

vector<CSqlFuture> TestCreateTables(shared_ptr<CSqlite> sqlite) {
    vector<future < CSqlite::SQLExeInfo>> v;
    v.push_back(sqlite->execute(u"CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)"));
    const char16_t* ct = u"CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
    v.push_back(sqlite->execute(ct));
    return v;
}

CSqlFuture InsertBLOBByPreparedStatement(shared_ptr<CSqlite> sqlite, CRowsetArray &ra) {
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

    //execute multiple sets of parameter data in one short
    return sqlite->execute(vData, [&ra](CSqlite &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    }, [&ra](CSqlite & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    });
}
