﻿using System;
using System.Collections.Generic;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Data;

#if USE_DATATABLE
using KeyValue = System.Collections.Generic.KeyValuePair<SocketProAdapter.UDB.CDBColumnInfoArray, System.Data.DataTable>;
#else
using KeyValue = System.Collections.Generic.KeyValuePair<SocketProAdapter.UDB.CDBColumnInfoArray, SocketProAdapter.UDB.CDBVariantArray>;
#endif

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("SocketPro performance test against a remote MySQL backend DB");
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Database name: ");
        string dbName = Console.ReadLine();
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20902, "root", "Smash123");
        Console.WriteLine("Asynchronous execution (0) or synchronous execution (1) ?");
        bool sync = (Console.ReadKey().KeyChar != '0');
        using (CSocketPool<CMysql> spMysql = new CSocketPool<CMysql>())
        {
            if (!spMysql.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote helloworld server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            Console.WriteLine("");
            Console.WriteLine("Computing ......");
            CMysql mysql = spMysql.Seek();
            CAsyncDBHandler.DResult dr = (handler, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
            };
            uint obtained = 0;
            bool ok = mysql.Open(dbName, dr);
            List<KeyValue> ra = new List<KeyValue>();
            CAsyncDBHandler.DExecuteResult er = (handler, res, errMsg, affected, fail_ok, id) =>
            {
                if (res != 0)
                    Console.WriteLine("fails = {0}, oks = {1}, res = {2}, errMsg: {3}", (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg);
                ra.Clear();
                ++obtained;
            };
            CAsyncDBHandler.DRows r = (handler, rowData) =>
            {
                //rowset data come here
                int last = ra.Count - 1;
                KeyValue item = ra[last];
#if USE_DATATABLE
                CAsyncDBHandler.AppendRowDataIntoDataTable(rowData, item.Value);
#else
                item.Value.AddRange(rowData);
#endif
            };
            CAsyncDBHandler.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
#if USE_DATATABLE
                DataTable dt = CAsyncDBHandler.MakeDataTable(handler.ColumnInfo);
                KeyValue item = new KeyValue(handler.ColumnInfo, dt);
#else
                KeyValue item = new KeyValue(handler.ColumnInfo, new CDBVariantArray());
#endif
                ra.Add(item);
            };
            obtained = 0;
            string sql = "select * from " + tableName;
            if (filter.Length > 0)
            {
                sql += " where " + filter;
            }
            int count = 10000;
            DateTime start = DateTime.Now;
            for (int n = 0; n < count; ++n)
            {
                ok = mysql.Execute(sql, er, r, rh);
                if (sync && ok)
                    ok = mysql.WaitAll();
                if (!ok)
                    break;
            }
            if (!sync && ok)
                ok = mysql.WaitAll();
            double diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} milliseconds for {1} query requests", diff, obtained);

            //you need to compile and run the sample project test_sharp before running the below code
            ok = mysql.Execute("USE mysqldb;delete from company where id > 3");
            string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
            ok = mysql.Prepare(sql_insert_parameter, dr);
            ok = mysql.WaitAll();
            int index = 0;
            count = 50000;
            Console.WriteLine();
            Console.WriteLine("Going to insert {0} records into the table mysqldb.company", count);
            start = DateTime.Now;
            CDBVariantArray vData = new CDBVariantArray();
            ok = mysql.BeginTrans();
            for (int n = 0; n < count; ++n)
            {
                vData.Add(n + 4);
                int data = (n % 3);
                switch (data)
                {
                    case 0:
                        vData.Add("Google Inc.");
                        vData.Add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
                        vData.Add(66000000000.12);
                        break;
                    case 1:
                        vData.Add("Microsoft Inc.");
                        vData.Add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
                        vData.Add(93600000001.24);
                        break;
                    default:
                        vData.Add("Apple Inc.");
                        vData.Add("1 Infinite Loop, Cupertino, CA 95014, USA");
                        vData.Add(234000000002.17);
                        break;
                }
                ++index;
                //send 2000 sets of parameter data onto server for processing in batch
                if (2000 == index)
                {
                    ok = mysql.Execute(vData, er);
                    ok = mysql.EndTrans();
                    vData.Clear();
                    Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
                    ok = mysql.BeginTrans();
                    index = 0;
                }
            }
            if (vData.Count > 0)
            {
                ok = mysql.Execute(vData, er);
                Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
            }
            ok = mysql.EndTrans();
            ok = mysql.WaitAll();
            diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} milliseconds for {1} insert requests", diff, count);
            Console.WriteLine();
            Console.WriteLine("Press any key to close the application ......");
            Console.ReadLine();
        }
    }
}

