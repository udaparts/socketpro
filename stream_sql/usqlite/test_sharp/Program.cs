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

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "usqlite_client", "password_for_usqlite");
        using (CSocketPool<CSqlite> spSqlite = new CSocketPool<CSqlite>())
        {
            //start a socket pool with 1 thread hosting 1 non-blocking socket
            if (!spSqlite.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote async sqlite server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CSqlite sqlite = spSqlite.Seek();
            //a container for receiving all tables data
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> lstRowset = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();
            try
            {
                //stream all DB requests with in-line batching for the best network efficiency
                //open a global database at server side because an empty string is given
                var topen = sqlite.open("");
                //prepare two test tables, COMPANY and EMPLOYEE
                Task<CAsyncDBHandler.SQLExeInfo>[] vT = TestCreateTables(sqlite);
                var tbt = sqlite.beginTrans(); //start manual transaction
                //test both prepare and query statements
                var tp0 = TestPreparedStatements(sqlite, lstRowset);
                //test both prepare and query statements involved with reading and updating BLOB and large text
                var tp1 = InsertBLOBByPreparedStatement(sqlite, lstRowset);
                var tet = sqlite.endTrans(); //end manual transaction
                var vB = TestBatch(sqlite, lstRowset);

                Console.WriteLine("All SQL requests streamed, and waiting for their results");
                Console.WriteLine(topen.Result);
                foreach (var e in vT)
                {
                    Console.WriteLine(e.Result);
                }
                Console.WriteLine(tbt.Result);
                Console.WriteLine(tp0.Result);
                Console.WriteLine(tp1.Result);
                Console.WriteLine(tet.Result);
                foreach (var e in vB)
                {
                    Console.WriteLine(e.Result);
                }
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
            //display received rowsets
            int index = 0;
            Console.WriteLine();
            Console.WriteLine("+++++ Start rowsets +++");
            foreach (KeyValuePair<CDBColumnInfoArray, CDBVariantArray> it in lstRowset)
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

    static Task<CAsyncDBHandler.SQLExeInfo>[] TestBatch(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        var v = new Task<CAsyncDBHandler.SQLExeInfo>[2];
        CDBVariantArray vParam = new CDBVariantArray();
        vParam.Add(1); //ID
        vParam.Add(2); //EMPLOYEEID
        //there is no manual transaction if isolation is tiUnspecified
        v[0] = sqlite.executeBatch(tagTransactionIsolation.tiUnspecified,
            "Select datetime('now');select * from COMPANY where ID=?;select * from EMPLOYEE where EMPLOYEEID=?",
            vParam, (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        }, (handler) =>
        {
            //rowset header meta info comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        });
        vParam.Clear();
        vParam.Add(1); //ID
        vParam.Add(2); //EMPLOYEEID
        vParam.Add(2); //ID
        vParam.Add(3); //EMPLOYEEID
        //Same as sqlite.BeginTrans();
        //Select datetime('now');select * from COMPANY where ID=1;select * from COMPANY where ID=2;Select datetime('now');
        //select * from EMPLOYEE where EMPLOYEEID=2;select * from EMPLOYEE where EMPLOYEEID=3
        //ok = sqlite.EndTrans();
        v[1] = sqlite.executeBatch(tagTransactionIsolation.tiReadCommited,
            "Select datetime('now');select * from COMPANY where ID=?;Select datetime('now');select * from EMPLOYEE where EMPLOYEEID=?",
            vParam, (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
            }, (handler) =>
            {
                //rowset header meta info comes here
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
                ra.Add(item);
            });
        return v;
    }

    static Task<CAsyncDBHandler.SQLExeInfo> TestPreparedStatements(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        //a complex SQL statement combined with query and insert prepare statements
        sqlite.Prepare("Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)");

        CDBVariantArray vData = new CDBVariantArray();
        vData.Add(1);
        vData.Add("Google Inc.");
        vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.Add(66000000000.0);

        vData.Add(2);
        vData.Add("Microsoft Inc.");
        vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.Add(93600000000.0);

        vData.Add(3);
        vData.Add("Apple Inc.");
        vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.Add(234000000000.0);

        //send three sets of parameterized data in one shot for processing
        return sqlite.execute(vData, (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        }, (handler) =>
        {
            //rowset header meta info comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        });
    }

    static Task<CAsyncDBHandler.SQLExeInfo> InsertBLOBByPreparedStatement(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        //a complex SQL statement combined with two insert and query prepare statements
        sqlite.Prepare("insert or replace into employee(EMPLOYEEID,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?);select * from employee where employeeid=?");
        CDBVariantArray vData = new CDBVariantArray();
        using (CScopeUQueue sbBlob = new CScopeUQueue())
        {
            //first set of data
            vData.Add(1);
            vData.Add(1); //google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(254000.0);
            vData.Add(1);

            //second set of data
            vData.Add(2);
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(m_str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_str);
            vData.Add(20254000.0);
            vData.Add(2);

            //third set of data
            vData.Add(3);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(m_wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(m_wstr);
            vData.Add(6254000.0);
            vData.Add(3);
        }
        //send three sets of parameterized data in one shot for processing
        return sqlite.execute(vData, (handler, rowData) =>
        {
            //rowset data come here
            int last = ra.Count - 1;
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
            item.Value.AddRange(rowData);
        }, (handler) =>
        {
            //rowset header meta info comes here
            KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
            ra.Add(item);
        });
    }

    static Task<CAsyncDBHandler.SQLExeInfo>[] TestCreateTables(CSqlite sqlite)
    {
        var v = new Task<CAsyncDBHandler.SQLExeInfo>[2];
        v[0] = sqlite.execute("CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)");
        v[1] = sqlite.execute("CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))");
        return v;
    }
}
