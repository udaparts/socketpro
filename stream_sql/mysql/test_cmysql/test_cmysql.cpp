#include "stdafx.h"
#include <iostream>
#include "../../../include/mysql/umysql.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMyHandler;
typedef std::future<CMyHandler::SQLExeInfo> CSqlFuture;
typedef SPA::ClientSide::CSocketPool<CMyHandler> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

std::vector<CSqlFuture> TestCreateTables(std::shared_ptr<CMyHandler> pMysql);
CSqlFuture TestPreparedStatements(std::shared_ptr<CMyHandler> pMysql);
CSqlFuture InsertBLOBByPreparedStatement(std::shared_ptr<CMyHandler> pMysql);
CSqlFuture TestStoredProcedure(std::shared_ptr<CMyHandler> pMysql, CRowsetArray&ra, CDBVariantArray &vPData);
CSqlFuture TestBatch(std::shared_ptr<CMyHandler> pMysql, CRowsetArray&ra, CDBVariantArray &vData);

int main(int argc, char* argv[]) {
    CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
#ifdef FOR_MIDDLE_SERVER
    cc.Port = 20901;
#else
    cc.Port = 20902;
#endif
    cc.UserId = L"root";
    cc.Password = L"Smash123";
#ifndef NDEBUG
    CMyPool spMysql(true, 600000); //600 seconds for your server debug
#else
    CMyPool spMysql;
#endif
    if (!spMysql.StartSocketPool(cc, 1)) {
        std::cout << "Failed in connecting to remote async mysql server" << std::endl;
        std::cout << "Press any key to close the application ......" << std::endl;
        ::getchar();
        return 0;
    }
    auto pMysql = spMysql.Seek();

    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //ok = pMysql->GetSocket()->GetClientQueue().StartQueue("sqlite", 24 * 3600, false); //time-to-live 1 day and true for encryption

    CRowsetArray ra;
    CMyHandler::DRows r = [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    try{
#ifdef FOR_MIDDLE_SERVER
        auto fopen = pMysql->open(u"mysqldb");
#else
        auto fopen = pMysql->open(u"");
#endif
        auto vF = TestCreateTables(pMysql);
        auto fD = pMysql->execute(L"delete from employee;delete from company");
        auto fP0 = TestPreparedStatements(pMysql);
        auto fP1 = InsertBLOBByPreparedStatement(pMysql);
        auto fS = pMysql->execute(L"SELECT * from company;select * from employee;select curtime()", r, rh);
        CDBVariantArray vPData;
        auto fP2 = TestStoredProcedure(pMysql, ra, vPData);
        std::wcout << fopen.get().ToString() << std::endl;
        for (auto& f : vF) {
            std::wcout << f.get().ToString() << std::endl;
        }
        std::wcout << fD.get().ToString() << std::endl;
        std::wcout << fP0.get().ToString() << std::endl;
        std::wcout << fP1.get().ToString() << std::endl;
        std::wcout << fS.get().ToString() << std::endl;
        CMyHandler::SQLExeInfo &sei0 = fP2.get();
        std::wcout << sei0.ToString() << std::endl;
        std::cout << std::endl;
        std::cout << "There are " << pMysql->GetOutputs() * sei0.oks << " output data returned" << std::endl;

        CDBVariantArray vData;
        auto fP3 = TestBatch(pMysql, ra, vData);
        CMyHandler::SQLExeInfo& sei1 = fP3.get();
        std::wcout << sei1.ToString() << std::endl;
        std::cout << std::endl;
        std::cout << "There are " << pMysql->GetOutputs() * 3 << " output data returned" << std::endl;

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
    }

    catch(SPA::ClientSide::CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(SPA::ClientSide::CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(std::exception & ex) {
        std::wcout << "Some unexpected error: " << ex.what() << std::endl;
    }
    std::cout << "Press any key to close the application ......" << std::endl;
    ::getchar();
    return 0;
}

CSqlFuture InsertBLOBByPreparedStatement(std::shared_ptr<CMyHandler> pMysql) {
    std::wstring wstr;
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }

    std::string str;
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }

    const wchar_t *sqlInsert = L"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
    pMysql->Prepare(sqlInsert);

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
    sbBlob << str;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(str.c_str());
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
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(wstr.c_str());
    vData.push_back(6254000.0);

    //execute multiple sets of parameter data in one short
    return pMysql->execute(vData);
}

CSqlFuture TestBatch(std::shared_ptr<CMyHandler> pMysql, CRowsetArray&ra, CDBVariantArray &vData) {
    //sql with delimiter '|'
    std::wstring sql = L"delete from employee;delete from company| \
		INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)| \
		insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)| \
		SELECT * from company;select * from employee;select curtime()| \
		call sp_TestProc(?,?,?)";
    std::wstring wstr; //make test data
    while (wstr.size() < 128 * 1024) {
        wstr += L"广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
    }
    std::string str; //make test data
    while (str.size() < 256 * 1024) {
        str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
    }
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
    sbBlob << str;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(str.c_str());
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
    sbBlob << wstr;
    vData.push_back(CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
    vData.push_back(wstr.c_str());
    vData.push_back(6254000.0);

    vData.push_back(0);
    vData.push_back(8.16);
    //output not important, but they are used for receiving proper types of data on mysql
    vData.push_back(0);

    CMyHandler::DRows r = [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            //avoid memory repeatedly allocation/de-allocation for better performance
            row_data.push_back(std::move(d));
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };

    CMyHandler::DRowsetHeader batchHeader = [](CMyHandler & handler) {
        std::cout << "Batch header comes here" << std::endl;
    };

    //first, execute delete from employee;delete from company
    //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
    //fourth, SELECT * from company;select * from employee;select curtime()
    //last, three sets of call sp_TestProc(?,?,?)
    return pMysql->executeBatch(tiReadUncommited, sql.c_str(), vData, r, rh, L"|", batchHeader);
}

CSqlFuture TestPreparedStatements(std::shared_ptr<CMyHandler> pMysql) {
    const wchar_t *sql_insert_parameter = L"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
    pMysql->Prepare(sql_insert_parameter);

    CDBVariantArray vData;
    DECIMAL dec;
    memset(&dec, 0, sizeof (dec));

    //first set
    vData.push_back(1);
    vData.push_back("Google Inc.");
    vData.push_back("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");

    dec.scale = 2;
    dec.Lo64 = 6600000000015;
    vData.push_back(dec);

    //second set
    vData.push_back(2);
    vData.push_back("Microsoft Inc.");
    vData.push_back("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
    dec.scale = 2;
    dec.Lo64 = 9360000000012;
    vData.push_back(dec);

    //third set
    vData.push_back(3);
    vData.push_back("Apple Inc.");
    vData.push_back("1 Infinite Loop, Cupertino, CA 95014, USA");
    dec.scale = 2;
    dec.Lo64 = 23400000000014;
    vData.push_back(dec);
    return pMysql->execute(vData);
}

std::vector<CSqlFuture> TestCreateTables(std::shared_ptr<CMyHandler> pMysql) {
    std::vector<CSqlFuture> v;
    const wchar_t *create_database = L"Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
    v.push_back(pMysql->execute(create_database));

    const wchar_t *create_table = L"CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)";
    v.push_back(pMysql->execute(create_table));

    create_table = L"CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
    v.push_back(pMysql->execute(create_table));
    const wchar_t *create_proc = L"DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int, inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END";
    v.push_back(pMysql->execute(create_proc));
    return v;
}

CSqlFuture TestStoredProcedure(std::shared_ptr<CMyHandler> pMysql, CRowsetArray&ra, CDBVariantArray &vPData) {
    pMysql->Prepare(L"call mysqldb.sp_TestProc(?,?,?)");
    CMyHandler::DRows r = [&ra](CMyHandler &handler, CDBVariantArray & vData) {
        //rowset data come here
        assert((vData.size() % handler.GetColumnInfo().size()) == 0);
        CDBVariantArray &row_data = ra.back().second;
        for (size_t n = 0; n < vData.size(); ++n) {
            auto &d = vData[n];
            row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
        }
    };

    CMyHandler::DRowsetHeader rh = [&ra](CMyHandler & handler) {
        //rowset header comes here
        auto &vColInfo = handler.GetColumnInfo();
        CPColumnRowset column_rowset_pair;
        column_rowset_pair.first = vColInfo;
        ra.push_back(column_rowset_pair);
    };
    vPData.clear();
    //first set
    vPData.push_back(1);
    vPData.push_back(1.25);
    //output not important, but they are used for receiving proper types of data on mysql
    vPData.push_back(0);

    //second set
    vPData.push_back(2);
    vPData.push_back(1.14);
    //output not important, but they are used for receiving proper types of data on mysql
    vPData.push_back(0);
    //process multiple sets of parameters in one shot
    return pMysql->execute(vPData, r, rh);
}