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
            boolean ok = spHw.StartSocketPool(cc, 1); //1
            HelloWorld hw = spHw.getAsyncHandlers()[0];
            if (!ok) {
                System.out.println("No connection error code = " + hw.getAttachedClientSocket().getErrorCode());
                return;
            }
            CMyStruct ms, msO = CMyStruct.MakeOne();
            try {
                //process requests one by one synchronously
                System.out.println(hw.SayHello("Jone", "Dole")); //2
                ok = hw.Sleep(5000); //3
                ms = hw.Echo(msO); //4
                assert ms == msO;
            } catch (Exception err) {
                System.out.println(err.getMessage());
            }
            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Jack").Save("Smith"), (ar) -> { //5
                String ret = ar.LoadString();
                System.out.println(ret);
            });
            ok = hw.SendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Donald").Save("Trump"), (ar) -> { //6
                String ret = ar.LoadString();
                System.out.println(ret);
            });
            ok = hw.SendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Hilary").Save("Clinton"), (ar) -> { //7
                String ret = ar.LoadString();
                System.out.println(ret);
            });
            ok = hw.SendRequest(hwConst.idSleep, new CScopeUQueue().Save(5000), null); //8
            final UFuture<CMyStruct> f = new UFuture<>();
            try {
                ok = hw.SendRequest(hwConst.idEcho, new CScopeUQueue().Save(msO), (ar) -> { //9
                    f.set(ar.Load(CMyStruct.class));
                }, (ash, canceled) -> {
                    f.setCanceled();
                });
                ms = f.get(); //10
                assert ms == msO;
            } catch (Exception err) {
                System.out.println(err.getMessage());
            }
            System.out.println("Press ENTER key to shutdown the demo application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
