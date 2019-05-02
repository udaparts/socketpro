

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using SampleThreeShared;
using System.Collections;

public class CTThree : CAsyncServiceHandler
{
    public CTThree(CClientSocket cs, IAsyncResultsHandler pDefaultAsyncResultsHandler)
        : base(TThreeConst.sidCTThree, cs, pDefaultAsyncResultsHandler)
    {
    }

    public CTThree(CClientSocket cs)
        : base(TThreeConst.sidCTThree, cs)
    {
    }

    public CTThree()
        : base(TThreeConst.sidCTThree)
    {
    }

	public CTestItem GetOneItem()
	{
        CTestItem ti;
        bool bProcessRy = ProcessR1(TThreeConst.idGetOneItemCTThree, out ti);
        return ti;
	}

	public void SendOneItem(CTestItem Item)
	{
        bool bProcessRy = ProcessR0(TThreeConst.idSendOneItemCTThree, Item);
	}

    public Stack GetManyItems(int nCount)
	{
        m_Stack.Clear();
        bool bProcessR0 = ProcessR0(TThreeConst.idGetManyItemsCTThree, nCount);
        return m_Stack;
	}

    public void SendManyItems(Stack outStack)
    {
        m_OutStack = outStack;
        SendBatchItems();
        WaitAll();
        //at last, we inform a server that we finish sending items
        bool bProcessRy = ProcessR0(TThreeConst.idSendManyItemsCTThree);
    }

    private void SendBatchItems()
    {
        int nSndSize;
        const int BATCH_SIZE_IN_BYTE = 40 * 1024;
        const int BATCH_ITEM_COUNT = 200;
        int nBatch = 0;
        using (CScopeUQueue su = new CScopeUQueue())
        {
            CUQueue UQueue = su.UQueue;
            UQueue.SetSize(0);
            nSndSize = GetAttachedClientSocket().GetUSocket().BytesInSndMemory;
            while (nBatch < BATCH_ITEM_COUNT && nSndSize < BATCH_SIZE_IN_BYTE && m_OutStack != null && m_OutStack.Count > 0)
            {
                CTestItem Item = (CTestItem)m_OutStack.Pop();
                UQueue.Push(Item);
                nBatch++;
                if (nBatch == BATCH_ITEM_COUNT)
                {
                    SendRequest(TThreeConst.idSendBatchItemsCTThree, UQueue);
                    UQueue.SetSize(0);
                    nSndSize = GetAttachedClientSocket().GetUSocket().BytesInSndMemory;
                    if (nSndSize < BATCH_SIZE_IN_BYTE)
                        nBatch = 0;
                    else
                        break;
                }
            }
            if (UQueue.GetSize() > 0)
            {
                SendRequest(TThreeConst.idSendBatchItemsCTThree, UQueue);
                UQueue.SetSize(0);
            }
        }
    }

    protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
    {
        switch (sRequestID)
        {
            case TThreeConst.idGetBatchItemsCTThree:
                while (UQueue.GetSize() > 0)
                {
                    CTestItem Item;
                    UQueue.Pop(out Item);
                    m_Stack.Push(Item);
                }
                break;
            case TThreeConst.idSendBatchItemsCTThree:
                SendBatchItems();
                break;
            default:
                break;
        }
    }

    private Stack m_OutStack;
    private Stack m_Stack = new Stack();
}
