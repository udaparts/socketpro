
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using System.Data;

public class CRAdo : CAsyncAdohandler 
{
    public CRAdo() 
        : base(RAdoConst.sidCRAdo)
    {
    }

    public CRAdo(CClientSocket cs)
        : base(RAdoConst.sidCRAdo, cs)
    {
    }

    public CRAdo(CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler)
        : base(RAdoConst.sidCRAdo, cs, DefaultAsyncResultsHandler)
    {
    }

	public DataTable CurrentDataTable
	{
		get
		{
			return m_AdoSerialier.CurrentDataTable;
		}
	}

	public DataSet GetDataSet(string strSQL0, string strSQL2)
	{
        bool bProcessRy = ProcessR0(RAdoConst.idGetDataSetCRAdo, strSQL0, strSQL2);
		return m_AdoSerialier.CurrentDataSet;
	}

	public DataTable GetDataReader(string strSQL)
	{
        bool bProcessRy = ProcessR0(RAdoConst.idGetDataReaderCRAdo, strSQL);
		return m_AdoSerialier.CurrentDataTable;
	}

	public bool SendDataSet(DataSet ds)
	{
        bool b;
        Send(ds);
        bool bProcessR1 = ProcessR1(RAdoConst.idSendDataSetCRAdo, out b);
		return b;
	}

	public bool SendDataReader(IDataReader dr)
	{
        bool b;
        Send(dr);
        bool bProcessRy = ProcessR1(RAdoConst.idSendDataReaderCRAdo, out b);
		return b;
	}
}
