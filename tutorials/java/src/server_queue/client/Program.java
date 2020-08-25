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

        try (CSocketPool<CAsyncQueue> spAq = new CSocketPool<>(CAsyncQueue.class)) {
            boolean ok = spAq.StartSocketPool(cc, 1);
            CAsyncQueue aq = spAq.getAsyncHandlers()[0];
            if (!ok) {
                System.out.println("No connection error code = " + aq.getSocket().getErrorCode());
                in.nextLine();
                return;
            }
            try {
                TestEnqueue(aq);
                UFuture<CAsyncQueue.DeqInfo> f = TestDequeue(aq);
                System.out.println(f.get());
            } catch (CSocketError | CServerError ex) {
                System.out.println(ex);
            } catch (Exception ex) {
                //bad parameter
                System.out.println("Unexpected error: " + ex.getMessage());
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
                aq.raise("Enqueue", CAsyncQueue.idEnqueue);
            }
        }
    }

    private static UFuture<CAsyncQueue.DeqInfo> TestDequeue(CAsyncQueue aq) throws CSocketError {
        //prepare callback for parsing messages dequeued from server side
        aq.ResultReturned = (sender, idReq, q) -> {
            boolean processed = false;
            switch (idReq) {
                case idMessage0:
                case idMessage1:
                case idMessage2:
                    System.out.print("message id=" + idReq);
                     {
                        //parse a dequeued message which should be the same as the above
                        //enqueued message (two unicode strings and one int)
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

        UFuture<CAsyncQueue.DeqInfo> f = new UFuture<>("Dequeue", CAsyncQueue.idDequeue);
        CAsyncQueue.DDiscarded aborted = CAsyncQueue.get_aborted(f);
        CAsyncQueue.DOnExceptionFromServer se = CAsyncQueue.get_se(f);

        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue.DDequeue d = (asyncq, messages, fileSize, msgs_dequeued, bytes) -> {
            if (bytes > 0) {
                System.out.print("Total message count=" + messages);
                System.out.print(", queue file size=" + fileSize);
                System.out.print(", messages dequeued=" + msgs_dequeued);
                System.out.println(", message bytes dequeued=" + bytes);
            }
            if (messages > 0) {
                //there are more messages left at server queue, we re-send a request to dequeue
                asyncq.Dequeue(TEST_QUEUE_KEY, asyncq.getLastDequeueCallback(), 0, aborted, se);
            } else if (!f.isDone()) {
                f.set(asyncq.new DeqInfo(messages, fileSize, msgs_dequeued, bytes));
            }
        };
        System.out.println("Going to dequeue message ......");
        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        if (!(aq.Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se) && aq.Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se))) {
            aq.raise(f);
        }
        return f;
    }
}
