using System;
using System.Data;
using System.Data.SqlClient;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("MS SQL .NET provider performance test against a remote MS SQL backend DB");
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        Console.WriteLine("Database name: ");
        string dbName = Console.ReadLine();
        string sql_conn = "Server=" + host;
        sql_conn += (";User Id=sa;Password=Smash123;Database=" + dbName);
        SqlConnection conn = new SqlConnection(sql_conn);
        conn.Open();
        Console.WriteLine("Table name: ");
        string tableName = Console.ReadLine();
        Console.WriteLine("sql filter: ");
        string filter = Console.ReadLine();
        string sql = "select * from " + tableName;
        if (filter.Length > 0)
        {
            sql += " where " + filter;
        }
        Console.WriteLine("Computing ......");
        int count = 50000;
        DateTime start = DateTime.Now;
        for (int n = 0; n < count; ++n)
        {
            DataTable dt = new DataTable();
            SqlCommand cmd = new SqlCommand(sql, conn);
            SqlDataAdapter adapter = new SqlDataAdapter(cmd);
            adapter.Fill(dt);
        }
        double diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} millseconds for {1} query requests", diff, count);

        //you need to compile and run the sample project test_sharp before running the below code
        SqlCommand cmdSql = new SqlCommand("USE sqltestdb;delete from company where id > 3", conn);
        int res = cmdSql.ExecuteNonQuery();
        string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(@ID,@NAME,@ADDRESS,@Income)";
        SqlParameter id = new SqlParameter("@ID", SqlDbType.Int);
        SqlParameter name = new SqlParameter("@NAME", SqlDbType.Char, 64);
        SqlParameter address = new SqlParameter("@ADDRESS", SqlDbType.VarChar, 256);
        SqlParameter income = new SqlParameter("@Income", SqlDbType.Float);
        cmdSql.CommandText = sql_insert_parameter;
        cmdSql.Parameters.Add(id);
        cmdSql.Parameters.Add(name);
        cmdSql.Parameters.Add(address);
        cmdSql.Parameters.Add(income);
        cmdSql.Prepare();
        SqlTransaction sqlTran = conn.BeginTransaction();
        cmdSql.Transaction = sqlTran;
        int index = 0;
        count = 250000;
        Console.WriteLine();
        Console.WriteLine("Going to insert {0} records into the table sqltestdb.company", count);
        start = DateTime.Now;
        for (int n = 0; n < count; ++n)
        {
            cmdSql.Parameters[0].Value = n + 4;
            int data = (n % 3);
            switch (data)
            {
                case 0:
                    cmdSql.Parameters[1].Value = "Google Inc.";
                    cmdSql.Parameters[2].Value = "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA";
                    cmdSql.Parameters[3].Value = 66000000000.12;
                    break;
                case 1:
                    cmdSql.Parameters[1].Value = "Microsoft Inc.";
                    cmdSql.Parameters[2].Value = "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA";
                    cmdSql.Parameters[3].Value = 93600000001.24;
                    break;
                default:
                    cmdSql.Parameters[1].Value = "Apple Inc.";
                    cmdSql.Parameters[2].Value = "1 Infinite Loop, Cupertino, CA 95014, USA";
                    cmdSql.Parameters[3].Value = 234000000002.17;
                    break;
            }
            ++index;
            res = cmdSql.ExecuteNonQuery();
            if (2000 == index)
            {
                sqlTran.Commit();
                Console.WriteLine("Commit {0} records into the table sqltestdb.company", index);
                sqlTran = conn.BeginTransaction();
                cmdSql.Transaction = sqlTran;
                index = 0;
            }
        }
        sqlTran.Commit();
        Console.WriteLine("Commit {0} records into the table sqltestdb.company", index);
        diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} milliseconds for {1} insert requests", diff, count);
        Console.WriteLine();
        Console.WriteLine("Press any key to shutdown the application ......");
        Console.ReadLine();
    }
}

