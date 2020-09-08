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
void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vPData, unsigned int &oks);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    cout << "Remote host: \n";
    getline(cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spOdbc;
    bool ok = spOdbc.StartSocketPool(cc, 1);
    if (!ok) {
        cout << "No connection to remote async ODBC server\n";
        cout << "Press any key to close the demo ......\n";
        ::getchar();
        return 0;
    }
    shared_ptr<CMyHandler> pOdbc = spOdbc.Seek();

    CMyHandler::DResult dr = [](CSender &handler, int res, const wstring & errMsg) {
        cout << "res = " << res;
        wcout << L", errMsg: " << errMsg << endl;
    };

    CMyHandler::DExecuteResult er = [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    };

    CRowsetArray ra;
    CMyHandler::DRows r = [&ra](CSender &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CMyHandler::DRowsetHeader rh = [&ra](CSender & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    ok = pOdbc->Open(u"dsn=ToOracle64;uid=scott;pwd=tiger", dr);

    TestCreateTables(pOdbc);
    ok = pOdbc->Execute(u"delete from ora_emp", er);
    ok = pOdbc->Execute(u"delete from company", er);
    TestPreparedStatements(pOdbc);
    InsertBLOBByPreparedStatement(pOdbc);
    ok = pOdbc->Execute(L"SELECT * from company", er, r, rh);
    ok = pOdbc->Execute(L"select * from ora_emp", er, r, rh);
    ok = pOdbc->Execute(L"select LOCALTIMESTAMP(6) from dual", er, r, rh);
    CDBVariantArray vPData;
    unsigned int oks = 0;
    TestStoredProcedure(pOdbc, ra, vPData, oks);
    pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * oks << " output data returned\n";
    //ok = pOdbc->Execute(L"select TABLE_NAME from all_tables where OWNER = 'SCOTT'", er, r, rh);

    ok = pOdbc->Tables(L"", L"SCOTT", L"%", L"TABLE", er, r, rh);
    ok = pOdbc->WaitAll();
    if (ra.size()) {
        auto pTables = ra.back();
        size_t columns = pTables.first.size();
        size_t tables = pTables.second.size() / pTables.first.size();
        for (size_t n = 0; n < tables; ++n) {
            SPA::CDBString sql = SPA::CDBString(u"select * from ") + (const SPA::UTF16*)pTables.second[n * columns + 2].bstrVal;
            ok = pOdbc->Execute(sql.c_str(), er, r, rh);
        }
        ok = pOdbc->WaitAll();
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

    const wchar_t *sqlInsert = L"insert into ora_emp(EmployeeId,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)";
    bool ok = pOdbc->Prepare(sqlInsert, [](CSender &handler, int res, const wstring & errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
    vData.push_back(1); //employee id
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

    //second set of data
    vData.push_back(2); //employee id
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

    //third set of data
    vData.push_back(3); //employee id
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

    //execute multiple sets of parameter data in one short
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestPreparedStatements(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *sql_insert_parameter = L"INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
    bool ok = pOdbc->Prepare(sql_insert_parameter, [](CSender &handler, int res, const wstring & errMsg) {
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
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestCreateTables(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *create_table = L"CREATE TABLE company(ID number(10)PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income BINARY_DOUBLE not null)";
    bool ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    create_table = L"CREATE TABLE Ora_emp(EMPLOYEEID number(12)PRIMARY KEY NOT NULL,CompanyId number(12)not null,name NVARCHAR2(64)NOT NULL,JoinDate TIMESTAMP default null,image BLOB,DESCRIPTION NCLOB,Salary BINARY_DOUBLE,FOREIGN KEY(CompanyId) REFERENCES company(id))";
    ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    const wchar_t *create_proc = L"CREATE OR REPLACE PROCEDURE sp_TestProc(cid IN number,ss IN OUT BINARY_DOUBLE,dt OUT TIMESTAMP,rset OUT SYS_REFCURSOR)AS BEGIN select sum(salary)+ss INTO ss from ora_emp where companyid>=cid;select LOCALTIMESTAMP(6)INTO dt from dual;OPEN rset FOR select * from ora_emp where companyid>=cid;END;";
    ok = pOdbc->Execute(create_proc, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vPData, unsigned int &oks) {
    bool ok = pOdbc->Prepare(L"{call sp_TestProc(?,?,?)} ", [](CSender &handler, int res, const wstring & errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
    CMyHandler::DRows r = [&ra](CSender &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray& va = ra.back().second;
        if (va.empty())
            va = move(vData);
        else
            move(vData.begin(), vData.end(), back_inserter(va));
    };

    CMyHandler::DRowsetHeader rh = [&ra](CSender & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    //first set
    vPData.push_back(1);
    vPData.push_back(2.3); //bind to double
    //output not important, but they are used for receiving proper types of data from Oracle
    vPData.push_back(0);

    //second set
    vPData.push_back(2);
    vPData.push_back(2.1); //bind to double
    //output not important, but they are used for receiving proper types of data from Oracle
    vPData.push_back(0);
    oks = 0;

    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vPData, [&oks](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        oks = (unsigned int) fail_ok;
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
                wcout << errMsg << endl;
    }, r, rh);
}
