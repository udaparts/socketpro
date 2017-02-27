using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class MyAdo : CAsyncAdohandler
{
    public MyAdo()
        : base(repConst.sidRAdoRep)
    {
    }
}
