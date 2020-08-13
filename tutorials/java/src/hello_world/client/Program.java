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
            spHw.setQueueName("helloworld");
            boolean ok = spHw.StartSocketPool(cc, 1);
            HelloWorld hw = spHw.getAsyncHandlers()[0];
            if (!ok) {
                System.out.println("No connection error code = " + hw.getAttachedClientSocket().getErrorCode());
                return;
            }
            CMyStruct ms, msO = CMyStruct.MakeOne();
            try {
                //process requests one by one synchronously
                System.out.println(hw.SayHello("John", "Dole"));
                hw.Sleep(5000);
                ms = hw.Echo(msO);
                assert ms == msO;
            } catch (CServerError ex) {
                System.out.println("----SERVER EXCEPTION----");
                System.out.println(ex.getWhere());
                System.out.println(ex.getMessage());
                System.out.println("----EXCEPTION----");
            } catch (CSocketError ex) {
                System.out.println(ex.getMessage());
            }
            try {
                //asynchronously process multiple requests with inline batching for best network efficiency
                UFuture<CScopeUQueue> f0 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Jack").Save("Smith"));
                UFuture<CScopeUQueue> f1 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Donald").Save("Trump"));
                UFuture<CScopeUQueue> f2 = hw.sendRequest(hwConst.idSleep, new CScopeUQueue().Save(6000));
                UFuture<CScopeUQueue> f3 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Hilary").Save("Clinton"));
                UFuture<CScopeUQueue> f4 = hw.sendRequest(hwConst.idEcho, new CScopeUQueue().Save(msO));
                System.out.println(f0.get().getUQueue().LoadString());
                System.out.println(f1.get().getUQueue().LoadString());
                assert 0 == f2.get().getUQueue().getSize();
                System.out.println(f3.get().getUQueue().LoadString());
                CScopeUQueue sb = f4.get();
                ms = sb.getUQueue().Load(uqueue_demo.CMyStruct.class);
                assert ms == msO;
            } catch (CServerError ex) {
                System.out.println("----SERVER EXCEPTION----");
                System.out.println(ex.getMessage());
                System.out.println(ex.getWhere());
                System.out.println("----EXCEPTION----");
            } catch (CSocketError ex) {
                System.out.println(ex.getMessage());
            }
            System.out.println("Press ENTER key to shutdown the demo application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
