using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class CHwAsyncHandler : CAsyncServiceHandler
{
    public CHwAsyncHandler()
        : base(HwConst.sidHelloWorld)
    {
    }

    protected override void OnExceptionFromServer(ushort reqId, string errMessage, string errWhere, int errCode)
    {
        Console.WriteLine("ReqId = " + reqId + ", errMsg = " + errMessage + ", where = " + errWhere + ", errCode = " + errCode);
    }

    public string SayHelloFromClient(string fullName, int index)
    {
        string str;
        if (ProcessR1(HwConst.idSayHello, fullName, index, out str))
            return str;
        return null;
    }
};
