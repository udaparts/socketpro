
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        const int sessions_per_host = 2;
        const int cycles = 10000;
        string[] vHost = { "localhost", "192.168.2.172" };
        using (CSocketPool<CSqlite> sp = new CSocketPool<CSqlite>())
        {
            sp.QueueName = "ar_sharp"; //set a local message queue to backup requests for auto fault recovery
            CConnectionContext[,] ppCc = new CConnectionContext[1, vHost.Length * sessions_per_host]; //one thread enough
            for (int n = 0; n < vHost.Length; ++n)
            {
                for (int j = 0; j < sessions_per_host; ++j)
                {
                    ppCc[0, n * sessions_per_host + j] = new CConnectionContext(vHost[n], 20901, "AClientUserId", "Mypassword");
                }
            }
            bool ok = sp.StartSocketPool(ppCc);
            if (!ok)
            {
                Console.WriteLine("There is no connection and press any key to close the application ......");
                Console.Read(); return;
            }
            string sql = "SELECT max(amount), min(amount), avg(amount) FROM payment";
            Console.WriteLine("Input a filter for payment_id");
            string filter = Console.ReadLine();
            if (filter.Length > 0) sql += (" WHERE " + filter);
            var v = sp.AsyncHandlers;
            foreach (var h in v)
            {
                ok = h.Open("sakila.db", (hsqlite, res, errMsg) =>
                {
                    if (res != 0)
                        Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                });
            }
            int returned = 0;
            double dmax = 0.0, dmin = 0.0, davg = 0.0;
            SocketProAdapter.UDB.CDBVariantArray row = new SocketProAdapter.UDB.CDBVariantArray();
            CAsyncDBHandler.DExecuteResult er = (h, res, errMsg, affected, fail_ok, lastId) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                {
                    dmax += double.Parse(row[0].ToString());
                    dmin += double.Parse(row[1].ToString());
                    davg += double.Parse(row[2].ToString());
                }
                ++returned;
            };
            CAsyncDBHandler.DRows r = (h, vData) =>
            {
                row.Clear();
                row.AddRange(vData);
            };
            CSqlite sqlite = sp.SeekByQueue(); //get one handler for querying one record
            ok = sqlite.Execute(sql, er, r);
            ok = sqlite.WaitAll();
            Console.WriteLine("Result: max = {0}, min = {1}, avg = {2}", dmax, dmin, davg);
            returned = 0;
            dmax = 0.0; dmin = 0.0; davg = 0.0;
            Console.WriteLine("Going to get {0} queries for max, min and avg", cycles);
            for (int n = 0; n < cycles; ++n)
            {
                sqlite = sp.SeekByQueue();
                ok = sqlite.Execute(sql, er, r);
            }
            foreach (var h in v)
            {
                ok = h.WaitAll();
            }
            Console.WriteLine("Returned = {0}, max = {1}, min = {2}, avg = {3}", returned, dmax, dmin, davg);
            Console.WriteLine("Press any key to close the application ......"); Console.Read();
        }
    }
}
