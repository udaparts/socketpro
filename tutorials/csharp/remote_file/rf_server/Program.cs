using System;
using System.Runtime.InteropServices;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [DllImport("ustreamfile")]
    static extern void SetRootDirectory([In] [MarshalAs(UnmanagedType.LPWStr)] string root);

    protected override bool OnSettingServer()
    {
        //load SocketPro file streaming server plugin located at the directory ../socketpro/bin
        IntPtr p = CSocketProServer.DllManager.AddALibrary("ustreamfile");
        if (p.ToInt64() != 0)
        {
            SetRootDirectory("C:\\boost_1_60_0\\stage\\lib64");
            return true;
        }
        return false;
    }

    static void Main(string[] args)
    {
        CMySocketProServer MySocketProServer = new CMySocketProServer();
        if (!MySocketProServer.Run(20901))
            Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
        Console.WriteLine("Input a line to close the application ......");
        Console.ReadLine();
        MySocketProServer.StopSocketProServer(); //or MySocketProServer.Dispose();
    }
}
