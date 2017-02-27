
using System;
using System.Data;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        Console.WriteLine("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }

    //create a service for messages from database event
    [ServiceAttr(CMySqlCachePeer.sidMySqlCache)] //CMySqlCachePeer.sidMySqlCache = 268436480
    private CSocketProService<CMySqlCachePeer> m_cache = new CSocketProService<CMySqlCachePeer>();

    static void Main(string[] args)
    {
        CMySqlCachePeer.CCacheSource source = new CMySqlCachePeer.CCacheSource("root", "Smash123");

        //server cache contains five tables (city, language, store, category, and staff) of MySql sample database sakila
        source.TableFilter["sakila.city"] = "";
        source.TableFilter["sakila.language"] = "";
        source.TableFilter["sakila.store"] = "";
        source.TableFilter["sakila.category"] = "";
        source.TableFilter["sakila.staff"] = "";

        const string mysql_host = "ws-yye-1";
        //query all the above five tables into cache on one given MySql server located at ws-yye-1
        string errMsg = CMySqlCachePeer.InitializeCache(mysql_host, source);
        if (errMsg != null && errMsg.Length > 0)
            Console.WriteLine(errMsg);
        else
        {
#if DEBUG
            Console.WriteLine("Real-time server cache initialized from MySql server " + mysql_host);
            //query a datatable from cache
            DataTable dt = CMySqlCachePeer.Get(mysql_host, "sakila.staff");
            //query a subset of records from cache
            dt = CMySqlCachePeer.Get(mysql_host, "sakila.city", "city_id >= 10 and city_id <= 19", "city_id");
#endif
        }
        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}

