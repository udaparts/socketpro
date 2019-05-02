/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
using System.Data;
using System.Runtime.InteropServices;
using System.Data.SqlClient;

//server implementation for service CMyAdoHandler
public class CAsyncAdoPeer : CAdoClientPeer //CClientPeer
{
    private static string m_strConn = "server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind";
	
	protected void GetDataTable(string strSQL, out string strError)
	{
		strError = null;
        IDataReader dr = null;
        SqlConnection conn = null;
        try
        {
            conn = new SqlConnection(m_strConn);
            conn.Open();
            SqlCommand cmd = new SqlCommand(strSQL, conn);
            dr = cmd.ExecuteReader();
        }
        catch (Exception err)
        {
            conn.Close();
            Console.WriteLine(err.Message);
            strError = err.Message;
            return;
        }
        Send(dr);
        conn.Close();
	}

	protected void ExecuteNoQuery(string strSQL, out string strError, out int ExecuteNoQueryRtn)
	{
		strError = null;
		ExecuteNoQueryRtn = 0;
        SqlConnection conn = null;
        try
        {
            conn = new SqlConnection(m_strConn);
            conn.Open();
            SqlCommand cmd = new SqlCommand(strSQL, conn);
            ExecuteNoQueryRtn = cmd.ExecuteNonQuery();
        }
        catch (Exception err)
        {
            conn.Close();
            Console.WriteLine(err.Message);
            strError = err.Message;
            return;
        }
        conn.Close();
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{

	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case AsyncAdoWebConst.idGetDataTableCAsyncAdo:
		{
			string strSQL = null;
			string strError;
			m_UQueue.Load(out strSQL);
            bool bBatching = IsBatching;
            if (!bBatching)
                StartBatching();
			GetDataTable(strSQL, out strError);
            SendResult(sRequestID, strError);
            if (!bBatching)
                CommitBatching();
		}
			break;
		case AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo:
		{
			string strSQL = null;
			string strError;
			int ExecuteNoQueryRtn;
			m_UQueue.Load(out strSQL);
            bool bBatching = IsBatching;
            if (!bBatching)
                StartBatching();
			ExecuteNoQuery(strSQL, out strError, out ExecuteNoQueryRtn);
            SendResult(sRequestID, strError, ExecuteNoQueryRtn);
            if (!bBatching)
                CommitBatching();
		}
			break;
		default:
			break;
		}
		return 0;
	}
}

public class CMySocketProServer : CSocketProServer
{
	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
		//give permission to all
		return true;
	}

	protected override void OnAccept(int hSocket, int nError)
	{
		//when a socket is initially established
	}

	protected override void OnClose(int hSocket, int nError)
	{
		//when a socket is closed
	}

    protected override bool OnSettingServer()
    {
        //try amIntegrated and amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        AddService();

        return true;
    }

    private CSocketProService<CAsyncAdoPeer> m_CAsyncAdo = new CSocketProService<CAsyncAdoPeer>();

    [DllImport("uodbsvr.dll")]
    private static extern void SetGlobalOLEDBConnectionString([MarshalAs(UnmanagedType.LPWStr)]string strOLEDBConnection);

	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CAsyncAdo.AddMe(AsyncAdoWebConst.sidCAsyncAdo, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CAsyncAdo.AddSlowRequest(AsyncAdoWebConst.idGetDataTableCAsyncAdo);
		ok = m_CAsyncAdo.AddSlowRequest(AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo);

        //load UDAParts generic remote database service using MS OLEDB technology
        IntPtr libRDB = CBaseService.AddALibrary("uodbsvr.dll");
        if (libRDB == IntPtr.Zero)
        {
            Console.WriteLine("Can't load remote database service library uodbsvr.dll!");
        }
        else
        {
            SetGlobalOLEDBConnectionString("Provider=sqloledb;Data Source=CHARLIEDEVVM;User ID=sa;Password=cye;Initial Catalog=Northwind");
        }
	}
}

