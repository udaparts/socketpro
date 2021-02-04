using System;
using System.Runtime.InteropServices;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [DllImport("ustreamfile")]
    static extern bool SetSPluginGlobalOptions([In] [MarshalAs(UnmanagedType.LPStr)] string jsonUtf8);

    protected override bool OnSettingServer()
    {
        //load SocketPro file streaming server plugin located at the directory ../socketpro/bin
        IntPtr p = CSocketProServer.DllManager.AddALibrary("ustreamfile");
        if (p.ToInt64() != 0)
        {
            bool ok = SetSPluginGlobalOptions("{\"root_directory\":\"C:\\\\boost_1_60_0\\\\stage\\\\lib64\"}");
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
