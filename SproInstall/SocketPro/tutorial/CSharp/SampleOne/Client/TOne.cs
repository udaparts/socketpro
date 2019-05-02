
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;


public class CTOne : CAsyncServiceHandler
{
    public CTOne()
        : base(TOneConst.sidCTOne)
    {
    }

    public CTOne(CClientSocket cs)
        : base(TOneConst.sidCTOne, cs)
    {
    }
    
    public CTOne(CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler)
        : base(TOneConst.sidCTOne, cs, DefaultAsyncResultsHandler)
    {
    }

	public int QueryCount()
	{
		int QueryCountRtn;
		bool bProcessRy = ProcessR1(TOneConst.idQueryCountCTOne, out QueryCountRtn);
		return QueryCountRtn;
	}

	public int QueryGlobalCount()
	{
		int QueryGlobalCountRtn;
		bool bProcessRy = ProcessR1(TOneConst.idQueryGlobalCountCTOne, out QueryGlobalCountRtn);
		return QueryGlobalCountRtn;
	}

	public int QueryGlobalFastCount()
	{
		int QueryGlobalFastCountRtn;
		bool bProcessRy = ProcessR1(TOneConst.idQueryGlobalFastCountCTOne, out QueryGlobalFastCountRtn);
		return QueryGlobalFastCountRtn;
	}

	public void Sleep(int nTime)
	{
		bool bProcessRy = ProcessR0(TOneConst.idSleepCTOne, nTime);
	}

	public object Echo(object objInput)
	{
		object EchoRtn;
		bool bProcessRy = ProcessR1(TOneConst.idEchoCTOne, objInput, out EchoRtn);
		return EchoRtn;
	}

    public void GetAllCounts(out int nCount, out int nGlobalCount, out int nGlobalFastCount)
    {
        int n = 0, global = 0, fast = 0;
        BeginBatching();
        SendRequest(TOneConst.idQueryCountCTOne, delegate(CAsyncResult ar) {
            ar.UQueue.Pop(out n);
        });
        SendRequest(TOneConst.idQueryGlobalCountCTOne, delegate(CAsyncResult ar) {
            ar.UQueue.Pop(out global);
        });
        SendRequest(TOneConst.idQueryGlobalFastCountCTOne, delegate(CAsyncResult ar) {
            ar.UQueue.Pop(out fast);
        });
        CommitBatch(true); //true -- ask server to send three results back in one batch
        WaitAll();
        nCount = n;
        nGlobalCount = global;
        nGlobalFastCount = fast;
    }

    public void GetAllCounts(out int nCount, out int nGlobalCount)
    {
        int n = 0, global = 0;
        BeginBatching();
        SendRequest(TOneConst.idQueryCountCTOne, delegate(CAsyncResult ar)
        {
            ar.UQueue.Pop(out n);
        });
        SendRequest(TOneConst.idQueryGlobalCountCTOne, delegate(CAsyncResult ar)
        {
            ar.UQueue.Pop(out global);
        });
        CommitBatch(true); //true -- ask server to send two results back in one batch
        WaitAll();
        nCount = n;
        nGlobalCount = global;
    }

    public void GetAllCounts(out int nCount)
    {
        nCount = QueryCount();
    }
}
