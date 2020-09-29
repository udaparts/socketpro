from spa.serverside import CClientPeer
from spa import CScopeUQueue as Sb, CServerError as Se
from msstruct import CMyStruct
import time


class CHelloWorldPeer(CClientPeer):

    def sayHello(self):
        fName = self.UQueue.LoadString()
        if not fName or not len(fName):
            raise Se(12345, 'First name cannot be empty!')
        lName = self.UQueue.LoadString()
        res = u'Hello ' + fName + ' ' + lName
        print(res)
        return Sb().SaveString(res)

    def sleep(self):
        ms = self.UQueue.LoadInt()
        if ms < 0:
            raise Se(54321, 'Sleep time cannot be less than zero')
        time.sleep(ms/1000.0)

    def echo(self):
        # return self.UQueue #this also works
        ms = CMyStruct()
        ms.LoadFrom(self.UQueue)
        return ms.SaveTo(Sb())
