using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class PiWorker : CAsyncServiceHandler
{
	public PiWorker() : base(piConst.sidPiWorker)
	{
	}

    protected override void OnResultReturned(ushort sRequestId, CUQueue UQueue)
    {
        if (RouteeRequest)
        {
            switch (sRequestId)
            {
                case piConst.idComputePi:
                    {
                        double dStart;
                        double dStep;
                        int nNum;
                        UQueue.Load(out dStart).Load(out dStep).Load(out nNum);
                        double dX = dStart + dStep / 2;
                        double dd = dStep * 4.0;
                        double ComputeRtn = 0.0;
                        for (int n = 0; n < nNum; n++)
                        {
                            dX += dStep;
                            ComputeRtn += dd / (1 + dX * dX);
                        }
                        SendRouteeResult(ComputeRtn, dStart);
                    } 
                    break;
                default:
                    break;
            }
        }
    }
}
