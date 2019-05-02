/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Data;

#if MONO

#else
using System.Data.SqlClient;
using Microsoft.SqlServer.Server;

#endif

//server implementation for service RAdo
public class RAdoPeer : CAdoClientPeer
{
    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        Console.WriteLine("Base request id = " + reqId);
    }

    public RAdoPeer()
    {
        OnAdonetLoaded += OnAdoNet;
    }

    private void OnAdoNet(CAdoClientPeer peer, ushort reqId)
    {
        switch (reqId)
        {
            case CAdoSerializationHelper.idDataReaderRecordsArrive:
                if (AdoSerializer.CurrentDataTable.Rows.Count > 10 * 1024)
                {
#if MONO
					//For Mono, make sure you call the below two methods to avoid memory leak
					AdoSerializer.FinalizeRecords();
#endif
                    AdoSerializer.CurrentDataTable.Clear();
#if MONO
					AdoSerializer.CurrentDataTable = AdoSerializer.CurrentDataTable.Clone();
#endif
                }
                break;
            default:
                break;
        }
        if (reqId == CAdoSerializationHelper.idEndDataReader)
        {
            if (AdoSerializer.CurrentDataTable.Rows.Count > 100)
            {
                Console.WriteLine("Table rowset size = " + AdoSerializer.CurrentDataTable.Rows.Count);
            }
            else
            {
                foreach (DataRow dr in AdoSerializer.CurrentDataTable.Rows)
                {
                    int n = 0;
                    foreach (object obj in dr.ItemArray)
                    {
                        if (n > 0)
                            Console.Write(",\t");
                        Console.Write(obj.ToString());
                        ++n;
                    }
                    Console.WriteLine();
                }
            }
        }
    }
    private bool m_suc = false;

#if MONO
	static string m_connection = "server=localhost;user=root;database=world;port=3306;password=MyPassword;";
	[RequestAttr(RAdoConst.idGetDataSetRAdo, true)]
	private void GetDataSet (string sql0, string sql1)
	{
		DataSet ds = new DataSet("MyDataSet");
	}

	[RequestAttr(RAdoConst.idGetDataReaderRAdo, true)]
	private void GetDataReader(string sql)
	{
        IDataReader dr = null;
	}

#else
    static string m_connection = "server=cyewin8;Integrated Security=SSPI;database=AdventureWorks2012";
    [RequestAttr(RAdoConst.idGetDataSetRAdo, true)]
    private void GetDataSet(string sql0, string sql1)
    {
        DataSet ds = new DataSet("MyDataSet");
        SqlConnection conn = new SqlConnection(m_connection);
        try
        {
            conn.Open();
            SqlCommand cmd = new SqlCommand(sql0, conn);
            SqlDataAdapter adapter = new SqlDataAdapter(cmd);
            SqlCommand cmd1 = new SqlCommand(sql1, conn);
            SqlDataAdapter adapter1 = new SqlDataAdapter(cmd1);
            adapter.Fill(ds, "Table1");
            adapter1.Fill(ds, "Table2");
            ulong res = Send(ds);
            m_suc = (res != CClientPeer.REQUEST_CANCELED && res != CClientPeer.SOCKET_NOT_FOUND);
        }
        catch
        {
            m_suc = false;
        }
        finally
        {
            conn.Close();
        }
    }

    [RequestAttr(RAdoConst.idGetDataReaderRAdo, true)]
    private void GetDataReader(string sql)
    {
        IDataReader dr = null;
        SqlConnection conn = new SqlConnection(m_connection);
        try
        {
            conn.Open();
            SqlCommand cmd = new SqlCommand(sql, conn);
            dr = cmd.ExecuteReader();
            ulong res = Send(dr);
            m_suc = (res != CClientPeer.REQUEST_CANCELED && res != CClientPeer.SOCKET_NOT_FOUND);
        }
        catch
        {
            m_suc = false;
        }
        finally
        {
            if (dr != null)
                dr.Close();
            conn.Close();
        }
    }
#endif

    [RequestAttr(RAdoConst.idSendDataSetRAdo)]
    private bool SendDataSet()
    {
        return m_suc;
    }

    [RequestAttr(RAdoConst.idSendDataReaderRAdo)]
    private bool SendDataReader()
    {
        return m_suc;
    }

    [RequestAttr(RAdoConst.idSendDataTableRAdo)]
    private bool SendDataTable()
    {
        return m_suc;
    }

    [RequestAttr(CAdoSerializationHelper.idDmlTriggerMessage, true)]
    private void OnDmlTriggerMessage(int triggerEvent, string fullDbObjectName, object param)
    {
#if MONO
		Console.WriteLine("FullDbTableName = {0}, event = {1}, param = {2}", fullDbObjectName, triggerEvent, param);
#else
        TriggerAction ta = (TriggerAction)triggerEvent;
        Console.WriteLine("FullDbTableName = {0}, event = {1}, param = {2}", fullDbObjectName, ta, param);
#endif
    }

    [RequestAttr(CAdoSerializationHelper.idRecordsetName, true)]
    private void OnQueryMessage(string recordsetName)
    {
        Console.WriteLine("Recordset = " + recordsetName);
    }

    [RequestAttr(CAdoSerializationHelper.idDbEventTriggerMessage, true)]
    private void OnDbTriggerMessage(int triggerEvent, string instance, string eventData)
    {
        Console.WriteLine("DB Logon event = " + instance + ", eventData = " + eventData);
    }
}
