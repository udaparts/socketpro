/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
//server implementation for service RemSum
public class RemSumPeer : CClientPeer
{
    private int m_nSum = 0;
    private int m_nStart = 0;
    private int m_nEnd = 0;

	private void DoSum(int start, int end, out /*out*/int DoSumRtn)
	{
        //initialize stateful members
        m_nSum = 0;
        m_nStart = start;
        m_nEnd = end;

        //do calculation
        DoSumRtn = Compute();
	}

    private int Compute()
    {
        int n;
        for (n = m_nStart; n <= m_nEnd; ++n)
        {
            m_nSum += n;
            System.Threading.Thread.Sleep(100); //simulate slow request
            int rtn = SendResult(RemoteSumConst.idReportProgress, n, m_nSum);
            if (rtn == CClientPeer.REQUEST_CANCELED || rtn == CClientPeer.SOCKET_NOT_FOUND)
                break;
        }
        m_nStart = (n + 1);
        return m_nSum;
    }

    private void RedoSum(out int RedoSumRtn)
    {
        RedoSumRtn = Compute();
    }

    private void Pause(out int PauseRtn)
	{
        PauseRtn = m_nSum;
	}

    protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case RemoteSumConst.idPauseRemSum:
            M_I0_R1<int>(Pause);
			break;
		default:
			break;
		}
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case RemoteSumConst.idDoSumRemSum:
            M_I2_R1<int, int, int>(DoSum);
			break;
		case RemoteSumConst.idRedoSumRemSum:
            M_I0_R1<int>(RedoSum);
			break;
		default:
			break;
		}
		return 0;
	}
}

public class CMySocketProServer : CSocketProServer
{
	protected override bool OnSettingServer()
	{
		//try amIntegrated and amMixed instead by yourself
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

		//add service(s) into SocketPro server
		AddService();

		//You may set others here

		return true; //true -- ok; false -- no listening server
	}

	private CSocketProService<RemSumPeer> m_RemSum = new CSocketProService<RemSumPeer>();
	//One SocketPro server supports any number of services. You can list them here!

	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_RemSum.AddMe(RemoteSumConst.sidRemSum, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_RemSum.AddSlowRequest(RemoteSumConst.idDoSumRemSum);
		ok = m_RemSum.AddSlowRequest(RemoteSumConst.idRedoSumRemSum);

		//Add all of other services into SocketPro server here!
	}
}

