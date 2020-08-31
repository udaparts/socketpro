using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Threading.Tasks;
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

    static void TestEnqueue(CAsyncQueue aq)
    {
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
            //System.Threading.Thread.Sleep(100);
            //enqueue two unicode strings and one int
            if (!aq.Enqueue(TEST_QUEUE_KEY, idMessage, "SampleName", str, n))
            {
                aq.raise("Enqueue", CAsyncQueue.idEnqueue);
            }
        }
    }

    static Task<CAsyncQueue.DeqInfo> TestDequeue(CAsyncQueue aq)
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

        TaskCompletionSource<CAsyncQueue.DeqInfo> tcs = new TaskCompletionSource<CAsyncQueue.DeqInfo>();
        CAsyncQueue.DDiscarded aborted = CAsyncQueue.get_aborted(tcs, "Dequeue", CAsyncQueue.idDequeue);
        CAsyncQueue.DOnExceptionFromServer se = CAsyncQueue.get_se(tcs);

        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = (asyncq, messageCount, fileSize, messages, bytes) =>
        {
            if (messages > 0)
            {
                Console.WriteLine("Total message count={0}, queue file size={1}, messages dequeued={2}, message bytes dequeued={3}", messageCount, fileSize, messages, bytes);
            }
            if (messageCount > 0)
            {
                //there are more messages left at server queue, we re-send a request to dequeue
                asyncq.Dequeue(TEST_QUEUE_KEY, asyncq.LastDequeueCallback, 0, aborted, se);
            }
            else
            {
                tcs.TrySetResult(new CAsyncQueue.DeqInfo(messageCount, fileSize, messages, bytes));
            }
        };

        Console.WriteLine("Going to dequeue messages ......");
        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        if (!(aq.Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se) && aq.Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se)))
        {
            aq.raise("Dequeue", CAsyncQueue.idDequeue);
        }
        return tcs.Task;
    }

    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "async_queue_client", "pwd_for_async_queue");
        using (CSocketPool<CAsyncQueue> spAq = new CSocketPool<CAsyncQueue>())
        {
            //spAq.QueueName = "qname";
            if (!spAq.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Failed in connecting to remote async queue server");
                Console.WriteLine("Press key ENTER to close the application ......");
                Console.ReadLine();
                return;
            }
            CAsyncQueue aq = spAq.Seek();
            try
            {
                TestEnqueue(aq);
                Console.WriteLine(TestDequeue(aq).Result);
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
                //Socket is already closed before sending a request
                Console.WriteLine(ex);
            }
            catch (Exception ex)
            {
                //bad operations such as invalid arguments, bad operations and de-serialization errors, and so on
                Console.WriteLine(ex);
            }
            Console.WriteLine("Press key ENTER to close the application ......");
            Console.ReadLine();
        }
    }
}