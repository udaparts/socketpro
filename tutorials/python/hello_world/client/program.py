
from asynchelloworld import CHelloWorld
from spa.clientside import CSocketPool, CConnectionContext, CUQueue
from consts import hwConst
from msstruct import CMyStruct
import sys

with CSocketPool(CHelloWorld) as sp:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    ok = sp.StartSocketPool(cc, 1)
    hw = sp.Seek()
    ok = hw.AttachedClientSocket.ClientQueue.StartQueue('pqueue', 3600, False)

    #process requests one by one synchronously -- three round trips
    print(hw.sayHello(u'Jack', u'Smith'))
    hw.sleep(5000)
    ms = CMyStruct.MakeOne()
    res = hw.echo(ms)

    # asynchronously process all three requests with inline batching for best network efficiency
    def cbSayHello(ar):
        ret = ar.LoadString()
        print(ret)
    ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, CUQueue().SaveString('Jack').SaveString('Smith'), cbSayHello)
    ok = hw.SendRequest(hwConst.idSleepHelloWorld, CUQueue().SaveUInt(5000), None)
    def cbEcho(ar):
        hw.ms = ar.Load(CMyStruct())
    ok = hw.SendRequest(hwConst.idEchoHelloWorld, CUQueue().Save(ms), cbEcho)

    ok = hw.WaitAll()
    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()