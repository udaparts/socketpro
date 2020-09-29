using System;
using SocketProAdapter.ServerSide;

public class CMySocketProServer : CSocketProServer
{
    [ServiceAttr(hwConst.sidHelloWorld)]
    private CSocketProService<HelloWorldPeer> m_HelloWorld = new CSocketProService<HelloWorldPeer>();
    //One SocketPro server supports any number of services. You can list them here!

    protected override bool OnSettingServer()
    {
        //amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        PushManager.AddAChatGroup(1, "R&D Department");
        PushManager.AddAChatGroup(2, "Sales Department");
        PushManager.AddAChatGroup(3, "Management Department");
        
        return true; //true -- ok; false -- no listening server
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        return true; //true -- give permission; false -- connection denied
    }

    static void Main(string[] args)
    {
        CMySocketProServer MySocketProServer = new CMySocketProServer();

        //test certificate and private key files are located at ../SocketProRoot/bin
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            MySocketProServer.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
        else
        {
            MySocketProServer.UseSSL("intermediate.pfx", "", "mypassword");
            //or load cert and private key from windows system cert store
            //MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "");
        }

        if (!MySocketProServer.Run(20901))
            Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
        Console.WriteLine("Input a line to close the application ......");
        Console.ReadLine();
        MySocketProServer.StopSocketProServer(); //or MySocketProServer.Dispose();
    }
}
