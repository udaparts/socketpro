﻿using System;
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

    protected override bool OnSettingServer()
    {
        return CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
    }

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
