using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Runtime.InteropServices;

public class CMySocketProServer : CSocketProServer
{
    [DllImport("ssqlite", EntryPoint="SetSPluginGlobalOptions")]
    static extern void SQLite_SetSPluginGlobalOptions([In] [MarshalAs(UnmanagedType.LPStr)] string jsonUtf8);

    [DllImport("smysql", EntryPoint = "DoSPluginAuthentication")]
    static extern int MySQL_Authentication(ulong hSocket, [In][MarshalAs(UnmanagedType.LPWStr)] string userId, [In][MarshalAs(UnmanagedType.LPWStr)] string password, uint svsId, [In][MarshalAs(UnmanagedType.LPWStr)] string toMysql);

    [DllImport("sodbc", EntryPoint = "DoSPluginAuthentication")]
    static extern int ODBC_Authentication(ulong hSocket, [In][MarshalAs(UnmanagedType.LPWStr)] string userId, [In][MarshalAs(UnmanagedType.LPWStr)] string password, uint svsId, [In][MarshalAs(UnmanagedType.LPWStr)] string dsn);

    [ServiceAttr(hwConst.sidHelloWorld)]
    private CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<HelloWorldPeer>();

    [ServiceAttr(BaseServiceID.sidHTTP)]
    private CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();

    //Routing requires registering two services in pair
    [ServiceAttr(piConst.sidPi)]
    private CSocketProService<CClientPeer> m_Pi = new CSocketProService<CClientPeer>();
    [ServiceAttr(piConst.sidPiWorker)]
    private CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<CClientPeer>();

    [ServiceAttr(radoConst.sidRAdo)]
    private CSocketProService<RAdoPeer> m_RAdo = new CSocketProService<RAdoPeer>();

    //for db push from ms sql server
    [ServiceAttr(repConst.sidRAdoRep)]
    private CSocketProService<DBPushPeer> m_RAdoRep = new CSocketProService<DBPushPeer>();

    static void Main(string[] args)
    {
        using (CMySocketProServer server = new CMySocketProServer())
        {
            //test certificate and private key files are located at ../socketpro/bin
            /*
            if (System.Environment.OSVersion.Platform == PlatformID.Unix)
                server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            else
            {
                server.UseSSL("intermediate.pfx", "", "mypassword");
                //or load cert and private key from windows system cert store
                //server.UseSSL("root", "UDAParts Intermediate CA", "");
            }
            */
            if (!server.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }

    protected override bool OnSettingServer()
    {
        //amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        PushManager.AddAChatGroup(7, "HR Department");
        PushManager.AddAChatGroup(SocketProAdapter.UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Subscribe/publish for front clients");

        //load socketpro async sqlite library located at the directory ../bin/free_services/sqlite
        IntPtr p = CSocketProServer.DllManager.AddALibrary("ssqlite");
        if (p.ToInt64() != 0) 
        {
            //monitoring sakila.db table events (DELETE, INSERT and UPDATE) for tables actor, language, category, country and film_actor
            SQLite_SetSPluginGlobalOptions("{\"global_connection_string\":\"usqlite.db\",\"monitored_tables\":\"sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor\"}");
        }
        //load socketPro asynchronous persistent message queue library at the directory ../bin/free_services/queue
        p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes

        //load socketPro file streaming library at the directory ../bin/free_services/file
        p = CSocketProServer.DllManager.AddALibrary("ustreamfile");

        //load MySQL/MariaDB socketPro server plugin library at the directory ../bin/free_services/mm_middle
        p = CSocketProServer.DllManager.AddALibrary("smysql");

        //load ODBC socketPro server plugin library at the directory ../bin/win or ../bin/linux
        p = CSocketProServer.DllManager.AddALibrary("sodbc");

        bool ok = CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);

        return true; //true -- ok; false -- no listening server
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        //you can use database for authentication by calling MySQL_Authentication or ODBC_Authentication
        Console.WriteLine("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }

    protected override void OnClose(ulong hSocket, int nError)
    {
        CBaseService bs = CBaseService.SeekService(hSocket);
        if (bs != null)
        {
            CSocketPeer sp = bs.Seek(hSocket);
            // ......
        }
    }
}
