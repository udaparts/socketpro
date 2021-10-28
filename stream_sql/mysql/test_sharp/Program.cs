using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Threading.Tasks;

using KeyValue = System.Collections.Generic.KeyValuePair<SocketProAdapter.UDB.CDBColumnInfoArray, SocketProAdapter.UDB.CDBVariantArray>;

class Program
{
    static readonly string m_wstr;
    static readonly string m_str;

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
#if FOR_MIDDLE_SERVER
        CConnectionContext cc = new CConnectionContext(host, 20901, "root", "Smash123");
#else
        CConnectionContext cc = new CConnectionContext(host, 20902, "root", "Smash123");
#endif
        using (CSocketPool<CMysql> spMysql = new CSocketPool<CMysql>())
        {
            //spMysql.QueueName = "qmysql";
            if (!spMysql.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote async mysql server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CMysql mysql = spMysql.Seek();
            CDBVariantArray vData = null;
            List<KeyValue> ra = new List<KeyValue>();
            CMysql.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValue item = ra[last];
                item.Value.AddRange(rowData);
            };

            CMysql.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
                KeyValue item = new KeyValue(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };
            try
            {
                //stream all requests with in-line batching for the best network efficiency
#if FOR_MIDDLE_SERVER
                //enable in-line query batching for better performance if plugin and MySQL/Mariadb are located at different machines
                var tOpen = mysql.open("", SocketProAdapter.UDB.DB_CONSTS.USE_QUERY_BATCHING);
#else
                var tOpen = mysql.open("");
#endif
                var vT = TestCreateTables(mysql);
                var tDs = mysql.execute("delete from employee;delete from company");
                var tP0 = TestExecuteEx(mysql);
                var tP1 = TestBLOBByPreparedStatement(mysql);
                var tSs = mysql.execute("SELECT * from company;select * from employee;select curtime(6)", r, rh);
                var tStore = TestStoredProcedureByExecuteEx(mysql, ra);
                var tB = TestBatch(mysql, ra, out vData);
                Console.WriteLine();

                Console.WriteLine("All SQL requests are streamed, and waiting results in order ......");
                Console.WriteLine(tOpen.Result);
                foreach (var t in vT)
                {
                    Console.WriteLine(t.Result);
                }
                Console.WriteLine(tDs.Result);
                Console.WriteLine(tP0.Result);
                Console.WriteLine(tP1.Result);
                Console.WriteLine(tSs.Result);
                Console.WriteLine(tStore.Result);
                Console.WriteLine("There are {0} output data returned", 2 * 2);
                Console.WriteLine(tB.Result);
                Console.WriteLine("There are {0} output data returned", 2 * 3);
            }
            catch (AggregateException ex)
            {
                foreach (Exception e in ex.InnerExceptions)
                {
                    //An exception from server (CServerError), Socket closed after sending a request (CSocketError) or request canceled (CSocketError),
                    Console.WriteLine(e);
                }
            }
            catch (CSocketError ex)
            {
                //Socket is already closed before sending a request
                Console.WriteLine(ex);
            }
            catch (Exception ex)
            {
                //bad operations such as invalid arguments, bad operations and de-serialization errors, and so on
                Console.WriteLine(ex);
            }
            int index = 0;
            Console.WriteLine();
            Console.WriteLine("+++++ Start rowsets +++");
            foreach (KeyValue it in ra)
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

    static Program()
    {
        m_wstr = ""; //make test data
        while (m_wstr.Length < 128 * 1024)
        {
            m_wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        m_str = ""; //make test data
        while (m_str.Length < 256 * 1024)
        {
            m_str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
    }

    static Task<CAsyncDBHandler.SQLExeInfo>[] TestCreateTables(CMysql mysql)
    {
        Task<CAsyncDBHandler.SQLExeInfo>[] v = new Task<CAsyncDBHandler.SQLExeInfo>[4];
        string create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
        v[0] = mysql.execute(create_database);
        string create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        v[1] = mysql.execute(create_table);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        v[2] = mysql.execute(create_table);
        string create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(20,2),out p_last_dt datetime(6))BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now(6)into p_last_dt;END";
        v[3] = mysql.execute(create_proc);
        return v;
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestExecuteEx(CMysql mysql)
    {
        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.Add(1);
        vData.Add("Google Inc.");
        vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.Add(66000700000.15m);

        //second set
        vData.Add("Microsoft Inc.");
        vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.Add(93600800000.24);

        //third set
        vData.Add("Apple Inc.");
        vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");

        string sql = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);" +
            "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,?);" +
            "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234000600000.75)";

        return mysql.execute(sql, vData);
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestBatch(CMysql mysql, List<KeyValue> ra, out CDBVariantArray vData)
    {
        //sql with delimiter '|'
        string sql = @" delete from employee;delete from company|
                        INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|
                        insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|
                        SELECT * from company;select * from employee;select curtime(6)|
                        call sp_TestProc(?,?,?)";
        vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {
            //first set
            vData.Add(1);
            vData.Add("Google Inc.");
            vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
            vData.Add(66000000000.15);
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(254000.26);
            vData.Add(1); //input
            vData.Add(1.4); //input-output
            vData.Add(0); //output

            //second set
            vData.Add(2);
            vData.Add("Microsoft Inc.");
            vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
            vData.Add(93600000000.37);
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(m_str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_str);
            vData.Add(20254000.85);
            vData.Add(2); //input
            vData.Add(2.5); //input-output
            vData.Add(0); //output

            //third set
            vData.Add(3);
            vData.Add("Apple Inc.");
            vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
            vData.Add(234000000000.09);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(6254000.55);
            vData.Add(0); //input
            vData.Add(4.5); //input-output
            vData.Add(0); //output
        }
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
            KeyValue item = new KeyValue(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        //first, start a transaction with ReadCommited isolation 
        //second, execute delete from employee;delete from company
        //third, prepare and execute three sets of
        //       INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        //fourth, prepare and execute three sets of 
        //insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        //fifth, SELECT * from company;select * from employee;select curtime()
        //sixth, prepare and three sets of call sp_TestProc(?,?,?)
        //last, commit transaction if there is no error, and rollback if there is one or more errors
        return mysql.executeBatch(tagTransactionIsolation.tiReadCommited, sql, vData, r, rh, "|");
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestBLOBByPreparedStatement(CMysql mysql)
    {
        mysql.Prepare("insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)");
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {
            //first set of data
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(254000.0);

            //second set of data
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(m_str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_str);
            vData.Add(20254000.0);

            //third set of data
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(6254000.0);

            //send three sets of parameterized data in one shot for processing
            return mysql.execute(vData);
        }
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestStoredProcedureByExecuteEx(CMysql mysql, List<KeyValue> ra)
    {
        CMysql.DRows r = (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValue item = ra[last];
            item.Value.AddRange(rowData);
        };
        CMysql.DRowsetHeader rh = (handler) =>
        {
            //rowset header comes here
            KeyValue item = new KeyValue(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        };
        //process multiple sets of parameters in one shot & pay attention to out for output parameters
        string sql = "call sp_TestProc(1,? out,? out);select curtime(6);" +
            "call sp_TestProc(2,? out,? out);call sp_TestProc(3,? out,? out);select 1,2,3";
        return mysql.execute(sql, new CDBVariantArray() { 45673.45, 0, 7345.25, 0, 16345.75, 0 }, r, rh);
    }
}
