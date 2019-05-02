/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
using SampleThreeShared;
using System.Collections;

//server implementation for service CTThree
public class CTThreePeer : CClientPeer
{
	protected override void OnSwitchFrom(int nServiceID)
	{
        m_TThreeSvs = (CTThreeSvs)GetBaseService();
        Console.WriteLine("Socket is switched for the service CTThreeSvs");
	}

	protected override void OnReleaseResource(bool bClosing, int nInfo)
	{
		if(bClosing)
		{
            Console.WriteLine("Socket is closed with error code = " + nInfo);
		}
		else
		{
            Console.WriteLine("Socket is going to be switched to new service with service id = " + nInfo);
		}
        m_Stack.Clear();
	}

	protected void GetOneItem(out CTestItem GetOneItemRtn)
	{
        if (m_TThreeSvs.m_Stack.Count > 0)
            GetOneItemRtn = (CTestItem)m_TThreeSvs.m_Stack.Pop();
        else
            GetOneItemRtn = null;
	}

	protected void SendOneItem(CTestItem Item)
	{
        m_TThreeSvs.m_Stack.Push(Item);
	}

	protected void GetManyItems()
	{
        int nRtn = 0;
        m_UQueue.SetSize(0);
        while (m_Stack.Count > 0)
        {
            //a client may either shut down the socket connection or call IUSocket::Cancel
            if (nRtn == SOCKET_NOT_FOUND || nRtn == REQUEST_CANCELED)
                break;
            CTestItem Item = (CTestItem)m_Stack.Pop();
            m_UQueue.Push(Item);
           
            //20 kbytes per batch at least
            //also shouldn't be too large. 
            //If the size is too large, it will cost more memory resource and reduce conccurency if online compressing is enabled.
            //for an opimal value, you'd better test it by yourself
            if (m_UQueue.GetSize() > 20480)
            {
                nRtn = SendResult(TThreeConst.idGetBatchItemsCTThree, m_UQueue);
                m_UQueue.SetSize(0);
            }
        }
        if (nRtn == SOCKET_NOT_FOUND || nRtn == REQUEST_CANCELED)
        {

        }
        else if (m_UQueue.GetSize() > sizeof(int))
        {
            nRtn = SendResult(TThreeConst.idGetBatchItemsCTThree, m_UQueue);
        }
	}

	protected void SendManyItems()
	{
	    while(m_Stack.Count > 0)
	    {
            m_TThreeSvs.m_Stack.Push(m_Stack.Pop());
	    }
	}

    protected void SendBatchItems()
    {
        while(m_UQueue.GetSize() > 0)
	    {
		    CTestItem Item = new CTestItem();
            m_UQueue.Pop(out Item);
            m_Stack.Push(Item);
	    }
    }

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case TThreeConst.idGetOneItemCTThree:
            M_I0_R1<CTestItem>(GetOneItem);
			break;
		case TThreeConst.idSendOneItemCTThree:
            M_I1_R0<CTestItem>(SendOneItem);
			break;
		case TThreeConst.idSendManyItemsCTThree:
            M_I0_R0(SendManyItems);
			break;
		default:
			break;
		}
	}

    private int RetrieveCount()
    {
        int nCount = 0;
        m_UQueue.Pop(out nCount);
        return nCount;
    }

    protected override void OnDispatchingSlowRequest(short sRequestID)
    {
        if(sRequestID == TThreeConst.idGetManyItemsCTThree) 
	    {
            int nCount = RetrieveCount();
            m_Stack.Clear();
            while (m_TThreeSvs.m_Stack.Count > 0 && nCount > 0)
            {
                m_Stack.Push(m_TThreeSvs.m_Stack.Pop());
                nCount--;
            }
	    }
    }

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
                
		case TThreeConst.idGetManyItemsCTThree:
            M_I0_R0(GetManyItems);
			break;
        case TThreeConst.idSendBatchItemsCTThree:
            M_I0_R0(SendBatchItems);
            break;
		default:
			break;
		}
		return 0;
	}
    private Stack m_Stack = new Stack();
    private CTThreeSvs m_TThreeSvs;
}

public class CTThreeSvs : CSocketProService<CTThreePeer>
{
    internal Stack m_Stack = new Stack();
}

