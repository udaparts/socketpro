using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;


class CPerfStudyPeer : CClientPeer
{
    [RequestAttr(PerfStudyConst.idDoMyEcho)]
    string DoEcho(string str)
    {
        return str;
    }

    [RequestAttr(PerfStudyConst.idSlowEcho, true)]
    string DoSlowEcho(string str)
    {
        return str;
    }
}

