

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class CTOne : CAsyncServiceHandler
{
    public CTOne()
        : base(TOneConst.sidCTOne)
    {
    }

    public int QueryCount()
    {
        int QueryCountRtn;
        bool bProcessRy = ProcessR1(TOneConst.idQueryCountCTOne, out QueryCountRtn);
        return QueryCountRtn;
    }

    public int QueryGlobalCount()
    {
        int QueryGlobalCountRtn;
        bool bProcessRy = ProcessR1(TOneConst.idQueryGlobalCountCTOne, out QueryGlobalCountRtn);
        return QueryGlobalCountRtn;
    }

    public int QueryGlobalFastCount()
    {
        int QueryGlobalFastCountRtn;
        bool bProcessRy = ProcessR1(TOneConst.idQueryGlobalFastCountCTOne, out QueryGlobalFastCountRtn);
        return QueryGlobalFastCountRtn;
    }

    public void Sleep(int nTime)
    {
        bool bProcessRy = ProcessR0(TOneConst.idSleepCTOne, nTime);
    }

    public object Echo(object objInput)
    {
        object EchoRtn;
        bool bProcessRy = ProcessR1(TOneConst.idEchoCTOne, objInput, out EchoRtn);
        return EchoRtn;
    }

    public bool EchoEx(sbyte[] str, string wstr, MyStruct ms, out sbyte[] strOut, out string wstrOut)
    {
        bool EchoExRtn;
        bool bProcessRy = ProcessR3(TOneConst.idEchoExCTOne, str, wstr, ms, out strOut, out wstrOut, out EchoExRtn);
        return EchoExRtn;
    }
}
