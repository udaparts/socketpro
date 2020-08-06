from consts import piConst
from spa.clientside import *
import sys


class Pi(CAsyncServiceHandler):
    def __init__(self):
        super(Pi, self).__init__(piConst.sidPi)


print('Client: tell me load balance host address:')
cc = CConnectionContext(sys.stdin.readline().strip(), 20901, "lb_client", "pwd_lb_client")
with CSocketPool(Pi) as spPi:
    ok = spPi.StartSocketPool(cc, 1)
    cs = spPi.Sockets[0]
    ok = cs.ClientQueue.StartQueue("pi_queue", 24 * 3600, False)
    cs.ClientQueue.RoutingQueueIndex = True
    pi = spPi.AsyncHandlers[0]
    pi.WaitAll()  # make sure all existing queued requests are processed before executing next requests
    pi.dPi = 0.0
    nDivision = 1000
    nNum = 10000
    dStep = 1.0 / nNum / nDivision
    pi.nReturns = 0
    def cb(ar):
        pi.dPi += ar.LoadDouble()
        pi.nReturns += 1
    for n in range(0, nDivision):
        dStart = float(n) / nDivision
        ok = pi.SendRequest(piConst.idComputePi, CScopeUQueue().SaveDouble(dStart).SaveDouble(dStep).SaveInt(nNum), cb)
    ok = pi.WaitAll()
    print('Your pi = ' + str(pi.dPi) + ', returns = ' + str(pi.nReturns))
    print('Press key ENTER to shutdown the demo application ......')
    sys.stdin.readline()
