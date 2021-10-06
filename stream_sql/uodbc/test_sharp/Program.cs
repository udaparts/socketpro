using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;

class Program
{
    static CSqlServer.DResult dr = (handler, res, errMsg) =>
    {
        Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
    };

    static CSqlServer.DExecuteResult er = (handler, res, errMsg, affected, fail_ok, id) =>
    {
        Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg);
    };

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20903, "sa", "Smash123"); //20901 for plugindev
        using (CSocketPool<CSqlServer> spSql = new CSocketPool<CSqlServer>(true))
        {
            if (!spSql.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote async sql server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CSqlServer sql = spSql.Seek();
            bool ok = sql.Open("sqltestdb", dr); //use default database
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();

            CSqlServer.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
            };

            CSqlServer.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };
            TestCreateTables(sql);
            ok = sql.Execute("delete from employee;delete from company;delete from test_rare1;delete from SpatialTable;INSERT INTO SpatialTable(mygeometry, mygeography)VALUES(geometry::STGeomFromText('LINESTRING(100 100,20 180,180 180)',0),geography::Point(47.6475,-122.1393,4326))", er);
            ok = sql.Execute("INSERT INTO test_rare1(mybool,mymoney,myxml,myvariant,mydateimeoffset)values(1,23.45,'<sometest />', N'美国总统川普下个星期四','2017-05-02 00:00:00.0000000 -04:00');INSERT INTO test_rare1(mybool,mymoney,myvariant)values(0,1223.45,'This is a test for ASCII string inside sql_variant');INSERT INTO test_rare1(myvariant)values(283.45)", er);
            TestPreparedStatements(sql);
            TestPreparedStatements_2(sql);
            TestBLOBByPreparedStatement(sql);
            ok = sql.Execute("SELECT * from company;select * from employee;select CONVERT(datetime,SYSDATETIME());select * from test_rare1;select * from SpatialTable", er, r, rh);
            CDBVariantArray vPData = TestStoredProcedure(sql, ra);
            ok = sql.WaitAll();
            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", sql.Outputs * 2);
            CDBVariantArray vPData2 = TestStoredProcedure_2(sql, ra);
            ok = sql.WaitAll();
            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", sql.Outputs * 2);
            CDBVariantArray vPData3 = TestBatch(sql, ra);
            ok = sql.WaitAll();
            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", sql.Outputs * 2);
            int index = 0;
            Console.WriteLine();
            Console.WriteLine("+++++ Start rowsets +++");
            foreach (KeyValuePair<CDBColumnInfoArray, CDBVariantArray> it in ra)
            {
                Console.Write("Statement index = {0}", index);
                if (it.Key.Count > 0)
                    Console.WriteLine(", rowset with columns = {0}, records = {1}.", it.Key.Count, it.Value.Count / it.Key.Count);
                else
                    Console.WriteLine(", no rowset received.");
                ++index;
            }
            Console.WriteLine("+++++ End rowsets +++");
            Console.WriteLine();
            Console.WriteLine("Press any key to close the application ......");
            Console.Read();
        }
    }

    static void TestCreateTables(CSqlServer sql)
    {
        string create_database = "use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE name='sqltestdb')BEGIN CREATE DATABASE sqltestdb; END";
        bool ok = sql.Execute(create_database, er);
        string use_database = "Use sqltestdb";
        ok = sql.Execute(use_database, er);
        string create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company')create table company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)";
        ok = sql.Execute(create_table, er);
        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee')create table employee(EMPLOYEEID bigint PRIMARY KEY NOT NULL,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME2(3)default null,MyIMAGE varbinary(max),DESCRIPTION nvarchar(max),Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        ok = sql.Execute(create_table, er);
        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='test_rare1')CREATE TABLE test_rare1(testid int IDENTITY(1,1)NOT NULL,myguid uniqueidentifier DEFAULT newid()NULL,mydate date DEFAULT getdate()NULL,mybool bit DEFAULT 0 NOT NULL,mymoney money default 0 NULL,mytinyint tinyint default 0 NULL,myxml xml DEFAULT '<myxml_root />' NULL,myvariant sql_variant DEFAULT 'my_variant_default' NOT NULL,mydateimeoffset datetimeoffset(4)NULL,PRIMARY KEY(testid))";
        ok = sql.Execute(create_table, er);
        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='SpatialTable')CREATE TABLE SpatialTable(id int IDENTITY(1,1)NOT NULL,mygeometry geometry NULL,mygeography geography NULL,PRIMARY KEY(id))";
        ok = sql.Execute(create_table, er);
        string drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc')drop proc sp_TestProc";
        ok = sql.Execute(drop_proc, er);
        string create_proc = "CREATE PROCEDURE sp_TestProc(@p_company_id int,@p_sum_salary decimal(15,2)output,@p_last_dt datetime out)as select * from employee where companyid>=@p_company_id;select @p_sum_salary=sum(salary)+@p_sum_salary from employee where companyid>=@p_company_id;select @p_last_dt=SYSDATETIME()";
        ok = sql.Execute(create_proc, er);
        drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestRare1')drop proc sp_TestRare1";
        ok = sql.Execute(drop_proc, er);
        create_proc = "CREATE PROCEDURE sp_TestRare1(@testid int,@myxml xml output,@tuuid uniqueidentifier output,@myvar sql_variant out)as insert into test_rare1(myguid,myxml)values(@tuuid,@myxml);select * from test_rare1 where testid>@testid;select @tuuid=NEWID();select @myvar=N'test_variant_from_sp_TestRare1'+convert(nvarchar(4000),@myxml);select @myxml='<myroot_testrare_from_MS_SQL/>'";
        ok = sql.Execute(create_proc, er);
    }

    static void TestPreparedStatements(CSqlServer sql)
    {
        string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
        bool ok = sql.Prepare(sql_insert_parameter, dr);

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
        string sql_insert_parameter = "INSERT INTO test_rare1(myguid,myxml,myvariant,mydateimeoffset)VALUES(?,?,?,?)";
        bool ok = sql.Prepare(sql_insert_parameter, dr);

        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.Add(Guid.NewGuid());
        vData.Add("<myxmlroot_0 />");
        vData.Add(23.456);
        vData.Add(DateTime.Now);

        //second set
        vData.Add(Guid.NewGuid());
        vData.Add("<myxmlroot_1 />");
        vData.Add("马拉阿歌俱乐部");
        vData.Add(DateTime.Now.AddMinutes(1));

        //third set
        vData.Add(Guid.NewGuid());
        vData.Add("<myxmlroot_2 />");
        vData.Add(1);
        vData.Add(DateTime.Now.AddMinutes(-1));

        ok = sql.Execute(vData, er);
    }

    static void TestBLOBByPreparedStatement(CSqlServer sql)
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
        string sqlInsert = "insert into employee(EmployeeId,CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)";
        bool ok = sql.Prepare(sqlInsert, dr);
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {
            //first set of data
            vData.Add(1);
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(254000.24m);

            //second set of data
            vData.Add(2);
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(str);
            vData.Add(20254000.15m);

            //third set of data
            vData.Add(3);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(6254000.08m);

            //send three sets of parameterized data in one shot for processing
            ok = sql.Execute(vData, er);
        }
    }

    static CDBVariantArray TestBatch(CSqlServer sql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        CDBVariantArray vPData = new CDBVariantArray();

        //first set
        vPData.Add(1.0); //@testid
        vPData.Add("<test_sqlserver_0 />"); //@myxml
        vPData.Add(Guid.NewGuid()); //@tuuid
        vPData.Add(""); //@myvar.
        vPData.Add(1);
        vPData.Add(2.35);//input/output
        vPData.Add(DateTime.Now);

        //second set
        vPData.Add(4.0f); //@testid
        vPData.Add("<test_sqlserver_1 />"); //@myxml
        vPData.Add(Guid.NewGuid()); //@tuuid
        vPData.Add(""); //@myvar.
        vPData.Add(2.5);
        vPData.Add(0.99m);//input/output
        vPData.Add(DateTime.Now);

        //Parameter info array can be ignored for some ODBC drivers like MySQL, MS SQL Server, etc but performance will be degraded for code simplicity
        CSqlServer.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };
        CSqlServer.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };

        CParameterInfo[] vInfo = { new CParameterInfo(), new CParameterInfo(), new CParameterInfo(), new CParameterInfo(), new CParameterInfo(), new CParameterInfo(), new CParameterInfo() };
        vInfo[0].DataType = tagVariantDataType.sdVT_I4;

        vInfo[1].DataType = tagVariantDataType.sdVT_XML;
        vInfo[1].Direction = tagParameterDirection.pdInputOutput;

        vInfo[2].DataType = tagVariantDataType.sdVT_CLSID;
        vInfo[2].Direction = tagParameterDirection.pdInputOutput;

        vInfo[3].DataType = tagVariantDataType.sdVT_VARIANT;
        vInfo[3].Direction = tagParameterDirection.pdOutput;

        vInfo[4].DataType = tagVariantDataType.sdVT_I4;

        vInfo[5].DataType = tagVariantDataType.sdVT_DECIMAL;
        vInfo[5].Precision = 15;
        vInfo[5].Scale = 2;
        vInfo[5].Direction = tagParameterDirection.pdInputOutput;

        vInfo[6].DataType = tagVariantDataType.sdVT_DATE;
        vInfo[6].Direction = tagParameterDirection.pdOutput;

        bool ok = sql.ExecuteBatch(tagTransactionIsolation.tiReadCommited, "select getdate();exec sp_TestRare1 ?,?,?,?;exec sqltestdb.dbo.sp_TestProc ?,?,?", vPData,
            er, r, rh, ";", null, null, true, tagRollbackPlan.rpDefault, vInfo);
        return vPData;
    }

    static CDBVariantArray TestStoredProcedure(CSqlServer sql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        CParameterInfo[] vInfo = { new CParameterInfo(), new CParameterInfo(), new CParameterInfo() };
        vInfo[0].DataType = tagVariantDataType.sdVT_I4;

        vInfo[1].DataType = tagVariantDataType.sdVT_DECIMAL;
        vInfo[1].Precision = 15;
        vInfo[1].Scale = 2;
        vInfo[1].Direction = tagParameterDirection.pdInputOutput;

        vInfo[2].DataType = tagVariantDataType.sdVT_DATE;
        vInfo[2].Direction = tagParameterDirection.pdOutput;

        CDBVariantArray vPData = new CDBVariantArray();
        //first set
        vPData.Add(1);
        vPData.Add(2.35);//input/output
        vPData.Add(DateTime.Now); //input/output

        //second set
        vPData.Add(2);
        vPData.Add(0.99m);//input/output
        vPData.Add(DateTime.Now); //input/output
        //Parameter info array can be ignored for some ODBC drivers like MySQL, MS SQL Server, etc but performance will be degraded for code simplicity
        bool ok = sql.Prepare("exec sp_TestProc ?,?,?", dr, vInfo);
        CSqlServer.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };
        CSqlServer.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        ok = sql.Execute(vPData, er, r, rh);
        return vPData;
    }

    static CDBVariantArray TestStoredProcedure_2(CSqlServer sql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        //vInfo is ignorable for MS SQL server ODBC drivers for code simplicity
        CParameterInfo[] vInfo = { new CParameterInfo(), new CParameterInfo(), new CParameterInfo(), new CParameterInfo() };
        vInfo[0].DataType = tagVariantDataType.sdVT_I4;

        vInfo[1].DataType = tagVariantDataType.sdVT_XML;
        vInfo[1].Direction = tagParameterDirection.pdInputOutput;

        vInfo[2].DataType = tagVariantDataType.sdVT_CLSID;
        vInfo[2].Direction = tagParameterDirection.pdInputOutput;

        vInfo[3].DataType = tagVariantDataType.sdVT_VARIANT;
        vInfo[3].Direction = tagParameterDirection.pdOutput;

        bool ok = sql.Prepare("exec sp_TestRare1 ?,?,?,?", dr, vInfo);

        CDBVariantArray vPData = new CDBVariantArray();
        //first set
        vPData.Add(1); //@testid
        vPData.Add("<test_sqlserver_0 />"); //@myxml
        vPData.Add(Guid.NewGuid()); //@tuuid
        vPData.Add(""); //@myvar.

        //second set
        vPData.Add(4); //@testid
        vPData.Add("<test_sqlserver_1 />"); //@myxml
        vPData.Add(Guid.NewGuid()); //@tuuid
        vPData.Add(""); //@myvar.
        CSqlServer.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };

        CSqlServer.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        ok = sql.Execute(vPData, er, r, rh);
        return vPData;
    }
}
