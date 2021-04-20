using System;
using System.Data;
using Microsoft.Data.SqlClient;

namespace myclienttests
{
    class Program
    {
        static void Main(string[] args)
        {
            string strConn = "Server=windesk;Database=AdventureWorks;user id=sa;password=Smash123;Packet Size=2920";
            try
            {
                using (SqlConnection conn = new SqlConnection(strConn))
                {
                    conn.Open();
                    string sql = "use sqltestdb;select employeeid,MyImAGE from employee where employeeid=1";
                    SqlCommand cmd = new SqlCommand(sql, conn);
                    SqlDataReader reader = cmd.ExecuteReader();
                    do
                    {
                        while (reader.Read())
                        {

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
