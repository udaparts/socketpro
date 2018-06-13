
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
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "usqlite_client", "pwd_for_sqlite");
        Console.WriteLine("Asynchronous execution (0) or synchronous execution (1) ?");
        bool sync = (Console.ReadKey().KeyChar != '0');
        using (CSocketPool<CSqlite> spSqlite = new CSocketPool<CSqlite>())
        {
            if (!spSqlite.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote helloworld server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            Console.WriteLine("");
            Console.WriteLine("Computing ......");
            CSqlite sqlite = spSqlite.Seek();
            CAsyncDBHandler.DResult dr = (handler, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("res = {0}, errMsg: {1}", res, errMsg);
            };
            uint obtained = 0;
            bool ok = sqlite.Open("sakila.db", dr);
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
            ok = sqlite.WaitAll();
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
                ok = sqlite.Execute(sql, er, r, rh);
                if (sync && ok)
                    ok = sqlite.WaitAll();
                if (!ok)
                    break;
            }
            if (!sync && ok)
                ok = sqlite.WaitAll();
            double diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} milliseconds for {1} query requests", diff, obtained);

            //you need to compile and run the sample project test_sharp before running the below code
            ok = sqlite.Open("", dr); //open a global database at remote server
            ok = sqlite.Execute("delete from company where id > 3");
            string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
            ok = sqlite.Prepare(sql_insert_parameter, dr);
            ok = sqlite.WaitAll();
            int index = 0;
            count = 50000;
            Console.WriteLine();
            Console.WriteLine("Going to insert {0} records into the table mysqldb.company", count);
            start = DateTime.Now;
            CDBVariantArray vData = new CDBVariantArray();
            ok = sqlite.BeginTrans();
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
                    ok = sqlite.Execute(vData, er);
                    ok = sqlite.EndTrans();
                    vData.Clear();
                    Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
                    ok = sqlite.BeginTrans();
                    index = 0;
                }
            }
            if (vData.Count > 0)
            {
                ok = sqlite.Execute(vData, er);
                Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
            }
            ok = sqlite.EndTrans();
            ok = sqlite.WaitAll();
            diff = (DateTime.Now - start).TotalMilliseconds;
            Console.WriteLine("Time required = {0} milliseconds for {1} insert requests", diff, count);
            Console.WriteLine("Press any key to close the application ......");
            Console.ReadLine();
        }
    }
}
