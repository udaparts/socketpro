
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static byte[] TEST_QUEUE_KEY;
    const ushort idMessage0 = (ushort)tagBaseRequestID.idReservedTwo + 100;
    const ushort idMessage1 = (ushort)tagBaseRequestID.idReservedTwo + 101;
    const ushort idMessage2 = (ushort)tagBaseRequestID.idReservedTwo + 102;

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
                default:
                    processed = false;
                    break;
            }
            return processed;
        };

        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = (asyncqueue, messageCount, fileSize, messages, bytes) =>
        {
            Console.WriteLine("Total message count={0}, queue file size={1}, messages dequeued={2}, message bytes dequeued={3}", messageCount, fileSize, messages, bytes);
            if (messageCount > 0)
            {
                //there are more messages left at server queue, we re-send a request to dequeue
                asyncqueue.Dequeue(TEST_QUEUE_KEY, asyncqueue.LastDequeueCallback);
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
        CConnectionContext cc = new CConnectionContext(host, 20901, "async_queue_client", "pwd_for_async_queue");
        using (CSocketPool<CAsyncQueue> spAq = new CSocketPool<CAsyncQueue>())
        {
            if (!spAq.StartSocketPool(cc, 1, 1))
            {
                Console.WriteLine("Failed in connecting to remote async queue server");
                Console.WriteLine("Press key ENTER to close the application ......");
                Console.ReadLine();
                return;
            }
            CAsyncQueue aq = spAq.Seek();

            TestEnqueue(aq);
            TestDequeue(aq);

            Console.WriteLine("Press key ENTER to close the application ......");
            Console.ReadLine();
        }
    }
}