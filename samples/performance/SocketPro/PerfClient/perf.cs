
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Data;

public class CPerf : CAsyncAdohandler
{
    public CPerf()
        : base(perfConst.sidCPerf)
    {
    }

    public string MyEcho(string strInput)
    {
        string MyEchoRtn;
        bool bProcessRy = ProcessR1(perfConst.idMyEchoCPerf, strInput, out MyEchoRtn);
        return MyEchoRtn;
    }

    public DataTable OpenRecords(string strSQL)
    {
        if (!ProcessR0(perfConst.idOpenRecordsCPerf, strSQL))
            return null;
        return AdoSerializer.CurrentDataTable;
    }

    public DataTable CurrentDataTable
    {
        get
        {
            return AdoSerializer.CurrentDataTable;
        }
    }
}
