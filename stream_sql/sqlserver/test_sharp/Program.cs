using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;


class Program
{
    static CSqlServer.DResult dr = (handler, res, errMsg) =>
    {
        if (res != 0)
            Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
    };

    static CAsyncDBHandler.DRowsetHeader rh = (h) =>
    {
        CDBColumnInfoArray v = h.ColumnInfo;
        if (v.Count > 0)
            Console.WriteLine("dbPath={0}, tablePath={1}", v[0].DBPath, v[0].TablePath);
    };

    static CAsyncDBHandler.DExecuteResult er = (h, res, errMsg, affected, fail_ok, vtId) =>
    {
        if (res != 0)
            Console.WriteLine("Error code = {0}, error message = {1}, affected = {2}, oks = {3}, fails = {4}", res, errMsg, affected, (uint)fail_ok, (uint)(fail_ok >> 32));
        else
            Console.WriteLine("Affected = {0}, oks = {1}, fails = {2}", affected, (uint)fail_ok, (uint)(fail_ok >> 32));
    };

    static CAsyncDBHandler.DRows rows = (h, vData) =>
    {
        //Console.WriteLine("Rows = " + vData.Count / h.ColumnInfo.Count);
    };

    static void TestCreateTables(CSqlServer sql)
    {
        string create_database = "use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE name = 'sqltestdb') BEGIN CREATE DATABASE sqltestdb END";
        bool ok = sql.Execute(create_database, er);

        string use_database = "Use sqltestdb";
        ok = sql.Execute(use_database, er);

        string create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company') create table company(ID bigint PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income float not null)";
        ok = sql.Execute(create_table, er);

        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee') create table employee(EMPLOYEEID bigint IDENTITY(1,1) PRIMARY KEY NOT NULL, CompanyId bigint not null, name CHAR(64) NOT NULL, JoinDate DATETIME2(3) default null, MyIMAGE varbinary(max), DESCRIPTION nvarchar(max), Salary decimal(15,2), FOREIGN KEY(CompanyId) REFERENCES company(id))";
        ok = sql.Execute(create_table, er);

        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='test_rare1') CREATE TABLE test_rare1(testid int IDENTITY(1,1) NOT NULL,myguid uniqueidentifier DEFAULT newid() NULL,mydate date DEFAULT getdate() NULL,mybool bit DEFAULT 0 NOT NULL,mymoney money default 0 NULL,mytinyint tinyint default 0 NULL,myxml xml DEFAULT '<myxml_root />' NULL,myvariant sql_variant DEFAULT 'my_variant_default' NOT NULL,mydateimeoffset datetimeoffset(4) NULL,PRIMARY KEY(testid))";
        ok = sql.Execute(create_table, er);

        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='SpatialTable') CREATE TABLE SpatialTable(id int IDENTITY(1,1) NOT NULL,mygeometry geometry NULL,mygeography geography NULL,PRIMARY KEY(id))";
        ok = sql.Execute(create_table, er);

        string drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc') drop proc sp_TestProc";
        ok = sql.Execute(drop_proc, er);

        string create_proc = "CREATE PROCEDURE sp_TestProc(@p_company_id int, @p_sum_salary float OUT, @p_last_dt datetime out) as select * from employee where companyid>=@p_company_id;select @p_sum_salary=sum(salary)+@p_sum_salary from employee where companyid>=@p_company_id;select @p_last_dt=SYSDATETIME()";
        ok = sql.Execute(create_proc, er);

        drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestRare1') drop proc sp_TestRare1";
        ok = sql.Execute(drop_proc, er);

        create_proc = "CREATE PROCEDURE sp_TestRare1(@testid int,@myxml xml output,@tuuid uniqueidentifier output,@myvar sql_variant out)as insert into test_rare1(myguid,myxml)values(@tuuid,@myxml);select * from test_rare1 where testid>@testid;select @myxml='<myroot_testrare/>';select @tuuid=NEWID();select @myvar=N'test_variant_from_sp_TestRare1'";
        ok = sql.Execute(create_proc, er);
    }

    static void TestPreparedStatements(CSqlServer sql)
    {
        string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(@ID,@NAME,@ADDRESS,@Income)";
        CParameterInfoArray vInfo = new CParameterInfoArray();
        CParameterInfo info = new CParameterInfo();
        info.ParameterName = "@ID";
        info.DataType = tagVariantDataType.sdVT_INT;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@NAME";
        info.DataType = tagVariantDataType.sdVT_BSTR;
        info.ColumnSize = 64;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@ADDRESS";
        info.DataType = tagVariantDataType.sdVT_BSTR;
        info.ColumnSize = 256;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@Income";
        info.DataType = tagVariantDataType.sdVT_R8; //double
        vInfo.Add(info);
        bool ok = sql.Prepare(sql_insert_parameter, dr, vInfo.ToArray());

        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.Add(1);
        vData.Add("Google Inc.");
        vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.Add(66000000000.0);

        //second set
        vData.Add(2);
        vData.Add("Microsoft Inc.");
        vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.Add(93600000000.0);

        //third set
        vData.Add(3);
        vData.Add("Apple Inc.");
        vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.Add(234000000000.0);
        ok = sql.Execute(vData, er);
    }

    static void TestPreparedStatements_2(CSqlServer sql)
    {
        CParameterInfoArray vInfo = new CParameterInfoArray();

        CParameterInfo info = new CParameterInfo();

        info.DataType = tagVariantDataType.sdVT_CLSID;
        info.ParameterName = "@myguid";
        vInfo.Add(info);

        info = new CParameterInfo();
        info.ParameterName = "@myxml";
        info.DataType = tagVariantDataType.sdVT_BSTR;
        info.ColumnSize = uint.MaxValue;
        vInfo.Add(info);

        info = new CParameterInfo();
        info.ParameterName = "@myvariant";
        info.DataType = tagVariantDataType.sdVT_VARIANT;
        vInfo.Add(info);

        info = new CParameterInfo();
        info.ParameterName = "@mydateimeoffset";
        info.DataType = tagVariantDataType.sdVT_DATE;
        vInfo.Add(info);

        //if a prepared statement contains UUID or sql_variant, you must specify an array of parameter definitions
        string sql_insert_parameter = "INSERT INTO test_rare1(myguid,myxml,myvariant,mydateimeoffset)VALUES(@myguid,@myxml,@myvariant,@mydateimeoffset)";
        bool ok = sql.Prepare(sql_insert_parameter, dr, vInfo.ToArray());

        CDBVariantArray vData = new CDBVariantArray();
        vData.Add(Guid.NewGuid());
        vData.Add("<myxmlroot />");
        vData.Add(23.456);
        vData.Add(DateTime.Now);

        vData.Add(Guid.NewGuid());
        vData.Add("<myxmlroot_2 />");
        vData.Add("马拉阿歌俱乐部");
        vData.Add(DateTime.Now.AddSeconds(123.45));

        ok = sql.Execute(vData, er);
    }

    static void InsertBLOBByPreparedStatement(CSqlServer sql)
    {
        string wstr = "";
        while (wstr.Length < 128 * 1024)
        {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }

        string str = "";
        while (str.Length < 256 * 1024)
        {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
        string sqlInsert = "insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(@CompanyId,@name,@JoinDate,@myimage,@DESCRIPTION,@Salary)";

        CParameterInfoArray vInfo = new CParameterInfoArray();
        CParameterInfo info = new CParameterInfo();
        info.ParameterName = "@CompanyId";
        info.DataType = tagVariantDataType.sdVT_INT;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@name";
        info.ColumnSize = 64;
        info.DataType = tagVariantDataType.sdVT_BSTR;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@JoinDate";
        info.DataType = tagVariantDataType.sdVT_DATE;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@myimage";
        info.ColumnSize = uint.MaxValue;
        info.DataType = tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@DESCRIPTION";
        info.ColumnSize = uint.MaxValue;
        info.DataType = tagVariantDataType.sdVT_BSTR;
        vInfo.Add(info);
        info = new CParameterInfo();
        info.ParameterName = "@Salary";
        info.ColumnSize = uint.MaxValue;
        info.DataType = tagVariantDataType.sdVT_DECIMAL;
        info.Precision = 15;
        info.Scale = 2;
        vInfo.Add(info);

        bool ok = sql.Prepare(sqlInsert, dr, vInfo.ToArray());


        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {

            //first set of data
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            byte[] bytes = sbBlob.UQueue.GetBuffer();
            vData.Add(bytes);
            vData.Add(wstr);
            vData.Add(254000.2460d);

            //second set of data
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            bytes = sbBlob.UQueue.GetBuffer();
            vData.Add(bytes);
            vData.Add(str);
            vData.Add(20254000.197d);

            //third set of data
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            bytes = sbBlob.UQueue.GetBuffer();
            vData.Add(bytes);
            vData.Add(wstr);
            vData.Add(6254000.5d);

            //execute multiple sets of parameter data in one short
            ok = sql.Execute(vData, er);
        }
    }

    static void TestStoredProcedure(CSqlServer sql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vPData)
    {
        CParameterInfoArray vPInfo = new CParameterInfoArray();

        CParameterInfo info = new CParameterInfo();
        info.ParameterName = "RetVal";
        info.DataType = tagVariantDataType.sdVT_INT;
        info.Direction = tagParameterDirection.pdReturnValue;
        vPInfo.Add(info);

        info = new CParameterInfo();
        info.ParameterName = "@p_company_id";
        info.DataType = tagVariantDataType.sdVT_INT;
        vPInfo.Add(info);
        //return direction can be ignorable

        info = new CParameterInfo();
        info.ParameterName = "@p_sum_salary";
        info.DataType = tagVariantDataType.sdVT_R8;
        info.Direction = tagParameterDirection.pdInputOutput;
        vPInfo.Add(info);

        info = new CParameterInfo();
        info.ParameterName = "@p_last_dt";
        info.DataType = tagVariantDataType.sdVT_DATE;
        info.Direction = tagParameterDirection.pdOutput;
        vPInfo.Add(info);
        bool ok = sql.Prepare("sqltestdb.dbo.sp_TestProc", dr, vPInfo.ToArray());
        //process multiple sets of parameters in one shot
        ok = sql.Execute(vPData, er, (h, v) =>
        {
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> p = ra[ra.Count - 1];
            p.Value.AddRange(v);
        }, (h) =>
        {
            CDBColumnInfoArray v = h.ColumnInfo;
            ra.Add(new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(v, new CDBVariantArray()));
            Console.WriteLine("dbPath={0}, tablePath={1}", v[0].DBPath, v[0].TablePath);
        });
    }

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "sa", "Smash123");
#if DEBUG
        using (CSocketPool<CSqlServer> spSql = new CSocketPool<CSqlServer>(true, 3600 * 1000))
