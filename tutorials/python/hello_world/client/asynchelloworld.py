
from spa.clientside import CAsyncServiceHandler, CUQueue, CSocketPool, CConnectionContext
from consts import hwConst
from msstruct import CMyStruct

class CHelloWorld(CAsyncServiceHandler):
    def __init__(self):
        super(CHelloWorld, self).__init__(hwConst.sidHelloWorld)

    def sayHello(self, firstName, lastName):
        def arh(ar):
            self._res_sayHello = ar.LoadString()
        if self.SendRequest(hwConst.idSayHelloHelloWorld, CUQueue().SaveString(firstName).SaveString(lastName), arh):
            self.WaitAll()
        return self._res_sayHello

    def sleep(self, ms):
        if self.SendRequest(hwConst.idSleepHelloWorld, CUQueue().SaveUInt(ms), None):
            self.WaitAll()

    def echo(self, ms):
        def arh(ar):
            self._res_echo = ar.Load(CMyStruct())
        if self.SendRequest(hwConst.idEchoHelloWorld, CUQueue().Save(ms), arh):
            self.WaitAll()
        return self._res_echo
