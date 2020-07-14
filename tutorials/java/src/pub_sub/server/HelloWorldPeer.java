package pub_sub.server;

import SPA.ServerSide.*;
import hello_world.hwConst;
import uqueue_demo.CMyStruct;

public class HelloWorldPeer extends CClientPeer {

    public static String ToString(int[] groups) {
        String s = "[";
        if (groups != null) {
            int n = 0;
            for (int id : groups) {
                if (n != 0) {
                    s += ", ";
                }
                s += id;
                ++n;
            }
        }
        s += "]";
        return s;
    }

    @Override
    protected void OnSwitchFrom(int oldServiceId) {
        getPush().Subscribe(1, 3);
    }

    @Override
    protected void OnSubscribe(int[] groups) {
        System.out.println(getUID() + " subscribes for groups " + ToString(groups));
    }

    @Override
    protected void OnUnsubscribe(int[] groups) {
        System.out.println(getUID() + " unsubscribes for groups " + ToString(groups));
    }

    @Override
    protected void OnPublish(Object message, int[] groups) {
        System.out.println(getUID() + " publishes a message (" + message + ") to groups " + ToString(groups));
    }

    @Override
    protected void OnSendUserMessage(String receiver, Object message) {
        System.out.println(getUID() + " sends a message (" + message + ") to " + receiver);
    }

    @RequestAttr(RequestID = hwConst.idSayHello)
    private String SayHello(String firstName, String lastName) {
        //processed within main thread
        assert (CSocketProServer.getIsMainThread());

        //notify a message to groups [2, 3] at server side
        getPush().Publish("Say hello from " + firstName + " " + lastName, 2, 3);

        String res = "Hello " + firstName + " " + lastName;
        System.out.println(res);
        return res;
    }

    @RequestAttr(RequestID = hwConst.idSleep, SlowRequest = true) //true -- slow request
    private void Sleep(int ms) {
        //processed within a worker thread
        assert (!CSocketProServer.getIsMainThread());
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
