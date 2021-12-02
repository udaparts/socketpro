#include <iostream>
#include "../../../include/udb_client.h"
#include "../../../include/postgres/upostgres.h"

using namespace SPA;
using namespace SPA::UDB;
using namespace SPA::ClientSide;
using namespace std;

typedef CAsyncDBHandler<Postgres::sidPostgres> CPostgres;
typedef CPostgres CMyHandler;
typedef CPostgres CSender;
typedef CSocketPool<CPostgres> CMyPool;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;

void TestCreateTables(shared_ptr<CMyHandler> pSql);
void TestPreparedStatements(shared_ptr<CMyHandler> pSql);
void TestBLOBByPreparedStatement(shared_ptr<CMyHandler> pSql);
void TestBatch(shared_ptr<CMyHandler> pSql, CDBVariantArray& vPData, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh);
void TestExecuteEx(shared_ptr<CMyHandler> pSql, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh);
void TestExecuteEx2(shared_ptr<CMyHandler> pSql, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh);

int main(int argc, char* argv[]) {
    bool ok = false;
    do {
        CConnectionContext cc;
        cout << "Remote host: " << endl;
        getline(cin, cc.Host);
        cc.Port = 20901;
        cc.UserId = L"postgres";
        cc.Password = L"Smash123";
        cc.V6 = false;
#ifndef NDEBUG
        CMyPool spSql(true, 600000);
#else
        CMyPool spSql;
#endif
        ok = spSql.StartSocketPool(cc, 1);
        if (!ok) {
            cout << "No connection to remote async Postgres SocketPro server plugin\n";
            break;
        }
        shared_ptr<CMyHandler> pSql = spSql.Seek();
        CMyHandler::DResult dr = [](CSender& handler, int res, const wstring& errMsg) {
            cout << "res = " << res;
            wcout << L", errMsg: " << errMsg << endl;
        };

        CMyHandler::DExecuteResult er = [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
            cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
            wcout << errMsg << endl;
        };

        CRowsetArray ra;
        CMyHandler::DRows r = [&ra](CSender& handler, CDBVariantArray& vData) {
            //rowset data come here
            assert((vData.size() % handler.GetColumnInfo().size()) == 0);
            CDBVariantArray& va = ra.back().second;
            if (va.empty())
                va = move(vData);
            else
                move(vData.begin(), vData.end(), back_inserter(va));
        };

        CMyHandler::DRowsetHeader rh = [&ra](CSender& handler) {
            //rowset header comes here
            auto& vColInfo = handler.GetColumnInfo();
            CPColumnRowset column_rowset_pair;
            column_rowset_pair.first = vColInfo;
            ra.push_back(column_rowset_pair);
        };
        ok = pSql->Open(u"", dr, UDB::USE_QUERY_BATCHING);
        TestCreateTables(pSql);
        ok = pSql->Execute(u"delete from employee;delete from company", er);
        TestExecuteEx(pSql, er, r, rh);
        TestExecuteEx2(pSql, er, r, rh);
        ok = pSql->Execute(u"delete from employee;delete from company", er);
        TestPreparedStatements(pSql);
        TestBLOBByPreparedStatement(pSql);
        ok = pSql->Execute(u"select * from employee;select * from company", er, r, rh);
        CDBVariantArray vData;
        TestBatch(pSql, vData, er, r, rh);
        //ok = pSql->BeginTrans(tagTransactionIsolation::tiReadCommited, dr);
        //ok = pSql->Execute(u"select * from myuuid;select * from film;update company set income=123456.78 where id<>1", er, r, rh);
        //ok = pSql->EndTrans(tagRollbackPlan::rpDefault, dr);
        ok = pSql->Prepare(u"call test_sp(?,?,?)", dr);
        CDBVariantArray vParam = { 1, L"23.45", (const char*) nullptr, "0", "1276.54", (const char*) nullptr, 1, u"45678.92", (const char*) nullptr };
        ok = pSql->Execute(vParam, er, r, rh);
        ok = pSql->WaitAll();
        cout << "\nThere are " << pSql->GetOutputs() * 3 << " output data returned\n";
        int index = 0;
        cout << "\n+++++ Start rowsets +++\n";
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
        cout << "+++++ End rowsets +++\n";
        std::cout << "Press any key to close the demo ......\n";
        int res = std::getchar();
    } while (false);
    return 0;
}

