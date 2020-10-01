using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Runtime.InteropServices;

public class CMySocketProServer : CSocketProServer
{
    //SocketPro sqlite server defines, which can be found at ../socketpro/include/sqlite/usqlite_server.h
    const int ENABLE_GLOBAL_SQLITE_UPDATE_HOOK = 0x1;
    const int USE_SHARED_CACHE_MODE = 0x2;
    const int USE_UTF16_ENCODING = 0x4;
    [DllImport("ssqlite")]
    static extern void SetSqliteDBGlobalConnectionString([In] [MarshalAs(UnmanagedType.LPWStr)] string sqliteDbFile);

    [DllImport("ustreamfile")]
    static extern void SetRootDirectory([In] [MarshalAs(UnmanagedType.LPWStr)] string root);

    [ServiceAttr(hwConst.sidHelloWorld)]
    private CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<HelloWorldPeer>();

    //for db push from ms sql server
    [ServiceAttr(repConst.sidRAdoRep)]
    private CSocketProService<DBPushPeer> m_RAdoRep = new CSocketProService<DBPushPeer>();

    //Routing requires registering two services in pair
    [ServiceAttr(piConst.sidPi)]
    private CSocketProService<CClientPeer> m_Pi = new CSocketProService<CClientPeer>();
    [ServiceAttr(piConst.sidPiWorker)]
    private CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<CClientPeer>();

    [ServiceAttr(radoConst.sidRAdo)]
    private CSocketProService<RAdoPeer> m_RAdo = new CSocketProService<RAdoPeer>();

    [ServiceAttr(BaseServiceID.sidHTTP)]
    private CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();

    static void Main(string[] args)
    {
        using (CMySocketProServer server = new CMySocketProServer())
        {
            //test certificate and private key files are located at ../SocketProRoot/bin
            //if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            //    server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            //else
            //{
            //    server.UseSSL("intermediate.pfx", "", "mypassword");
            //    //or load cert and private key from windows system cert store
            //    //server.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "");
            //}

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
            SetSqliteDBGlobalConnectionString("usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor");
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
