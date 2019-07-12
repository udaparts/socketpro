package replication;

import SPA.CScopeUQueue;
import SPA.ClientSide.*;
import hello_world.hwConst;
import hello_world.client.HelloWorld;
import uqueue_demo.CMyStruct;

public class Program {

    static void SetWorkDirectory() {
        switch (SPA.CUQueue.DEFAULT_OS) {
            case osWin:
                CClientSocket.QueueConfigure.setWorkDirectory("c:\\sp_test\\");
                break;
            case osWinCE:
                break;
            default:
                CClientSocket.QueueConfigure.setWorkDirectory("/home/yye/sp_test/");
                break;
        }
    }

    public static void main(String[] args) {
        CClientSocket.QueueConfigure.setMessageQueuePassword("MyQPassword");
        SetWorkDirectory();
        ReplicationSetting rs = new ReplicationSetting();
        try (CReplication<HelloWorld> hw = new CReplication<>(HelloWorld.class, rs)) {
            java.util.HashMap<String, CConnectionContext> ConnQueue = new java.util.HashMap<>();
            CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "replication", "p4localhost");
            ConnQueue.put("Tolocal", cc);
            cc = new CConnectionContext("192.168.1.109", 20901, "remote_rep", "PassOne");
            ConnQueue.put("ToLinux", cc);
            boolean ok = hw.Start(ConnQueue, "hw_root_queue_name");
            ok = hw.StartJob();
            ok = hw.Send(hwConst.idSayHelloHelloWorld, new CScopeUQueue().Save("David").Save("Young"));
            ok = hw.Send(hwConst.idEchoHelloWorld, new CScopeUQueue().Save(CMyStruct.MakeOne()));
            ok = hw.EndJob();
            System.out.println("Press key ENTER to shut down the application ......");
            new java.util.Scanner(System.in).nextLine();
            System.out.println("Application ended");
        }
    }
}
