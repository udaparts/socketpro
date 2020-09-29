package pub_sub.server;

import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    @ServiceAttr(ServiceID = hello_world.hwConst.sidHelloWorld)
    private final CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<>(HelloWorldPeer.class);

    @Override
    protected boolean OnSettingServer() {
        //amIntegrated and amMixed not supported yet
        Config.setAuthenticationMethod(tagAuthenticationMethod.amOwn);

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");

        return true; //true -- ok; false -- no listening server
    }

    @Override
    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        return true; //true -- give permission; false -- connection denied
    }

    public static void main(String[] args) {
        try (CMySocketProServer MySocketProServer = new CMySocketProServer()) {

            //test certificate and private key files are located at ../SocketProRoot/bin
            if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
                MySocketProServer.UseSSL("intermediate.pfx", "", "mypassword");
                //or load cert and private key from windows system cert store
                //MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "");
            } else {
                MySocketProServer.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            }

            if (!MySocketProServer.Run(20901)) {
                System.out.println("Error code = " + CSocketProServer.getLastSocketError());
            }
            System.out.println("Input a line to close the application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
