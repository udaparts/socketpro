﻿using System;
using System.Collections.Generic;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Data;
using System.Text;

class Program
{
    static CAsyncDBHandler.DResult dr = (handler, res, errMsg) =>
    {
        if (res != 0)
            Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
    };

    static void Main(string[] args)
    {
        Console.WriteLine("SocketPro performance test against a remote MS SQL backend DB");
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Wide Arae Netork ? (1 or 0)");
        string str = Console.ReadLine();
        int wan = 0;
        try
        {
            wan = int.Parse(str);
        }
        finally { }
        using (CSocketPool<CSqlServer> spSql = new CSocketPool<CSqlServer>())
        {
            //20901 -- SocketPro MS SQL middle tier plugin; 20903 -- SocketPro MS SQL DB server plugin
            CConnectionContext cc = new CConnectionContext(host, 20901, "sa", "Smash123");
            if (!spSql.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote helloworld server. Press any key to close the application ......");
                Console.Read();
                return;
            }
            Console.WriteLine("Database name: ");
            string dbName = Console.ReadLine();
            Console.WriteLine("Table name: ");
            string tableName = Console.ReadLine();
            Console.WriteLine("sql filter: ");
            string filter = Console.ReadLine();
            Console.WriteLine("Asynchronous execution (0) or synchronous execution (1) ?");
            bool sync = (Console.ReadKey().KeyChar != '0');
            Console.WriteLine("");
            Console.WriteLine("Computing ......");
            CSqlServer mssql = spSql.Seek();

            uint obtained = 0;
            bool ok = mssql.Open(dbName, dr, DB_CONSTS.USE_QUERY_BATCHING);
#if USE_DATATABLE
            List<KeyValuePair<CDBColumnInfoArray, DataTable>> ra = new List<KeyValuePair<CDBColumnInfoArray, DataTable>>();
#else
            List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>> ra = new List<KeyValuePair<CDBColumnInfoArray, CDBVariantArray>>();
#endif
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
#if USE_DATATABLE
                KeyValuePair<CDBColumnInfoArray, DataTable> item = ra[last];
                CAsyncDBHandler.AppendRowDataIntoDataTable(rowData, item.Value);
#else
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = ra[last];
                item.Value.AddRange(rowData);
#endif
            };
            CAsyncDBHandler.DRowsetHeader rh = (handler) =>
            {
                //rowset header comes here
#if USE_DATATABLE
                DataTable dt = CAsyncDBHandler.MakeDataTable(handler.ColumnInfo);
                KeyValuePair<CDBColumnInfoArray, DataTable> item = new KeyValuePair<CDBColumnInfoArray, DataTable>(handler.ColumnInfo, dt);
#else
                KeyValuePair<CDBColumnInfoArray, CDBVariantArray> item = new KeyValuePair<CDBColumnInfoArray, CDBVariantArray>(handler.ColumnInfo, new CDBVariantArray());
#endif
                ra.Add(item);
            };
            obtained = 0;
            string sql = "select * from " + tableName;
            if (filter.Length > 0)
            {
                sql += " where " + filter;
            }
            int count = (wan == 0) ? 50000 : 5000;
            DateTime start = DateTime.Now;
            for (int n = 0; n < count; ++n)
            {
                ok = mssql.Execute(sql, er, r, rh);
                if (sync && ok)
                    ok = mssql.WaitAll();
                if (!ok)
                    break;
            }
            if (!sync && ok)
                ok = mssql.WaitAll();
            double diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} milliseconds for {1} query requests", diff, obtained);

            //run the sample socketpro/stream_sql/sqlserver/test_sharp first to create the test table before the following inserting test
            ok = mssql.Execute("USE sqltestdb;delete from company where id > 3");
            ok = mssql.WaitAll();
            TestParamPerformance(wan, sync, mssql, er);
            Console.WriteLine("Press any key to close the application ......");
            Console.ReadLine();
        }
    }

    private static void TestParamPerformance(int wan, bool sync, CSqlServer mssql, CAsyncDBHandler.DExecuteResult er)
    {
        int index = 0;
        int count = (wan == 0) ? 250000 : 25000;
        Console.WriteLine();
        Console.WriteLine("Going to insert {0} records into the table sqltestdb.company", count);
        string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
        bool ok = mssql.Prepare(sql_insert_parameter, dr);
        DateTime start = DateTime.Now;
        CDBVariantArray vData = new CDBVariantArray();
        ok = mssql.BeginTrans();
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
            //send 200 sets of parameter data onto server for processing in batch
            if (200 == index)
            {
                ok = mssql.Execute(vData, er);
                ok = mssql.EndTrans();
                vData.Clear();
                if (sync)
                    ok = mssql.WaitAll();
                //Console.WriteLine("Commit {0} records into the table sqltestdb.company", index);
                ok = mssql.BeginTrans();
                index = 0;
            }
        }
        if (vData.Count > 0)
        {
            ok = mssql.Execute(vData, er);
            //Console.WriteLine("Commit {0} records into the table sqltestdb.company", index);
        }
        ok = mssql.EndTrans();
        ok = mssql.WaitAll();
        double diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} milliseconds for {1} insert requests", diff, count);
        Console.WriteLine();
    }
}
