
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Text;

class Program {
    static byte[] TEST_QUEUE_KEY = System.Text.Encoding.UTF8.GetBytes("qperf");
    const ushort idMessage = (ushort)tagBaseRequestID.idReservedTwo + 128;

    static void EnqueueToServer(CAsyncQueue sq, string message, int cycles) {
        Console.WriteLine("Going to enqueue " + cycles + " messages ......");
        byte[] utf8 = Encoding.UTF8.GetBytes(message);
        System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
        sw.Start();
        for (int n = 0; n < cycles; ++n) {
            sq.Enqueue(TEST_QUEUE_KEY, idMessage, utf8);
        }
        sq.WaitAll();
        sw.Stop();
        Console.WriteLine(cycles + " messages sent to server and enqueued within " + sw.ElapsedMilliseconds + " ms");
    }

    static void EnqueueToServerBatch(CAsyncQueue sq, string message, int cycles, uint batchSize = 8 * 1024) {
        Console.WriteLine("Going to enqueue " + cycles + " messages ......");
        using (CScopeUQueue sb = new CScopeUQueue()) {
            CUQueue q = sb.UQueue;
            byte[] utf8 = Encoding.UTF8.GetBytes(message);
            System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
            sw.Start();
            for (int n = 0; n < cycles; ++n) {
                CAsyncQueue.BatchMessage(idMessage, utf8, q);
                if (q.GetSize() >= batchSize) {
                    sq.EnqueueBatch(TEST_QUEUE_KEY, q);
                }
            }
            if (q.GetSize() > 0) {
                sq.EnqueueBatch(TEST_QUEUE_KEY, q);
            }
            sq.WaitAll();
            sw.Stop();
            Console.WriteLine(cycles + " messages sent to server and enqueued within " + sw.ElapsedMilliseconds + " ms");
        }
    }

    static void DequeueFromServer(CAsyncQueue sq) {
        uint messages_dequeued = 0;
        System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
        CAsyncQueue.DDequeue d = (aq, messageCount, fileSize, messages, bytes) => {
            if (messageCount > 0) {
                //there are more messages left at server queue, we re-send a request to dequeue
                aq.Dequeue(TEST_QUEUE_KEY, sq.LastDequeueCallback);
            } else {
                //set dequeue callback to null and stop dequeuing
                aq.LastDequeueCallback = null;
            }
        };

        sq.ResultReturned += (sender, reqId, q) => {
            bool processed = false;
            switch (reqId) {
                case idMessage: {
                        byte[] utf8 = q.IntenalBuffer;
                        string s = CUQueue.ToString(utf8, (int)q.GetSize());
                        ++messages_dequeued;
                    }
                    processed = true;
                    break;
                default:
                    break;
            }
            return processed;
        };
        Console.WriteLine("Going to dequeue message ......");
        sw.Start();
        bool ok = sq.Dequeue(TEST_QUEUE_KEY, d);

        //optionally, add one or two extra to improve processing concurrency at both client and server sides for better performance and throughput
        ok = sq.Dequeue(TEST_QUEUE_KEY, d);
        ok = sq.Dequeue(TEST_QUEUE_KEY, d);
        sq.WaitAll();
        sw.Stop();
        Console.WriteLine(messages_dequeued + " messages dequeued from server within " + sw.ElapsedMilliseconds + " ms");
    }

    static void Main(string[] args) {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "root", "Smash123");
        using (CSocketPool<CAsyncQueue> spAq = new CSocketPool<CAsyncQueue>()) {
            if (!spAq.StartSocketPool(cc, 1, 1)) {
                Console.WriteLine("Failed in connecting to remote async queue server, and press any key to close the application ......");
                Console.Read();
                return;
            }

            CAsyncQueue sq = spAq.Seek();

            string s4 = "Sock";
            EnqueueToServer(sq, s4, 200000000);
            DequeueFromServer(sq);

            //Manually batching messages improves throughput for high volume of tiny messages
            EnqueueToServerBatch(sq, s4, 200000000, 8 * 1024);
            DequeueFromServer(sq);

            string s32 = "SocketPro is a world-leading pac";
            EnqueueToServer(sq, s32, 200000000);
            DequeueFromServer(sq);

            //Manually batching messages improves throughput for high volume of small messages
            EnqueueToServerBatch(sq, s32, 200000000, 8 * 1024);
            DequeueFromServer(sq);

            //a string having 200 chars
            string s = "SocketPro is a world-leading package of secured communication software components written with request batching, asynchrony and parallel computation in mind. It offers superior performance and scalabi";
            EnqueueToServer(sq, s, 50000000);
            DequeueFromServer(sq);

            //Batching messages improves throughput for high volume of middle size massages
            EnqueueToServerBatch(sq, s, 50000000, 8 * 1024);
            DequeueFromServer(sq);

            string s1024 = "";
            for (int n = 0; n < 6; ++n) {
                s1024 += s;
            }
            s1024 = s1024.Substring(0, 1024);
            EnqueueToServer(sq, s1024, 10000000);
            DequeueFromServer(sq);

            string s10240 = "";
            for (int n = 0; n < 10; ++n) {
                s10240 += s1024;
            }
            EnqueueToServer(sq, s10240, 1000000);
            DequeueFromServer(sq);

            Console.WriteLine("Press key ENTER to complete the application ......");
            Console.ReadLine();
        }
    }
}
