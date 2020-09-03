package loading_balance.server;

import SPA.ServerSide.*;
import loading_balance.piConst;

public class CMySocketProServer extends CSocketProServer {

    //Routing requires registering two services in pair
    @ServiceAttr(ServiceID = piConst.sidPi)
    private final CSocketProService<CClientPeer> m_Pi = new CSocketProService<>(CClientPeer.class);
    @ServiceAttr(ServiceID = piConst.sidPiWorker)
    private final CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<>(CClientPeer.class);

    @Override
    protected boolean OnSettingServer() {
        //true -- ok; false -- no listening server
        return CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
    }

    public static void main(String[] args) {
        try (CMySocketProServer server = new CMySocketProServer()) {
            if (!server.Run(20901)) {
                System.out.println("Error code = " + server.getLastSocketError());
            }
            System.out.println("Input a line to close the application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
