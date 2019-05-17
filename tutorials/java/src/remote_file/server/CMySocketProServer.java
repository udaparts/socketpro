package remote_file.server;

import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    @Override
    protected boolean OnSettingServer() {
        long handle = CSocketProServer.DllManager.AddALibrary("ustreamfile");
        if (handle != 0) {
            if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
                Sfile.SetRootDirectory("C:\\boost_1_60_0\\stage\\lib64");
            }
        }
        return true;
    }

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
