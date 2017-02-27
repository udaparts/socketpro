package remote_file.server;

import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    @ServiceAttr(ServiceID = remote_file.RemFileConst.sidRemotingFile)
    private final CSocketProService<RemotingFilePeer> m_RemotingFile = new CSocketProService<>(RemotingFilePeer.class);
    //One SocketPro server supports any number of services. You can list them here!

    public static void main(String[] args) {
        CMySocketProServer MySocketProServer = new CMySocketProServer();
        if (!MySocketProServer.Run(20901)) {
            System.out.println("Error code = " + CSocketProServer.getLastSocketError());
        }
        System.out.println("Input a line to close the application ......");
        new java.util.Scanner(System.in).nextLine();
    }
}
