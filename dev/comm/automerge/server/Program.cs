
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(piConst.sidPi)]
    private CSocketProService<CPiPeer> m_Pi = new CSocketProService<CPiPeer>();

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