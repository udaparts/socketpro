
from spa.serverside import CClientPeer, CUQueue
from msstruct import CMyStruct
import time

class CHelloWorldPeer(CClientPeer):
    def sayHello(self):
        fName = self.UQueue.LoadString()
        lName = self.UQueue.LoadString()
        res = u'Hello ' + fName + ' ' + lName
        print(res)
        return CUQueue().SaveString(res)

    def sleep(self):
        ms = self.UQueue.LoadUInt()
        time.sleep(ms/1000.0)

    def echo(self):
        ms = CMyStruct()
        ms.LoadFrom(self.UQueue)
        q = CUQueue()
        ms.SaveTo(q)
        return q
        #return self.UQueue #this also works
