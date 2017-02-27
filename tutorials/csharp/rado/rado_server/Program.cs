using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(radoConst.sidRAdo)]
    private CSocketProService<RAdoPeer> m_RAdo = new CSocketProService<RAdoPeer>();
    //One SocketPro server supports any number of services. You can list them here!
}

class Program
{
    static void Main(string[] args)
    {
        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}

