#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
static_assert(false, "No co_await support");
#endif
#include <iostream>
#include <deque>
#include "../../../include/async_mysql.h"

using namespace SPA::ClientSide;
using namespace SPA::UDB;
using namespace std;

typedef CSocketPool<CMysql> CMyPool;
typedef CConnectionContext CMyConnContext;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;
typedef shared_ptr<CMysql> PMySQL;
using Aw = CMysql::SqlWaiter;

deque<Aw> TestCreateTables(PMySQL pMysql);
deque<Aw> TestPreparedStatements(PMySQL pMysql);
Aw TestBLOBByPreparedStatement(PMySQL pMysql);
Aw TestStoredProcedure(PMySQL pMysql, CRowsetArray& ra);
Aw TestBatch(PMySQL pMysql, CRowsetArray& ra, CDBVariantArray& vData);
void MakeLargeStrings();

wstring g_wstr;
string g_str;

CAwTask MyTest(PMySQL& pMysql, CRowsetArray& ra, CMysql::DRows& r, CMysql::DRowsetHeader& rh) {
    try {
        auto wopen = pMysql->wait_open(u"", USE_QUERY_BATCHING);
        auto vW = TestCreateTables(pMysql);
        auto wD = pMysql->wait_execute(u"delete from employee;delete from company");
        auto wP0 = TestPreparedStatements(pMysql);
        auto wP1 = TestBLOBByPreparedStatement(pMysql);
        auto wS = pMysql->wait_execute(u"SELECT * from company;select * from employee;select curtime(6)", r, rh);
        auto wP2 = TestStoredProcedure(pMysql, ra);
        CDBVariantArray vData;
        auto wP3 = TestBatch(pMysql, ra, vData);
        cout << "All SQL requests streamed ";
        cout << "and waiting for results ......\n";
        wcout << (co_await wopen).ToString() << "\n";
        while (vW.size()) {
            wcout << (co_await vW.front()).ToString() << "\n";
            vW.pop_front();
        }
        wcout << (co_await wD).ToString() << "\n";
        while (wP0.size()) {
            wcout << (co_await wP0.front()).ToString() << "\n";
            wP0.pop_front();
        }
        wcout << (co_await wP1).ToString() << "\n";
        wcout << (co_await wS).ToString() << "\n";
        CMysql::SQLExeInfo sei0 = co_await wP2;
        wcout << sei0.ToString() << "\n";
        cout << "There are " << 2 * sei0.oks << " output data returned\n";
        CMysql::SQLExeInfo sei1 = co_await wP3;
        wcout << sei1.ToString() << "\n";
        cout << "There are " << pMysql->GetOutputs() * 3 << " output data returned\n";
    }
    catch (CServerError& ex) {
        wcout << ex.ToString() << endl;
    }

    catch (CSocketError& ex) {
        wcout << ex.ToString() << endl;
    }

    catch (exception& ex) {
        wcout << "Unexpected error: " << ex.what() << endl;
    }
    //print out all received rowsets
    int index = 0;
    cout << "\n++++ Start rowsets ++++\n";
    for (auto it = ra.begin(), end = ra.end(); it != end; ++it) {
        cout << "Statement index: " << index;
        if (it->first.size()) {
            cout << ", rowset with columns: " << it->first.size() << ", records: " << it->second.size() / it->first.size() << ".\n";
        }
        else {
            cout << ", no rowset received.\n";
        }
        ++index;
    }
    cout << "++++ End rowsets ++++\n";
}

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -pthread

