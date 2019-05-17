package hello_world.server;

import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    @Override
    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        System.out.println("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }

    @Override
    protected void OnClose(long hSocket, int nError) {
        CBaseService bs = CBaseService.SeekService(hSocket);
        if (bs != null) {
            CSocketPeer sp = bs.Seek(hSocket);
            // ......
        }
    }
    @ServiceAttr(ServiceID = hello_world.hwConst.sidHelloWorld)
    private final CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<>(HelloWorldPeer.class);

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
