//#define USE_SQLCLIENT

#if USE_SQLCLIENT
using System.Data.SqlClient;
#else
using System.Data.OleDb;
#endif

using System;
using System.Data;


namespace DefMyCallsInterface
{
    public interface IMyCalls
    {
        DataTable OpenRowset(string strSQL);
        string MyEcho(string strInput);
    }

    public class CMyCallsImpl : MarshalByRefObject, IMyCalls
    {
#if USE_SQLCLIENT
        SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
        OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=C:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
#region IMyCalls Members
        public System.Data.DataTable OpenRowset(string strSQL)
        {
            DataTable dt = null;
            if(conn.State != ConnectionState.Open)
                conn.Open();
#if USE_SQLCLIENT
            SqlCommand cmd = new SqlCommand(strSQL, conn);
            SqlDataAdapter adapter = new SqlDataAdapter(cmd);
#else
            OleDbCommand cmd = new OleDbCommand(strSQL, conn);
            OleDbDataAdapter adapter = new OleDbDataAdapter(cmd);             
#endif
            dt = new DataTable("MyDataSet");
            adapter.Fill(dt);
//            ds.SchemaSerializationMode = SchemaSerializationMode.ExcludeSchema;
            dt.RemotingFormat = SerializationFormat.Binary;
            return dt;
        }
        public string MyEcho(string strInput)
        {
            return strInput;
        }
#endregion
    }
}
