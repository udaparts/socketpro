package hello_world.client;

import SPA.CScopeUQueue;
import SPA.ClientSide.CAsyncResult;
import SPA.ClientSide.CAsyncServiceHandler;
import hello_world.hwConst;

public class HelloWorld extends CAsyncServiceHandler {

    public HelloWorld() {
        super(hello_world.hwConst.sidHelloWorld);
    }

    public final String SayHello(String firstName, String lastName) {
        final String[] arr = {null};
        boolean ok = SendRequest(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save(firstName).Save(lastName), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                arr[0] = ar.LoadString();
            }
        }) && WaitAll();

        //JYI: It is recommended to use lambda expression with Java 1.8 or later
        /*
         boolean ok = SendRequest(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save(firstName).Save(lastName), (ar) -> {
         arr[0] = ar.LoadString();
         }) && WaitAll();
         */
        return arr[0];
    }

    public final void Sleep(int ms) {
        boolean ok = SendRequest(hwConst.idSleepHelloWorld, new CScopeUQueue().Save(ms), null) && WaitAll();
    }

    public final uqueue_demo.CMyStruct Echo(uqueue_demo.CMyStruct ms) {
        final uqueue_demo.CMyStruct[] arr = {null};
        boolean ok = SendRequest(hwConst.idEchoHelloWorld, new CScopeUQueue().Save(ms), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                arr[0] = ar.Load(uqueue_demo.CMyStruct.class);
            }
        }) && WaitAll();
        return arr[0];
    }
}
