/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
using System.Diagnostics;
using System.Runtime.InteropServices;

//server implementation for service CTOne
public class CTOnePeer : CClientPeer
{
    [DllImport("Kernel32.dll")]
    public static extern int GetCurrentThreadId();

	protected override void OnSwitchFrom(int nServiceID)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == GetCurrentThreadId()); 

        m_uCount = 0; //initialize the object here
        Console.WriteLine("Socket is switched for the service CTOneSvs");
	}

	protected override void OnReleaseResource(bool bClosing, int nInfo)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == GetCurrentThreadId()); 

		if(bClosing)
		{
            Console.WriteLine("Socket is closed with error code = " + nInfo);
		}
		else
		{
            Console.WriteLine("Socket is going to be switched to new service with service id = " + nInfo);
		}

		//release all of your resources here as early as possible
	}

    protected override void OnDispatchingSlowRequest(short sRequestID)
    {
        m_uGlobalCount++;
    }

	protected void QueryCount(out int QueryCountRtn)
	{
        QueryCountRtn = (int)m_uCount;
	}

	protected void QueryGlobalCount(out int QueryGlobalCountRtn)
	{
        QueryGlobalCountRtn = (int)m_uGlobalCount;
	}

	protected void QueryGlobalFastCount(out int QueryGlobalFastCountRtn)
	{
        QueryGlobalFastCountRtn = (int)m_uGlobalFastCount;
	}

	protected void Sleep(int nTime)
	{
        int []groups = {1,2};
        //inform all of joined clients that idSleep is called
        Push.Broadcast("Sleep called", groups);

        if (TransferServerException && nTime < 200)
            throw new CSocketProServerException(12345, "Sleeping time is too short!");
        System.Threading.Thread.Sleep(nTime);
	}

	protected void Echo(object objInput, out object EchoRtn)
	{
        EchoRtn = objInput;
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == GetCurrentThreadId()); 

        m_uCount++;
        m_uGlobalFastCount++;
        m_uGlobalCount++;

        switch (sRequestID)
        {
            case TOneConst.idQueryCountCTOne:
                //Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
                M_I0_R1<int>(QueryCount);
                break;
            case TOneConst.idQueryGlobalCountCTOne:
                M_I0_R1<int>(QueryGlobalCount);
                break;
            case TOneConst.idQueryGlobalFastCountCTOne:
                M_I0_R1<int>(QueryGlobalFastCount);
                break;
            case TOneConst.idEchoCTOne:
                M_I1_R1<object, object>(Echo);
                break;
            default:
                break;
        }
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
        //always processed within a worker thread
        Trace.Assert(CSocketProServer.MainThreadID != GetCurrentThreadId()); 
        m_uCount++;

        switch (sRequestID)
        {
            case TOneConst.idSleepCTOne:
                //Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
                M_I1_R0<int>(Sleep);
                break;
            default:
                break;
        }

		return 0;
	}

    //m_GlobalCount doesn't need to be synchronized 
	//because it is accessed only within main thread
	//although is is accessed from different socket clients
    private static uint m_uGlobalCount = 0;

    //m_uGlobalFastCount doesn't need to be synchronized 
    //because it is accessed within main thread, 
    //although is is accessed from different socket clients
    private static uint m_uGlobalFastCount = 0;

    //m_uCount doesn't need to be synchronized 
    //because it is always accessed from one socket client only, 
    //even though it may be accessed within different threads.
    private uint m_uCount;
}

public class CMySocketProServer : CSocketProServer
{
    private void ReuseLibraries()
    {
        //those libraries are distributed in the directory ..\bin

        IntPtr hInst = CBaseService.AddALibrary("uodbsvr.dll", 0);
        if (hInst == (IntPtr)0)
        {
            Console.WriteLine("library uodbsvr.dll not available.");
        }

        hInst = CBaseService.AddALibrary("ufilesvr.dll", 0);
        if (hInst == (IntPtr)0)
        {
            Console.WriteLine("library ufilesvr.dll not available.");
        }

        hInst = CBaseService.AddALibrary("udemo.dll", 0);
        if (hInst == (IntPtr)0)
        {
            Console.WriteLine("library udemo.dll not available.");
        }
    }

    private void SetBuiltinChatService()
    {
        bool ok = PushManager.AddAChatGroup(1, "Group for SOne");

        Trace.Assert(ok);
    }

    private bool IsAllowed(string strUserID, string strPassword)
    {
        if (strPassword != "PassOne")
            return false;
        return (strUserID.ToLower() == "socketpro");
    }

	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == CTOnePeer.GetCurrentThreadId());

        string strUID = GetUserID(hSocket);

        //password is available ONLY IF authentication method to either amOwn or amMixed
        string strPassword = GetPassword(hSocket);

        Console.WriteLine("For service = {0}, User ID = {1}, Password = {2}", nSvsID, strUID, strPassword);

        tagAuthenticationMethod am = Config.AuthenticationMethod;

        if (am == tagAuthenticationMethod.amOwn || am == tagAuthenticationMethod.amMixed)
        {
            //do my own authentication
            return IsAllowed(strUID, strPassword);
        }

        return true; 
	}

	protected override void OnAccept(int hSocket, int nError)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == CTOnePeer.GetCurrentThreadId()); 

        Console.WriteLine("A socket is initially establised");
	}

	protected override void OnClose(int hSocket, int nError)
	{
        //always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID == CTOnePeer.GetCurrentThreadId()); 

        Console.WriteLine("A socket is closed with error code = " + nError);
	}

    protected override bool OnSettingServer()
    {
        //amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amMixed;

        Config.SharedAM = true;

        //limit the max number of connections to 2 for a client machine
        Config.MaxConnectionsPerClient = 2;

        //add service(s) into SocketPro server
        AddService();

        SetBuiltinChatService();

        //use TLSv1 to secure all of data communication between a client and a SocketPro server
        //udacert.pem contains both key and certificate, which is distributed in the ..\bin as sample certificate and key
        //UseSSL("C:\\Program Files\\UDAParts\\SocketPro\\bin\\udacert.pfx", "mypassword", "udaparts", tagEncryptionMethod.MSTLSv1);

        //reuse my high performance C/C++ libraries
        ReuseLibraries();

        return true; //true -- ok; false -- no listening server
    }

    private CSocketProService<CTOnePeer> m_CTOne = new CSocketProService<CTOnePeer>();
    private CTThreeSvs m_CTThree = new CTThreeSvs();

	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CTOne.AddMe(TOneConst.sidCTOne, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CTOne.AddSlowRequest(TOneConst.idSleepCTOne);

        //No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CTThree.AddMe(TThreeConst.sidCTThree, 0, tagThreadApartment.taNone);
        //If ok is false, very possibly you have two services with the same service id!

        ok = m_CTThree.AddSlowRequest(TThreeConst.idGetManyItemsCTThree);
        ok = m_CTThree.AddSlowRequest(TThreeConst.idSendBatchItemsCTThree);
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