#else
        using (CSocketPool<CSqlServer> spSql = new CSocketPool<CSqlServer>())
#endif
        {
            if (!spSql.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async sql server. Press any key to close the application ......");
                Console.Read();
                return;
            }
            CSqlServer sql = spSql.Seek();
            sql.AttachedClientSocket.Push.OnPublish += (sender, messageSender, group, msg) =>
            {
                if (group[0] == DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID)
                {
                    object[] vMsg = (object[])msg;
                    tagUpdateEvent ue = (tagUpdateEvent)(int)(vMsg[0]);
                    string server = (string)vMsg[1];
                    string user = (string)vMsg[2];
                    string database = (string)vMsg[3];
                    Console.WriteLine("DML event={0}, dbPath={1}, user={2}", ue, server + "." + database, user);
                }
            };
            bool ok = sql.Open("", dr, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
            TestCreateTables(sql);
            ok = sql.Execute("select * from SpatialTable", er, rows, rh);
            ok = sql.Execute("delete from employee;delete from company;delete from test_rare1;delete from SpatialTable;INSERT INTO SpatialTable(mygeometry, mygeography)VALUES(geometry::STGeomFromText('LINESTRING(100 100,20 180,180 180)',0),geography::Point(47.6475,-122.1393,4326))", er);
            ok = sql.Execute("INSERT INTO test_rare1(mybool,mymoney,myxml,myvariant,mydateimeoffset)values(1,23.45,'<sometest />', N'美国总统川普下个星期四','2017-05-02 00:00:00.0000000 -04:00');INSERT INTO test_rare1(mybool,mymoney,myvariant)values(0,1223.45,'This is a test for ASCII string inside sql_variant');INSERT INTO test_rare1(myvariant)values(283.45)", er);
            TestPreparedStatements(sql);
            TestPreparedStatements_2(sql);
            InsertBLOBByPreparedStatement(sql);
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();
            CDBVariantArray vPData = new CDBVariantArray();
            vPData.Add(0); //retval
            vPData.Add(1);
            vPData.Add(21.2);
            vPData.Add(null);
            vPData.Add(0); //retval
            vPData.Add(2);
            vPData.Add(11.42);
            vPData.Add(null);
            TestStoredProcedure(sql, ra, vPData);
            ok = sql.Execute("", er, rows, rh);
            sql.WaitAll();
            Console.WriteLine("Press any key to close the application ......");
            Console.Read();
        }
    }


}

