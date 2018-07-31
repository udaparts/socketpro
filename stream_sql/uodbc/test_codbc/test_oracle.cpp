

#include "stdafx.h"
#include <iostream>
#include "../../../include/async_odbc.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::COdbc CMyHandler;
typedef SPA::ClientSide::CSocketPool<CMyHandler> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;
typedef SPA::ClientSide::COdbcBase CSender;


void TestCreateTables(std::shared_ptr<CMyHandler> pOdbc);
void TestPreparedStatements(std::shared_ptr<CMyHandler> pOdbc);
void InsertBLOBByPreparedStatement(std::shared_ptr<CMyHandler> pOdbc);
void TestStoredProcedure(std::shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vPData, unsigned int &oks);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spOdbc;
    bool ok = spOdbc.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote async mysql server" << std::endl;
        std::cout << "Press any key to close the application ......" << std::endl;
        ::getchar();
        return 0;
    }
    std::shared_ptr<CMyHandler> pOdbc = spOdbc.Seek();

    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //ok = pOdbc->GetAttachedClientSocket()->GetClientQueue().StartQueue("sqlite", 24 * 3600, false); //time-to-live 1 day and true for encryption

    CMyHandler::DResult dr = [](CSender &handler, int res, const std::wstring & errMsg){
        std::cout << "res = " << res;
        std::wcout << L", errMsg: " << errMsg << std::endl;
    };

    CMyHandler::DExecuteResult er = [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg;
        std::cout << std::endl;
    };

    CRowsetArray ra;
    CMyHandler::DRows r = [&ra](CSender &handler, CDBVariantArray & vData){
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CSender & handler){
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    ok = pOdbc->Open(L"dsn=ToOracle64;uid=scott;pwd=tiger", dr);

    TestCreateTables(pOdbc);
    ok = pOdbc->Execute(L"delete from ora_emp", er);
    ok = pOdbc->Execute(L"delete from company", er);
    TestPreparedStatements(pOdbc);
    InsertBLOBByPreparedStatement(pOdbc);
    ok = pOdbc->Execute(L"SELECT * from company", er, r, rh);
    ok = pOdbc->Execute(L"select * from ora_emp", er, r, rh);
    ok = pOdbc->Execute(L"select LOCALTIMESTAMP(6) from dual", er, r, rh);
    CDBVariantArray vPData;
    unsigned int oks = 0;
    TestStoredProcedure(pOdbc, ra, vPData, oks);
    pOdbc->WaitAll();
    std::cout << std::endl;
    std::cout << "There are " << pOdbc->GetOutputs() * oks << " output data returned" << std::endl;
    //ok = pOdbc->Execute(L"select TABLE_NAME from all_tables where OWNER = 'SCOTT'", er, r, rh);

    ok = pOdbc->Tables(L"", L"SCOTT", L"%", L"TABLE", er, r, rh);
    ok = pOdbc->WaitAll();
    if (ra.size()) {
        auto pTables = ra.back();
        size_t columns = pTables.first.size();
        size_t tables = pTables.second.size() / pTables.first.size();
        for (size_t n = 0; n < tables; ++n) {
            std::wstring sql = std::wstring(L"select * from ") + pTables.second[n * columns + 2].bstrVal;
            ok = pOdbc->Execute(sql.c_str(), er, r, rh);
        }
        ok = pOdbc->WaitAll();
    }

    //print out all received rowsets
    int index = 0;
    std::cout << std::endl;
    std::cout << "+++++ Start rowsets +++" << std::endl;
    for (auto it = ra.begin(), end = ra.end(); it != end; ++it) {
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

void InsertBLOBByPreparedStatement(std::shared_ptr<CMyHandler> pOdbc) {
    std::wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    std::string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t *sqlInsert = L"insert into ora_emp(EmployeeId,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)";
    bool ok = pOdbc->Prepare(sqlInsert, [](CSender &handler, int res, const std::wstring & errMsg){
        std::cout << "res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
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
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg;
                std::cout << std::endl;
    });
}

void TestPreparedStatements(std::shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *sql_insert_parameter = L"INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
    bool ok = pOdbc->Prepare(sql_insert_parameter, [](CSender &handler, int res, const std::wstring & errMsg){
        std::cout << "res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
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
    ok = pOdbc->Execute(vData, [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg;
                std::cout << std::endl;
    });
}

void TestCreateTables(std::shared_ptr<CMyHandler> pOdbc) {
    const wchar_t *create_table = L"CREATE TABLE company(ID number(10)PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income BINARY_DOUBLE not null)";
    bool ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });

    create_table = L"CREATE TABLE Ora_emp(EMPLOYEEID number(12)PRIMARY KEY NOT NULL,CompanyId number(12)not null,name NVARCHAR2(64)NOT NULL,JoinDate TIMESTAMP default null,image BLOB,DESCRIPTION NCLOB,Salary BINARY_DOUBLE,FOREIGN KEY(CompanyId) REFERENCES company(id))";
    ok = pOdbc->Execute(create_table, [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });

    const wchar_t *create_proc = L"CREATE OR REPLACE PROCEDURE sp_TestProc(cid IN number,ss OUT BINARY_DOUBLE,dt OUT TIMESTAMP,rset OUT SYS_REFCURSOR)AS BEGIN select sum(salary)INTO ss from ora_emp where companyid>=cid;select LOCALTIMESTAMP(6)INTO dt from dual;OPEN rset FOR select * from ora_emp where companyid>=cid;END;";
    ok = pOdbc->Execute(create_proc, [](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << (unsigned int) fail_ok << ", res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    });
}

void TestStoredProcedure(std::shared_ptr<CMyHandler> pOdbc, CRowsetArray&ra, CDBVariantArray &vPData, unsigned int &oks) {
    CParameterInfoArray vPInfo;

    CParameterInfo info;
    info.DataType = VT_I4;
    vPInfo.push_back(info);

    info.DataType = VT_R8;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    info.DataType = VT_DATE;
    info.Direction = pdOutput;
    vPInfo.push_back(info);

    bool ok = pOdbc->Prepare(L"{call sp_TestProc(?, ?, ?)} ", [](CSender &handler, int res, const std::wstring & errMsg){
        std::cout << "res = " << res << ", errMsg: ";
        std::wcout << errMsg << std::endl;
    }, vPInfo);
    CMyHandler::DRows r = [&ra](CSender &handler, CDBVariantArray & vData){
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CSender & handler){
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    //first set
    vPData.push_back(1);
    //output not important, but they are used for receiving proper types of data from Oracle
    vPData.push_back(0);
    vPData.push_back(0);

    //second set
    vPData.push_back(2);
    //output not important, but they are used for receiving proper types of data from Oracle
    vPData.push_back(0);
    vPData.push_back(0);
    oks = 0;

    //process multiple sets of parameters in one shot
    ok = pOdbc->Execute(vPData, [&oks](CSender &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId){
        oks = (unsigned int) fail_ok;
        std::cout << "affected = " << affected << ", fails = " << (unsigned int) (fail_ok >> 32) << ", oks = " << oks << ", res = " << res << ", errMsg: ";
                std::wcout << errMsg;
                std::cout << std::endl;
    }, r, rh);
}