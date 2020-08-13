package hello_world.client;

import SPA.CServerError;
import SPA.CScopeUQueue;
import SPA.ClientSide.CAsyncServiceHandler;
import SPA.ClientSide.CSocketError;
import SPA.ClientSide.UFuture;
import hello_world.hwConst;

public class HelloWorld extends CAsyncServiceHandler {

    public HelloWorld() {
        super(hello_world.hwConst.sidHelloWorld);
    }

    public String SayHello(String firstName, String lastName) throws CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            UFuture<CScopeUQueue> f = sendRequest(hwConst.idSayHello, sb.Save(firstName).Save(lastName));
            return f.get().getUQueue().LoadString();
        }
    }

    public void Sleep(int ms) throws CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            UFuture<SPA.CScopeUQueue> f = sendRequest(hwConst.idSleep, sb.Save(ms));
            f.get(); //wait until it returns
        }
    }

    public uqueue_demo.CMyStruct Echo(uqueue_demo.CMyStruct ms) throws CSocketError, CServerError {
        try (CScopeUQueue sb = new CScopeUQueue()) {
            UFuture<SPA.CScopeUQueue> f = sendRequest(hwConst.idEcho, sb.Save(ms));
            uqueue_demo.CMyStruct res = f.get().getUQueue().Load(uqueue_demo.CMyStruct.class);
            return res;
        }
    }
}
