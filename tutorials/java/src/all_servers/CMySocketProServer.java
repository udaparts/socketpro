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

        return true; //true -- ok; false -- no listening server
    }

    public static void main(String[] args) {
        if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            CSocketProServer.QueueManager.setWorkDirectory("c:\\sp_test");
        } else {
            CSocketProServer.QueueManager.setWorkDirectory("/home/yye/sp_test/");
        }
        CMySocketProServer MySocketProServer = new CMySocketProServer();

        //CSocketProServer.QueueManager.setMessageQueuePassword("MyPasswordForMsgQueue");
        //load socketpro async sqlite and queue server libraries located at the directory ../socketpro/bin
        long handle = CSocketProServer.DllManager.AddALibrary("ssqlite");
        if (handle != 0) {
            Sqlite.SetSqliteDBGlobalConnectionString("usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor");
        }
        handle = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 16 * 1024); //16 * 1024 batch dequeuing size in bytes

        handle = CSocketProServer.DllManager.AddALibrary("ustreamfile");
        if (handle != 0) {
            if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
                Sfile.SetRootDirectory("C:\\boost_1_60_0\\stage\\lib64");
            }
        }

        //test certificate, private key and DH params files are located at ../SocketProRoot/bin
        //MySocketProServer.UseSSL("server.pem", "server.pem", "test", "dh512.pem");
        if (!MySocketProServer.Run(20901)) {
            System.out.println("Error code = " + CSocketProServer.getLastSocketError());
        } else {
            CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker);
        }
        System.out.println("Input a line to close the application ......");
        new java.util.Scanner(System.in).nextLine();
    }
}
