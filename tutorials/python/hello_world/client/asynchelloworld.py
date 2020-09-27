from spa import CScopeUQueue as Sb
from spa.clientside import CAsyncServiceHandler as CAHandler
from consts import hwConst
from msstruct import CMyStruct


class CHelloWorld(CAHandler):
    def __init__(self):
        super(CHelloWorld, self).__init__(hwConst.sidHelloWorld)

    def say_hello(self, fName, lName):
        buffer = Sb()
        buffer.SaveString(fName).SaveString(lName)
        future = self.sendRequest(hwConst.idSayHello, buffer)
        sb = future.result()
        return sb.LoadString()

    def sleep(self, ms):
        sb = self.sendRequest(hwConst.idSleep, Sb().SaveInt(ms)).result()

    def echo(self, ms):
        return self.sendRequest(hwConst.idEcho, Sb().Save(ms)).result().Load(CMyStruct())
