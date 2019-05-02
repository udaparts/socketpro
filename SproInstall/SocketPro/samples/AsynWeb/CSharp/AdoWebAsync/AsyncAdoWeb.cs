
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using System.Data;

public class CMyAdoHandler : CAsyncAdohandler 
{
    public CMyAdoHandler()
        : base(AsyncAdoWebConst.sidCAsyncAdo)
    {
    }
	
	public string m_strError_GetDataTable;
	public void GetDataTableAsync(string strSQL)
	{
        SendRequest(AsyncAdoWebConst.idGetDataTableCAsyncAdo, strSQL);
	}

	public string m_strError_ExecuteNoQuery;
	public int m_ExecuteNoQueryRtn;
	public void ExecuteNoQueryAsync(string strSQL)
	{
        SendRequest(AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo, strSQL);
	}

	//We can process returning results inside the function.
	protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
	{
		switch(sRequestID)
		{
            case CAsyncAdoSerializationHelper.idDataReaderHeaderArrive:
            case CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
            case CAsyncAdoSerializationHelper.idEndDataReader:
                base.OnResultReturned(sRequestID, UQueue);
                break;
		    case AsyncAdoWebConst.idGetDataTableCAsyncAdo:
                UQueue.Load(out m_strError_GetDataTable);
			    break;
		    case AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo:
			    UQueue.Load(out m_strError_ExecuteNoQuery);
			    UQueue.Pop(out m_ExecuteNoQueryRtn);
			    break;
		    default:
			break;
		}
	}

    public DataTable CurrentDataTable
    {
        get
        {
            return m_AdoSerialier.CurrentDataTable;
        }
    }
	public DataTable GetDataTable(string strSQL, out string strError)
	{
		GetDataTableAsync(strSQL);
		GetAttachedClientSocket().WaitAll();
		strError = m_strError_GetDataTable;
        return m_AdoSerialier.CurrentDataTable;
	}

	public int ExecuteNoQuery(string strSQL, out string strError)
	{
		ExecuteNoQueryAsync(strSQL);
		GetAttachedClientSocket().WaitAll();
		strError = m_strError_ExecuteNoQuery;
		return m_ExecuteNoQueryRtn;
	}
}
