using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Threading.Tasks;
using System.Collections.Generic;

using CMaster = SocketProAdapter.CMasterPool<CWebAsyncHandler, SocketProAdapter.CDataSet>;
using ss;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote middle tier host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Sakila.payment filter: ");
        //for example: payment_id between 1 and 49
        string filter = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20911, "SomeUserId", "A_Password_For_SomeUserId", tagEncryptionMethod.TLSv1);
#if WIN32_64
#else
        //CA file is located at the directory ../socketpro/bin
        bool ok = CClientSocket.SSL.SetVerifyLocation("ca.cert.pem");
#endif
        using (CMaster master = new CMaster(""))
        {
            master.DoSslServerAuthentication += (pool, cs) =>
            {
                int ret;
                IUcert cert = cs.UCert;
                string res = cert.Verify(out ret);
                return (ret == 0);
            };
            //master.QueueName = "mcqueue";
            ok = master.StartSocketPool(cc, 1);
            if (!ok)
            {
                Console.WriteLine("No connection to remote middle tier server, and press a key to kill the demo ......");
                Console.ReadLine();
                return;
            }
            //accessing real-time update cache
            CDataSet cache = master.Cache;
            CWebAsyncHandler handler = master.Seek();
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

            try
            {
                var tms = handler.sendRequest(Consts.idGetMasterSlaveConnectedSessions);
                var tmma = handler.sendRequest(Consts.idQueryMaxMinAvgs, filter);
                var tue = handler.sendRequest(Consts.idUploadEmployees, vData);
                var sb = tms.Result;
                Console.WriteLine("master connections: {0}, slave connections: {1}", sb.Load<uint>(), sb.Load<uint>());
                sb = tmma.Result;
                Console.WriteLine("QueryPaymentMaxMinAvgs");
                int res = sb.Load<int>();
                string errMsg = sb.Load<string>();
                if (res != 0)
                    Console.WriteLine("\terror code: {0}, error message: {1}", res, errMsg);
                else
                {
                    CMaxMinAvg mma = sb.Load<CMaxMinAvg>();
                    Console.WriteLine("\tmax: {0}, min: {1}, avg: {2}", mma.Max, mma.Min, mma.Avg);
                }
                if (tue.Wait(5000))
                {
                    sb = tue.Result;
                    res = sb.Load<int>();
                    errMsg = sb.Load<string>();
                    Console.WriteLine("UploadEmployees");
                    if (res != 0)
                        Console.WriteLine("\tError code: {0}, message: {1}", res, errMsg);
                    else
                    {
                        var vId = sb.Load<CInt64Array>();
                        foreach (object id in vId)
                        {
                            Console.WriteLine("\tLast id: " + id);
                        }
                    }
                }
                else
                {
                    Console.WriteLine("The request UploadEmployees not completed in 5 seconds");
                }
                Console.WriteLine("Press ENTER key to test requests server parallel processing ......");
                Console.ReadLine();
                CMaxMinAvg sum_mma = new CMaxMinAvg();
                Queue<Task<CScopeUQueue>> qT = new Queue<Task<CScopeUQueue>>();
                DateTime start = DateTime.Now;
                for (uint n = 0; n < 10000; ++n)
                    qT.Enqueue(handler.sendRequest(Consts.idQueryMaxMinAvgs, filter));

                int count = qT.Count;
                Console.WriteLine("QueryPaymentMaxMinAvgs");
                while (qT.Count > 0)
                {
                    sb = qT.Dequeue().Result;
                    res = sb.Load<int>();
                    errMsg = sb.Load<string>();
                    if (res != 0)
                        Console.WriteLine("error code: {0}, message: {1}", res, errMsg);
                    else
                    {
                        CMaxMinAvg mma = sb.Load<CMaxMinAvg>();
                        sum_mma.Avg += mma.Avg;
                        sum_mma.Max += mma.Max;
                        sum_mma.Min += mma.Min;
                    }
                    sb.Dispose();
                }
                Console.WriteLine("\tTime required: {0} seconds for {1} requests", (DateTime.Now - start).TotalSeconds, count);
                Console.WriteLine("\tsum_max: {0}, sum_min: {1}, sum_avg: {2}", sum_mma.Max, sum_mma.Min, sum_mma.Avg);

                Console.WriteLine("Press ENTER key to test server parallel processing and sequence returning ......");
                Console.ReadLine();
                for (long n = 0; n < 16000; ++n)
                    qT.Enqueue(handler.sendRequest(Consts.idGetRentalDateTimes, n + 1));
                long prev_rental_id = 0;
                Console.WriteLine("GetRentalDateTimes:");
                while (qT.Count > 0)
                {
                    sb = qT.Dequeue().Result;
                    CRentalDateTimes dates = sb.Load<CRentalDateTimes>();
                    res = sb.Load<int>();
                    errMsg = sb.Load<string>();
                    if (res != 0)
                    {
                        Console.WriteLine("\terror code: {0}, message: {1}", res, errMsg);
                    }
                    else if (dates.LastUpdate.Ticks == 0 && dates.Rental.Ticks == 0 && dates.Return.Ticks == 0)
                    {
                        Console.WriteLine("\trental_id: {0} not available", dates.rental_id);
                    }
                    else
                    {
                        if (0 == prev_rental_id || dates.rental_id == prev_rental_id + 1)
                        {
                            //Console.WriteLine("rental_id={0} and dates ({1}, {2}, {3})", dates.rental_id, dates.Rental, dates.Return, dates.LastUpdate);
                        }
                        else
                            Console.WriteLine("\t****** returned out of order ******");
                    }
                    prev_rental_id = dates.rental_id;
                    sb.Dispose();
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
                Console.WriteLine(ex);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
            Console.WriteLine("Press a key to kill the demo ......");
            Console.ReadLine();
        }
    }
}
