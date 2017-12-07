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
        Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
    };

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20903, "sa", "Smash123");
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
                msg = null;
            };

            bool ok = sql.Open("AdventureWorks2012", dr, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
            CParameterInfoArray vInfo = new CParameterInfoArray();
            string sqlPrepare = "INSERT INTO Person.Address(AddressLine1,AddressLine2,City,StateProvinceID,PostalCode,SpatialLocation,rowguid,ModifiedDate)VALUES(@AddressLine1,@AddressLine2,@City,@StateProvinceID,@PostalCode,@SpatialLocation,@rowguid,@ModifiedDate)";
            CParameterInfo info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_BSTR;
            info.ColumnSize = 60;
            info.ParameterName = "@AddressLine1";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_BSTR;
            info.ColumnSize = 60;
            info.ParameterName = "@AddressLine2";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_BSTR;
            info.ColumnSize = 30;
            info.ParameterName = "@City";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_INT;
            info.ParameterName = "@StateProvinceID";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_BSTR;
            info.ColumnSize = 15;
            info.ParameterName = "@PostalCode";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_BSTR;
            info.ColumnSize = 128;
            info.ParameterName = "@SpatialLocation";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_CLSID;
            info.ParameterName = "@rowguid";
            vInfo.Add(info);
            info = new CParameterInfo();
            info.DataType = tagVariantDataType.sdVT_DATE;
            info.ParameterName = "@ModifiedDate";
            vInfo.Add(info);
            ok = sql.Prepare(sqlPrepare, dr, vInfo.ToArray());
            ok = sql.BeginTrans(tagTransactionIsolation.tiReadCommited, dr);

            /*
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
            ok = sql.Execute("delete from employee;delete from company", er);
            TestPreparedStatements(sql);
            InsertBLOBByPreparedStatement(sql);
            ok = sql.Execute("SELECT * from company;select * from employee;select curtime()", er, r, rh);
            CDBVariantArray vPData = new CDBVariantArray();
            //first set
            vPData.Add(1);
            vPData.Add(1.4);
            vPData.Add(0);

            //second set
            vPData.Add(2);
            vPData.Add(2.5);
            vPData.Add(0);
            TestStoredProcedure(sql, ra, vPData);
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
 */
            ok = sql.EndTrans(tagRollbackPlan.rpDefault, dr);
            Console.WriteLine("Press any key to close the application ......");
            Console.Read();
            sql.Close();
            sql.WaitAll();
        }
    }

    static void TestCreateTables(CSqlServer sql)
    {
        string create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
        bool ok = sql.Execute(create_database, er);
        string create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        ok = sql.Execute(create_table, er);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        ok = sql.Execute(create_table, er);
        string create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(20,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END";
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
        string sqlInsert = "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
        bool ok = sql.Prepare(sqlInsert, dr);
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {
            //first set of data
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(254000.0);

            //second set of data
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(str);
            vData.Add(20254000.0);

            //third set of data
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(6254000.0);

            //send three sets of parameterized data in one shot for processing
            ok = sql.Execute(vData, er);
        }
    }

    static void TestStoredProcedure(CSqlServer sql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vPData)
    {
        bool ok = sql.Prepare("call sp_TestProc(?,?,?)", dr);
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
    }
}

