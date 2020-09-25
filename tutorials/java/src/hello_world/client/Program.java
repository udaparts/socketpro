package hello_world.client;

import SPA.*;
import SPA.ClientSide.*;
import uqueue_demo.CMyStruct;
import hello_world.hwConst;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        try (CSocketPool<HelloWorld> spHw = new CSocketPool<>(HelloWorld.class)) {
            //optionally start a client queue for auto failure recovery
            //spHw.setQueueName("helloworld");
            boolean ok = spHw.StartSocketPool(cc, 1);
            HelloWorld hw = spHw.Seek();
            if (ok) {
                CMyStruct ms, ms0 = CMyStruct.MakeOne();
                try {
                    //process requests one by one synchronously
                    System.out.println(hw.SayHello("John", "Dole"));
                    hw.Sleep(5000);
                    ms = hw.Echo(ms0);
                    assert ms == ms0;
                } catch (CServerError ex) {
                    System.out.println(ex);
                } catch (CSocketError ex) {
                    System.out.println(ex);
                } catch (Exception ex) {
                    //bad parameter, CUQueue de-serilization exception
                    System.out.println("Unexpected error: " + ex.getMessage());
                }
                try {
                    //streaming multiple requests with inline batching for the best network efficiency
                    UFuture<CScopeUQueue> f0 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Jack").Save("Smith"));
                    UFuture<CScopeUQueue> f1 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Donald").Save("Trump"));
                    UFuture<CScopeUQueue> f2 = hw.sendRequest(hwConst.idSleep, new CScopeUQueue().Save(6000));
                    UFuture<CScopeUQueue> f3 = hw.sendRequest(hwConst.idSayHello, new CScopeUQueue().Save("Hilary").Save("Clinton"));
                    UFuture<CScopeUQueue> f4 = hw.sendRequest(hwConst.idEcho, new CScopeUQueue().Save(ms0));
                    //hw.getSocket().Cancel();
                    System.out.println(f0.get().LoadString());
                    System.out.println(f1.get().LoadString());
                    assert 0 == f2.get().getUQueue().getSize();
                    //String bad = f2.get().getUQueue().LoadString();
                    System.out.println(f3.get().LoadString());
                    CScopeUQueue sb = f4.get();
                    ms = sb.Load(uqueue_demo.CMyStruct.class);
                    assert ms == ms0;
                } catch (CServerError ex) {
                    System.out.println(ex);
                } catch (CSocketError ex) {
                    System.out.println(ex);
                } catch (Exception ex) {
                    //bad parameter, CUQueue de-serilization exception
                    System.out.println("Unexpected error: " + ex.getMessage());
                }
            } else {
                System.out.println("No connection error code = " + spHw.getSockets()[0].getErrorCode());
            }
            System.out.println("Press ENTER key to kill the demo ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
