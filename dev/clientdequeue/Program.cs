using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Threading;

class Program
{
    const uint COUNT = 100000;
    static CConnectionContext m_cc;
    static ulong DoDequeue(CServerQueue sq, uint count)
    {
        ulong res = 0;
        bool ok = sq.SendRequest(SQueueConst.idDequeueCServerQueue, count, true, (ar) =>
        {
            ar.Load(out res);
        });
        sq.WaitAll();
        return res;
    }

    static void Enqueue()
    {
        uint n;
        bool ok;
        using (CSocketPool<CServerQueue> sqPool = new CSocketPool<CServerQueue>())
        {
            ok = sqPool.StartSocketPool(m_cc, 1, 1);
            CServerQueue sq = sqPool.AsyncHandlers[0];
            if (ok)
            {
                DateTime dtPrev = DateTime.Now;
                CAsyncServiceHandler.DAsyncResultHandler ash = null;
                for (n = 0; n < COUNT; ++n)
                {
                    ok = sq.SendRequest(SQueueConst.idEnqueueCServerQueue, "Effect of reduced power consumption", "Hello ", n, ash);
                    if (n != 0 && n % 500 == 0)
                        ok = sq.WaitAll();
                }
                ok = sq.WaitAll();
                Console.WriteLine("Time requied for enqueue = " + (DateTime.Now - dtPrev).TotalMilliseconds);

                sq.SendRequest(SQueueConst.idQueryTimes, (ar) =>
                {
                    long lSwitchTime;
                    long lEnqueueTime;
                    ar.Load(out lSwitchTime).Load(out lEnqueueTime);
                    Console.WriteLine("Time for switch = " + lSwitchTime + ", time for enqueue = " + lEnqueueTime);
                });
                ok = sq.WaitAll();
            }
            else
            {
                Console.WriteLine("Connection error = " + sq.AttachedClientSocket.ErrorMsg);
            }
        }
    }

    static void TestOne()
    {
        bool ok;
        System.Threading.Thread worker = new Thread(Enqueue);
        worker.Start();
        worker.Join();
        using (CSocketPool<CServerQueue> sqPool = new CSocketPool<CServerQueue>())
        {
            ok = sqPool.StartSocketPool(m_cc, 1, 1);
            CServerQueue sq = sqPool.AsyncHandlers[0];
            if (ok)
            {
                sq.ResultReturned += (sender, reqId, UQueue) =>
                {
                    bool processed = false;
                    switch (reqId)
                    {
                        case SQueueConst.idMyMessage:
                            {
                                string name;
                                string message;
                                int aInt;
                                UQueue.Load(out name).Load(out message).Load(out aInt);
                                //Console.WriteLine(message + name + ", int = " + aInt);
                                processed = true;
                            }
                            break;
                        default:
                            break;
                    }
                    return processed;
                };

                sq.AttachedClientSocket.BaseRequestProcessed += (sender, reqId) =>
                {
                    if (reqId == tagBaseRequestID.idMessageQueued)
                    {
                        Console.WriteLine("Messages available now");
                        DoDequeue(sq, 512);
                    }
                    else
                    {
                        Console.WriteLine("Base request id = " + reqId.ToString());
                    }
                };

                DateTime dtPrev = DateTime.Now;
                ulong res = DoDequeue(sq, 512);
                while (res > 0)
                {
                    res = DoDequeue(sq, 512);
                }
                Console.WriteLine("Time requied for dequeue = " + (DateTime.Now - dtPrev).TotalMilliseconds);
            }
            else
            {
                Console.WriteLine("Connection error = " + sq.AttachedClientSocket.ErrorCode);
            }
            string str = Console.ReadLine();
        }
    }

    static void EnqueueTwo()
    {

    }

    static void DoDequeueTwo()
    {

    }

    static void TestTwo()
    {
        bool ok;
        System.Threading.Thread worker = new Thread(Enqueue);
        worker.Start();
        worker.Join();
        using (CSocketPool<CServerQueue> sqPool = new CSocketPool<CServerQueue>())
        {
            ok = sqPool.StartSocketPool(m_cc, 1, 1);
            CServerQueue sq = sqPool.AsyncHandlers[0];
            if (ok)
            {
            }
            else
            {
                Console.WriteLine("Connection error = " + sq.AttachedClientSocket.ErrorCode);
            }
            string str = Console.ReadLine();
        }
    }

    static void Main(string[] args)
    {
        uint number;
        m_cc = new CConnectionContext("127.0.0.1", 20901, "SocketPro", "Password");
        Console.WriteLine("Input a number (odd for test 1, and even for test 2) .....");
        number = uint.Parse(Console.ReadLine());
        if ((number % 2) == 1)
            TestOne();
        else
            TestTwo();
    }
}

