

using System;
using System.Data;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

public class CPerf : CAsyncAdohandler 
{
    public CPerf(CClientSocket cs, IAsyncResultsHandler ar)
        : base(PerfConst.sidCPerf, cs, ar)
    {
    }

    public CPerf(CClientSocket cs)
        : base(PerfConst.sidCPerf, cs)
    {
    }

    public CPerf()
        : base(PerfConst.sidCPerf)
    {
    }

	protected string m_MyEchoRtn;
	public void MyEchoAsync(string strInput)
	{
        SendRequest(PerfConst.idMyEchoCPerf, strInput);
	}

    public void OpenRecordsAsync(string strSQL)
    {
        SendRequest(PerfConst.idOpenRecords, strSQL);
    }

	//We can process returning results inside the function.
	protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
	{
		switch(sRequestID)
		{
	    case PerfConst.idMyEchoCPerf:
		    UQueue.Load(out m_MyEchoRtn);
		    break;
        case PerfConst.idOpenRecords:
            break;
	    default:
            base.OnResultReturned(sRequestID, UQueue); //chain down for processing ADO.NET objects
		    break;
		}
	}
	public string MyEcho(string strInput)
	{
		MyEchoAsync(strInput);
		GetAttachedClientSocket().WaitAll();
		return m_MyEchoRtn;
	}

    public DataTable OpenRecords(string strSQL)
    {
        GetAttachedClientSocket().BeginBatching();
        OpenRecordsAsync(strSQL);
        GetAttachedClientSocket().Commit(true);
        GetAttachedClientSocket().WaitAll();
        return m_AdoSerialier.CurrentDataTable;
    }

    public DataTable CurrentDataTable
    {
        get
        {
            return m_AdoSerialier.CurrentDataTable;
        }
    }
}
