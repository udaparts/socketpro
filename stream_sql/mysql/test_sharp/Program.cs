using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;


class Program
{
    static CMysql.DResult dr = (handler, res, errMsg) =>
    {
        Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
    };

    static CMysql.DExecuteResult er = (handler, res, errMsg, affected, fail_ok, id) =>
    {
        Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
    };

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20902, "root", "Smash123");
        using (CSocketPool<CMysql> spMysql = new CSocketPool<CMysql>(true, 600000))
        {
            if (!spMysql.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async mysql server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CMysql mysql = spMysql.Seek();
            bool ok = mysql.Open("", dr);
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();

            CMysql.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
            };

            CMysql.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };
            TestCreateTables(mysql);
            ok = mysql.Execute("delete from employee;delete from company", er);
            TestPreparedStatements(mysql);
            InsertBLOBByPreparedStatement(mysql);
            ok = mysql.Execute("SELECT * from company;select * from employee;select curtime()", er, r, rh);

            CDBVariantArray vPData = TestStoredProcedure(mysql, ra);
            ok = mysql.WaitAll();
            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", mysql.Outputs * 2);

            CDBVariantArray vData = TestBatch(mysql, ra);
            ok = mysql.WaitAll();
            Console.WriteLine();
            Console.WriteLine("There are {0} output data returned", mysql.Outputs * 3);

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

    static void TestCreateTables(CMysql mysql)
    {
        string create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
        bool ok = mysql.Execute(create_database, er);
        string create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        ok = mysql.Execute(create_table, er);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        ok = mysql.Execute(create_table, er);
        string create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(20,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END";
        ok = mysql.Execute(create_proc, er);
    }

    static void TestPreparedStatements(CMysql mysql)
    {
        string sql_insert_parameter = "INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
        bool ok = mysql.Prepare(sql_insert_parameter, dr);

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

        ok = mysql.Execute(vData, er);
    }

    static CDBVariantArray TestBatch(CMysql mysql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        //sql with delimiter '|'
        string sql = @" delete from employee;delete from company|
                        INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|
                        insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|
                        SELECT * from company;select * from employee;select curtime()|
                        call sp_TestProc(?,?,?)";
        string wstr = ""; //make test data
        while (wstr.Length < 128 * 1024) {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        string str = ""; //make test data
        while (str.Length < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue()) {
            //first set
            vData.Add(1);
            vData.Add("Google Inc.");
            vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
            vData.Add(66000000000.0);
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(254000.0);
            vData.Add(1);
            vData.Add(1.4);
            vData.Add(0);

            //second set
            vData.Add(2);
            vData.Add("Microsoft Inc.");
            vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
            vData.Add(93600000000.0);
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(str);
            vData.Add(20254000.0);
            vData.Add(2);
            vData.Add(2.5);
            vData.Add(0);

            //third set
            vData.Add(3);
            vData.Add("Apple Inc.");
            vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
            vData.Add(234000000000.0);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(6254000.0);
            vData.Add(0);
            vData.Add(4.5);
            vData.Add(0);
        }
        CMysql.DRows r = (handler, rowData) => {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };
        CMysql.DRowsetHeader rh = (handler) => {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        //first, execute delete from employee;delete from company
        //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        //third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        //fourth, SELECT * from company;select * from employee;select curtime()
        //last, three sets of call sp_TestProc(?,?,?)
        bool ok = mysql.ExecuteBatch(tagTransactionIsolation.tiUnspecified, sql, vData, er, r, rh, (h) => {
            //called before rh, r and er
            //ra.Clear();
        }, null, tagRollbackPlan.rpDefault, (h, canceled)=> {
            //called when canceling or socket closed if client queue is NOT used
        }, "|");
        return vData;
    }

    static void InsertBLOBByPreparedStatement(CMysql mysql)
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
        string sqlInsert = "insert into employee(CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values (?, ?, ?, ?, ?, ?)";
        bool ok = mysql.Prepare(sqlInsert, dr);
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
            ok = mysql.Execute(vData, er);
        }
    }

    static CDBVariantArray TestStoredProcedure(CMysql mysql, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        CDBVariantArray vPData = new CDBVariantArray();
        //first set
        vPData.Add(1);
        vPData.Add(1.4);
        vPData.Add(0);

        //second set
        vPData.Add(2);
        vPData.Add(2.5);
        vPData.Add(0);

        bool ok = mysql.Prepare("call sp_TestProc(?, ?, ?)", dr);
        CMysql.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        };

        CMysql.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        ok = mysql.Execute(vPData, er, r, rh);
        return vPData;
    }
}

