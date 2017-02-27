package loading_balance.worker;

import loading_balance.piConst;

public class PiWorker extends SPA.ClientSide.CAsyncServiceHandler {

    public PiWorker() {
        super(loading_balance.piConst.sidPiWorker);
    }

    @Override
    protected void OnResultReturned(short sRequestId, SPA.CUQueue UQueue) {
        if (getRouteeRequest()) {
            switch (sRequestId) {
                case piConst.idComputePi: {
                    double dStart = UQueue.LoadDouble();
                    double dStep = UQueue.LoadDouble();
                    int nNum = UQueue.LoadInt();
                    double dX = dStart + dStep / 2;
                    double dd = dStep * 4.0;
                    double ComputeRtn = 0.0;
                    for (int n = 0; n < nNum; n++) {
                        dX += dStep;
                        ComputeRtn += dd / (1 + dX * dX);
                    }
                    SendRouteeResult(new SPA.CScopeUQueue().Save(ComputeRtn));
                }
                break;
                default:
                    break;
            }
        }
    }
}
