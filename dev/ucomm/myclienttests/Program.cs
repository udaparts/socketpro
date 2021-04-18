using System;
using System.Data;
using Microsoft.Data.SqlClient;

namespace myclienttests
{
    class Program
    {
        static void Main(string[] args)
        {
            string strConn = "Server=windesk;Database=sakila;user id=sa;password=Smash123;";
            try
            {
                using (SqlConnection conn = new SqlConnection(strConn))
                {
                    conn.Open();
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
