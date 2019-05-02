
using System;
using System.Data;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class RAdo : CAsyncAdohandler
{
	public RAdo() : base(RAdoConst.sidRAdo)
	{
	}

    protected override void OnExceptionFromServer(ushort reqId, string errMessage, string errWhere, int errCode)
    {
        Console.WriteLine("ReqId = " + reqId + ", errMsg = " + errMessage + ", where = " + errWhere + ", errCode = " + errCode);
    }

	public DataSet GetDataSet(string sql0, string sql1)
	{
		bool bProcessRy = ProcessR0(RAdoConst.idGetDataSetRAdo, sql0, sql1);
        return AdoSerializer.CurrentDataSet;
	}

    public DataTable GetDataReader(string sql)
	{
		bool bProcessRy = ProcessR0(RAdoConst.idGetDataReaderRAdo, sql);
        return AdoSerializer.CurrentDataTable;
	}

	public bool SendDataSet(DataSet ds)
	{
		bool SendDataSetRtn;
        bool ok = Send(ds);
		ok = ProcessR1(RAdoConst.idSendDataSetRAdo, out SendDataSetRtn);
		return SendDataSetRtn;
	}

	public bool SendDataReader(IDataReader dr)
	{
		bool SendDataReaderRtn;
        bool ok = Send(dr);
		ok = ProcessR1(RAdoConst.idSendDataReaderRAdo, out SendDataReaderRtn);
		return SendDataReaderRtn;
	}

    public bool SendDataTable(DataTable dt)
    {
        bool SendDataTableRtn;
        bool ok = Send(dt);
        ok = ProcessR1(RAdoConst.idSendDataTableRAdo, out SendDataTableRtn);
        return SendDataTableRtn;
    }
}
