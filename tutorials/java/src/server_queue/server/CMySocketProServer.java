package server_queue.server;

import SPA.ServerSide.*;


public class CMySocketProServer extends CSocketProServer {

    public static void main(String[] args) {
        if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            CSocketProServer.QueueManager.setWorkDirectory("c:\\sp_test");
        } else {
            CSocketProServer.QueueManager.setWorkDirectory("/home/yye/sp_test/");
        }
        CMySocketProServer MySocketProServer = new CMySocketProServer();
        
        //CSocketProServer.QueueManager.setMessageQueuePassword("MyPasswordForMsgQueue");
        
        //load socketpro async queue server libraries located at the directory ../socketpro/bin
        long handle = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 16 * 1024); //16 * 1024 batch dequeuing size in bytes
        if (handle == 0) {
            System.out.println("Cannot load async queue library");
        }
        else if (!MySocketProServer.Run(20901)) {
            System.out.println("Error code = " + CSocketProServer.getLastSocketError());
        }
        System.out.println("Input a line to close the application ......");
        new java.util.Scanner(System.in).nextLine();
    }
}
