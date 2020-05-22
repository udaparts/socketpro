using System;
using SocketProAdapter.ClientSide;
using System.Data;

class Program
{
    static void Main(string[] args)
    {
        //set a two-dimensional array of socket connection contexts
#if PocketPC
        CConnectionContext[,] ccs = new CConnectionContext[1, 1];
#else
        CConnectionContext[,] ccs = new CConnectionContext[System.Environment.ProcessorCount, 1];
#endif
        int threads = ccs.GetLength(0);
        int sockets_per_thread = ccs.GetLength(1);
        for (int n = 0; n < threads; ++n)
        {
            for (int j = 0; j < sockets_per_thread; ++j)
            {
                string ipAddress;
                if (j == 0)
                    ipAddress = "192.168.1.111";
                else
                    ipAddress = "localhost";
                ccs[n, j] = new CConnectionContext(ipAddress, 20901, "adoclient", "password4AdoClient");
            }
        }

        using (CSocketPool<RAdo> spAdo = new CSocketPool<RAdo>(true)) //true -- automatic reconnecting
        {
            //start a pool of sockets
            if (!spAdo.StartSocketPool(ccs))
            {
                Console.WriteLine("No socket connection");
                return;
            }

            RAdo ado = spAdo.Seek();

            //process two requests one by one with synchronous communication style
            DataSet ds = ado.GetDataSet("select * from dimProduct", "select * from dimAccount");
            Console.WriteLine("Dataset returned with {0} tables", ds.Tables.Count);
            DataTable dt = ado.GetDataTable("select * from dimCustomer");
            Console.WriteLine("Datatable returned with columns = {0}, rows = {1}", dt.Columns.Count, dt.Rows.Count);

            //send two requests in parallel with asynchronous communication style
            RAdo ado1 = spAdo.Seek();
            bool ok = ado1.SendRequest(radoConst.idGetDataTableRAdo, "select * from dimCustomer", (ar) =>
            {
                Console.WriteLine("Datatable returned with columns = {0}, rows = {1}", ado1.CurrentDataTable.Columns.Count, ado1.CurrentDataTable.Rows.Count);
            });

            RAdo ado2 = spAdo.Seek();
            ok = ado2.SendRequest(radoConst.idGetDataSetRAdo, "select * from dimProduct", "select * from dimAccount", (ar) =>
            {
                Console.WriteLine("Dataset returned with {0} tables", ado2.CurrentDataSet.Tables.Count);
            });
            //ok = ado1.WaitAll() && ado2.WaitAll();
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}

