package hello_world.client;

import SPA.*;
import SPA.ClientSide.*;
import uqueue_demo.CMyStruct;
import hello_world.hwConst;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        try (CSocketPool<HelloWorld> spHw = new CSocketPool<>(HelloWorld.class, true)) {
            //optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
            //spHw.setQueueName("helloworld");
            boolean ok = spHw.StartSocketPool(cc, 1);
            HelloWorld hw = spHw.getAsyncHandlers()[0];
            if (!hw.getAttachedClientSocket().getConnected()) {
                System.out.println("No connection error code = " + hw.getAttachedClientSocket().getErrorCode());
                return;
            }
            CMyStruct ms, msOriginal = CMyStruct.MakeOne();
            try {
                //process requests one by one synchronously
                System.out.println(hw.SayHello("Jone", "Dole"));
                ok = hw.Sleep(5000);
                ms = hw.Echo(msOriginal);
                assert (ms == msOriginal);
            } catch (Exception err) {
                System.out.println(err.getMessage());
            }
            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save("Jack").Save("Smith"), (ar) -> {
                String ret = ar.LoadString();
                System.out.println(ret);
            });
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, new CScopeUQueue().Save(5000), null);
            final UFuture<CMyStruct> f = new UFuture<>();
            try {
                ok = hw.SendRequest(hwConst.idEchoHelloWorld, new CScopeUQueue().Save(msOriginal), (ar) -> {
                    f.set(ar.Load(CMyStruct.class));
                }, (ash, canceled) -> {
                    f.setCanceled();
                });
                ms = f.get();
                assert (ms == msOriginal);
            } catch (Exception err) {
                System.out.println(err.getMessage());
            }
            System.out.println("Press ENTER key to shutdown the demo application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
