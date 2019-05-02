/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CEchoSysPeer : CClientPeer
{
    [RequestAttr(TEchoDConst.idEchoMyStructCEchoSys, true)]
    private void EchoMyStruct(MyStruct my, out MyStruct EchoMyStructRtn)
    {
        EchoMyStructRtn = my;
    }

    [RequestAttr(TEchoDConst.idEchoUQueueCEchoSys)]
    private void EchoUQueue(CUQueue q, out CUQueue EchoUQueueRtn)
    {
        EchoUQueueRtn = q;
    }

    [RequestAttr(TEchoDConst.idEchoComplex0CEchoSys, true)]
    private void EchoComplex0(double d, string s, object simpleObj, bool b, out string sOut, out object EchoComplex0Rtn)
    {
        sOut = s;
        EchoComplex0Rtn = simpleObj;
    }

    [RequestAttr(TEchoDConst.idEchoComplex0CEchoSys + 1)]
    private uint DoRTest(string str, out string strOut)
    {
        strOut = str;
        return 123;
    }

    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        Console.WriteLine("Base request id = " + reqId.ToString());
    }
}

