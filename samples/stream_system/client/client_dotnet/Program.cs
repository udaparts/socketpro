using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Threading.Tasks;

using CMaster = SocketProAdapter.CMasterPool<CWebAsyncHandler, SocketProAdapter.CDataSet>;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Sakila.payment filter: ");
        string filter = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20911, "SomeUserId", "A_Password_For_SomeUserId", tagEncryptionMethod.TLSv1);

        //CA file is located at the directory ../socketpro/bin
        CClientSocket.SSL.SetVerifyLocation("ca.cert.pem");

        using (CMaster master = new CMaster(""))
        {
            master.DoSslServerAuthentication += (pool, cs) =>
            {
                int ret;
                IUcert cert = cs.UCert;
                string res = cert.Verify(out ret);
                return (ret == 0);
            };
            bool ok = master.StartSocketPool(cc, 4, 1);
            if (!ok)
            {
                Console.WriteLine("Failed in connecting to remote middle tier server, and press any key to close the application ......");
                Console.ReadLine();
                return;
            }

            CDataSet cache = CMaster.Cache; //accessing real-time update cache

            CWebAsyncHandler handler = master.Seek();
            ulong call_index = handler.GetMasterSlaveConnectedSessions((index, m, s) =>
            {
                Console.WriteLine("master connections: {0}, slave connections: {1}", m, s);
            });
            call_index = handler.QueryPaymentMaxMinAvgs(filter, (index, mma, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("QueryPaymentMaxMinAvgs error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("QueryPaymentMaxMinAvgs max: {0}, min: {1}, avg: {2}", mma.Max, mma.Min, mma.Avg);
            });

            CDBVariantArray vData = new CDBVariantArray();
            vData.Add(1); //Google company id
            vData.Add("Ted Cruz");
            vData.Add(DateTime.Now);
            vData.Add(1); //Google company id
            vData.Add("Donald Trump");
            vData.Add(DateTime.Now);
            vData.Add(2); //Microsoft company id
            vData.Add("Hillary Clinton");
            vData.Add(DateTime.Now);

            TaskCompletionSource<bool> tcs = new TaskCompletionSource<bool>();
            call_index = handler.UploadEmployees(vData, (index, res, errMsg, vId) =>
            {
                if (res != 0)
                    Console.WriteLine("UploadEmployees Error code = {0}, , error message = {1}", res, errMsg);
                else
                {
                    foreach (object id in vId)
                    {
                        Console.WriteLine("Last id: " + id);
                    }
                }
                tcs.SetResult(true);
            }, (index) =>
            {
                Console.WriteLine("Socket closed or request cancelled");
                tcs.SetResult(true);
            });
            if (call_index != 0)
            {
                if (!tcs.Task.Wait(5000))
                    Console.WriteLine("The above requests are not completed in 5 seconds");
            }
            else
                Console.WriteLine("Socket already closed before sending request");
            Console.WriteLine("Press a key to test random returning ......");
            Console.ReadLine();
            ss.CMaxMinAvg sum_mma = new ss.CMaxMinAvg();
            DateTime start = DateTime.Now;
            uint returned = 0;
            for (uint n = 0; n < 10000; ++n)
            {
                handler = master.Seek(); //find a handler from a pool of sockets
                if (handler == null)
                {
                    Console.WriteLine("All sockets already closed");
                    break;
                }
                call_index = handler.QueryPaymentMaxMinAvgs(filter, (index, mma, res, errMsg) =>
                {
                    if (res != 0)
                        Console.WriteLine("QueryPaymentMaxMinAvgs call index: {0}, error code: {1}, error message: {2}", index, res, errMsg);
                    else
                    {
                        sum_mma.Avg += mma.Avg;
                        sum_mma.Max += mma.Max;
                        sum_mma.Min += mma.Min;
                        //Console.WriteLine("QueryPaymentMaxMinAvgs call index = " + index);
                    }
                    ++returned;
                });
            }
            foreach (CWebAsyncHandler h in master.AsyncHandlers)
            {
                ok = h.WaitAll();
            }
            Console.WriteLine("Time required: {0} seconds for {1} requests", (DateTime.Now - start).TotalSeconds, returned);
            Console.WriteLine("QueryPaymentMaxMinAvgs sum_max: {0}, sum_min: {1}, sum_avg: {2}", sum_mma.Max, sum_mma.Min, sum_mma.Avg);

            Console.WriteLine("Press a key to test sequence returning ......");
            Console.ReadLine();

            CWebAsyncHandler.DRentalDateTimes rdt = (index, dates, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("GetRentalDateTimes call index: {0}, error code: {1}, error message: ", index, res, errMsg);
                else if (dates.rental_id == 0)
                    Console.WriteLine("GetRentalDateTimes call index: {0} rental_id={1} not available", index, dates.rental_id);
                else
                    Console.WriteLine("GetRentalDateTimes call index: {0} rental_id={1} and dates ({2}, {3}, {4})", index, dates.rental_id, dates.Rental, dates.Return, dates.LastUpdate);
            };
            handler = master.Seek();
            if (handler != null)
            {
                for (int n = 0; n < 1000; ++n)
                {
                    call_index = handler.GetRentalDateTimes(n + 1, rdt);
                }
                handler.WaitAll();
            }
            Console.WriteLine("Press a key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
