package sqclient;

import SPA.*;
import SPA.ClientSide.*;
import java.util.*;

public class Program {

    private final static byte[] TEST_QUEUE_KEY = "qperf".getBytes(java.nio.charset.Charset.forName("UTF-8"));
    private final static short idMessage = tagBaseRequestID.idReservedTwo + 128;
    private final static java.nio.charset.Charset UTF8 = java.nio.charset.Charset.forName("UTF-8");

    private static void EnqueueToServer(CAsyncQueue sq, String message, int cycles) {
        System.out.println("Going to enqueue " + cycles + " messages ......");
        byte[] utf8 = message.getBytes(UTF8);
        Date start = new Date();
        for (int n = 0; n < cycles; ++n) {
            if (!sq.Enqueue(TEST_QUEUE_KEY, idMessage, utf8)) {
                break;
            }
        }
        sq.WaitAll();
        Date stop = new Date();
        System.out.println(cycles + " messages sent to server and enqueued within " + (stop.getTime() - start.getTime()) + " ms");
    }

    private static void EnqueueToServerBatch(CAsyncQueue sq, String message, int cycles, int batchSize) {
        System.out.println("Going to enqueue " + cycles + " messages ......");
        SPA.CUQueue q = CScopeUQueue.Lock();
        byte[] utf8 = message.getBytes(UTF8);
        Date start = new Date();
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
        Date stop = new Date();
        System.out.println(cycles + " messages sent to server and enqueued within " + (stop.getTime() - start.getTime()) + " ms");
        CScopeUQueue.Unlock(q);
    }

    private static void DequeueFromServer(final CAsyncQueue sq) {
        final int messages_dequeued[] = {0};
        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = new CAsyncQueue.DDequeue() {
            @Override
            public void invoke(long messageCount, long fileSize, int messagesDequeuedInBatch, int bytesDequeuedInBatch) {
                if (messageCount > 0) {
                    //there are more messages left at server queue, we re-send a request to dequeue
                    sq.Dequeue(TEST_QUEUE_KEY, sq.getLastDequeueCallback());
                } else {
                    //set dequeue callback to null and stop dequeuing
                    sq.setLastDequeueCallback(null);
                }
            }
        };

        sq.ResultReturned = new CAsyncServiceHandler.DOnResultReturned() {
            @Override
            public boolean invoke(CAsyncServiceHandler cash, short reqId, CUQueue q) {
                boolean processed = false;
                switch (reqId) {
                    case idMessage: {
                        byte[] bytes = q.getIntenalBuffer();
                        messages_dequeued[0] += 1;
                        String s = new String(bytes, 0, q.getSize(), UTF8);
                        processed = true;
                    }
                    break;
                    default:
                        break;
                }
                return processed;
            }
        };
        System.out.println("Going to dequeue message ......");
        Date start = new Date();
        boolean ok = sq.Dequeue(TEST_QUEUE_KEY, d);

        //optionally, add one or two extra to improve processing concurrency at both client and server sides for better performance and through-output
        ok = sq.Dequeue(TEST_QUEUE_KEY, d);
        ok = sq.Dequeue(TEST_QUEUE_KEY, d);
        sq.WaitAll();
        Date stop = new Date();
        System.out.println(messages_dequeued[0] + " messages dequeued from server within " + (stop.getTime() - start.getTime()) + " ms");
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20901;
        cc.UserId = "async_queue_client_java";
        cc.Password = "pwd_for_async_queue";

        CSocketPool<CAsyncQueue> spAq = new CSocketPool<>(CAsyncQueue.class);
        boolean ok = spAq.StartSocketPool(cc, 1, 1);
        CAsyncQueue sq = spAq.getAsyncHandlers()[0];
        if (!ok) {
            System.out.println("No connection error code = " + sq.getAttachedClientSocket().getErrorCode());
            new java.util.Scanner(System.in).nextLine();
            return;
        }

        String s4 = "Sock";
        EnqueueToServer(sq, s4, 200000000);
        DequeueFromServer(sq);

        //Manually batching improves throughput for high volume of tiny messages
        EnqueueToServerBatch(sq, s4, 200000000, 8 * 1024);
        DequeueFromServer(sq);

        String s32 = "SocketPro is a world-leading pac";
        EnqueueToServer(sq, s32, 200000000);
        DequeueFromServer(sq);

        //Manually batching improves throughput for high volume of small messages
        EnqueueToServerBatch(sq, s32, 200000000, 8 * 1024);
        DequeueFromServer(sq);

        String s = "SocketPro is a world-leading package of secured communication software components written with request batching, asynchrony and parallel computation in mind. It offers superior performance and scalabi";
        EnqueueToServer(sq, s, 50000000);
        DequeueFromServer(sq);

        //Manually batching improves throughput for high volume of middle messages
        EnqueueToServerBatch(sq, s, 50000000, 8 * 1024);
        DequeueFromServer(sq);

        String s1024 = s;
        for (int n = 0; n < 5; ++n) {
            s1024 += s;
        }
        s1024 = s1024.substring(0, 1024);
        EnqueueToServer(sq, s1024, 10000000);
        DequeueFromServer(sq);

        String s10240 = "";
        for (int n = 0; n < 10; ++n) {
            s10240 += s1024;
        }
        EnqueueToServer(sq, s10240, 1000000);
        DequeueFromServer(sq);

        System.out.println("Press key ENTER to complete dequeuing messages from server ......");
        in.nextLine();
    }
}
