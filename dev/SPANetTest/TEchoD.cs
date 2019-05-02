
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;


public class CEchoSys : CAsyncServiceHandler
{
    public CEchoSys()
        : base(TEchoDConst.sidCEchoSys)
    {
    }

    public MyStruct EchoMyStruct(MyStruct my)
    {
        MyStruct EchoMyStructRtn;
        bool bProcessRy = ProcessR1(TEchoDConst.idEchoMyStructCEchoSys, my, out EchoMyStructRtn);
        return EchoMyStructRtn;
    }

    public CUQueue EchoUQueue(CUQueue q)
    {
        CUQueue EchoUQueueRtn;
        bool bProcessRy = ProcessR1(TEchoDConst.idEchoUQueueCEchoSys, q, out EchoUQueueRtn);
        return EchoUQueueRtn;
    }

    public object EchoComplex0(double d, string s, object simpleObj, bool b, out string sOut)
    {
        object EchoComplex0Rtn;
        bool bProcessRy = ProcessR2(TEchoDConst.idEchoComplex0CEchoSys, d, s, simpleObj, b, out sOut, out EchoComplex0Rtn);
        return EchoComplex0Rtn;
    }

    public uint ToTest(string str, out string strOut)
    {
        uint rtn;
        bool bProcessRy = ProcessR2(TEchoDConst.idEchoComplex0CEchoSys + 1, str, out strOut, out rtn);
        return rtn;
    }
}
