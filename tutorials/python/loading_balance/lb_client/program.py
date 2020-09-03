from consts import piConst
from spa import CScopeUQueue as Sb
from spa.clientside import *
import sys


class Pi(CAsyncServiceHandler):
    def __init__(self):
        super(Pi, self).__init__(piConst.sidPi)


print('Client: tell me load balance host address:')
cc = CConnectionContext(sys.stdin.readline().strip(), 20901, "lb_client", "pwd_lb_client")
with CSocketPool(Pi) as spPi:
    spPi.QueueName = 'lbqname'
    if spPi.StartSocketPool(cc, 1):
        pi = spPi.SeekByQueue()
        dPi = 0.0
        nDivision = 1000
        nNum = 10000000
        dStep = 1.0 / nNum / nDivision
        vF = []
        for n in range(0, nDivision):
            dStart = float(n) / nDivision
            vF.append(pi.sendRequest(piConst.idComputePi, Sb().SaveDouble(dStart).SaveDouble(dStep).SaveInt(nNum)))
        for f in vF:
            sb = f.result()
            dPi += sb.LoadDouble()
            # print('dStart: ' + str(sb.LoadDouble()))
        print('pi: ' + str(dPi) + ', returns: ' + str(len(vF)))
    else:
        print('No connection to ' + cc.Host)
    print('Press key ENTER to kill the demo ......')
    sys.stdin.readline()
