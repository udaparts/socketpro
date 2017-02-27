
from consts import piConst
from spa.clientside import CAsyncServiceHandler, CUQueue

class PiWorker(CAsyncServiceHandler):
    def __init__(self):
        super(PiWorker, self).__init__(piConst.sidPiWorker)

    def OnResultReturned(self, reqId, q):
        if self.RouteeRequest and reqId == piConst.idComputePi:
            dStart = q.LoadDouble()
            dStep = q.LoadDouble()
            nNum = q.LoadUInt()
            dX = dStart + dStep / 2
            dd = dStep * 4.0
            ComputeRtn = 0.0
            for n in range(0, nNum):
                dX += dStep
                ComputeRtn += dd / (1 + dX * dX)
            q.SetSize(0)
            self.SendRouteeResult(CUQueue().SaveDouble(ComputeRtn))