int main(int argc, char* argv[]) {
    MakeLargeStrings();
    CMyConnContext cc;
    cout << "Remote host: " << "\n";
    getline(cin, cc.Host);
    //cc.Host = "localhost";
#ifdef FOR_MIDDLE_SERVER
    cc.Port = 20901;
#else
    cc.Port = 20902;
#endif
    cc.UserId = L"root";
    cc.Password = L"Smash123";
    CMyPool spMysql;
    if (!spMysql.StartSocketPool(cc, 1)) {
        cout << "No connection to a remote async mysql server\n";
        cout << "Press any key to kill the demo ......\n";
        ::getchar();
        return 0;
    }
    auto pMysql = spMysql.Seek();

    CRowsetArray ra;
    CMysql::DRows r = [&ra](CMysql& handler, CDBVariantArray& vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CMysql::DRowsetHeader rh = [&ra](CMysql& handler) {
        //rowset header comes here
        auto& vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };
    MyTest(pMysql, ra, r, rh);
    cout << "\nPress any key to kill the demo ......\n";
    ::getchar();
    return 0;
}

Aw TestBLOBByPreparedStatement(PMySQL pMysql) {
    pMysql->Prepare(u"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)");

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
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
    vData.push_back(g_wstr.c_str());
    vData.push_back(254000.0);

    //second set of data
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
    vData.push_back(g_str.c_str());
    vData.push_back(20254000.0);

    //third set of data
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
    vData.push_back(g_wstr.c_str());
    vData.push_back(6254000.0);

    //execute multiple sets of parameter data in one short
    return pMysql->wait_execute(vData);
}

Aw TestBatch(PMySQL pMysql, CRowsetArray& ra, CDBVariantArray& vData) {
    //sql with delimiter '|'
    u16string sql = u"delete from employee;delete from company| \
		INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)| \
		insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)| \
		SELECT * from company;select * from employee;select curtime(6)| \
		call sp_TestProc(?,?,?)";

    SYSTEMTIME st;
    DECIMAL dec;
    memset(&dec, 0, sizeof(dec));
    vData.clear();
    SPA::CScopeUQueue sbBlob;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    dec.scale = 2;
    dec.Lo64 = 6600000000015;
    vData.push_back(dec);

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
    vData.push_back(g_wstr.c_str());
    vData.push_back(254000.0);

    vData.push_back(1);
    vData.push_back(1.25);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(0);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    dec.scale = 2;
    dec.Lo64 = 9360000000012;
    vData.push_back(dec);

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
    vData.push_back(g_str.c_str());
    vData.push_back(20254000.0);

    vData.push_back(2);
    vData.push_back(1.14);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(0);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    dec.scale = 2;
    dec.Lo64 = 23400000000014;
    vData.push_back(dec);

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
    vData.push_back(g_wstr.c_str());
    vData.push_back(6254000.0);

    vData.push_back(0);
    vData.push_back(8.16);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(0);

    CMysql::DRows r = [&ra](CMysql& handler, CDBVariantArray& vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CMysql::DRowsetHeader rh = [&ra](CMysql& handler) {
        //rowset header comes here
        auto& vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    CMysql::DRowsetHeader batchHeader = [](CMysql& handler) {
        cout << "**** Batch header comes here ****\n";
    };

    //first, start manual transaction
    //second, execute delete from employee;delete from company
    //third, prepare and three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //fourth, prepare and three sets of insert into employee values(?,?,?,?,?,?)
    //fifth, SELECT * from company;select * from employee;select curtime()
    //sixth, prepare and three sets of call sp_TestProc(?,?,?)
    //last, end manual transction
    return pMysql->wait_executeBatch(tagTransactionIsolation::tiReadUncommited, sql.c_str(), vData, r, rh, u"|", batchHeader);
}

deque<Aw> TestPreparedStatements(PMySQL pMysql) {
    DECIMAL dec;
    memset(&dec, 0, sizeof(dec));
    dec.scale = 2;
    dec.Lo64 = 6600060000015;

    const char* company = "Google Inc.";
    const char* address = "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA";

    deque<Aw> q;
    q.push_back(pMysql->wait_execute(u"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)", {1, company, address, dec}));

    //second set
    company = "Microsoft Inc.";
    address = "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA";
    q.push_back(pMysql->wait_execute(L"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,93600500000.12)", {company, address}));

    //third set
    company = "Apple Inc.";
    address = "1 Infinite Loop, Cupertino, CA 95014, USA";
    q.push_back(pMysql->wait_execute(L"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234007000000.14)", { company, address }));
    return q;
}

deque<Aw> TestCreateTables(PMySQL pMysql) {
    deque<Aw> q;
    q.push_back(pMysql->wait_execute(u"Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb"));
    const char16_t* create_table = u"CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)";
    q.push_back(pMysql->wait_execute(create_table));
    create_table = u"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
    q.push_back(pMysql->wait_execute(create_table));
    const char16_t* create_proc = u"DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int, inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime(6))BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now(6)into p_last_dt;END";
    q.push_back(pMysql->wait_execute(create_proc));
    return q;
}

Aw TestStoredProcedure(PMySQL pMysql, CRowsetArray& ra) {
    CMysql::DRows r = [&ra](CMysql& handler, CDBVariantArray& vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };
    CMysql::DRowsetHeader rh = [&ra](CMysql& handler) {
        //rowset header comes here
        auto& vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };
    //process multiple sets of parameters in one shot
    return pMysql->wait_execute(u"call mysqldb.sp_TestProc(1,? out,? out)|select curtime(6)|call mysqldb.sp_TestProc(2,? out,? out)", {1.25, 0, 8.15, 0}, r, rh, u"|");
}

void MakeLargeStrings() {
    while (g_wstr.size() < 128 * 1024) {
        g_wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }
    while (g_str.size() < 256 * 1024) {
        g_str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }
}
