package hello_world.client;

import SPA.CScopeUQueue;
import SPA.ClientSide.CAsyncServiceHandler;
import SPA.ClientSide.UFuture;
import hello_world.hwConst;

public class HelloWorld extends CAsyncServiceHandler {

    public HelloWorld() {
        super(hello_world.hwConst.sidHelloWorld);
    }

    public final String SayHello(String firstName, String lastName) throws Exception {
        UFuture<String> f = new UFuture<>();
        try (CScopeUQueue sb = new CScopeUQueue()) {
            if (!SendRequest(hwConst.idSayHelloHelloWorld, sb.Save(firstName).Save(lastName), (ar) -> {
                f.set(ar.LoadString());
            }, (ash, canceled) -> {
                f.setCanceled();
            })) {
                throw new Exception(getAttachedClientSocket().getErrorMsg());
            }
        }
        return f.get();
    }

    public boolean Sleep(int ms) throws Exception {
        UFuture<Boolean> f = new UFuture<>();
        try (CScopeUQueue sb = new CScopeUQueue()) {
            if (!SendRequest(hwConst.idSleepHelloWorld, sb.Save(ms), (ar) -> {
                f.set(true);
            }, (ash, canceled) -> {
                f.setCanceled();
            })) {
                throw new Exception(getAttachedClientSocket().getErrorMsg());
            }
        }
        return f.get();
    }

    public uqueue_demo.CMyStruct Echo(uqueue_demo.CMyStruct ms) throws Exception {
        UFuture<uqueue_demo.CMyStruct> f = new UFuture<>();
        try (CScopeUQueue sb = new CScopeUQueue()) {
            if (!SendRequest(hwConst.idEchoHelloWorld, sb.Save(ms), (ar) -> {
                f.set(ar.Load(uqueue_demo.CMyStruct.class));
            }, (ash, canceled) -> {
                f.setCanceled();
            })) {
                throw new Exception(getAttachedClientSocket().getErrorMsg());
            }
        }
        return f.get();
    }
}
