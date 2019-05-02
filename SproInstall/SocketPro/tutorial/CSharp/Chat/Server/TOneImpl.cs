/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants


class CMyDummyPeer : CClientPeer
{
    protected override void OnChatRequestComing(USOCKETLib.tagChatRequestID ChatRequestId, object Param0, object Param1)
    {
        string str = "";
        int[] Groups;
        switch (ChatRequestId)
        {
            case tagChatRequestID.idEnter:
            case tagChatRequestID.idXEnter:
                Groups = (int[])Param0;
                foreach (int n in Groups)
                {
                    if (str.Length > 0) str += ", ";
                    str += n.ToString();
                }
                Console.WriteLine("User {0} joins chat groups {1}", UserID, str);
                break;
            case tagChatRequestID.idSpeak:
            case tagChatRequestID.idXSpeak:
                Groups = (int[])Param1;
                foreach(int n in Groups)
                {
                    if (str.Length > 0) str += ", ";
                    str += n.ToString();
                }
                if (Param0 == null)
                    Param0 = "null";
                Console.WriteLine("User {0} sends a message '{1}' to chat groups {2}", UserID, Param0.ToString(), str);
                break;
            case tagChatRequestID.idExit:
                Console.WriteLine("User {0} exits his or her chat groups", UserID);
                break;
            case tagChatRequestID.idSendUserMessage:
                if (Param0 == null)
                    Param0 = "null";
                if (Param1 == null)
                    Param1 = "null";
                Console.WriteLine("User {0} sends a message '{1}' to {2}", UserID, Param1.ToString(), Param0.ToString());
                break;
            default:
                break;
        }
    }

    protected override void OnFastRequestArrive(short sRequestID, int nLen)
    {
        //should never come here
    }

    protected override int OnSlowRequestArrive(short sRequestID, int nLen)
    {
        //should never come here
        return 0;
    }
}

public class CMySocketProServer : CSocketProServer
{
	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
		//give permission to all
		return true;
	}

	protected override void OnAccept(int hSocket, int nError)
	{
        Console.WriteLine("Socket {0} accepted with error = {1}", hSocket, nError);
	}

	protected override void OnClose(int hSocket, int nError)
	{
        Console.WriteLine("Socket {0} closed with error = {1}", hSocket, nError);
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
    private CSocketProService<CMyDummyPeer> m_ChatSvs = new CSocketProService<CMyDummyPeer>();

	private void AddService()
	{
		bool ok;

        ok = PushManager.AddAChatGroup(1, "Group for SOne");
        ok = PushManager.AddAChatGroup(2, "DB Service");
        ok = PushManager.AddAChatGroup(4, "Management Department");
        ok = PushManager.AddAChatGroup(9, "IT Department");
        ok = PushManager.AddAChatGroup(16, "Sales Department");

        //one default page
        CHttpPushPeer.Default = "httppush.htm";
        
        ok = m_HttpSvs.AddMe((int)USOCKETLib.tagServiceID.sidHTTP, 0, tagThreadApartment.taNone);
        ok = m_HttpSvs.AddSlowRequest((short)USOCKETLib.tagHttpRequestID.idGet);
        ok = m_HttpSvs.AddSlowRequest((short)USOCKETLib.tagHttpRequestID.idPost);

        ok = m_ChatSvs.AddMe((int)USOCKETLib.tagServiceID.sidChat, 0, tagThreadApartment.taNone);
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

