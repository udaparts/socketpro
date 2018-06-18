
using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "usqlite_client", "password_for_usqlite");
        using (CSocketPool<CSqlite> spSqlite = new CSocketPool<CSqlite>())
        {
            //start a socket pool with 1 thread hosting 1 non-blocking socket
            if (!spSqlite.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async sqlite server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CSqlite sqlite = spSqlite.Seek();

            //open a global database at server side because an empty string is given
            bool ok = sqlite.Open("", (handler, res, errMsg) =>
            {
                Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
            });

            //prepare two test tables, COMPANY and EMPLOYEE
            TestCreateTables(sqlite);

            //a container for receiving all tables data
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> lstRowset = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();

            ok = sqlite.BeginTrans(); //start manual transaction

            //test both prepare and query statements
            TestPreparedStatements(sqlite, lstRowset);

            //test both prepare and query statements involved with reading and updating BLOB and large text
            InsertBLOBByPreparedStatement(sqlite, lstRowset);

            ok = sqlite.EndTrans(); //end manual transaction
            TestBatch(sqlite, lstRowset);
            sqlite.WaitAll();

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

    static void TestBatch(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        CDBVariantArray vParam = new CDBVariantArray();
        vParam.Add(1); //ID
        vParam.Add(2); //EMPLOYEEID
        //there is no manual transaction if isolation is tiUnspecified
        bool ok = sqlite.ExecuteBatch(tagTransactionIsolation.tiUnspecified,
            "Select datetime('now');select * from COMPANY where ID=?;select * from EMPLOYEE where EMPLOYEEID=?",
            vParam, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}",
                affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        }, (handler, rowData) =>
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
        ok = sqlite.ExecuteBatch(tagTransactionIsolation.tiReadCommited,
            "Select datetime('now');select * from COMPANY where ID=?;Select datetime('now');select * from EMPLOYEE where EMPLOYEEID=?",
            vParam, (handler, res, errMsg, affected, fail_ok, id) =>
            {
                Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}",
                    affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
            }, (handler, rowData) =>
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

    static void TestPreparedStatements(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        //a complex SQL statement combined with query and insert prepare statements
        string sql_insert_parameter = "Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
        bool ok = sqlite.Prepare(sql_insert_parameter, (handler, res, errMsg) =>
        {
            Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
        });

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
        ok = sqlite.Execute(vData, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        }, (handler, rowData) =>
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

    static void InsertBLOBByPreparedStatement(CSqlite sqlite, List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra)
    {
        string wstr = "";
        //prepare junk data for testing
        while (wstr.Length < 128 * 1024)
        {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        string str = "";
        while (str.Length < 256 * 1024)
        {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }

        //a complex SQL statement combined with two insert and query prepare statements
        string sqlInsert = "insert or replace into employee(EMPLOYEEID,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?);select * from employee where employeeid=?";
        bool ok = sqlite.Prepare(sqlInsert, (handler, res, errMsg) =>
        {
            Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
        });
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
            vData.Add(254000.0);
            vData.Add(1);

            //second set of data
            vData.Add(2);
            vData.Add(1); //google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            sbBlob.UQueue.SetSize(0);
            sbBlob.Save(str);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(str);
            vData.Add(20254000.0);
            vData.Add(2);

            //third set of data
            vData.Add(3);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);
            sbBlob.Save(wstr);
            vData.Add(sbBlob.UQueue.GetBuffer());
            vData.Add(wstr);
            vData.Add(6254000.0);
            vData.Add(3);
        }
        //send three sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        }, (handler, rowData) =>
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

    static void TestCreateTables(CSqlite sqlite)
    {
        string create_table = "CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)";
        bool ok = sqlite.Execute(create_table, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        });
        create_table = "CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
        ok = sqlite.Execute(create_table, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        });
    }
}
