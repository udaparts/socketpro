using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.UDB;
using SocketProAdapter.ClientSide;
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
        CConnectionContext cc = new CConnectionContext(host, 20901, "postgres", "Smash123");

        using (CSocketPool<CPostgres> spPostgres = new CSocketPool<CPostgres>())
        {
            if (!spPostgres.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote async postgres server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CPostgres postgres = spPostgres.Seek();
            List<KeyValue> ra = new List<KeyValue>();
            CPostgres.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValue item = ra[last];
                item.Value.AddRange(rowData);
            };

            CPostgres.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
                KeyValue item = new KeyValue(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            };

            try
            {
                //stream all requests with in-line batching for the best network efficiency
                //enable in-line query batching for better performance if plugin and PostgreSQL are located at different machines
                var tOpen = postgres.open("", DB_CONSTS.USE_QUERY_BATCHING);

                var vT = TestCreateTables(postgres);
                var tDs = postgres.execute("delete from employee;delete from company");
                var tP0 = TestExecuteEx(postgres, r, rh);
                var tPE = TestExecuteEx2(postgres, r, rh);
                var tD = postgres.execute("delete from employee;delete from company");
                var t0 = TestByPreparedStatement(postgres, r, rh);
                var t1 = TestBLOBByPreparedStatement(postgres, r, rh);

                Console.WriteLine("All SQL requests are streamed, and waiting results in order ......");
                Console.WriteLine(tOpen.Result);
                foreach (var t in vT)
                {
                    Console.WriteLine(t.Result);
                }
                Console.WriteLine(tDs.Result);
                Console.WriteLine(tP0.Result);
                Console.WriteLine(tPE.Result);
                Console.WriteLine(tD.Result);
                Console.WriteLine(t0.Result);
                Console.WriteLine(t1.Result);
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

    static Task<CAsyncDBHandler.SQLExeInfo>[] TestCreateTables(CPostgres postgres)
    {
        Task<CAsyncDBHandler.SQLExeInfo>[] v = new Task<CAsyncDBHandler.SQLExeInfo>[4];
        string create_table = "CREATE TABLE IF NOT EXISTS Company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income double precision not null)";
        v[0] = postgres.execute(create_table);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigserial not null,CompanyId bigint not null,name char(64)NOT NULL,JoinDate TIMESTAMP default null,IMAGE bytea,DESCRIPTION text,Salary DECIMAL(14,2),FOREIGN KEY(CompanyId)REFERENCES public.company(id))";
        v[1] = postgres.execute(create_table);
        string create_func = "create or replace function sp_TestProc(p_company_id int,inout p_sum_salary decimal(14,2),out p_last_dt timestamp)as $func$ select sum(salary)+p_sum_salary,localtimestamp from employee where companyid>=p_company_id $func$ language sql";
        v[2] = postgres.execute(create_func);
        string create_proc = "CREATE OR REPLACE PROCEDURE test_sp(id integer,INOUT mymoney numeric,INOUT dt timestamp) LANGUAGE 'plpgsql' AS $BODY$ BEGIN select mymoney+sum(salary),localtimestamp into mymoney,dt from employee where companyid>id;END $BODY$";
        v[3] = postgres.execute(create_proc);
        return v;
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestExecuteEx(CPostgres postgres, CPostgres.DRows r, CPostgres.DRowsetHeader rh)
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

        string sql = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);select 5;" +
            "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,?);select CURRENT_TIMESTAMP(6);" +
            "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234000600000.75)returning *";
        return postgres.execute(sql, vData, r, rh);
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestByPreparedStatement(CPostgres postgres, CPostgres.DRows r, CPostgres.DRowsetHeader rh)
    {
        postgres.Prepare("INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)returning *");

        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.Add(1);
        vData.Add("Google Inc.");
        vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.Add(66000700000.15m);

        //second set
        vData.Add(2);
        vData.Add("Microsoft Inc.");
        vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.Add(93600800000.24);

        //third set
        vData.Add(3);
        vData.Add("Apple Inc.");
        vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.Add(234000600000.75);
        return postgres.execute(vData, r, rh);
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestExecuteEx2(CPostgres postgres, CPostgres.DRows r, CPostgres.DRowsetHeader rh)
    {
        using (CScopeUQueue sb = new CScopeUQueue())
        {
            CDBVariantArray vData = new CDBVariantArray();

            //first set
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sb.Save(m_wstr);
            vData.Add(sb.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(254000.07);

            //second set of data
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sb.UQueue.SetSize(0);
            sb.Save(m_str);
            vData.Add(sb.UQueue.GetBuffer());
            vData.Add(m_str);
            vData.Add(20254000.15);

            //third set of data
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sb.Save(m_wstr);
            vData.Add(sb.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(6254000.12);

            vData.Add(1276.54m); //for call test_sp

            string sql = "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?)returning *;" +
                "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?)returning *;select 245;" +
                "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(2,?,?,?,?,?);call test_sp(1,?,null)";
            return postgres.execute(sql, vData, r, rh);
        }
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestBLOBByPreparedStatement(CPostgres postgres, CPostgres.DRows r, CPostgres.DRowsetHeader rh)
    {
        postgres.Prepare("insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)returning *");
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
            vData.Add(254070.25);

            //second set of data
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(m_str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_str);
            vData.Add(20254090.73m);

            //third set of data
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(6254000.16);

            //send three sets of parameterized data in one shot for processing
            return postgres.execute(vData, r, rh);
        }
    }
}
