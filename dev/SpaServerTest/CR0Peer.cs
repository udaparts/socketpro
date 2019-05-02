using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ServerSide;


class CR0Peer : CClientPeer
{
    [RequestAttr(TEchoDConst.idRoutorClientCount, true)]
    private uint GetClientCount()
    {
        using (CScopeUQueue su = new CScopeUQueue())
        {
            su.Save("# CR0Peer:TestFake #");
            MakeRequest(TEchoDConst.idRouteFake0, su);
        }
        return CSocketProServer.CountOfClients;
    }
}

