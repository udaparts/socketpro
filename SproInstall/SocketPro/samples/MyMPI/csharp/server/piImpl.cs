/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
//server implementation for service CPPi
public class CPPiPeer : CClientPeer
{
	protected void Compute(double dStart, double dStep, int nNum, out double ComputeRtn)
	{
		int n;
		int n100 = nNum/100;
		double dX = dStart;
		dX += dStep/2;
		double dd = dStep * 4.0;
		ComputeRtn = 0.0;
		for(n=0; n<nNum; n++)
		{
			dX += dStep;
			ComputeRtn += dd/(1 + dX*dX);
			if(n100 > 0 && ((n+1)%n100) == 0 && n>0)
			{
                if (IsCanceled || IsClosing())
                    break;
			}
		}
	}

	protected override void OnFastRequestArrive(short sRequestID, int nLen)
	{
	
	}

	protected override int OnSlowRequestArrive(short sRequestID, int nLen)
	{
		switch(sRequestID)
		{
		case piConst.idComputeCPPi:
		{
			double dStart;
			double dStep;
			int nNum;
			double ComputeRtn;
			m_UQueue.Pop(out dStart);
			m_UQueue.Pop(out dStep);
			m_UQueue.Pop(out nNum);
			Compute(dStart, dStep, nNum, out ComputeRtn);
            SendResult(sRequestID, ComputeRtn);
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

    private CSocketProService<CPPiPeer> m_CPPi = new CSocketProService<CPPiPeer>();
	private void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CPPi.AddMe(piConst.sidCPPi, 0, tagThreadApartment.taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CPPi.AddSlowRequest(piConst.idComputeCPPi);
	}
}

