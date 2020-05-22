/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter.ServerSide;
using System.Data;
using System.Data.SqlClient;

//server implementation for service RAdo
public class RAdoPeer : CAdoClientPeer
{
    private SqlConnection m_sqlConnection = new SqlConnection("Data Source=localhost;Integrated Security=SSPI;Initial Catalog=AdventureWorksDW2012");

	[RequestAttr(radoConst.idGetDataSetRAdo, true)]
	private void GetDataSet(string sql0, string sql1)
	{
        DataSet ds = new DataSet("MyDataSet");
        try
        {
            m_sqlConnection.Open();
            SqlCommand cmd = new SqlCommand(sql0, m_sqlConnection);
            SqlDataAdapter adapter = new SqlDataAdapter(cmd);
            SqlCommand cmd1 = new SqlCommand(sql1, m_sqlConnection);
            SqlDataAdapter adapter1 = new SqlDataAdapter(cmd1);
            adapter.Fill(ds, "Table1");
            adapter1.Fill(ds, "Table2");
            Send(ds);
        }
        catch(Exception err)
        {
            Console.WriteLine(err.Message);
        }
        finally
        {
            m_sqlConnection.Close();
        }
	}

	[RequestAttr(radoConst.idGetDataTableRAdo, true)]
	private void GetDataTable(string sql)
	{
        IDataReader dr = null;
        try
        {
            m_sqlConnection.Open();
            SqlCommand cmd = new SqlCommand(sql, m_sqlConnection);
            dr = cmd.ExecuteReader();
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
            m_sqlConnection.Close();
        }
	}
}
