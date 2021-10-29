using System;
using System.Data;
using MySql.Data.MySqlClient;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("MySQL .NET provider performance test against a remote MySQL backend DB");
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
        int count = 10000;
        DateTime start = DateTime.Now;
        for (int n = 0; n < count; ++n)
        {
            DataTable dt = new DataTable();
            MySqlCommand cmd = new MySqlCommand(sql, conn);
            MySqlDataAdapter adapter = new MySqlDataAdapter(cmd);
            adapter.Fill(dt);
        }
        double diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} millseconds for {1} query requests", diff, count);

        //you need to compile and run the sample project test_sharp before running the below code
        MySqlCommand cmdMysql = new MySqlCommand("USE mysqldb;delete from company where id > 3", conn);
        int res = cmdMysql.ExecuteNonQuery();
        string sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(@ID,@NAME,@ADDRESS,@Income)";
        MySqlParameter id = new MySqlParameter("@ID", MySqlDbType.Int32);
        MySqlParameter name = new MySqlParameter("@NAME", MySqlDbType.String, 64);
        MySqlParameter address = new MySqlParameter("@ADDRESS", MySqlDbType.String, 256);
        MySqlParameter income = new MySqlParameter("@Income", MySqlDbType.Double);
        cmdMysql.CommandText = sql_insert_parameter;
        cmdMysql.Parameters.Add(id);
        cmdMysql.Parameters.Add(name);
        cmdMysql.Parameters.Add(address);
        cmdMysql.Parameters.Add(income);
        cmdMysql.Prepare();
        MySqlTransaction sqlTran = conn.BeginTransaction();
        int index = 0;
        count = 50000;
        Console.WriteLine();
        Console.WriteLine("Going to insert {0} records into the table mysqldb.company", count);
        start = DateTime.Now;
        for (int n = 0; n < count; ++n)
        {
            cmdMysql.Parameters[0].Value = n + 4;
            int data = (n % 3);
            switch (data)
            {
                case 0:
                    cmdMysql.Parameters[1].Value = "Google Inc.";
                    cmdMysql.Parameters[2].Value = "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA";
                    cmdMysql.Parameters[3].Value = 66000000000.12;
                    break;
                case 1:
                    cmdMysql.Parameters[1].Value = "Microsoft Inc.";
                    cmdMysql.Parameters[2].Value = "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA";
                    cmdMysql.Parameters[3].Value = 93600000001.24;
                    break;
                default:
                    cmdMysql.Parameters[1].Value = "Apple Inc.";
                    cmdMysql.Parameters[2].Value = "1 Infinite Loop, Cupertino, CA 95014, USA";
                    cmdMysql.Parameters[3].Value = 234000000002.17;
                    break;
            }
            ++index;
            res = cmdMysql.ExecuteNonQuery();
            if (2000 == index)
            {
                sqlTran.Commit();
                //Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
                sqlTran = conn.BeginTransaction();
                index = 0;
            }
        }
        sqlTran.Commit();
        //Console.WriteLine("Commit {0} records into the table mysqldb.company", index);
        diff = (DateTime.Now - start).TotalMilliseconds;
        Console.WriteLine("Time required = {0} milliseconds for {1} insert requests", diff, count);
        Console.WriteLine();
        Console.WriteLine("Press any key to shutdown the application ......");
        Console.ReadLine();
    }
}

