/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ServerSide.PLG;
using USOCKETLib; //you may need it for accessing various constants

//server implementation for service CPPi
public class CPiPeer : CClientPeer, IPeerJobContext
{
	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
        //intercept data inside m_UQueue, and modify it here if neccessary
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
        //intercept data inside m_UQueue, and modify it here if neccessary
		return 0;
	}
    private IJobManager m_JobManager;
    #region IPeerJobContext Members
    IJobManager IPeerJobContext.JobManager
    {
        get
        {
            return m_JobManager;
        }
        set
        {
            m_JobManager = value;
        }
    }

    void IPeerJobContext.OnAddingTask(IJobContext JobContext, short sRequestId)
    {
    }

    void IPeerJobContext.OnEnqueuingJob(IJobContext JobContext, short sRequestId)
    {
    }

    void IPeerJobContext.OnJobEnqueued(IJobContext JobContext, short sRequestId)
    {
    }

    void IPeerJobContext.OnJobJustCreated(IJobContext JobContext, short sRequestId)
    {
    }

    void IPeerJobContext.OnPeerDataSent(IJobContext JobContext, short sRequestId)
    {
    }

    bool IPeerJobContext.OnSendingPeerData(IJobContext JobContext, short sRequestId, CUQueue UQueue)
    {
        //you can modify data inside UQueue here if neccessary
        return true; //true, will send result data in UQueue onto client peer; false, will not
    }

    void IPeerJobContext.OnTaskJustAdded(IJobContext JobContext, int nTaskId, short sRequestId)
    { 
    }

    void IPeerJobContext.OnWaitable(IJobContext JobContext, int nTaskId, short sRequestId)
    {
        //you can call JobContext.Wait() here to barrier for result or something else.
        //Must pay close attention to main thread or worker thread.
        //In general, don't call JobContext.Wait() within main thread but worker thread only.
    }
    #endregion
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
		//when a socket is initially established
	}

	protected override void OnClose(int hSocket, int nError)
	{
		//when a socket is closed
	}

    protected override bool OnSettingServer()
    {
        //try amIntegrated and amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

        //add service(s) into SocketPro server
        AddService();

        return true;
    }

    private CPLGService<CPiPeer> m_LoadingBalance = new CPLGService<CPiPeer>();

	private void AddService()
	{
		bool        ok;
        int         n;
        const int   Count = 5;

        const int sidCPPi = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 1212);
        SocketProAdapter.ServerSide.PLG.CPLGService<CPiPeer>.ServiceIdOnRealServer = sidCPPi;

        CConnectionContext[] pConnectionContext = new CConnectionContext[Count];
        for (n = 0; n < Count; n++)
            pConnectionContext[n] = new CConnectionContext();

        //set connection contexts
        pConnectionContext[0].m_strHost = "127.0.0.1";
        pConnectionContext[1].m_strHost = "localhost";
        pConnectionContext[2].m_strHost = "localhost";
        pConnectionContext[3].m_strHost = "127.0.0.1";
        pConnectionContext[4].m_strHost = "localhost";
        for (n = 0; n < Count; n++)
        {
            pConnectionContext[n].m_nPort = 20901;
            pConnectionContext[n].m_strPassword = "SocketPro";
            pConnectionContext[n].m_strUID = "PassOne";
            pConnectionContext[n].m_EncrytionMethod = tagEncryptionMethod.NoEncryption;
            pConnectionContext[n].m_bZip = false;
        }
        
        ok = m_LoadingBalance.SocketPool.StartSocketPool(pConnectionContext, 2, 3);
        ok = m_LoadingBalance.AddMe(sidCPPi, 0, tagThreadApartment.taFree);
	}
}

