package test_java;

import SPA.*;
import SPA.ClientSide.*;

public class Test_java {

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

        try (CSocketPool<CAsyncQueue> spAq = new CSocketPool<>(CAsyncQueue.class)) {
            boolean ok = spAq.StartSocketPool(cc, 1);
            CAsyncQueue aq = spAq.getAsyncHandlers()[0];
            if (!ok) {
                System.out.println("No connection error code = " + aq.getSocket().getErrorCode());
                in.nextLine();
                return;
            }
            try {
                //Optionally, you can enqueue messages with transaction style by calling the methods StartQueueTrans and EndQueueTrans in pair
                UFuture<Integer> fs = aq.startQueueTrans(TEST_QUEUE_KEY);
                TestEnqueue(aq);
                UFuture<Integer> fe = aq.endQueueTrans();
                System.out.println("StartTrans/res: " + fs.get());
                System.out.println("EndTrans/res: " + fe.get());

                TestDequeue(aq);
                aq.WaitAll();

                //test GetKeys
                UFuture<String[]> fk = aq.getKeys();
                UFuture<CAsyncQueue.QueueInfo> ffq = aq.flushQueue(TEST_QUEUE_KEY);

                int index = 0;
                System.out.print("[");
                for (String s : fk.get()) {
                    if (index != 0) {
                        System.out.print(", ");
                    }
                    System.out.print(s);
                }
                System.out.println("]");

                //get a queue key two parameters, message count and queue file size by default option oMemoryCached
                System.out.println(ffq.get());

                UFuture<Integer> fec = aq.closeQueue(TEST_QUEUE_KEY);
                System.out.println("CloseQueue/res: " + fec.get());
            } catch (CSocketError | CServerError ex) {
                System.out.println(ex);
            }
            System.out.println("Press a key to complete dequeuing messages from server ......");
            in.nextLine();
        }
    }

    private static void TestEnqueue(CAsyncQueue aq) throws CSocketError {
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
            if (!aq.Enqueue(TEST_QUEUE_KEY, idMessage, new CScopeUQueue().Save("SampleName").Save(str).Save(n))) {
                aq.raise(CAsyncQueue.idEnqueue);
            }
        }
    }

    private static void TestDequeue(final CAsyncQueue aq) throws CSocketError {
        //prepare callback for parsing messages dequeued from server side
        aq.ResultReturned = (CAsyncServiceHandler sender, short idReq, CUQueue q) -> {
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
        CAsyncQueue.DDequeue d = (CAsyncQueue sender, long messageCount, long fileSize, int messagesDequeuedInBatch, int bytesDequeuedInBatch) -> {
            System.out.print("Total message count=" + messageCount);
            System.out.print(", queue file size=" + fileSize);
            System.out.print(", messages dequeued=" + messagesDequeuedInBatch);
            System.out.println(", message bytes dequeued=" + bytesDequeuedInBatch);
            if (messageCount > 0) {
                //there are more messages left at server queue, we re-send a request to dequeue
                sender.Dequeue(TEST_QUEUE_KEY, sender.getLastDequeueCallback());
            }
        };

        System.out.println("Going to dequeue message ......");
        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        if (!(aq.Dequeue(TEST_QUEUE_KEY, d) && aq.Dequeue(TEST_QUEUE_KEY, d))) {
            aq.raise(CAsyncQueue.idDequeue);
        }
    }
}
