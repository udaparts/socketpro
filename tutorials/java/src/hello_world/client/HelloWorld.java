package hello_world.client;

import SPA.CServerError;
import SPA.CScopeUQueue;
import SPA.ClientSide.CAsyncServiceHandler;
import SPA.ClientSide.CSocketError;
import hello_world.hwConst;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

public class HelloWorld extends CAsyncServiceHandler {

    public HelloWorld() {
        super(hello_world.hwConst.sidHelloWorld);
    }

    public String SayHello(String firstName, String lastName) throws InterruptedException, ExecutionException, CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            Future<CScopeUQueue> f = sendRequest(hwConst.idSayHello, sb.Save(firstName).Save(lastName));
            return f.get().getUQueue().LoadString();
        }
    }

    public void Sleep(int ms) throws InterruptedException, ExecutionException, CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            Future<SPA.CScopeUQueue> f = sendRequest(hwConst.idSleep, sb.Save(ms));
            f.get(); //wait until it returns
        }
    }

    public uqueue_demo.CMyStruct Echo(uqueue_demo.CMyStruct ms) throws InterruptedException, ExecutionException, CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            Future<SPA.CScopeUQueue> f = sendRequest(hwConst.idEcho, sb.Save(ms));
            uqueue_demo.CMyStruct res = f.get().getUQueue().Load(uqueue_demo.CMyStruct.class);
            return res;
        }
    }
}
