/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Data;
using System.Data.SqlClient;

//server implementation for service CPerf
public class CPerfPeer : CAdoClientPeer
{
    private readonly SqlConnection m_sqlConnection = new SqlConnection("Data Source=localhost;Integrated Security=SSPI;Initial Catalog=AdventureWorks2012");

    protected override void OnReleaseResource(bool closing, uint nInfo)
    {
        m_sqlConnection.Close();
    }

    [RequestAttr(perfConst.idMyEchoCPerf)]
    private string MyEcho(string strInput)
    {
        return strInput;
    }

    [RequestAttr(perfConst.idOpenRecordsCPerf, true)]
    private void OpenRecords(string strSQL)
    {
        IDataReader dr = null;
        try
        {
            if (m_sqlConnection.State != ConnectionState.Open)
                m_sqlConnection.Open();
            dr = (new SqlCommand(strSQL, m_sqlConnection)).ExecuteReader();
            Send(dr);
        }
        catch (Exception err)
        {
            Console.WriteLine(err.Message);
        }
        finally
        {
            if (dr != null)
                dr.Close();
        }
    }
}

public class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(perfConst.sidCPerf)]
    private CSocketProService<CPerfPeer> m_CPerf = new CSocketProService<CPerfPeer>();
}

