/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants

public class CMySocketProServer : CSocketProServer
{
	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
        Console.WriteLine("A socket connection is permitted");

		//give permission to all
		return true;
	}

	protected override void OnAccept(int hSocket, int nError)
	{
        Console.WriteLine("A socket is initially establised");
	}

	protected override void OnClose(int hSocket, int nError)
	{
        Console.WriteLine("A socket is closed with error code = " + nError);
	}

    protected override bool OnSettingServer()
    {
        //try amIntegrated and amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        //add service(s) into SocketPro server
        AddService();

        return true;
    }

    private CSocketProService<CHttpPeer> m_HttpSvs = new CSocketProService<CHttpPeer>();

	private void AddService()
	{
		bool ok;

        ok = m_HttpSvs.AddMe((int)USOCKETLib.tagServiceID.sidHTTP, 0, tagThreadApartment.taNone);
        ok = m_HttpSvs.AddSlowRequest((short)USOCKETLib.tagHttpRequestID.idGet);
        ok = m_HttpSvs.AddSlowRequest((short)USOCKETLib.tagHttpRequestID.idPost);
        ok = m_HttpSvs.AddSlowRequest((short)USOCKETLib.tagHttpRequestID.idMultiPart);
	}

    static void Main(string[] args)
    {
        CMySocketProServer MySocketProServer = new CMySocketProServer();
        if (!MySocketProServer.Run(20901))
        {
            Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
        }
        Console.WriteLine("Input a line to close the application ......");
        string str = Console.ReadLine();
        MySocketProServer.StopSocketProServer();
    }
}

