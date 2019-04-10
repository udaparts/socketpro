using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Data;
using System.Collections.Generic;

class Program {
    static void Main(string[] args) {
        CSpConfig config = SpManager.SetConfig(true, @"c:\cyetest\socketpro\samples\stream_system\sp_config.json");
        CYourServer.Master = SpManager.GetPool("masterdb") as CSqlMasterPool<CMysql, CDataSet>;
        CYourServer.Slave = SpManager.GetPool("slavedb0") as CSqlMasterPool<CMysql, CDataSet>.CSlavePool;
        CDataSet Cache = CYourServer.Master.Cache;
        using (CYourServer server = new CYourServer(2)) {
            //Cache is ready for use now
            List<KeyValuePair<string, string>> v0 = Cache.DBTablePair;
            if (v0.Count == 0)
                Console.WriteLine("There is no table cached");
            else {
                Console.WriteLine("Table cached:");
                foreach (KeyValuePair<string, string> p in v0) {
                    Console.WriteLine("DB name = {0}, table name = {1}", p.Key, p.Value);
                }
                DataColumn[] keys = Cache.FindKeys(v0[0].Key, v0[0].Value);
                foreach (DataColumn dc in keys) {
                    Console.WriteLine("Key ordinal = {0}, key column name = {1}", dc.Ordinal, dc.ColumnName);
                }
            }
            DataTable tbl = Cache.Find("sakila", "actor", "actor_id >= 1 and actor_id <= 10");
            CYourServer.CreateTestDB();
            Console.WriteLine();

            //test certificate and private key files are located at the directory ../socketpro/bin
#if WIN32_64
            //or load cert and private key from windows system cert store
            server.UseSSL("C:\\cyetest\\socketpro\\bin\\intermediate.pfx", "", "mypassword");
#else //non-windows platforms
            server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
#endif
            //start listening socket with standard TLSv1.x security
            if (!server.Run(20911))
                Console.WriteLine("Error happens with error message = " + CSocketProServer.ErrorMessage);

            Console.WriteLine("Press any key to shut down the application ......");
            Console.ReadLine();
            CYourServer.Slave.ShutdownPool();
            CYourServer.Master.ShutdownPool();
        }
    }
}
