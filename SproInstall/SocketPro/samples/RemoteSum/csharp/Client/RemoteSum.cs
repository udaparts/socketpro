

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

public class RemSum : CAsyncServiceHandler
{
	public RemSum() : base(RemoteSumConst.sidRemSum)
	{
	}

	public RemSum(CClientSocket cs) : base(RemoteSumConst.sidRemSum, cs)
	{
	}

	public RemSum(CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler) : base(RemoteSumConst.sidRemSum, cs, DefaultAsyncResultsHandler)
	{
	}

	protected int m_DoSumRtn;
	protected void DoSumAsyn(int start, int end)
	{
		//make sure that the handler is attached to a client socket before calling the below statement
		SendRequest(RemoteSumConst.idDoSumRemSum, start, end);
	}

	protected int m_PauseRtn;
	protected void PauseAsyn()
	{
		//make sure that the handler is attached to a client socket before calling the below statement
		SendRequest(RemoteSumConst.idPauseRemSum);
	}

	protected int m_RedoSumRtn;
	protected void RedoSumAsyn()
	{
		//make sure that the handler is attached to a client socket before calling the below statement
		SendRequest(RemoteSumConst.idRedoSumRemSum);
	}

	//We can process returning results inside the function.
	protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
	{
		switch(sRequestID)
		{
		case RemoteSumConst.idDoSumRemSum:
			UQueue.Pop(ref m_DoSumRtn);
			break;
		case RemoteSumConst.idPauseRemSum:
			UQueue.Pop(ref m_PauseRtn);
			break;
		case RemoteSumConst.idRedoSumRemSum:
			UQueue.Pop(ref m_RedoSumRtn);
			break;
		default:
			break;
		}
	}
	public int DoSum(int start, int end)
	{
		DoSumAsyn(start, end);
		GetAttachedClientSocket().WaitAll();
		return m_DoSumRtn;
	}

	public int Pause()
	{
		PauseAsyn();
		GetAttachedClientSocket().WaitAll();
		return m_PauseRtn;
	}

	public int RedoSum()
	{
		RedoSumAsyn();
		GetAttachedClientSocket().WaitAll();
		return m_RedoSumRtn;
	}
}
