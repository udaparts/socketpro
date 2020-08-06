from loading_balance.lb_worker.piworker import PiWorker
from spa.clientside import CConnectionContext, CSocketPool
import sys, multiprocessing
print('Worker: tell me load balance host address:')
cc = CConnectionContext(sys.stdin.readline().strip(), 20901, "lb_worker", "pwdForlbworker")
with CSocketPool(PiWorker) as spPi:
    ok = spPi.StartSocketPool(cc, 1, multiprocessing.cpu_count())
    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()