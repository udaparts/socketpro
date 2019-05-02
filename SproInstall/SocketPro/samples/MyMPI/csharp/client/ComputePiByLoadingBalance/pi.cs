
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

public class CPPi : CAsyncServiceHandler
{
    public CPPi(CClientSocket cs, IAsyncResultsHandler pDefaultAsyncResultsHandler)
        :base(piConst.sidCPPi, cs, pDefaultAsyncResultsHandler)
    {
    }

    public CPPi(CClientSocket cs)
        : base(piConst.sidCPPi, cs)
    {
    }

    public CPPi()
        : base(piConst.sidCPPi)
    {
    }

	private double m_ComputeRtn;
	public void ComputeAsync(double dStart, double dStep, int nNum)
	{
        m_ComputeRtn = 0.0;
		SendRequest(piConst.idComputeCPPi, dStart, dStep, nNum);
	}

	//When a result comes from a remote SocketPro server, the below virtual function will be called.
	//We always process returning results inside the function.
	protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
	{
		switch(sRequestID)
		{
		    case piConst.idComputeCPPi:
			    UQueue.Pop(out m_ComputeRtn);
			    break;
		    default:
			    break;
		}
	}

    public double GetCompute()
    {
        return m_ComputeRtn;
    }
}
