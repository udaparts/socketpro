

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

public class CMySecure : CAsyncServiceHandler
{
    public CMySecure(CClientSocket cs, IAsyncResultsHandler pDefaultAsyncResultsHandler)
        : base(MyBlowFishConst.sidCMySecure, cs, pDefaultAsyncResultsHandler)
    {
    }

    public CMySecure(CClientSocket cs)
        : base(MyBlowFishConst.sidCMySecure, cs)
    {
    }

    public CMySecure()
        : base(MyBlowFishConst.sidCMySecure)
    {
    }

    public int m_nErrorCode = 0;
    public string m_strErrorMessage;

	protected string m_OpenRtn;
	protected void OpenAsyn(string strUserIDToDB, string strPasswordToDB)
	{
        SendRequest(MyBlowFishConst.idOpenCMySecure, strUserIDToDB, strPasswordToDB);
	}

	protected bool m_BeginTransRtn;
	public void BeginTransAsyn()
	{
		SendRequest(MyBlowFishConst.idBeginTransCMySecure);
	}

	protected bool m_ExecuteNoQueryRtn;
	public void ExecuteNoQueryAsyn(string strSQL)
	{
		SendRequest(MyBlowFishConst.idExecuteNoQueryCMySecure, strSQL);
	}

	protected bool m_CommitRtn;
	public void CommitAsyn(bool bSmart)
	{
		SendRequest(MyBlowFishConst.idCommitCMySecure, bSmart);
	}

	//When a result comes from a remote SocketPro server, the below virtual function will be called.
	//We always process returning results inside the function.
	protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
	{
        UQueue.Pop(out m_nErrorCode);
        UQueue.Load(out m_strErrorMessage);

		switch(sRequestID)
		{
		case MyBlowFishConst.idOpenCMySecure:
			UQueue.Load(out m_OpenRtn);
			break;
		case MyBlowFishConst.idBeginTransCMySecure:
			UQueue.Pop(out m_BeginTransRtn);
			break;
		case MyBlowFishConst.idExecuteNoQueryCMySecure:
			UQueue.Pop(out m_ExecuteNoQueryRtn);
			break;
		case MyBlowFishConst.idCommitCMySecure:
			UQueue.Pop(out m_CommitRtn);
			break;
		default:
			break;
		}
	}
	public string Open(string strUserIDToDB, string strPasswordToDB)
	{
		OpenAsyn(strUserIDToDB, strPasswordToDB);
		GetAttachedClientSocket().WaitAll();
		return m_OpenRtn;
	}

	public bool BeginTrans()
	{
		BeginTransAsyn();
		GetAttachedClientSocket().WaitAll();
		return m_BeginTransRtn;
	}

	public bool ExecuteNoQuery(string strSQL)
	{
		ExecuteNoQueryAsyn(strSQL);
		GetAttachedClientSocket().WaitAll();
		return m_ExecuteNoQueryRtn;
	}

	public bool Commit(bool bSmart)
	{
		CommitAsyn(bSmart);
		GetAttachedClientSocket().WaitAll();
		return m_CommitRtn;
	}
}
