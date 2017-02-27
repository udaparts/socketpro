using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{

    protected override bool OnSettingServer()
    {
        IntPtr p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 16 * 1024); //16 * 1024 batch dequeuing size in bytes
        return (p.ToInt64() != 0);
    }

    static void Main(string[] args)
    {
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            CSocketProServer.QueueManager.WorkDirectory = "/home/yye/sp_test/";
        else
            CSocketProServer.QueueManager.WorkDirectory = "c:\\sp_test";

        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            //CSocketProServer.QueueManager.MessageQueuePassword = "MyPasswordForMsgQueue";
            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}

