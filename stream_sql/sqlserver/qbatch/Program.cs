using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Threading.Tasks;

class Program
{
    static readonly string m_wstr;
    static readonly string m_str;

    static Program()
    {
        m_wstr = "";
        while (m_wstr.Length < 128 * 1024)
        {
            m_wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        m_str = "";
        while (m_str.Length < 256 * 1024)
        {
            m_str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
    }

    static CSqlServer.DResult dr = (handler, res, errMsg) =>
    {
        Console.WriteLine("res={0}, em:{1}", res, errMsg);
    };

    static readonly DateTime DT1900 = new DateTime(1900, 1, 1, 0, 0, 0, DateTimeKind.Local);

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        var cc = new CConnectionContext(host, 20901, "sa", "Smash123"); //connection to all_servers
        using (var spSql = new CSocketPool<CSqlServer>())
        {
            if (!spSql.StartSocketPool(cc, 1)) //one pool having one session
            {
                Console.WriteLine("Failed in connecting to remote async sql server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CSqlServer sql = spSql.Seek();
            bool ok = sql.Open("", dr); //use default database
            //creating DB mydevdb, two tables and a stored procedure
            var lstT = TestCreateTables(sql);
            var ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();
            CSqlServer.DRows r = (handler, rowData) => {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
            };

            CSqlServer.DRowsetHeader rh = (handler) => {
                //rowset header comes here
                var item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };
            var v = new CDBVariantArray() {
                1, 23456.23, DT1900,
                3, 1245.76m, DT1900,
                0, 87678.75m, DT1900
            };
            //enable in-line query batching
            ok = sql.Open("", dr, DB_CONSTS.USE_QUERY_BATCHING);
            var vT = new List<Task<CAsyncDBHandler.SQLExeInfo>>(lstT);
            try {
                //parameterzied statements
                vT.Add(sql.execute("delete from employee;delete from company"));
                vT.Add(TestComapny(sql));
                vT.Add(TestEmployee(sql));
                vT.Add(TestStoredProc(sql, v, r, rh));

                //session variables
                vT.Add(sql.execute("delete from employee;delete from company"));
                vT.Add(TestComapnyEx(sql, r, rh));
                vT.Add(TestEmployeeEx(sql, r, rh));
            }
            catch (AggregateException ex) {
                foreach (Exception e in ex.InnerExceptions) {
                    //An exception from server (CServerError), Socket closed
                    //after sending a request (CSocketError) or canceled (CSocketError),
                    Console.WriteLine(e);
                }
            }
            catch (CSocketError ex) {
                //Socket is already closed before sending a request
                Console.WriteLine(ex);
            }
            catch (Exception ex) {
                //bad operations such as invalid arguments,
                //bad operations and de-serialization errors, and so on
                Console.WriteLine(ex);
            }
            foreach (var t in vT) {
                Console.WriteLine(t.Result);
            }
            int index = 0;
            Console.WriteLine();
            Console.WriteLine("+++++ Start rowsets +++");
            foreach (KeyValuePair<CDBColumnInfoArray, CDBVariantArray> it in ra) {
                Console.Write("Statement index = {0}", index);
                if (it.Key.Count > 0) {
                    Console.WriteLine(", rowset with columns = {0}, records = {1}.",
                        it.Key.Count, it.Value.Count / it.Key.Count);
                }
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

    static Task<CSqlServer.SQLExeInfo> TestComapny(CSqlServer sql)
    {
        bool ok = sql.Prepare("INSERT INTO company VALUES(?,?,?,?)", dr);
        var vData = new CDBVariantArray() {
            //first set
            1, "Google Inc.",
            "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
            66000007000,
            //second set
            2, "Microsoft Inc.",
            "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA",
            93600004000.15,
            //third set
            3, "Apple Inc.",
            "1 Infinite Loop, Cupertino, CA 95014, USA",
            234000000000.45m
        };
        return sql.execute(vData);
    }

    static Task<CSqlServer.SQLExeInfo> TestComapnyEx(CSqlServer sql, CSqlServer.DRows r,
        CSqlServer.DRowsetHeader rh)
    {
        var vData = new CDBVariantArray() {
            //first set
            1, "Google Inc.",
            "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
            66000007000,
            //second set
            2, "Microsoft Inc.",
            "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA",
            //third set
            "Apple Inc.",
            "1 Infinite Loop, Cupertino, CA 95014, USA"
        };
        string stmt = "INSERT INTO company VALUES(?,?,?,?);select SYSDATETIME();" +
        "INSERT INTO company VALUES(?,?,?,93600090060.12);select 1,2,3;" +
        "INSERT INTO company OUTPUT Inserted.* VALUES(3,?,?,234000300070.14)";
        return sql.execute(stmt, vData, r, rh);
    }

    static Task<CSqlServer.SQLExeInfo> TestEmployee(CSqlServer sql) {
        string wstr = m_wstr;
        string str = m_str;
        string stmt = "insert into employee values(?,?,?,?,?,?)";
        bool ok = sql.Prepare(stmt, dr);
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue()) {
            //first set of data
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(254000.24f);

            //second set of data
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(str);
            vData.Add(20254060.15);

            //third set of data
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(6254009.28m);

            //send three sets of parameterized data in one shot for processing
            return sql.execute(vData);
        }
    }

    static Task<CSqlServer.SQLExeInfo> TestEmployeeEx(CSqlServer sql, CSqlServer.DRows r,
        CSqlServer.DRowsetHeader rh) {
        string wstr = m_wstr;
        string str = m_str;
        CDBVariantArray vData = new CDBVariantArray();
        CScopeUQueue sbBlob = new CScopeUQueue();
        //first set of data
        vData.Add("Ted Cruz");
        vData.Add(DateTime.Now);
        sbBlob.Save(wstr);
        vData.Add(sbBlob.UQueue.GetBuffer());
        vData.Add(wstr);
        vData.Add(254000.24f);

        //second set of data
        vData.Add("Donald Trump");
        vData.Add(DateTime.Now);
        sbBlob.UQueue.SetSize(0);
        sbBlob.Save(str);
        vData.Add(sbBlob.UQueue.GetBuffer());
        vData.Add(str);
        vData.Add(20254060.15);

        //third set of data
        vData.Add("Hillary Clinton");
        vData.Add(DateTime.Now);
        sbBlob.Save(wstr);
        vData.Add(sbBlob.UQueue.GetBuffer());
        vData.Add(wstr);

        //exec sp_TestProc ?,? out,? out;exec sp_TestProc 0,? out,? out
        vData.Add(1);
        vData.Add(1276.54m);
        vData.Add(DT1900);
        vData.Add(81274.54);
        vData.Add(DT1900);

        var stmt = "insert into employee OUTPUT Inserted.* values(1,?,?,?,?,?);" +
        "insert into employee values(1,?,?,?,?,?);" +
        "insert into employee OUTPUT Inserted.* values(2,?,?,?,?,6254030.42);select 245;" +
        "exec sp_TestProc ?,? out,? out;exec sp_TestProc 0,? out,? out";

        return sql.execute(stmt, vData, r, rh);
    }

    static Task<CSqlServer.SQLExeInfo> TestStoredProc(CSqlServer sql, CDBVariantArray vPData,
        CSqlServer.DRows r, CSqlServer.DRowsetHeader rh) {
        CParameterInfo[] vInfo = { new CParameterInfo(), new CParameterInfo(), new CParameterInfo() };
        vInfo[0].DataType = tagVariantDataType.sdVT_I4;

        CParameterInfo p = vInfo[1];
        p.DataType = tagVariantDataType.sdVT_DECIMAL;
        p.Direction = tagParameterDirection.pdInputOutput;
        p.Precision = 15;
        p.Scale = 2;

        vInfo[2].DataType = tagVariantDataType.sdVT_DATE;
        vInfo[2].Direction = tagParameterDirection.pdOutput;

        bool ok = sql.Prepare("exec sp_TestProc ?,?,?", dr, vInfo);
        return sql.execute(vPData, r, rh);
    }

    static List<Task<CSqlServer.SQLExeInfo>> TestCreateTables(CSqlServer sql) {
        string create_database = "use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE " +
            "name='mydevdb')BEGIN CREATE DATABASE mydevdb;END";
        var t0 = sql.execute(create_database);
        string use_database = "Use mydevdb";
        var t1 = sql.execute(use_database);
        string create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company')" +
            "create table company(ID bigint PRIMARY KEY,name CHAR(64)NOT NULL,ADDRESS " +
            "varCHAR(256)not null,Income float not null)";
        var t2 = sql.execute(create_table);
        create_table = "IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee')create" +
            " table employee(EMPLOYEEID bigint identity PRIMARY KEY,CompanyId bigint not null" +
            ",name CHAR(64)NOT NULL,JoinDate DATETIME2(3),MyIMAGE varbinary(max),DESCRIPTION" +
            " nvarchar(max),Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        var t3 = sql.execute(create_table);
        string drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc')" +
            "drop proc sp_TestProc";
        var t4 = sql.execute(drop_proc);
        string create_proc = "CREATE PROCEDURE sp_TestProc(@p_company_id int,@p_sum_salary " +
            "decimal(15,2)output,@p_last_dt datetime2(3) out)as select * from employee where" +
            " companyid>=@p_company_id;select @p_sum_salary=sum(salary)+@p_sum_salary from " +
            "employee where companyid>=@p_company_id;select @p_last_dt=SYSDATETIME()";
        var t5 = sql.execute(create_proc);
        return new List<Task<CSqlServer.SQLExeInfo>>() { t0, t1, t2, t3, t4, t5};
    }
}
