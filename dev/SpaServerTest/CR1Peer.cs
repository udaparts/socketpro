using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

class CR1Peer : CClientPeer
{
    [RequestAttr(TEchoDConst.idCheckRouteeServiceId, true)]
    private uint CheckRouteeServiceId()
    {
        using (CScopeUQueue su = new CScopeUQueue())
        {
            su.Save("= CR1Peer:TestFake =");
            MakeRequest(TEchoDConst.idRouteFake0, su);
        }
        return TEchoDConst.sidRouteSvs0;
    }
}
