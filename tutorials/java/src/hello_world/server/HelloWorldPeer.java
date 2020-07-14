package hello_world.server;

import SPA.ServerSide.*;
import hello_world.hwConst;
import uqueue_demo.CMyStruct;

public class HelloWorldPeer extends CClientPeer {

    @RequestAttr(RequestID = hwConst.idSayHello)
    private String SayHello(String firstName, String lastName) {
        String res = "Hello " + firstName + " " + lastName;
        System.out.println(res);
        return res;
    }

    @RequestAttr(RequestID = hwConst.idSleep, SlowRequest = true) //true -- slow request
    private void Sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException err) {
        }
    }

    @RequestAttr(RequestID = hwConst.idEcho)
    private CMyStruct Echo(CMyStruct ms) {
        return ms;
    }
}
