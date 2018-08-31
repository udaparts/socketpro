
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static byte[] TEST_QUEUE_KEY;
    const ushort idMessage0 = (ushort)tagBaseRequestID.idReservedTwo + 100;
    const ushort idMessage1 = (ushort)tagBaseRequestID.idReservedTwo + 101;
    const ushort idMessage2 = (ushort)tagBaseRequestID.idReservedTwo + 102;
    const ushort idMessage3 = (ushort)tagBaseRequestID.idReservedTwo + 103;
    const ushort idMessage4 = (ushort)tagBaseRequestID.idReservedTwo + 104;

    static Program()
    {
        TEST_QUEUE_KEY = System.Text.Encoding.UTF8.GetBytes("queue_name_0");
    }

    static bool TestEnqueue(CAsyncQueue aq)
    {
        bool ok = true;
        Console.WriteLine("Going to enqueue 1024 messages ......");
        for (int n = 0; n < 1024; ++n)
        {
            string str = n + " Object test";
            ushort idMessage;
            switch (n % 3)
            {
                case 0:
                    idMessage = idMessage0;
                    break;
                case 1:
                    idMessage = idMessage1;
                    break;
                default:
                    idMessage = idMessage2;
                    break;
            }
            //enqueue two unicode strings and one int
            ok = aq.Enqueue(TEST_QUEUE_KEY, idMessage, "SampleName", str, n);
            if (!ok)
                break;
        }
        return ok;
    }

    static void TestDequeue(CAsyncQueue aq)
    {
        //prepare callback for parsing messages dequeued from server side
        aq.ResultReturned += (sender, idReq, q) =>
        {
            bool processed = true;
            switch (idReq)
            {
                case idMessage0:
                case idMessage1:
                case idMessage2:
                    Console.Write("message id={0}", idReq);
                    {
                        string name, str;
                        int index;
                        //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
                        q.Load(out name).Load(out str).Load(out index);
                        Console.WriteLine(", name={0}, str={1}, index={2}", name, str, index);
                    }
                    break;
                case idMessage3:
                    {
                        string s1, s2;
                        q.Load(out s1).Load(out s2);
                        Console.WriteLine("{0} {1}", s1, s2);
                    }
                    break;
                case idMessage4:
                    {
                        bool b;
                        double dbl;
                        string s;
                        q.Load(out b).Load(out dbl).Load(out s);
                        Console.WriteLine("b= {0}, d= {1}, s= {2}", b, dbl, s);
                    }
                    break;
                default:
                    processed = false;
                    break;
            }
            return processed;
        };

        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = (sender, messageCount, fileSize, messages, bytes) =>
        {
            Console.WriteLine("Total message count={0}, queue file size={1}, messages dequeued={2}, message bytes dequeued={3}", messageCount, fileSize, messages, bytes);
            if (messageCount > 0)
            {
                //there are more messages left at server queue, we re-send a request to dequeue
                sender.Dequeue(TEST_QUEUE_KEY, sender.LastDequeueCallback);
            }
        };

        Console.WriteLine("Going to dequeue messages ......");
        bool ok = aq.Dequeue(TEST_QUEUE_KEY, d);

        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        ok = aq.Dequeue(TEST_QUEUE_KEY, d);
    }

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20902, "async_queue_client", "pwd_for_async_queue");
        using (CSocketPool<CAsyncQueue> spAq = new CSocketPool<CAsyncQueue>())
        {
            if (!spAq.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async queue server");
                Console.WriteLine("Press any key to close the application ......");
                Console.Read();
                return;
            }
            CAsyncQueue aq = spAq.Seek();

            //Optionally, you can enqueue messages with transaction style by calling the methods StartQueueTrans and EndQueueTrans in pair
            aq.StartQueueTrans(TEST_QUEUE_KEY, (sender, errCode) =>
            {
                //error code could be one of CAsyncQueue.QUEUE_OK, CAsyncQueue.QUEUE_TRANS_ALREADY_STARTED, ......
            });
            TestEnqueue(aq);

            //test message batching
            using (CScopeUQueue sb = new CScopeUQueue())
            {
                CUQueue q = sb.UQueue;
                CAsyncQueue.BatchMessage(idMessage3, "Hello", "World", q);
                CAsyncQueue.BatchMessage(idMessage4, true, 234.456, "MyTestWhatever", q);
                aq.EnqueueBatch(TEST_QUEUE_KEY, q, (sender, res) =>
                {
                    System.Diagnostics.Debug.Assert(res == 2);
                });
            }
            aq.EndQueueTrans(false);
            TestDequeue(aq);
            aq.WaitAll();

            //get a queue key two parameters, message count and queue file size by default option oMemoryCached
            aq.FlushQueue(TEST_QUEUE_KEY, (sender, messageCount, fileSize) =>
            {
                Console.WriteLine("Total message count={0}, queue file size={1}", messageCount, fileSize);
            });

            aq.GetKeys((sender, keys) =>
            {
                keys = null;
            });

            aq.CloseQueue(TEST_QUEUE_KEY, (sender, errCode) =>
            {
                //error code could be one of CAsyncQueue.QUEUE_OK, CAsyncQueue.QUEUE_TRANS_ALREADY_STARTED, ......
            });

            Console.WriteLine("Press any key to close the application ......");
            Console.Read();
        }
    }
}

