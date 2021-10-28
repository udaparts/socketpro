#include "stdafx.h"
#include <iostream>
#include "../../../include/mysql/umysql.h"
using namespace SPA::ClientSide;
using namespace SPA::UDB;
using namespace std;

typedef CAsyncDBHandler<SPA::Mysql::sidMysql> CSQLHandler;
typedef future<CSQLHandler::SQLExeInfo> CSqlFuture;
typedef CSocketPool<CSQLHandler> CMyPool;
typedef CConnectionContext CMyConnContext;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;
typedef shared_ptr<CSQLHandler> PMySQL;

vector<CSqlFuture> TestCreateTables(PMySQL pMysql);
CSqlFuture TestExecuteEx(PMySQL pMysql);
CSqlFuture TestBLOBByPreparedStatement(PMySQL pMysql);
CSqlFuture TestStoredProcedureByExecuteEx(PMySQL pMysql, CRowsetArray&ra);
CSqlFuture TestBatch(PMySQL pMysql, CRowsetArray&ra, CDBVariantArray &vData);

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
    CMyConnContext cc;
    cout << "Remote host: " << endl;
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
    CSQLHandler::DRows r = [&ra](CSQLHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CSQLHandler::DRowsetHeader rh = [&ra](CSQLHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    try{
        //connection string should be something like 'host=localhost;port=3306;timeout=30;uid=MyUserID;pwd=MyPassword;database=sakila' if there is no DB authentication at server side
#ifdef FOR_MIDDLE_SERVER
        //enable in-line query batching for better performance
        auto fopen = pMysql->open(u"", SPA::UDB::USE_QUERY_BATCHING);
#else
        auto fopen = pMysql->open(u"");
#endif
        auto vF = TestCreateTables(pMysql);
        auto fD = pMysql->execute(u"delete from employee;delete from company");
        auto fP0 = TestExecuteEx(pMysql);
        auto fP1 = TestBLOBByPreparedStatement(pMysql);
        auto fS = pMysql->execute(u"SELECT * from company;select * from employee;select curtime(6)", r, rh);
        auto fP2 = TestStoredProcedureByExecuteEx(pMysql, ra);
        CDBVariantArray vData;
        auto fP3 = TestBatch(pMysql, ra, vData);
        cout << "All SQL requests streamed ";
        cout << "and waiting for results ......\n";
        wcout << fopen.get().ToString() << endl;
        for (auto& f : vF) {
            wcout << f.get().ToString() << endl;
        }
        wcout << fD.get().ToString() << endl;
        wcout << fP0.get().ToString() << endl;
        wcout << fP1.get().ToString() << endl;
        wcout << fS.get().ToString() << endl;
        wcout << fP2.get().ToString() << endl;
        CSQLHandler::SQLExeInfo sei1 = fP3.get();
        wcout << sei1.ToString() << endl;
        cout << "There are " << pMysql->GetOutputs() * 3 << " output data returned\n";}

    catch(CServerError & ex) {
        wcout << ex.ToString() << endl;
    }

    catch(CSocketError & ex) {
        wcout << ex.ToString() << endl;
    }

    catch(exception & ex) {
        wcout << "Unexpected error: " << ex.what() << endl;
    }
    //print out all received rowsets
    int index = 0;
    cout << "\n+++++ Start rowsets +++\n";
    for (auto it = ra.begin(), end = ra.end(); it != end; ++it) {
        cout << "Statement index: " << index;
        if (it->first.size()) {
            cout << ", rowset with columns: " << it->first.size() << ", records: " << it->second.size() / it->first.size() << ".\n";
        } else {
            cout << ", no rowset received.\n";
        }
        ++index;
    }
    cout << "+++++ End rowsets +++\n";
    cout << "\nPress any key to kill the demo ......\n";
    ::getchar();
    return 0;
}

CSqlFuture TestBLOBByPreparedStatement(PMySQL pMysql) {
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
    return pMysql->execute(vData);
}

CSqlFuture TestBatch(PMySQL pMysql, CRowsetArray&ra, CDBVariantArray &vData) {
    //sql with delimiter '|'
    u16string sql = u"delete from employee;delete from company| \
		INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)| \
		insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)| \
		SELECT * from company;select * from employee;select curtime(6)| \
		call sp_TestProc(?,?,?)";

    SYSTEMTIME st;
    DECIMAL dec;
    memset(&dec, 0, sizeof (dec));
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

    CSQLHandler::DRows r = [&ra](CSQLHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CSQLHandler::DRowsetHeader rh = [&ra](CSQLHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    CSQLHandler::DRowsetHeader batchHeader = [](CSQLHandler & handler) {
        cout << "Batch header comes here" << endl;
    };
    //first, start manual transaction
    //second, execute delete from employee;delete from company
    //third, prepare and three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //fourth, prepare and three sets of insert into employee values(?,?,?,?,?,?)
    //fifth, SELECT * from company;select * from employee;select curtime()
    //sixth, prepare and three sets of call sp_TestProc(?,?,?)
    //last, end manual transction
    return pMysql->executeBatch(tagTransactionIsolation::tiReadCommited, sql.c_str(), vData, r, rh, u"|", batchHeader);
}

CSqlFuture TestExecuteEx(PMySQL pMysql) {
    CDBVariantArray vData;
    DECIMAL dec;
    memset(&dec, 0, sizeof (dec));

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    dec.scale = 2;
    dec.Lo64 = 6600004000015;
    vData.push_back(dec);

    //second set
    vData.push_back(u"Microsoft Inc.");
    vData.push_back(u"700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");

    //third set
    vData.push_back(L"Apple Inc.");
    vData.push_back(L"1 Infinite Loop, Cupertino, CA 95014, USA");
    return pMysql->execute(u"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,93600700000.12);INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234005000000.14)", vData);
}

vector<CSqlFuture> TestCreateTables(PMySQL pMysql) {
    vector<CSqlFuture> v;
    v.push_back(pMysql->execute(u"Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb"));
    const char16_t *create_table = u"CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)";
    v.push_back(pMysql->execute(create_table));
    create_table = u"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
    v.push_back(pMysql->execute(create_table));
    const char16_t *create_proc = u"DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int, inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime(6))BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now(6)into p_last_dt;END";
    v.push_back(pMysql->execute(create_proc));
    return v;
}

CSqlFuture TestStoredProcedureByExecuteEx(PMySQL pMysql, CRowsetArray&ra) {
    CSQLHandler::DRows r = [&ra](CSQLHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CSQLHandler::DRowsetHeader rh = [&ra](CSQLHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };
    //process multiple sets of parameters in one shot & pay attention to out for returning results
    return pMysql->execute(L"call mysqldb.sp_TestProc(?,? out,? out);select curtime(6);call mysqldb.sp_TestProc(2,? out,? out);select 1,2,3", {1, 6751.25, 0, 16785.14, 0}, r, rh);
}