void TestCreateTables(shared_ptr<CMyHandler> pSql) {
    const wchar_t* create_table = L"CREATE TABLE IF NOT EXISTS Company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income double precision not null)";
    bool ok = pSql->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_table = L"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigserial not null,CompanyId bigint not null,name char(64)NOT NULL,JoinDate TIMESTAMP default null,IMAGE bytea,DESCRIPTION text,Salary DECIMAL(14,2),FOREIGN KEY(CompanyId)REFERENCES public.company(id))";
    ok = pSql->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* create_proc = L"create or replace function sp_TestProc(p_company_id int,inout p_sum_salary decimal(14,2),out p_last_dt timestamp)as $func$ select sum(salary)+p_sum_salary,localtimestamp from employee where companyid>=p_company_id $func$ language sql";
    ok = pSql->Execute(create_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_proc = L"CREATE OR REPLACE PROCEDURE test_sp(id integer,INOUT mymoney numeric,INOUT dt timestamp) LANGUAGE 'plpgsql' AS $BODY$ BEGIN select mymoney+sum(salary),localtimestamp into mymoney,dt from employee where companyid>id;END $BODY$";
    ok = pSql->Execute(create_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestPreparedStatements(shared_ptr<CMyHandler> pSql) {
    const wchar_t* sql_insert_parameter = L"INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?,?,?,?)";
    bool ok = pSql->Prepare(sql_insert_parameter, [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    CDBVariantArray vData;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000700000.15);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600300000.28);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000900000.67);
    ok = pSql->Execute(vData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestBLOBByPreparedStatement(shared_ptr<CMyHandler> pSql) {
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t* sqlInsert = L"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
    bool ok = pSql->Prepare(sqlInsert, [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
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
    vData.push_back(254000.07);

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
    sbBlob << str;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(str.c_str());
    vData.push_back(20254000.15);

    //third set of data
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
    vData.push_back(6254000.12);

    //execute multiple sets of parameter data in one short
    ok = pSql->Execute(vData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestBatch(shared_ptr<CMyHandler> pSql, CDBVariantArray& vData, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh) {
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const char16_t* sql = u"delete from employee;delete from company|" \
        "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|" \
        "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)RETURNING employeeid|" \
        "select * from company;select * from employee";

    SYSTEMTIME st;
    SPA::CScopeUQueue sbBlob;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000600000.15);

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
    vData.push_back(254000.07);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600100000.28);

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
    vData.push_back(20254000.15);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000400000.67);

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
    vData.push_back(6254000.12);

    //process multiple sets of parameters in one shot
    bool ok = pSql->ExecuteBatch(tagTransactionIsolation::tiReadCommited, sql, vData, er, r, rh, u"|");
}

void TestExecuteEx(shared_ptr<CMyHandler> pSql, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh) {
    CDBVariantArray vData;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphit'heatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000700000.15);

    //second set
    vData.push_back("Microsof'/\\t Inc.");
    vData.push_back("700 Bell\\evue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600300000.28);

    //third set
    DECIMAL dec;
    memset(&dec, 0, sizeof(dec));
    dec.scale = 2;
    dec.Lo64 = 23400090000067;
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(dec);

    const char16_t* sql = u"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);select CURRENT_TIMESTAMP(6);" \
        "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,?);select 5;" \
        "INSERT INTO company(ID, NAME, ADDRESS, Income)VALUES(3,?,?,?)";

    bool ok = pSql->Execute(sql, vData, er, r, rh);
}

void TestExecuteEx2(shared_ptr<CMyHandler> pSql, CMyHandler::DExecuteResult er, CMyHandler::DRows r, CMyHandler::DRowsetHeader rh) {
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
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
    vData.push_back(254000.07);

    //second set of data
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
    vData.push_back(20254000.15);

    //third set of data
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
    vData.push_back(6254000.12);

    vData.push_back(1276.54); //for call test_sp

    const char16_t* sql = u"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);" \
        "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);select 245;" \
        "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(2,?,?,?,?,?);call test_sp(1,?,null)";

    bool ok = pSql->Execute(sql, vData, er, r, rh);
}
