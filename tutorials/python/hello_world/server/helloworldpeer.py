
from spa.serverside import CClientPeer, ServicetAttr, RequestAttr
from spa import CUQueue
from msstruct import CMyStruct
import time
from consts import hwConst

@ServicetAttr(hwConst.sidHelloWorld)
class CHelloWorldPeer(CClientPeer):
    @RequestAttr(hwConst.idSayHelloHelloWorld)
    def sayHello(self):
        fName = self.UQueue.LoadString()
        lName = self.UQueue.LoadString()
        res = u'Hello ' + fName + ' ' + lName
        print(res)
        return CUQueue().SaveString(res)

    @RequestAttr(hwConst.idSleepHelloWorld, True)
    def sleep(self):
        ms = self.UQueue.LoadUInt()
        time.sleep(ms/1000.0)

    @RequestAttr(hwConst.idEchoHelloWorld)
    def echo(self):
        ms = CMyStruct()
        ms.LoadFrom(self.UQueue)
        q = CUQueue()
        ms.SaveTo(q)
        return q
        #return self.UQueue #this also works
