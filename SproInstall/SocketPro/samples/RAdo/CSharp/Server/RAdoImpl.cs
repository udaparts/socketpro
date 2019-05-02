/* **** including all of defines, service id(s) and request id(s) ***** */
//#define USE_SQLCLIENT

#if USE_SQLCLIENT
using System.Data.SqlClient;
#else
using System.Data.OleDb;
#endif

using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Data;
using USOCKETLib; //you may need it for accessing various constants

//server implementation for service CRAdo
public class CRAdoPeer : CAdoClientPeer //CClientPeer
{
	private bool m_bSuc = false;

	protected void GetDataSet(string strSQL0, string strSQL1)
	{
		DataSet ds = new DataSet("MyDataSet");
#if USE_SQLCLIENT
		SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
        OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
		try
		{
			conn.Open();
		}
		catch (Exception err)
		{
			Console.WriteLine(err.Message);
			return;
		}
#if USE_SQLCLIENT
		SqlCommand cmd = new SqlCommand(strSQL0, conn);
		SqlDataAdapter adapter = new SqlDataAdapter(cmd);
		SqlCommand cmd1 = new SqlCommand(strSQL1, conn);
		SqlDataAdapter adapter1 = new SqlDataAdapter(cmd1);
#else
        OleDbCommand cmd = new OleDbCommand(strSQL0, conn);
        OleDbDataAdapter adapter = new OleDbDataAdapter(cmd);
        OleDbCommand cmd1 = new OleDbCommand(strSQL1, conn);
        OleDbDataAdapter adapter1 = new OleDbDataAdapter(cmd1);
#endif
		try
		{
			adapter.Fill(ds, "Table1");
			adapter1.Fill(ds, "Table2");
		}
		catch (Exception err)
		{
			Console.WriteLine(err.Message);
			conn.Close();
			return;
		}
		Send(ds);
		conn.Close();
	}

	protected void GetDataReader(string strSQL)
	{
		IDataReader dr = null;
#if USE_SQLCLIENT
		SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
        OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
		try
		{
			conn.Open();
		}
		catch (Exception err)
		{
			Console.WriteLine(err.Message);
			return;
		}

#if USE_SQLCLIENT
		SqlCommand cmd = new SqlCommand(strSQL, conn);
#else
        OleDbCommand cmd = new OleDbCommand(strSQL, conn);
#endif
		try
		{
			dr = cmd.ExecuteReader();
		}
		catch (Exception err)
		{
			Console.WriteLine(err.Message);
			conn.Close();
			return;
		}
		Send(dr);
        dr.Close();
		conn.Close();
	}

	protected void SendDataSet(out bool SendDataSetRtn)
	{
        DataSet ds = m_AdoSerialier.CurrentDataSet;

        //do whatever you like here ......

        SendDataSetRtn = m_bSuc; 
	}

	protected void SendDataReader(out bool SendDataReaderRtn)
	{
        DataTable dt = m_AdoSerialier.CurrentDataTable;

        //do whatever you like here ......

        SendDataReaderRtn = m_bSuc;
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
			case CAsyncAdoSerializationHelper.idDataSetHeaderArrive:
			case CAsyncAdoSerializationHelper.idDataTableHeaderArrive:
			case CAsyncAdoSerializationHelper.idDataReaderHeaderArrive:
			case CAsyncAdoSerializationHelper.idEndDataTable:
			case CAsyncAdoSerializationHelper.idEndDataReader:
			case CAsyncAdoSerializationHelper.idEndDataSet:
				base.OnFastRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
				break;
			case RAdoConst.idSendDataSetCRAdo:
                M_I0_R1<bool>(SendDataSet);
				break;
			case RAdoConst.idSendDataReaderCRAdo:
                M_I0_R1<bool>(SendDataReader);
				break;
			default:
                SendResult(sRequestID);
				break;
		}
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
			case CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
			case CAsyncAdoSerializationHelper.idDataTableRowsArrive:
				base.OnSlowRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
				break;
			case RAdoConst.idGetDataSetCRAdo:
                M_I2_R0<string, string>(GetDataSet);
				break;
			case RAdoConst.idGetDataReaderCRAdo:
                M_I1_R0<string>(GetDataReader);
			    break;
		default:
            SendResult(sRequestID);
			break;
		}
		return 0;
	}
}

public class CMySocketProServer : CSocketProServer
{
	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
		Console.WriteLine("Switch to service = " + nSvsID);
		//give permission to all
		return true;
	}

	protected override void OnAccept(int hSocket, int nError)
	{
		//when a socket is initially established
	}

	protected override void OnClose(int hSocket, int nError)
	{
		Console.WriteLine("Socket closed = " + hSocket);
	}

    protected override bool OnSettingServer()
    {
        //try amIntegrated and amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        AddService();

        return true;
    }

    private CSocketProService<CRAdoPeer> m_CRAdo = new CSocketProService<CRAdoPeer>();

	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CRAdo.AddMe(RAdoConst.sidCRAdo, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CRAdo.AddSlowRequest(RAdoConst.idGetDataSetCRAdo);
		ok = m_CRAdo.AddSlowRequest(RAdoConst.idGetDataReaderCRAdo);
        ok = m_CRAdo.AddSlowRequest(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive);
        ok = m_CRAdo.AddSlowRequest(CAsyncAdoSerializationHelper.idDataTableRowsArrive);
	}
}

