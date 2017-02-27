
using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void SetWorkDirectory()
    {
        switch (System.Environment.OSVersion.Platform)
        {
            case PlatformID.Win32NT:
            case PlatformID.Win32S:
            case PlatformID.Win32Windows:
                CClientSocket.QueueConfigure.WorkDirectory = "c:\\sp_test\\";
                break;
            case PlatformID.WinCE:
                break;
            default:
                CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/sp_test/";
                break;
        }
    }
    static void Main(string[] args)
    {
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyQPassword";
        SetWorkDirectory();
        ReplicationSetting rs = new ReplicationSetting();
        using (CReplication<HelloWorld> hw = new CReplication<HelloWorld>(rs))
        {
            Dictionary<string, CConnectionContext> ConnQueue = new Dictionary<string, CConnectionContext>();
            CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "replication", "p4localhost");
#if PocketPC
#else
            ConnQueue["Tolocal"] = cc;
#endif
            cc = new CConnectionContext("192.168.1.109", 20901, "remote_rep", "PassOne");
            ConnQueue["ToLinux"] = cc;
            bool ok = hw.Start(ConnQueue, "hw_root_queue_name");
            hw.StartJob();
            ok = hw.Send(hwConst.idSayHelloHelloWorld, "David", "Young");
            ok = hw.Send(hwConst.idEchoHelloWorld, CMyStruct.MakeOne());
            hw.EndJob();
            Console.WriteLine("Press key ENTER to shut down the application ......");
            Console.ReadLine();
        }
    }
}

