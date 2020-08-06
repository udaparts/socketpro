package all_servers;

import SPA.ServerSide.*;
import pub_sub.server.HelloWorldPeer;
import loading_balance.piConst;

import webdemo.CMyHttpPeer;

public class CMySocketProServer extends CSocketProServer {

    @ServiceAttr(ServiceID = hello_world.hwConst.sidHelloWorld)
    private final CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<>(HelloWorldPeer.class);

    //Routing requires registering two services in pair
    @ServiceAttr(ServiceID = piConst.sidPi)
    private final CSocketProService<CClientPeer> m_Pi = new CSocketProService<>(CClientPeer.class);
    @ServiceAttr(ServiceID = piConst.sidPiWorker)
    private final CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<>(CClientPeer.class);

    @ServiceAttr(ServiceID = SPA.BaseServiceID.sidHTTP)
    private final CSocketProService<CMyHttpPeer> m_http = new CSocketProService<>(CMyHttpPeer.class);

    @Override
    protected boolean OnSettingServer() {
        //amIntegrated and amMixed not supported yet
        Config.setAuthenticationMethod(tagAuthenticationMethod.amOwn);

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        PushManager.AddAChatGroup(7, "HR Department");
        PushManager.AddAChatGroup(SPA.UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Subscribe/publish for front clients");

        return CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker); //true -- ok; false -- no listening server
    }

    public static void main(String[] args) {
        try (CMySocketProServer MySocketProServer = new CMySocketProServer()) {
            //test certificate and private key files are located at ../SocketProRoot/bin
            //if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            //    MySocketProServer.UseSSL("intermediate.pfx", "", "mypassword");
            //    //MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", ""); //or load cert and private key from windows system cert store
            //} else {
            //    MySocketProServer.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            //}

            //load socketpro async sqlite library located at the directory ../bin/free_services/sqlite
            long handle = CSocketProServer.DllManager.AddALibrary("ssqlite");
            if (handle != 0) {
                //monitoring sakila.db table events (DELETE, INSERT and UPDATE) for tables actor, language, category, country and film_actor
                Sqlite.SetSqliteDBGlobalConnectionString("usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor");
            }
            //load socketPro asynchronous persistent message queue library at the directory ../bin/free_services/queue
            handle = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024); //24 * 1024 batch dequeuing size in bytes

            //load socketPro file streaming library at the directory ../bin/free_services/file
            handle = CSocketProServer.DllManager.AddALibrary("ustreamfile");

            //load MySQL/MariaDB socketPro server plugin library at the directory ../bin/free_services/mm_middle
            handle = CSocketProServer.DllManager.AddALibrary("smysql");

            //load ODBC socketPro server plugin library at the directory ../bin/win or ../bin/linux
            handle = CSocketProServer.DllManager.AddALibrary("sodbc");

            //test certificate, private key and DH params files are located at ../SocketProRoot/bin
            //MySocketProServer.UseSSL("server.pem", "server.pem", "test", "dh512.pem");
            if (!MySocketProServer.Run(20901)) {
                System.out.println("Error code = " + CSocketProServer.getLastSocketError());
            }
            System.out.println("Input a line to close the application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
