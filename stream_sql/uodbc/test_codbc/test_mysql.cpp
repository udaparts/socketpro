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
void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vData, unsigned int &oks);
void TestBatch(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vData, unsigned int &oks);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    cout << "Remote host: " << endl;
    getline(cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"root";
    cc.Password = L"Smash123";
    CMyPool spOdbc;
    bool ok = spOdbc.StartSocketPool(cc, 1);
    if (!ok) {
        cout << "No connection to remote async ODBC server\n";
        cout << "Press any key to close the demo ......\n";
        ::getchar();
        return 0;
    }
    shared_ptr<CMyHandler> pOdbc = spOdbc.Seek();

    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //ok = pOdbc->GetSocket()->GetClientQueue().StartQueue("sqlite", 24 * 3600, false); //time-to-live 1 day and true for encryption

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

    ok = pOdbc->Open(u"dsn=ToMySQL;uid=root;pwd=Smash123", dr);
    TestCreateTables(pOdbc);
    ok = pOdbc->Execute(u"delete from employee", er);
    ok = pOdbc->Execute(u"delete from company", er);
    TestPreparedStatements(pOdbc);
    InsertBLOBByPreparedStatement(pOdbc);
    ok = pOdbc->Execute(u"SELECT * from company", er, r, rh);
    ok = pOdbc->Execute(u"select * from employee", er, r, rh);
    ok = pOdbc->Execute(u"select curtime()", er, r, rh);

    unsigned int oks = 0;
    CDBVariantArray vPData;
    TestStoredProcedure(pOdbc, ra, vPData, oks);
    ok = pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * oks << " output data returned\n";

    CDBVariantArray vData;
    TestBatch(pOdbc, ra, vData, oks);
    ok = pOdbc->Tables(u"sakila", u"", u"%", u"TABLE", er, r, rh);
    ok = pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * oks << " output data returned\n";

    ok = pOdbc->Execute(u"use sakila", er);
    auto pTables = ra.back();
    size_t columns = pTables.first.size();
    size_t tables = pTables.second.size() / pTables.first.size();
    for (size_t n = 0; n < tables; ++n) {
        SPA::CDBString sql = SPA::CDBString(u"select * from ") + (const SPA::UTF16*)pTables.second[n * columns + 2].bstrVal;
        ok = pOdbc->Execute(sql.c_str(), er, r, rh);
    }
    ok = pOdbc->WaitAll();

    //print out all received rowsets
    int index = 0;
    cout << "\n+++++ Start rowsets +++\n";
    for (auto it = ra.begin(), end = ra.end(); it != end; ++it) {
        cout << "Statement index = " << index;
        if (it->first.size()) {
            cout << ", rowset with columns = " << it->first.size() << ", records = " << it->second.size() / it->first.size() << ".\n";
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

    const wchar_t *sqlInsert = L"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
    bool ok = pOdbc->Prepare(sqlInsert, [](CSender &handler, int res, const wstring & errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
    vData.push_back(1); //google company id
    vData.push_back("Ted Cruz");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(wstr.c_str());
    vData.push_back(254000.12);

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
    vData.push_back(SPA::Utilities::ToWide(str.c_str(), str.size()).c_str());
    vData.push_back(20254000.17);

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
    vData.push_back(6254000.02);

    //execute multiple sets of parameter data in one short
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestPreparedStatements(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *sql_insert_parameter = L"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
    bool ok = pOdbc->Prepare(sql_insert_parameter, [](CSender &handler, int res, const wstring & errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    CDBVariantArray vData;

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000000000.15);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600000000.24);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000000000.05);
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestBatch(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vData, unsigned int &oks) {
    oks = 0;
    //sql with delimiter ';'

    //first, execute delete from employee;delete from company
    //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
    //fourth, SELECT * from company;select * from employee;select curtime()
    //last, three sets of call sp_TestProc(?,?,?)
    wstring sql = L"delete from employee;delete from company; \
                        INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?); \
                        insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?); \
                        SELECT * from company;select * from employee;select curtime(); \
						{call sp_TestProc(?,?,?)}";
    CParameterInfoArray vInfo;
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    SYSTEMTIME st;
    SPA::CScopeUQueue sbBlob;
    vData.clear();

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
    vData.push_back(66000000000.15);

    vData.push_back(1); //google company id
    vData.push_back("Ted Cruz");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(SPA::Utilities::ToUTF8(wstr.c_str(), wstr.size()).c_str());
    vData.push_back(254000.12);

    vData.push_back(1);
    DECIMAL dec;
    memset(&dec, 0, sizeof (dec));
    dec.scale = 2;
    dec.Lo64 = 235; //2.35
    vData.push_back(dec);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(1.2);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    vData.push_back(93600000000.24);

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
    vData.push_back(20254000.17);

    vData.push_back(2);
    dec.Lo64 = 15; //0.15
    vData.push_back(dec);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(true);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    vData.push_back(234000000000.05);

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
    vData.push_back(SPA::Utilities::ToUTF8(wstr.c_str(), wstr.size()).c_str());
    vData.push_back(6254000.02);

    vData.push_back(0);
    dec.Lo64 = 215; //2.15
    vData.push_back(dec);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(true);

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

    if (pOdbc->ExecuteBatch(tiUnspecified, sql.c_str(), vData,
            [](CSender & handler, int res, const wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
                cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
                wcout << errMsg << endl;
            }, r, rh, L";", [](CSender & handler) {
                //called before rh, r and er
            }, [](CAsyncServiceHandler *handler, bool canceled) {
                //called when canceling or socket closed if client queue is NOT used
            })) {
    oks = 3;
}
}

void TestCreateTables(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *create_database = L"Create database if not exists mysqldb character set utf8 collate utf8_general_ci";
    bool ok = pOdbc->Execute(create_database, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    const wchar_t *use_db = L"USE mysqldb";
    ok = pOdbc->Execute(use_db, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    const wchar_t *create_table = L"CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64) NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
    ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    create_table = L"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null, name CHAR(64) NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
    ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    const wchar_t *drop_proc = L"DROP PROCEDURE IF EXISTS sp_TestProc";
    ok = pOdbc->Execute(drop_proc, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });

    const wchar_t *create_proc = L"CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(15,2),out p_last_dt datetime)BEGIN select * from employee where companyid >= p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END";
    ok = pOdbc->Execute(create_proc, [](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    });
}

void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vData, unsigned int &oks) {
    bool ok = pOdbc->Prepare(L"{call mysqldb.sp_TestProc(?,?,?)}", [](CSender &handler, int res, const wstring & errMsg) {
        if (res) {
            cout << "res = " << res << ", errMsg: ";
            wcout << errMsg << endl;
        }
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

    oks = 0;
    vData.clear();

    DECIMAL dec;
    memset(&dec, 0, sizeof (dec));

    //first set
    vData.push_back(1);
    dec.scale = 2;
    dec.Lo64 = 235; //2.35
    vData.push_back(dec);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(1.2);

    //second set
    vData.push_back(2);
    dec.Lo64 = 15; //0.15
    vData.push_back(dec);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(true);

    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vData, [&oks](CSender &handler, int res, const wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
        oks = (unsigned int) fail_ok;
        cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
    }, r, rh);
}
