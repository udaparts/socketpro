using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;


public class CMySocketProServer : CSocketProServer
{
    //for db push from ms sql server
    [ServiceAttr(repConst.sidRAdoRep)]
    private CSocketProService<DBPushPeer> m_RAdoRep = new CSocketProService<DBPushPeer>();

    //Routing requires registering two services in pair
    [ServiceAttr(piConst.sidPi)]
    private CSocketProService<CClientPeer> m_Pi = new CSocketProService<CClientPeer>();
    [ServiceAttr(piConst.sidPiWorker)]
    private CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<CClientPeer>();

    static void Main(string[] args)
    {
        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            else
                CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}
