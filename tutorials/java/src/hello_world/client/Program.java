package hello_world.client;

import SPA.*;
import SPA.ClientSide.*;
import uqueue_demo.CMyStruct;
import hello_world.hwConst;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        CSocketPool<HelloWorld> spHw = new CSocketPool<>(HelloWorld.class, true);
        boolean ok = spHw.StartSocketPool(cc, 1, 1);
        HelloWorld hw = spHw.getAsyncHandlers()[0];
        if (!hw.getAttachedClientSocket().getConnected()) {
            System.out.println("No connection error code = " + hw.getAttachedClientSocket().getErrorCode());
            return;
        }
        //optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
        ok = hw.getAttachedClientSocket().getClientQueue().StartQueue("helloworld", 24 * 3600, false); //time-to-live 1 day and true for encryption

        //process requests one by one synchronously
        System.out.println(hw.SayHello("Jone", "Dole"));
        hw.Sleep(5000);
        final CMyStruct msOriginal = CMyStruct.MakeOne();
        CMyStruct ms = hw.Echo(msOriginal);

        //asynchronously process multiple requests with inline batching for best network efficiency
        ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save("Jack").Save("Smith"), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                String ret = ar.LoadString();
                System.out.println(ret);
            }
        });

        //JYI: It is recommended to use lambda expression with Java 1.8 or later
        /*
         ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save("Jack").Save("Smith"), (ar) -> {
         String ret = ar.LoadString();
         System.out.println(ret);
         });
         */
        ok = hw.SendRequest(hwConst.idSleepHelloWorld, new CScopeUQueue().Save(5000), null);
        ok = hw.SendRequest(hwConst.idEchoHelloWorld, new CScopeUQueue().Save(msOriginal), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                CMyStruct res = ar.Load(CMyStruct.class);
                assert (res.equals(msOriginal));
            }
        });
        ok = hw.WaitAll();
        System.out.println("Press ENTER key to shutdown the demo application ......");
        new java.util.Scanner(System.in).nextLine();
    }
}
