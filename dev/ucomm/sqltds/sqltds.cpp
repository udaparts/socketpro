#include "tdschannel.h"
#include "prelogin.h"
#include "sqlbatch.h"
#include <iostream>
#include <chrono>
using namespace std::chrono;
typedef std::chrono::milliseconds ms;

const unsigned int CYCLES = 50000;

int TestSqlServer(tds::CSqlBatch& sqlClient);

int main() {
    SPA::CSessionPool<tds::CTdsChannel> pool(1);
    auto handler = pool.FindAClosedHandler();
    bool ok = handler->Connect("windesk", 1433, SPA::tagEncryptionMethod::NoEncryption, false, true);

    char serverName[128];
    handler->GetServerName(serverName, sizeof (serverName));

    tds::CPrelogin pl(*handler);
    int res = pl.SendTDSMessage();
    pl.Wait(30000);

    tds::SqlLogin rec;
    rec.database = u"sqltestdb";
    rec.userName = u"sa";
    rec.password = u"Smash123";
    rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));
    rec.useSSPI = true;

    tds::CSqlBatch sqlbatch(*handler);
    tds::CSqlBatch::FeatureExtension fe;
    res = sqlbatch.SendTDSMessage(rec, fe);
    SPA::CDBString errMsg;
    res = sqlbatch.GetError(res, errMsg);

#if 0
    SPA::CDBString sql;
    system_clock::time_point start = system_clock::now();
    for (unsigned int n = 1; n <= CYCLES; ++n) {
        if (sql.size()) {
            sql.push_back(';');
        }
        sql += u"select * from actor where actor_id between 11 and 20";
        if ((n % 10) == 0) {
            res = sqlbatch.SendTDSMessage(sql.c_str(), (unsigned int) sql.size());
            sql.clear();
            if (res) {
                break;
            }
        }
    }
    system_clock::time_point stop = system_clock::now();
    ms d = duration_cast<ms>(stop - start);
    std::cout << "Time required: " << d.count() << " ms\n\n";
#endif
    //res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtBeginTrans, tds::CSqlBatch::tagIsolationLevel::ilReadCommitted);
    unsigned int parameters;
    SPA::UDB::CParameterInfoArray vPInfo;
    SPA::UDB::CParameterInfo pi;
    SPA::UDB::CDBVariantArray vParam;
#if 1
    system_clock::time_point start = system_clock::now();
    pi.DataType = VT_I4;
    vPInfo.push_back(pi);
    pi.DataType = VT_I4;
    pi.Direction = SPA::UDB::tagParameterDirection::pdInputOutput;
    vPInfo.push_back(pi);
    pi.DataType = VT_BSTR;
    pi.Direction = SPA::UDB::tagParameterDirection::pdOutput;
    pi.ColumnSize = 1024;
    vPInfo.push_back(pi);
    res = sqlbatch.Prepare(u"exec GetSomeData @n=?,@nout=?,@dec=?", vPInfo, parameters);
    /*vParam.push_back(10);
    vParam.push_back(12);
    vParam.push_back(u"");
    res = sqlbatch.SendTDSMessage(vParam.data(), (unsigned int)vParam.size());*/
    for (unsigned int n = 0; n < 100000; ++n) {
        for (unsigned int m = 0; m < 50; ++m) {
            vParam.push_back(10);
            vParam.push_back(12);
            vParam.push_back(u"");
        }
        res = sqlbatch.SendTDSMessage(vParam.data(), (unsigned int) vParam.size());
        vParam.clear();
    }
    system_clock::time_point stop = system_clock::now();
    ms d = duration_cast<ms>(stop - start);
    std::cout << "Time required: " << d.count() << " ms\n\n";
#endif

