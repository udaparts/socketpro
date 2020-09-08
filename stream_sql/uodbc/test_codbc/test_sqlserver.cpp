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
void TestPreparedStatements_2(shared_ptr<CMyHandler> pOdbc);
void InsertBLOBByPreparedStatement(shared_ptr<CMyHandler> pOdbc);
void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData, unsigned int& oks);
void TestStoredProcedure_2(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData, unsigned int& oks);
void TestBatch(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    cout << "Remote host: " << endl;
    getline(cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"sa";
    cc.Password = L"Smash123";
#ifndef NDEBUG
    CMyPool spOdbc(true, 600000);
#else
    CMyPool spOdbc;
#endif
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

    ok = pOdbc->Open(u"", dr);
    TestCreateTables(pOdbc);
    ok = pOdbc->Execute(u"delete from employee;delete from company;delete from test_rare1;delete from SpatialTable;INSERT INTO SpatialTable(mygeometry, mygeography)VALUES(geometry::STGeomFromText('LINESTRING(100 100,20 180,180 180)',0),geography::Point(47.6475,-122.1393,4326))", er);
    ok = pOdbc->Execute(L"INSERT INTO test_rare1(mybool,mymoney,myxml,myvariant,mydateimeoffset)values(1,23.45,'<sometest />', N'美国总统川普下个星期四','2017-05-02 00:00:00.0000000 -04:00');INSERT INTO test_rare1(mybool,mymoney,myvariant)values(0,1223.45,'This is a test for ASCII string inside sql_variant');INSERT INTO test_rare1(myvariant)values(283.45)", er);
    TestPreparedStatements(pOdbc);
    TestPreparedStatements_2(pOdbc);
    InsertBLOBByPreparedStatement(pOdbc);
    ok = pOdbc->Execute(L"SELECT * from company;select * from employee;select CONVERT(datetime,SYSDATETIME());select * from test_rare1;select * from SpatialTable", er, r, rh);
    ok = pOdbc->Tables(L"sqltestdb", L"%", L"%", L"TABLE", er, r, rh);

    CDBVariantArray vPData;
    unsigned int oks = 0;
    TestStoredProcedure(pOdbc, ra, vPData, oks);
    ok = pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * oks << " output data returned\n";
    vPData.clear();

    oks = 0;
    TestStoredProcedure_2(pOdbc, ra, vPData, oks);
    ok = pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * oks << " output data returned\n";
    vPData.clear();

    TestBatch(pOdbc, ra, vPData);
    ok = pOdbc->WaitAll();
    cout << "\nThere are " << pOdbc->GetOutputs() * 2 << " output data returned\n";
    ok = pOdbc->Tables(u"AdventureWorks", u"%", u"%", u"TABLE", er, r, rh);
    ok = pOdbc->Execute(u"use AdventureWorks", er);
    ok = pOdbc->WaitAll();

    auto pTables = ra.back();
    size_t columns = pTables.first.size();
    size_t tables = pTables.second.size() / pTables.first.size();
    for (size_t n = 0; n < tables; ++n) {
        SPA::CDBString sql = SPA::CDBString(u"select * from ") + (const SPA::UTF16*)pTables.second[n * columns + 1].bstrVal + u"." + (const SPA::UTF16*)pTables.second[n * columns + 2].bstrVal;
        ok = pOdbc->Execute(sql.c_str(), er, r, rh);
    }
    ok = pOdbc->WaitAll();

    ok = pOdbc->Tables(L"AdventureWorksDW", L"%", L"%", L"TABLE", er, r, rh);
    ok = pOdbc->WaitAll();
    ok = pOdbc->Execute(L"use AdventureWorksDW", er);

    pTables = ra.back();
    columns = pTables.first.size();
    tables = pTables.second.size() / pTables.first.size();
    for (size_t n = 0; n < tables; ++n) {
        SPA::CDBString sql = SPA::CDBString(u"select * from ") + (const SPA::UTF16*)pTables.second[n * columns + 1].bstrVal + u"." + (const SPA::UTF16*)pTables.second[n * columns + 2].bstrVal;
        ok = pOdbc->Execute(sql.c_str(), er, r, rh);
    }
    ok = pOdbc->WaitAll();

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

void TestBatch(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData) {
    GUID guid;
    memset(&guid, 0, sizeof(guid));
    //first set
    guid.Data1 = 12345;
    vPData.push_back(-1); //return int
    vPData.push_back(1); //@testid
    vPData.push_back(L"<test_sqlserver />"); //@myxml
    vPData.push_back(guid); //@tuuid
    vPData.push_back(true); //@myvar
    vPData.push_back(1);
    //output not important, but they are used for receiving proper types of data from sql server
    vPData.push_back(true);
    vPData.push_back(1.1);

    //second set
    guid.Data1 = 1234578;
    vPData.push_back(-2); //return int
    vPData.push_back(4); //@testid
    vPData.push_back(L"<test_sqlserver_again />"); //@myxml
    vPData.push_back(guid); //@tuuid
    vPData.push_back(false); //@myvar
    vPData.push_back(2);
    //output not important, but they are used for receiving proper types of data from sql server
    vPData.push_back(0);
    vPData.push_back(-1);

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

    CParameterInfoArray vPInfo;
    CParameterInfo info;

    info.DataType = VT_I4;
    vPInfo.push_back(info);
    //return direction can be ignorable

    info.DataType = VT_I4;
    info.Direction = pdInput;
    vPInfo.push_back(info);

    info.DataType = SPA::VT_XML;
    info.Direction = pdInputOutput;
    vPInfo.push_back(info);

    info.DataType = VT_CLSID;
    info.Direction = pdInputOutput;
    vPInfo.push_back(info);

    info.DataType = VT_VARIANT;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    info.DataType = VT_I4;
    info.Direction = pdInput;
    vPInfo.push_back(info);

    info.DataType = VT_R8;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    info.DataType = VT_DATE;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    //process multiple sets of parameters in one shot
    bool ok = pOdbc->ExecuteBatch(tiUnspecified, u"select getdate();{?=call sp_TestRare1(?,?,?,?)};{call sqltestdb.dbo.sp_TestProc(?,?,?)}", vPData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        }, r, rh, u";", [](CSender& handler) {
        }, nullptr, false, rpDefault, vPInfo);
}

void InsertBLOBByPreparedStatement(shared_ptr<CMyHandler> pOdbc) {
    wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    wstring str;
    while (str.size() < 256 * 1024) {
        str += L"The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t* sqlInsert = L"insert into employee(EmployeeId,CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary) values(?,?,?,?,?,?,?)";
    bool ok = pOdbc->Prepare(sqlInsert, [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    SYSTEMTIME st;
    CDBVariantArray vData;
    SPA::CScopeUQueue sbBlob;

    //first set of data
    vData.push_back(1);
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
    const char* strDec = "254000.2460";
    DECIMAL dec;
    SPA::ParseDec(strDec, dec);
    vData.push_back(dec);

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
    strDec = "20254000.197";
    SPA::ParseDec(strDec, dec);
    vData.push_back(dec);

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
    //vData.push_back(6254000.572);
    strDec = "6254000.5";
    SPA::ParseDec(strDec, dec);
    vData.push_back(dec);

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

void TestPreparedStatements_2(shared_ptr<CMyHandler> pOdbc) {
    //if a prepared statement contains UUID or sql_variant, you must specify an array of parameter definitions
    const wchar_t* sql_insert_parameter = L"INSERT INTO test_rare1(myguid,myxml,myvariant,mydateimeoffset)VALUES(?,?,?,?)";
    bool ok = pOdbc->Prepare(sql_insert_parameter, [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    CDBVariantArray vData;
    SYSTEMTIME st;
    GUID guid;
    memset(&guid, 0, sizeof(guid));

    vData.push_back(guid);
    vData.push_back(L"<myxmlroot />");
    vData.push_back(23.456);
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);

    vData.push_back(guid);
    vData.push_back(u"<myxmlroot_2 />");
    vData.push_back(u"马拉阿歌俱乐部");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);

    ok = pOdbc->Execute(vData, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestCreateTables(shared_ptr<CMyHandler> pOdbc) {
    const wchar_t* create_database = L"use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE name = 'sqltestdb') BEGIN CREATE DATABASE sqltestdb END";
    bool ok = pOdbc->Execute(create_database, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* use_database = L"Use sqltestdb";
    ok = pOdbc->Execute(use_database, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* create_table = L"IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company') create table company(ID bigint PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income float not null)";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_table = L"IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee') create table employee(EMPLOYEEID bigint PRIMARY KEY NOT NULL, CompanyId bigint not null, name CHAR(64) NOT NULL, JoinDate DATETIME2(3) default null, MyIMAGE varbinary(max), DESCRIPTION nvarchar(max), Salary decimal(15,2), FOREIGN KEY(CompanyId) REFERENCES company(id))";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_table = L"IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='test_rare1') CREATE TABLE test_rare1(testid int IDENTITY(1,1) NOT NULL,myguid uniqueidentifier DEFAULT newid() NULL,mydate date DEFAULT getdate() NULL,mybool bit DEFAULT 0 NOT NULL,mymoney money default 0 NULL,mytinyint tinyint default 0 NULL,myxml xml DEFAULT '<myxml_root />' NULL,myvariant sql_variant DEFAULT 'my_variant_default' NOT NULL,mydateimeoffset datetimeoffset(4) NULL,PRIMARY KEY(testid))";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_table = L"IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='SpatialTable') CREATE TABLE SpatialTable(id int IDENTITY(1,1) NOT NULL,mygeometry geometry NULL,mygeography geography NULL,PRIMARY KEY(id))";
    ok = pOdbc->Execute(create_table, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* drop_proc = L"IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc') drop proc sp_TestProc";
    ok = pOdbc->Execute(drop_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    const wchar_t* create_proc = L"CREATE PROCEDURE sp_TestProc(@p_company_id int, @p_sum_salary float out, @p_last_dt datetime out) as select * from employee where companyid>=@p_company_id;select @p_sum_salary=sum(salary) from employee where companyid>=@p_company_id;select @p_last_dt=SYSDATETIME()";
    ok = pOdbc->Execute(create_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    drop_proc = L"IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestRare1') drop proc sp_TestRare1";
    ok = pOdbc->Execute(drop_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });

    create_proc = L"CREATE PROCEDURE sp_TestRare1(@testid int,@myxml xml output,@tuuid uniqueidentifier output,@myvar sql_variant out)as insert into test_rare1(myguid,myxml)values(@tuuid,@myxml);select * from test_rare1 where testid>@testid;select @myxml='<myroot_testrare/>';select @tuuid=NEWID();select @myvar=N'test_variant_from_sp_TestRare1'";
    ok = pOdbc->Execute(create_proc, [](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        });
}

void TestStoredProcedure_2(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData, unsigned int& oks) {
    CParameterInfoArray vPInfo;

    CParameterInfo info;

    info.DataType = VT_I4;
    vPInfo.push_back(info);
    //return direction can be ignorable

    info.DataType = VT_I4;
    info.Direction = pdInput;
    vPInfo.push_back(info);

    info.DataType = SPA::VT_XML;
    info.Direction = pdInputOutput;
    vPInfo.push_back(info);

    info.DataType = VT_CLSID;
    info.Direction = pdInputOutput;
    vPInfo.push_back(info);

    info.DataType = VT_VARIANT;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    bool ok = pOdbc->Prepare(L"{?=call sp_TestRare1(?, ?, ?, ?)}", [](CSender& handler, int res, const wstring& errMsg) {
        cout << "res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        }, vPInfo);
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

    GUID guid;
    memset(&guid, 0, sizeof(guid));

    //first set
    guid.Data1 = 12345;
    vPData.push_back(-1); //return int
    vPData.push_back(1); //@testid
    vPData.push_back(L"<test_sqlserver />"); //@myxml
    vPData.push_back(guid); //@tuuid
    vPData.push_back(true); //@myvar

    //second set
    guid.Data1 = 1234578;
    vPData.push_back(-2); //return int
    vPData.push_back(4); //@testid
    vPData.push_back(L"<test_sqlserver_again />"); //@myxml
    vPData.push_back(guid); //@tuuid
    vPData.push_back(false); //@myvar
    oks = 0;
    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vPData, [&oks](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        oks = (unsigned int)fail_ok;
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        }, r, rh);
}

void TestStoredProcedure(shared_ptr<CMyHandler> pOdbc, CRowsetArray& ra, CDBVariantArray& vPData, unsigned int& oks) {
    //first set
    vPData.push_back(1);
    vPData.push_back(1.25);
    vPData.push_back(CDBVariant());

    //second set
    vPData.push_back(2);
    vPData.push_back(2.41);
    vPData.push_back(CDBVariant());
    bool ok = pOdbc->Prepare(L"{call sqltestdb.dbo.sp_TestProc(?, ?, ?)} ", [](CSender& handler, int res, const wstring& errMsg) {
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

    oks = 0;

    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vPData, [&oks](CSender& handler, int res, const wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant& vtId) {
        oks = (unsigned int)fail_ok;
        cout << "affected = " << affected << ", fails = " << (unsigned int)(fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
        wcout << errMsg << endl;
        }, r, rh);
}
