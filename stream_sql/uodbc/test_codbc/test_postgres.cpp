#include "stdafx.h"
#include <iostream>
#include "../../../include/async_odbc.h"

using namespace SPA::ClientSide;
using namespace SPA::UDB;
using namespace std;

typedef COdbc CMyHandler;
typedef CSocketPool<CMyHandler> CMyPool;
typedef CConnectionContext CMyConnContext;
typedef pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef vector<CPColumnRowset> CRowsetArray;
typedef COdbcBase CSender;

void TestCreateTables(shared_ptr<CMyHandler> pOdbc);
void TestPreparedStatements(shared_ptr<CMyHandler> pOdbc);
void InsertBLOBByPreparedStatement(shared_ptr<CMyHandler> pOdbc);
void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    cout << "Remote host: \n";
    getline(cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spOdbc(true, 600000);
    bool ok = spOdbc.StartSocketPool(cc, 1);
    if (!ok) {
        cout << "No connection to remote async ODBC server\n";
        cout << "Press any key to close the demo ......\n";
        ::getchar();
        return 0;
    }
    shared_ptr<CMyHandler> pOdbc = spOdbc.Seek();
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

    ok = pOdbc->Open(L"dsn=ToPostgres;uid=postgres;pwd=Smash123", dr);
    pOdbc->WaitAll();
    TestCreateTables(pOdbc);
    pOdbc->WaitAll();
    ok = pOdbc->Execute(L"delete from employee;delete from company", er);
    TestPreparedStatements(pOdbc);
    InsertBLOBByPreparedStatement(pOdbc);
    ok = pOdbc->Execute(L"select * from company;select * from employee", er, r, rh);
    TestStoredProcedure(pOdbc, ra);
    pOdbc->WaitAll();

    ok = pOdbc->Tables(L"MYPDB", L"PUBLIC", L"%", L"TABLE", er, r, rh);
    pOdbc->WaitAll();

    //print out all received rowsets
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
    cout << "+++++ End rowsets +++\n\n";
    cout << "Press any key to close the demo ......\n";
    ::getchar();

    return 0;
}

void InsertBLOBByPreparedStatement(shared_ptr<CMyHandler> pOdbc) {
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t* sqlInsert = L"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
    bool ok = pOdbc->Prepare(sqlInsert, [](CSender& handler, int res, const wstring& errMsg) {
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
    ok = pOdbc->Execute(vData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestPreparedStatements(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t* sql_insert_parameter = L"INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
    bool ok = pOdbc->Prepare(sql_insert_parameter, [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    CDBVariantArray vData;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000000000.0);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600000000.0);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000000000.0);
    ok = pOdbc->Execute(vData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestCreateTables(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t* set_db = L"SET search_path=mypdb,public";
    bool ok = pOdbc->Execute(set_db, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* create_table = L"CREATE TABLE IF NOT EXISTS Company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income double precision not null)";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_table = L"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigserial not null,CompanyId bigint not null,name char(64)NOT NULL,JoinDate TIMESTAMP default null,IMAGE bytea,DESCRIPTION text,Salary DECIMAL(14,2),FOREIGN KEY(CompanyId)REFERENCES public.company(id))";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* create_proc = L"create or replace function mypdb.public.sp_TestProc(p_company_id int,inout p_sum_salary decimal(14,2),out p_last_dt timestamp)as $func$ select sum(salary)+p_sum_salary,localtimestamp from employee where companyid>=p_company_id $func$ language sql";
    ok = pOdbc->Execute(create_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra) {
    //stored procedure name may be case-sensitive
    bool ok = pOdbc->Prepare(L"select * from PUBLIC.sp_TestProc(?,?)", [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
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

    DECIMAL dec;
    memset(&dec, 0, sizeof(dec));

    CDBVariantArray vPData;

    //first set
    vPData.push_back(1);
    dec.Lo64 = 235;
    dec.scale = (unsigned char)2;
    vPData.push_back(dec);

    //second set
    vPData.push_back(2);
    dec.Lo64 = 801;
    dec.scale = (unsigned char)2;
    vPData.push_back(dec);

    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vPData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        unsigned int oks = (unsigned int) fail_ok;
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        }, r, rh);
}
