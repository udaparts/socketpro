using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class CPerfStudy : CAsyncServiceHandler
{
    public CPerfStudy()
        : base(PerfStudyConst.sidPerfStudy)
    {
    }

    public string DoEcho(string str)
    {
        string res;
        bool ok = ProcessR1(PerfStudyConst.idDoMyEcho, str, out res);
        return res;
    }

    public string DoSlowEcho(string str)
    {
        string res;
        bool ok = ProcessR1(PerfStudyConst.idSlowEcho, str, out res);
        return res;
    }
}

