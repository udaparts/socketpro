/* **** including all of defines, service id(s) and request id(s) ***** */

//#define USE_SQLCLIENT

#if USE_SQLCLIENT
using System.Data.SqlClient;
#else
using System.Data.OleDb;
#endif

using System;
using System.Data;
using System.Runtime.InteropServices;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants

//server implementation for service CPerf
public class CPerfPeer : CAdoClientPeer //CClientPeer
{
#if USE_SQLCLIENT
    SqlCommand cmd;
    SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
    OleDbCommand cmd;
    OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
    protected override void OnSwitchFrom(int nSvsID)
    {
#if USE_SQLCLIENT
        cmd = new SqlCommand("", conn);
#else
        cmd = new OleDbCommand("", conn);
#endif 
    }

    protected override void OnReleaseResource(bool bClosing, int nInfo)
    {
        cmd.Dispose();
        if (conn.State == ConnectionState.Open)
            conn.Close();
    }
    private void OpenRecords(string strSQL)
    {
        IDataReader dr = null;
        if (conn.State != ConnectionState.Open)
        {
            try
            {
                conn.Open();
            }
            catch (Exception err)
            {
                Console.WriteLine(err.Message);
                return;
            }
        }

        cmd.CommandText = strSQL;

        try
        {
            dr = cmd.ExecuteReader();
        }
        catch (Exception err)
        {
            Console.WriteLine(err.Message);
            return;
        }
        Send(dr);
        dr.Close();
    }

	protected void MyEcho(string strInput, out string MyEchoRtn)
	{
        MyEchoRtn = strInput; 
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case PerfConst.idMyEchoCPerf:
            M_I1_R1<string, string>(MyEcho);
		    break;
		default:
			break;
		}
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
        case PerfConst.idOpenRecords:
            M_I1_R0<string>(OpenRecords);
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
        string strUID = GetUserID(hSocket);

        //password is available ONLY IF authentication method to either amOwn or amMixed
        string strPassword = GetPassword(hSocket);

        Console.WriteLine("For service = {0}, User ID = {1}, Password = {2}", nSvsID, strUID, strPassword);

        return true; //true -- permission given; otherwise permission denied
	}

	protected override void OnAccept(int hSocket, int nError)
	{
		if(nError == 0)
            Console.WriteLine("A socket is initially establised");
	}

	protected override void OnClose(int hSocket, int nError)
	{
        Console.WriteLine("A socket is closed with error code = " + nError);
	}

    protected override bool OnSettingServer()
    {
        AddService();
        return true;
    }

    private CSocketProService<CPerfPeer> m_CPerf = new CSocketProService<CPerfPeer>();
	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CPerf.AddMe(PerfConst.sidCPerf, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

        ok = m_CPerf.AddSlowRequest(PerfConst.idOpenRecords);
	}
}

