
using System;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    static void Main(string[] args)
    {
        //Optionally, set a work directory where server queue files will be created
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            CSocketProServer.QueueManager.WorkDirectory = "/home/yye/cyetest/";
        else
            CSocketProServer.QueueManager.WorkDirectory = "c:\\sp_test";

        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            //pre-open a queue file, which may take long time if the existing queue file is very large
            CServerQueue sq = CSocketProServer.QueueManager.StartQueue("qperf", 24 * 3600);

            int param = 1;
            param <<= 24; //disable server enqueue notification
            param += 32 * 1024; //32 * 1024 batch dequeuing size in bytes

            IntPtr p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", param); //32 * 1024 batch dequeuing size in bytes
            if (p.ToInt64() == 0)
                Console.WriteLine("Cannot load async persistent queue library");
            else if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}
