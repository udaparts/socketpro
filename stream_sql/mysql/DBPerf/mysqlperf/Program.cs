using System;
using System.Data;
using MySql.Data.MySqlClient;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Database name: ");
        string dbName = Console.ReadLine();
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        string mysql_conn = "server=" + host;
        mysql_conn += (";Port=3306;Uid=root;Pwd=Smash123;Database=" + dbName);
        MySqlConnection conn = new MySqlConnection(mysql_conn);
        conn.Open();
        string sql = "select * from " + tableName;
        if (filter.Length > 0)
        {
            sql += " where " + filter;
        }
        Console.WriteLine("Computing ......");
        uint count = 10000;
        DateTime start = DateTime.Now;
        for (uint n = 0; n < count; ++n)
        {
            DataTable dt = new DataTable();
            MySqlCommand cmd = new MySqlCommand(sql, conn);
            MySqlDataAdapter adapter = new MySqlDataAdapter(cmd);
            adapter.Fill(dt);
        }
        double diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} millseconds for {1} requests", diff, count);
        Console.WriteLine("Press any key to shutdown the application ......");
        Console.ReadLine();
    }
}

