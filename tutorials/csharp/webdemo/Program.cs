using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    protected override bool OnSettingServer()
    {
        //amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        PushManager.AddAChatGroup(7, "HR Department");

        return true; //true -- ok; false -- no listening server
    }

    [ServiceAttr(hwConst.sidHelloWorld)]
    private CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<HelloWorldPeer>();
    [ServiceAttr(BaseServiceID.sidHTTP)]
    private CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();
    //One SocketPro server supports any number of services. You can list them here!

    static void Main(string[] args)
    {
        CMySocketProServer server = new CMySocketProServer();
        if (!server.Run(20901))
            Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
        Console.WriteLine("Input a line to close the application ......");
        Console.ReadLine();
        server.StopSocketProServer(); //or MySocketProServer.Dispose();
    }
}
