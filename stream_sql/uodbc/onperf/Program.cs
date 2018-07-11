using System;
using System.Data;
using System.Data.Odbc;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        string mysql_conn = "DSN=ToSqlServer64;Uid=sa;Pwd=Smash123;Database=sakila";
        OdbcConnection conn = new OdbcConnection(mysql_conn);
        conn.Open();
        string sql = "select * from " + tableName;
        if (filter.Length > 0)
        {
            sql += " where " + filter;
        }
        Console.WriteLine("Computing ......");
        uint count = 50000;
        DateTime start = DateTime.Now;
        for (uint n = 0; n < count; ++n)
        {
            DataTable dt = new DataTable();
            OdbcCommand cmd = new OdbcCommand(sql, conn);
            OdbcDataAdapter adapter = new OdbcDataAdapter(cmd);
            adapter.Fill(dt);
        }
        double diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} millseconds for {1} requests", diff, count);
        Console.WriteLine("Press any key to shutdown the application ......");
        Console.ReadLine();
    }
}

