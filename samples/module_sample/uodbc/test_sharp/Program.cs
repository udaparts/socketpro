using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;


class Program
{
    static COdbc.DResult dr = (handler, res, errMsg) =>
    {
        Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
    };

    static COdbc.DExecuteResult er = (handler, res, errMsg, affected, fail_ok, id) =>
    {
        Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
    };

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "umysql_client", "pwd_for_mysql");

        using (CSocketPool<COdbc> spOdbc = new CSocketPool<COdbc>())
        {
            if (!spOdbc.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async odbc server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            COdbc odbc = spOdbc.Seek();
            bool ok = odbc.Open("dsn=ToMySQL;uid=root;pwd=Smash123", dr);
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();

            COdbc.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
            };

            COdbc.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };
            TestCreateTables(odbc);
            ok = odbc.Execute("delete from employee;delete from company", er);
            TestPreparedStatements(odbc);
            InsertBLOBByPreparedStatement(odbc);
            ok = odbc.Execute("SELECT * from company;select * from employee;select curtime()", er, r, rh);
            CDBVariantArray vPData = new CDBVariantArray();
            //first set
            vPData.Add(1);
            vPData.Add(0);
            vPData.Add(0);

            //second set
            vPData.Add(2);
            vPData.Add(0);
            vPData.Add(0);
            TestStoredProcedure(odbc, ra, vPData);
            ok = odbc.WaitAll();

            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", odbc.Outputs * 2);

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

    static void TestCreateTables(COdbc odbc)
    {
        string create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
        bool ok = odbc.Execute(create_database, er);
        string create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income decimal(15,2) not null)";
        ok = odbc.Execute(create_table, er);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique, CompanyId bigint not null, name CHAR(64) NOT NULL, JoinDate DATETIME default null, IMAGE MEDIUMBLOB, DESCRIPTION MEDIUMTEXT, Salary decimal(15,2), FOREIGN KEY(CompanyId) REFERENCES company(id))";
        ok = odbc.Execute(create_table, er);
        string create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int, out p_sum_salary double, out p_last_dt datetime) BEGIN select * from employee where companyid >= p_company_id; select sum(salary) into p_sum_salary from employee where companyid >= p_company_id; select now() into p_last_dt;END";
        ok = odbc.Execute(create_proc, er);
    }

    static void TestPreparedStatements(COdbc odbc)
    {
        string sql_insert_parameter = "INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
        bool ok = odbc.Prepare(sql_insert_parameter, dr);

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

        ok = odbc.Execute(vData, er);
    }

    static void InsertBLOBByPreparedStatement(COdbc odbc)
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
        string sqlInsert = "insert into employee(CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?)";
        bool ok = odbc.Prepare(sqlInsert, dr);
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
            ok = odbc.Execute(vData, er);
        }
    }

    static void TestStoredProcedure(COdbc odbc, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vPData)
    {
        bool ok = odbc.Prepare("{call sp_TestProc(?, ?, ?)}", dr);
        COdbc.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };

        COdbc.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        ok = odbc.Execute(vPData, er, r, rh);
    }
}
