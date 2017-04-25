
using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Data;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("");
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "umysql_client", "pwd_for_mysql");
        Console.WriteLine("Asynchronous execution (0) or synchronous execution (1) ?");
        bool sync = (Console.ReadKey().KeyChar != '0');
        using (CSocketPool<COdbc> spOdbc = new CSocketPool<COdbc>())
        {
            if (!spOdbc.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async ODBC server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            Console.WriteLine("");
            Console.WriteLine("Computing ......");
            COdbc odbc = spOdbc.Seek();
            CAsyncDBHandler.DResult dr = (handler, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
            };
            uint obtained = 0;
            bool ok = odbc.Open("dsn=ToMySQL;uid=root;pwd=Smash123", dr);
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
            odbc.Execute("use sakila", er);
            ok = odbc.WaitAll();
            obtained = 0;
            string sql = "select * from " + tableName;
            if (filter.Length > 0)
            {
                sql += " where " + filter;
            }
            uint count = 10000;
            DateTime start = DateTime.Now;
            for (uint n = 0; n < count; ++n)
            {
                ok = odbc.Execute(sql, er, r, rh);
                if (sync && ok)
                    ok = odbc.WaitAll();
                if (!ok)
                    break;
            }
            if (!sync && ok)
                ok = odbc.WaitAll();
            double diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} millseconds for {1} requests", diff, obtained);
            Console.WriteLine("Press any key to close the application ......");
            Console.ReadLine();
        }
    }
}

