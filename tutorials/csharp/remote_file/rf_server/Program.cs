using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(RemFileConst.sidRemotingFile)]
    private CSocketProService<RemotingFilePeer> m_RemotingFile = new CSocketProService<RemotingFilePeer>();
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

}

