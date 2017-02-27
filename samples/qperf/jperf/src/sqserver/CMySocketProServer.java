package sqserver;

import java.util.Scanner;
import SPA.ServerSide.*;

public class CMySocketProServer extends CSocketProServer {

    public static void main(String[] args) {
        //Optionally, set a work directory where server queue files will be created
        if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            CSocketProServer.QueueManager.setWorkDirectory("c:\\sp_test");
        } else {
            CSocketProServer.QueueManager.setWorkDirectory("/home/yye/cyetest/");
        }

        CMySocketProServer MySocketProServer = new CMySocketProServer();

        //pre-open a queue file, which may take long time if the existing queue file is very large
        CServerQueue sq = CSocketProServer.QueueManager.StartQueue("qperf", 24 * 3600);

        int param = 1;
        param <<= 24; //disable auto enqueue notification
        param += 32 * 1024; //32 * 1024 batch dequeuing size in bytes

        //load socketpro async queue server libraries located at the directory ../socketpro/bin
        long handle = CSocketProServer.DllManager.AddALibrary("uasyncqueue", param);
        if (handle == 0) {
            System.out.println("Cannot load async persitent queue library");
        } else if (!MySocketProServer.Run(20901)) {
            System.out.println("Error code = " + CSocketProServer.getLastSocketError());
        }
        System.out.println("Input a line to close the application ......");
        new Scanner(System.in).nextLine();
    }
}
