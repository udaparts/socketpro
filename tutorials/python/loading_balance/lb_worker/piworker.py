from consts import piConst
from spa.clientside import CAsyncServiceHandler as CAHandler
from spa import CScopeUQueue as Csb

class PiWorker(CAHandler):
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
            with Csb() as sb:
                self.SendRouteeResult(sb.SaveDouble(ComputeRtn))
