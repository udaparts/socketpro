package webdemo;

public class CMyHttpPeer extends SPA.ServerSide.CHttpPeerBase {

    @Override
    protected void OnSubscribe(int[] groups) {
        System.out.println(getUID() + " subscribes for groups " + pub_sub.server.HelloWorldPeer.ToString(groups));
    }

    @Override
    protected void OnUnsubscribe(int[] groups) {
        System.out.println(getUID() + " unsubscribes for groups " + pub_sub.server.HelloWorldPeer.ToString(groups));
    }

    @Override
    protected void OnPublish(Object message, int[] groups) {
        System.out.println(getUID() + " publishes a message (" + message + ") to groups " + pub_sub.server.HelloWorldPeer.ToString(groups));
    }

    @Override
    protected void OnSendUserMessage(String receiver, Object message) {
        System.out.println(getUID() + " sends a message (" + message + ") to " + receiver);
    }

    @Override
    protected boolean DoAuthentication(String userId, String password) {
        getPush().Subscribe(1, 2, 7);
        System.out.print("User id = " + userId);
        System.out.println(", password = " + password);
        return true; //true -- permitted; and false -- denied
    }

    @Override
    protected void OnGet() {
        if (getPath().lastIndexOf('.') != 1) {
            DownloadFile(getPath().substring(1));
        } else {
            SendResult("test result --- GET ---");
        }
    }

    @Override
    protected void OnPost() {
        int res = SendResult("+++ POST +++ test result");
    }

    @Override
    protected void OnUserRequest() {
        switch (getRequestName()) {
            case "sleep":
                int ms = Integer.parseInt(getArgs()[0].toString());
                Sleep(ms);
                SendResult("");
                break;
            case "sayHello":
                SendResult(SayHello(getArgs()[0].toString(), getArgs()[1].toString()));
                break;
            default:
                SendResult("NO_SUPPORT");
                break;
        }
    }

    private String SayHello(String firstName, String lastName) {
        //notify a message to groups [2, 3] at server side
        getPush().Publish("Say hello from " + firstName + " " + lastName, 2, 3);
        return "Hello " + firstName + " " + lastName;
    }

    private void Sleep(int ms) {
        try {
            Thread.sleep(ms);
            String msg = getUID() + " called the method Sleep";
            getPush().Publish(msg, 2, 3);
        } catch (InterruptedException err) {
        }
    }
}
