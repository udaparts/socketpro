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
        Console.WriteLine("Remote middle tier host: ");
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
            bool ok = master.StartSocketPool(cc, 1, 1);
            if (!ok)
            {
                Console.WriteLine("Failed in connecting to remote middle tier server, and press any key to close the application ......");
                Console.ReadLine();
                return;
            }

            CDataSet cache = master.Cache; //accessing real-time update cache
            CWebAsyncHandler handler = master.Seek();
            ok = handler.GetMasterSlaveConnectedSessions((m, s) =>
            {
                Console.WriteLine("master connections: {0}, slave connections: {1}", m, s);
            });
            ok = handler.QueryPaymentMaxMinAvgs(filter, (mma, res, errMsg) =>
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
            ok = handler.UploadEmployees(vData, (res, errMsg, vId) =>
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
            }, (h, canceled) =>
            {
                Console.WriteLine("Socket closed or request cancelled");
                tcs.SetResult(true);
            });
            if (ok)
            {
                if (!tcs.Task.Wait(5000))
                    Console.WriteLine("The above requests are not completed in 5 seconds");
            }
            else
                Console.WriteLine("Socket already closed before sending request");

            Console.WriteLine("Press ENTER key to test requests parallel processing and fault tolerance at server side ......");
            Console.ReadLine();
            ss.CMaxMinAvg sum_mma = new ss.CMaxMinAvg();
            DateTime start = DateTime.Now;
            uint returned = 0;
            for (uint n = 0; n < 10000; ++n)
            {
                ok = handler.QueryPaymentMaxMinAvgs(filter, (mma, res, errMsg) =>
                {
                    if (res != 0)
                        Console.WriteLine("QueryPaymentMaxMinAvgs error code: {0}, error message: {1}", res, errMsg);
                    else
                    {
                        sum_mma.Avg += mma.Avg;
                        sum_mma.Max += mma.Max;
                        sum_mma.Min += mma.Min;
                    }
                    ++returned;
                });
            }
            ok = handler.WaitAll();
            Console.WriteLine("Time required: {0} seconds for {1} requests", (DateTime.Now - start).TotalSeconds, returned);
            Console.WriteLine("QueryPaymentMaxMinAvgs sum_max: {0}, sum_min: {1}, sum_avg: {2}", sum_mma.Max, sum_mma.Min, sum_mma.Avg);

            Console.WriteLine("Press ENTER key to test requests server parallel processing, fault tolerance and sequence returning ......");
            Console.ReadLine();
            long prev_rental_id = 0;
            CWebAsyncHandler.DRentalDateTimes rdt = (dates, res, errMsg) =>
            {
                if (res != 0)
                {
                    Console.WriteLine("GetRentalDateTimes call error code: {0}, error message: {1}", res, errMsg);
                    prev_rental_id = 0;
                }
                else if (dates.rental_id == 0)
                {
                    Console.WriteLine("GetRentalDateTimes call rental_id={0} not available", dates.rental_id);
                    prev_rental_id = 0;
                }
                else
                {
                    if (0 == prev_rental_id || dates.rental_id == prev_rental_id + 1)
                        Console.WriteLine("GetRentalDateTimes call rental_id={0} and dates ({1}, {2}, {3})", dates.rental_id, dates.Rental, dates.Return, dates.LastUpdate);
                    else
                        Console.WriteLine("****** GetRentalDateTimes returned out of order ******");
                    prev_rental_id = dates.rental_id;
                }
            };
            //all requests should be returned in sequence (max rental_id = 16049)
            for (int n = 0; n < 1000; ++n)
            {
                ok = handler.GetRentalDateTimes(n + 1, rdt);
            }
            handler.WaitAll();
            Console.WriteLine("Press a key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
