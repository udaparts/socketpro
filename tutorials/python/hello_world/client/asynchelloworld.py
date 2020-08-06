from spa import CScopeUQueue as Sb
from spa.clientside import UFuture as Future
from spa.clientside import CAsyncServiceHandler as CAHandler
from consts import hwConst
from msstruct import CMyStruct


class CHelloWorld(CAHandler):
    def __init__(self):
        super(CHelloWorld, self).__init__(hwConst.sidHelloWorld)

    def say_hello(self, f_name, l_name):
        f = Future()

        def cb_aborted(ah, canceled):
            if canceled:
                f.cancel()
            else:
                cs = self.Socket
                if cs.ErrCode:
                    f.set_exception(OSError(cs.ErrCode, cs.ErrMsg))
                else:
                    f.set_exception(OSError(2, 'Session closed after the request say_hello is sent'))
        if self.SendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString(f_name).SaveString(l_name),
                            lambda ar: f.set(ar.LoadString()),
                            cb_aborted,
                            lambda ah, se: f.set_exception(se)):
            return f.get()
        raise OSError(1, 'Session already closed before sending the request say_hello')

    def sleep(self, ms):
        f = Future()

        def cb_aborted(ah, canceled):
            if canceled:
                f.cancel()
            else:
                cs = self.Socket
                if cs.ErrCode:
                    f.set_exception(OSError(cs.ErrCode, cs.ErrMsg))
                else:
                    f.set_exception(OSError(2, 'Session closed after the request sleep is sent'))
        if self.SendRequest(hwConst.idSleepHelloWorld, Sb().SaveUInt(ms),
                            lambda ar: f.set(True),
                            cb_aborted,
                            lambda ah, se: f.set_exception(se)):
            return f.get()
        raise OSError(1, 'Session already closed before sending the request sleep')

    def echo(self, ms):
        f = Future()

        def cb_aborted(ah, canceled):
            if canceled:
                f.cancel()
            else:
                cs = self.Socket
                if cs.ErrCode:
                    f.set_exception(OSError(cs.ErrCode, cs.ErrMsg))
                else:
                    f.set_exception(OSError(2, 'Session closed after the request echo is sent'))
        if self.SendRequest(hwConst.idEchoHelloWorld, Sb().Save(ms),
                            lambda ar: f.set(ar.Load(CMyStruct())),
                            cb_aborted,
                            lambda ah, se: f.set_exception(se)):
            return f.get()
        raise OSError(1, 'Session already closed before sending the request echo')
