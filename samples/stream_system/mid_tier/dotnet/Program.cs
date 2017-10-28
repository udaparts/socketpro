using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class Program
{
    static void Main(string[] args)
    {
        CConfig config = CConfig.GetConfig();
        if (config.m_vccSlave.Count == 0 || config.m_nMasterSessions == 0 || config.m_nSlaveSessions == 0)
        { 
            Console.WriteLine("Wrong settings for remote MySQL master and slave servers, and press any key to stop the server ......");
            Console.ReadLine();
            return;
        }
        using(CYourServer server = new CYourServer(config.m_main_threads))
        {
            CYourServer.StartMySQLPools();
            //Cache is ready for use now
            List<KeyValuePair<string, string>> v0 = CSqlMasterPool<CMysql, CDataSet>.Cache.DBTablePair;
            if (v0.Count == 0)
                Console.WriteLine("There is no table cached");
            else {
                Console.WriteLine("Table cached:");
                foreach (KeyValuePair<string, string> p in v0) {
                    Console.WriteLine("DB name = {0}, table name = {1}", p.Key, p.Value);
                }
                DataColumn []keys = CSqlMasterPool<CMysql, CDataSet>.Cache.FindKeys(v0[0].Key, v0[0].Value);
                foreach (DataColumn dc in keys)
                {
                    Console.WriteLine("Key ordinal = {0}, key column name = {1}", dc.Ordinal, dc.ColumnName);
                }
            }
            Console.WriteLine();
        }
    }
}
