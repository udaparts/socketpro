using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Data;
using System.Collections.Generic;

#if USE_SQLITE
using CMaster = SocketProAdapter.CSqlMasterPool<SocketProAdapter.ClientSide.CSqlite, SocketProAdapter.CDataSet>;
#else
using CMaster = SocketProAdapter.CSqlMasterPool<SocketProAdapter.ClientSide.CMysql, SocketProAdapter.CDataSet>;
#endif

class Program
{
    static void Main(string[] args)
    {
        //load settings from some configuration file
        CConfig config = CConfig.GetConfig();
        if (config.m_vccSlave.Count == 0 || config.m_nMasterSessions == 0 || config.m_slave_threads == 0 || config.m_sessions_per_host == 0)
        {
            Console.WriteLine("Wrong settings for remote MySQL master and slave servers, and press any key to stop the server ......");
            Console.ReadLine();
            return;
        }
        using (CYourServer server = new CYourServer(config.m_main_threads))
        {
            //start two socket pools, master and slave
            CYourServer.StartMySQLPools();

            //Cache is ready for use now
            List<KeyValuePair<string, string>> v0 = CYourServer.Master.Cache.DBTablePair;
            if (v0.Count == 0)
                Console.WriteLine("There is no table cached");
            else
            {
                Console.WriteLine("Table cached:");
                foreach (KeyValuePair<string, string> p in v0)
                {
                    Console.WriteLine("DB name = {0}, table name = {1}", p.Key, p.Value);
                }
                DataColumn[] keys = CYourServer.Master.Cache.FindKeys(v0[0].Key, v0[0].Value);
                foreach (DataColumn dc in keys)
                {
                    Console.WriteLine("Key ordinal = {0}, key column name = {1}", dc.Ordinal, dc.ColumnName);
                }
            }
#if USE_SQLITE
            DataTable tbl = CYourServer.Master.Cache.Find("main", "actor", "actor_id >= 1 and actor_id <= 10");
#else
            DataTable tbl = CYourServer.Master.Cache.Find("sakila", "actor", "actor_id >= 1 and actor_id <= 10");
#endif
            CYourServer.CreateTestDB();
            Console.WriteLine();

            //test certificate and private key files are located at the directory ../socketpro/bin
#if WIN32_64
            if (config.m_store_or_pfx.IndexOf(".pfx") == -1)
            {
                //or load cert and private key from windows system cert store
                server.UseSSL(config.m_store_or_pfx/*"my"*/, config.m_password_or_subject, "");
            }
            else
            {
                server.UseSSL(config.m_store_or_pfx, "", config.m_password_or_subject);
            }
#else //non-windows platforms
            server.UseSSL(config.m_cert, config.m_key, config.m_password_or_subject);
#endif
            //start listening socket with standard TLSv1.x security
            if (!server.Run(config.m_nPort, 32, !config.m_bNoIpV6))
                Console.WriteLine("Error happens with error message = " + CSocketProServer.ErrorMessage);

            Console.WriteLine("Press any key to shut down the application ......");
            Console.ReadLine();
            CYourServer.Slave.ShutdownPool();
            CYourServer.Master.ShutdownPool();
        }
    }
}
