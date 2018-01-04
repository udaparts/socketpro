
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
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            CSocketProServer.QueueManager.WorkDirectory = "/home/yye/sp_test/";
        else
            CSocketProServer.QueueManager.WorkDirectory = "c:\\sp_test";
        using (CMySocketProServer MySocketProServer = new CMySocketProServer())
        {
            //CSocketProServer.QueueManager.MessageQueuePassword = "MyPasswordForMsgQueue";

            //test certificate, private key and DH params files are located at the directory ..\SocketProRoot\bin
            //MySocketProServer.UseSSL("server.pem", "server.pem", "test");

            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            else
            {
                CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
            }
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

        //load socketpro async sqlite server plugin located at the directory ../socketpro/bin
        IntPtr p = CSocketProServer.DllManager.AddALibrary("ssqlite");
        if (p.ToInt64() != 0) 
        {
            //monitoring sakila.db table events (DELETE, INSERT and UPDATE) for tables actor, language, category, country and film_actor
            SetSqliteDBGlobalConnectionString("usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor");
        }
        //load socketpro async queue server plugin located at the directory ../socketpro/bin
        p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes

        //load SocketPro file streaming server plugin located at the directory ../socketpro/bin
        p = CSocketProServer.DllManager.AddALibrary("ustreamfile");
        if (p.ToInt64() != 0)
        {
            SetRootDirectory("C:\\boost_1_60_0\\stage\\lib64");
        }

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
