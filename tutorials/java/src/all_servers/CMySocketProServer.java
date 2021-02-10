package all_servers;

import SPA.ServerSide.*;
import pub_sub.server.HelloWorldPeer;
import loading_balance.piConst;

import webdemo.CMyHttpPeer;

public class CMySocketProServer extends CSocketProServer {

    @Override
    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        int res = Plugin.AUTHENTICATION_NOT_IMPLEMENTED;
        switch (nSvsID) {
            case hello_world.hwConst.sidHelloWorld:
            case SPA.BaseServiceID.sidHTTP:
            case piConst.sidPi:
            case piConst.sidPiWorker:
            case SPA.BaseServiceID.sidFile:
            case SPA.BaseServiceID.sidQueue:
                res = Plugin.AUTHENTICATION_OK; //give permission to known services without authentication
                break;
            case SPA.BaseServiceID.sidODBC:
                res = Plugin.DoSPluginAuthentication("sodbc", hSocket, userId, password, nSvsID, "DRIVER={ODBC Driver 17 for SQL Server};Server=(local);database=sakila");
                break;
            case SPA.ClientSide.CMysql.sidMysql:
                res = Plugin.DoSPluginAuthentication("smysql", hSocket, userId, password, nSvsID, "database=sakila;server=localhost");
                break;
            case SPA.ClientSide.CSqlite.sidSqlite:
                res = Plugin.DoSPluginAuthentication("ssqlite", hSocket, userId, password, nSvsID, "usqlite.db");
                if (res == Plugin.AUTHENTICATION_PROCESSED) {
                    res = Plugin.AUTHENTICATION_OK; //give permission without authentication
                }
                break;
            default:
                break;
        }
        if (res >= Plugin.AUTHENTICATION_OK) {
            System.out.println(userId + "'s connecting permitted, and DB handle opened and cached");
        } else {
            switch (res) {
                case Plugin.AUTHENTICATION_PROCESSED:
                    System.out.println(userId + "'s connecting denied: no authentication implemented but DB handle opened and cached");
                    break;
                case Plugin.AUTHENTICATION_FAILED:
                    System.out.println(userId + "'s connecting denied: bad password");
                    break;
                case Plugin.AUTHENTICATION_INTERNAL_ERROR:
                    System.out.println(userId + "'s connecting denied: plugin internal error");
                    break;
                case Plugin.AUTHENTICATION_NOT_IMPLEMENTED:
                    System.out.println(userId + "'s connecting denied: no authentication implemented");
                    break;
                default:
                    System.out.println(userId + "'s connecting denied: unknown reseaon with res -- " + res);
                    break;
            }
        }
        return (res >= Plugin.AUTHENTICATION_OK);
    }

    @ServiceAttr(ServiceID = hello_world.hwConst.sidHelloWorld)
    private final CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<>(HelloWorldPeer.class);

    @ServiceAttr(ServiceID = SPA.BaseServiceID.sidHTTP)
    private final CSocketProService<CMyHttpPeer> m_http = new CSocketProService<>(CMyHttpPeer.class);

    //Routing requires registering two services in pair
    @ServiceAttr(ServiceID = piConst.sidPi)
    private final CSocketProService<CClientPeer> m_Pi = new CSocketProService<>(CClientPeer.class);
    @ServiceAttr(ServiceID = piConst.sidPiWorker)
    private final CSocketProService<CClientPeer> m_PiWorker = new CSocketProService<>(CClientPeer.class);

    @Override
    protected boolean OnSettingServer() {
        //amIntegrated and amMixed not supported yet
        Config.setAuthenticationMethod(tagAuthenticationMethod.amOwn);

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        PushManager.AddAChatGroup(7, "HR Department");
        PushManager.AddAChatGroup(SPA.UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID,
                "Subscribe/publish for front clients");

        //true -- ok; false -- no listening server
        return CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
    }

    public static void main(String[] args) {
        try (CMySocketProServer server = new CMySocketProServer()) {
            //load socketpro async sqlite library located at the directory ../bin/free_services/sqlite
            long handle = CSocketProServer.DllManager.AddALibrary("ssqlite");
            if (handle != 0) {
                //monitoring sakila.db table events (DELETE, INSERT and UPDATE)
                //for tables actor, language, category, country and film_actor
                String jsonOptions = "{\"monitored_tables\":\"sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor\"}";
                boolean ok = Plugin.SetSPluginGlobalOptions("ssqlite", jsonOptions);
            }
            //load async persistent message queue library at the directory ../bin/free_services/queue
            //24 * 1024 batch dequeuing size in bytes
            handle = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024);

            //load file streaming library at the directory ../bin/free_services/file
            handle = CSocketProServer.DllManager.AddALibrary("ustreamfile");

            //load MySQL/MariaDB server plugin library at the directory ../bin/free_services/mm_middle
            handle = CSocketProServer.DllManager.AddALibrary("smysql");

            //load ODBC socketPro server plugin library at the directory ../bin/win or ../bin/linux
            handle = CSocketProServer.DllManager.AddALibrary("sodbc");

            //test certificate and private key files are located at ../socketpro/bin
            //if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            //    server.UseSSL("intermediate.pfx", "", "mypassword");
            //    //or load cert and private key from windows system cert store
            //    //server.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "");
            //} else {
            //    server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            //}
            if (!server.Run(20901)) {
                System.out.println("Error code = " + CSocketProServer.getLastSocketError());
            }
            System.out.println("Input a line to close the application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