#if 0
    vPInfo.clear();
    res = sqlbatch.Prepare(u"insert into mynulltest values(?,?,?,?,?)", vPInfo, parameters);
    vParam.push_back(16);
    vParam.push_back(201);
    vParam.push_back(43157.68);
    vParam.push_back("21907.42");
    vParam.push_back(u"�?�?��?�湾疫情的愈加严峻，新冠疫苗也从�?人问津�?��?一剂难求。");
    vParam.push_back(17);
    vParam.push_back(135);
    vParam.push_back(4127.47);
    vParam.push_back("819067.43");
    vParam.push_back(u"�?�?��?�湾疫情的愈加严峻，新冠疫苗也从�?人问津�?��?一剂难求。");
    res = sqlbatch.SendTDSMessage(vParam.data(), (unsigned int) vParam.size());
#endif
    //res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtCommit);

#if 0
    SPA::CDBString sql = u"select * from employee;select * from mynulltest;select * from mymoneys";
    while (sql.size() <= tds::DEFAULT_PACKET_SIZE * 8) {
        sql.push_back(';');
        sql += u"select * from employee;select * from mynulltest;select * from mymoneys";
    }
    res = sqlbatch.SendTDSMessage(sql.c_str(), (unsigned int) sql.size());
#endif
    //TestSqlServer(sqlbatch);

    std::cout << "Press a key to shut down the application ......\n";
    ::getchar();
    return 0;
}

int TestSqlServer(tds::CSqlBatch& sqlClient) {
    int fail, ec;
    SPA::CDBString em;
    do {
        fail = sqlClient.SendTDSMessage(u"delete from employee;delete from company;delete from test_rare1;delete from SpatialTable;INSERT INTO SpatialTable(mygeometry, mygeography)VALUES(geometry::STGeomFromText('LINESTRING(100 100,20 180,180 180)',0),geography::Point(47.6475,-122.1393,4326))");
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
        fail = sqlClient.SendTDSMessage(u"INSERT INTO test_rare1(mybool,mymoney,myxml,myvariant,mydateimeoffset)values(1,23.45,'<sometest />', N'美国总统�?普下个星期四','2017-05-02 00:00:00.0000000 -04:00');INSERT INTO test_rare1(mybool,mymoney,myvariant)values(0,1223.45,'This is a test for ASCII string inside sql_variant');INSERT INTO test_rare1(myvariant)values(283.45)");
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
        unsigned int parameters;
        SPA::UDB::CParameterInfoArray vParamInfo;
        fail = sqlClient.Prepare(u"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)", vParamInfo, parameters);
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
        SPA::UDB::CDBVariantArray vData;

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

        fail = sqlClient.SendTDSMessage(vData.data(), (unsigned int) vData.size());
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
        vData.clear();
        fail = sqlClient.Prepare(u"insert into employee(EmployeeId,CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)", vParamInfo, parameters);
        if (fail) {
            break;
        }
        std::wstring wstr;
        while (wstr.size() < 128 * 1024) {
            wstr += L"广告�?�得�?那么夸张的就�?说了，看看这三家，都是正儿八�?的公立三甲，附属医院，�?是武警，也�?是部队，更�?是莆田，都在�?�生部门直接监管下，照样明目张胆地骗人。";
        }

        std::wstring str;
        while (str.size() < 256 * 1024) {
            str += L"The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }

        SYSTEMTIME st;
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
        vData.push_back(SPA::UDB::CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
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
        vData.push_back(SPA::UDB::CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
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
        vData.push_back(SPA::UDB::CDBVariant(sbBlob->GetBuffer(), sbBlob->GetSize()));
        vData.push_back(wstr.c_str());
        //vData.push_back(6254000.572);
        strDec = "6254000.5";
        SPA::ParseDec(strDec, dec);
        vData.push_back(dec);
        fail = sqlClient.SendTDSMessage(vData.data(), (unsigned int) vData.size());
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
        fail = sqlClient.SendTDSMessage(u"select * from company;select * from employee");
        if (fail) {
            break;
        }
        ec = sqlClient.GetSQLError(em);
    } while (false);
    return fail;
}
