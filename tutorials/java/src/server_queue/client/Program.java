package server_queue.client;

import SPA.*;
import SPA.ClientSide.*;

public class Program {

    private final static byte[] TEST_QUEUE_KEY = "queue_name_0".getBytes(java.nio.charset.Charset.forName("UTF-8"));
    private final static short idMessage0 = tagBaseRequestID.idReservedTwo + 100;
    private final static short idMessage1 = tagBaseRequestID.idReservedTwo + 101;
    private final static short idMessage2 = tagBaseRequestID.idReservedTwo + 102;

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
        CAsyncQueue aq = spAq.getAsyncHandlers()[0];
        if (!ok) {
            System.out.println("No connection error code = " + aq.getAttachedClientSocket().getErrorCode());
            in.nextLine();
            return;
        }

        ok = TestEnqueue(aq);
        TestDequeue(aq);

        System.out.println("Press a key to complete dequeuing messages from server ......");
        in.nextLine();
    }

    private static boolean TestEnqueue(CAsyncQueue aq) {
        boolean ok = true;
        System.out.println("Going to enqueue 1024 messages ......");
        for (int n = 0; n < 1024; ++n) {
            String str = n + " Object test";
            short idMessage;
            switch (n % 3) {
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
            ok = aq.Enqueue(TEST_QUEUE_KEY, idMessage, new CScopeUQueue().Save("SampleName").Save(str).Save(n));
            if (!ok) {
                break;
            }
        }
        return ok;
    }

    private static void TestDequeue(final CAsyncQueue aq) {
        //prepare callback for parsing messages dequeued from server side
        aq.ResultReturned = (sender, idReq, q) -> {
            boolean processed = false;
            switch (idReq) {
                case idMessage0:
                case idMessage1:
                case idMessage2:
                    System.out.print("message id=" + idReq);
                     {
                        //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
                        String name = q.LoadString(), str = q.LoadString();
                        int index = q.LoadInt();
                        System.out.print(", name=" + name);
                        System.out.print(", str=" + str);
                        System.out.println(", index=" + index);
                    }
                    processed = true;
                    break;
                default:
                    break;
            }
            return processed;
        };

        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = (asyncqueue, messageCount, fileSize, messagesDequeuedInBatch, bytesDequeuedInBatch) -> {
            System.out.print("Total message count=" + messageCount);
            System.out.print(", queue file size=" + fileSize);
            System.out.print(", messages dequeued=" + messagesDequeuedInBatch);
            System.out.println(", message bytes dequeued=" + bytesDequeuedInBatch);
            if (messageCount > 0) {
                //there are more messages left at server queue, we re-send a request to dequeue
                asyncqueue.Dequeue(TEST_QUEUE_KEY, aq.getLastDequeueCallback());
            }
        };

        System.out.println("Going to dequeue message ......");
        boolean ok = aq.Dequeue(TEST_QUEUE_KEY, d);

        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        ok = aq.Dequeue(TEST_QUEUE_KEY, d);
    }
}
