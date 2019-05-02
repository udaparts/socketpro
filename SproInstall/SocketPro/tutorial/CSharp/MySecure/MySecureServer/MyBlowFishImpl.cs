/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
//server implementation for service CMySecure
public class CMySecurePeer : CClientPeer
{
	protected override void OnSwitchFrom(int nServiceID)
	{
        //All return results are not required to be secure
        EncryptionMethod = tagEncryptionMethod.NoEncryption;
	}

    private int m_nErrorCode = 0;
    private string m_strErrorMessage;

    private void PushError()
    {
        m_UQueue.Push(m_nErrorCode);
        m_UQueue.Save(m_strErrorMessage);
    }

	protected override void OnReleaseResource(bool bClosing, int nInfo)
	{
		if(bClosing)
		{
			//closing the socket with error code = nInfo
		}
		else
		{
			//switch to a new service with the service id = nInfo
		}

		//release all of your resources here as early as possible
	}

	protected void Open(string strUserIDToDB, string strPasswordToDB, out string OpenRtn)
	{
        OpenRtn = "Oracle Database";
        m_strErrorMessage = "Ok!";
        m_nErrorCode = 0;
	}

	protected void BeginTrans(out bool BeginTransRtn)
	{
		BeginTransRtn = true;
        m_strErrorMessage = "BeginTrans OK!";
        m_nErrorCode = 0;
	}

	protected void ExecuteNoQuery(string strSQL, out bool ExecuteNoQueryRtn)
	{
		ExecuteNoQueryRtn = true;

		m_strErrorMessage = "ExecuteNoQuery OK!";
        m_nErrorCode = 0;
	}

	protected void Commit(bool bSmart, out bool CommitRtn)
	{
		CommitRtn = true;

        m_strErrorMessage = "Commit OK!";
        m_nErrorCode = 0;
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case MyBlowFishConst.idBeginTransCMySecure:
		{
			bool BeginTransRtn;
			BeginTrans(out BeginTransRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
            PushError();
			m_UQueue.Push(BeginTransRtn);
            SendResult(sRequestID, m_UQueue);
		}
			break;
		case MyBlowFishConst.idCommitCMySecure:
		{
			bool bSmart = false;
			bool CommitRtn;
			m_UQueue.Pop(out bSmart);
			Commit(bSmart, out CommitRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
            PushError();
			m_UQueue.Push(CommitRtn);
            SendResult(sRequestID, m_UQueue);
		}
			break;
		default:
			break;
		}
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case MyBlowFishConst.idOpenCMySecure:
		{
			string strUserIDToDB = null;
			string strPasswordToDB = null;
			string OpenRtn;
			m_UQueue.Load(out strUserIDToDB);
			m_UQueue.Load(out strPasswordToDB);
			Open(strUserIDToDB, strPasswordToDB, out OpenRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
            PushError();
			m_UQueue.Save(OpenRtn);
			SendResult(sRequestID, m_UQueue);
		}
			break;
		case MyBlowFishConst.idExecuteNoQueryCMySecure:
		{
			string strSQL = null;
			bool ExecuteNoQueryRtn;
			m_UQueue.Load(out strSQL);
			ExecuteNoQuery(strSQL, out ExecuteNoQueryRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
            PushError();
			m_UQueue.Push(ExecuteNoQueryRtn);
            SendResult(sRequestID, m_UQueue);
		}
			break;
		default:
			break;
		}
		return 0;
	}
}

public class CMySocketProServer : CSocketProServer
{
    private string GetPasswordFromStore(string strUserID)
    {
        if (string.Compare(strUserID, "SocketPro", true) == 0)
            return "PassOne";
        else if (string.Compare(strUserID, "RDBClient", true) == 0)
            return "PassTwo";
        return null;
    }

	protected override bool OnIsPermitted(int hSocket, int nSvsID)
	{
        string strUserID = GetUserID(hSocket);
        string strPassword = GetPasswordFromStore(strUserID);

        //set password so that the hash of the password with salt can be sent to a client for authentication on client side.
        SetPassword(hSocket, strPassword);
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

        Config.DefaultEncryptionMethod = tagEncryptionMethod.BlowFish;

        AddService();

        return true;
    }

    private CSocketProService<CMySecurePeer> m_CMySecure = new CSocketProService<CMySecurePeer>();

	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CMySecure.AddMe(MyBlowFishConst.sidCMySecure, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CMySecure.AddSlowRequest(MyBlowFishConst.idOpenCMySecure);
		ok = m_CMySecure.AddSlowRequest(MyBlowFishConst.idExecuteNoQueryCMySecure);
	}
}

