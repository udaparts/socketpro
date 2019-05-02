

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class CServerQueue : CAsyncServiceHandler
{
    public CServerQueue()
        : base(SQueueConst.sidCServerQueue)
    {
    }
}
