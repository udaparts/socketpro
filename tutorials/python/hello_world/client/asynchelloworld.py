from spa import CScopeUQueue as Sb
from spa.clientside import CAsyncServiceHandler as CAHandler
from consts import hwConst
from msstruct import CMyStruct


class CHelloWorld(CAHandler):
    def __init__(self):
        super(CHelloWorld, self).__init__(hwConst.sidHelloWorld)

    def say_hello(self, f_name, l_name):
        future = self.sendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString(f_name).SaveString(l_name))
        sb = future.result()
        return sb.LoadString()

    def sleep(self, ms):
        sb = self.sendRequest(hwConst.idSleepHelloWorld, Sb().SaveUInt(ms)).result()

    def echo(self, ms):
        return self.sendRequest(hwConst.idEchoHelloWorld, Sb().Save(ms)).result().Load(CMyStruct())
