package hello_world.server;

import SPA.ServerSide.*;
import hello_world.hwConst;
import uqueue_demo.CMyStruct;
import SPA.CServerError;

public class HelloWorldPeer extends CClientPeer {

    @RequestAttr(RequestID = hwConst.idSayHello)
    private String SayHello(String firstName, String lastName) throws CServerError {
        if (firstName == null || firstName.length() == 0) {
            throw new SPA.CServerError(62345, "First name cannot be empty");
        }
        String res = "Hello " + firstName + " " + lastName;
        System.out.println(res);
        return res;
    }

    @RequestAttr(RequestID = hwConst.idSleep, SlowRequest = true) //true -- slow request
    private void Sleep(int ms) throws CServerError {
        if (ms < 0) {
            throw new SPA.CServerError(12345, "ms cannot be negative");
        }
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
