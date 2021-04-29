using System;
using System.Data;
using Microsoft.Data.SqlClient;

namespace myclienttests
{
    class Program
    {
        static void Main(string[] args)
        {
            string strConn = "Server=windesk;Database=sqltestdb;user id=sa;password=Smash123;Packet Size=2920";
            try
            {
                using (SqlConnection conn = new SqlConnection(strConn))
                {
                    conn.Open();
                    string sql = "Select myxml from test_rare1 where testid=15373";
                    SqlCommand cmd = new SqlCommand(sql, conn);
                    SqlDataReader reader = cmd.ExecuteReader();
                    do
                    {
                        while (reader.Read())
                        {
                            var data = reader.GetSqlValue(0);
                        }
                        if (!reader.NextResult())
                            break;
                        while (reader.Read())
                        {

                        }
                    } while (false);
                    reader.Close();
                    conn.Close();
                }
            }
            catch (Exception ex)
            {
                //display error message
                Console.WriteLine("Exception: " + ex.Message);
            }
        }
    }
}
