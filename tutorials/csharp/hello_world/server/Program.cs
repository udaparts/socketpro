using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(hwConst.sidHelloWorld)]
    private CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<HelloWorldPeer>();
    //One SocketPro server supports any number of services. You can list them here!

    static void Main(string[] args)
    {
        CMySocketProServer MySocketProServer = new CMySocketProServer();
        if (!MySocketProServer.Run(20901))
            Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
        Console.WriteLine("Input a line to close the application ......");
        Console.ReadLine();
        MySocketProServer.StopSocketProServer(); //or MySocketProServer.Dispose();
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        Console.WriteLine("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }

    protected override void OnClose(ulong hSocket, int nError)
    {
        CBaseService bs = CBaseService.SeekService(hSocket);
        if (bs != null)
        {
            CSocketPeer sp = bs.Seek(hSocket);
            // ......
        }
    }
}
