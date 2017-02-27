
using System;
using System.ServiceModel;
using System.Data;
using System.Data.SqlClient;

namespace DefMyCalls
{
    //[ServiceContract(SessionMode = SessionMode.Required)]
    [ServiceContract]
    public interface IMyCalls
    {
        [OperationContract]
        string MyEcho(string strInput);
        [OperationContract]
        DataTable OpenRowset(string strSQL);
    }

    public class CMyCallsImpl : IMyCalls
    {
        private SqlConnection m_sqlConnection = new SqlConnection("Data Source=localhost;Integrated Security=SSPI;Initial Catalog=AdventureWorks2012");

#region IMyCalls Members
        public string MyEcho(string strInput)
        {
            return strInput;
        }
        public DataTable OpenRowset(string strSQL)
        {
            DataTable dt = null;
            if (m_sqlConnection.State != ConnectionState.Open)
                m_sqlConnection.Open();
            SqlDataAdapter adapter = new SqlDataAdapter(strSQL, m_sqlConnection);
            dt = new DataTable("MyDataTable");
            adapter.Fill(dt);
            dt.RemotingFormat = SerializationFormat.Binary;
            return dt;
        }
#endregion
    }
}
