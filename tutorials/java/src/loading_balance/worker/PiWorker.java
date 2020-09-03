package loading_balance.worker;

import loading_balance.piConst;
import SPA.*;
import SPA.ClientSide.CAsyncServiceHandler;

public class PiWorker extends CAsyncServiceHandler {

    public PiWorker() {
        super(loading_balance.piConst.sidPiWorker);
    }

    @Override
    protected void OnResultReturned(short sRequestId, CUQueue UQueue) {
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
                    try (CScopeUQueue sb = new CScopeUQueue()) {
                        SendRouteeResult(sb.Save(ComputeRtn).Save(dStart));
                    }
                }
                break;
                default:
                    break;
            }
        }
    }
}
