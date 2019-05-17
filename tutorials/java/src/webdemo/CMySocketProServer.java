package webdemo;

import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    @Override
    protected boolean OnSettingServer() {
        //amIntegrated and amMixed not supported yet
        Config.setAuthenticationMethod(tagAuthenticationMethod.amOwn);

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        PushManager.AddAChatGroup(7, "HR Department");

        return true; //true -- ok; false -- no listening server
    }

    @ServiceAttr(ServiceID = hello_world.hwConst.sidHelloWorld)
    private final CSocketProService<pub_sub.server.HelloWorldPeer> m_HelloWorld = new CSocketProService<>(pub_sub.server.HelloWorldPeer.class);
    @ServiceAttr(ServiceID = SPA.BaseServiceID.sidHTTP)
    private final CSocketProService<CMyHttpPeer> m_http = new CSocketProService<>(CMyHttpPeer.class);
    //One SocketPro server supports any number of services. You can list them here!

    public static void main(String[] args) {
        try (CMySocketProServer MySocketProServer = new CMySocketProServer()) {
            if (!MySocketProServer.Run(20901)) {
                System.out.println("Error code = " + CSocketProServer.getLastSocketError());
            }
            System.out.println("Input a line to close the application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
