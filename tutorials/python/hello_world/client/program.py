from hello_world.client.asynchelloworld import CHelloWorld
from spa import CScopeUQueue as Sb
from spa.clientside import UFuture as Future
from spa.clientside import CSocketPool, CConnectionContext
from consts import hwConst
from msstruct import CMyStruct
import sys

with CSocketPool(CHelloWorld) as sp:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    sp.QueueName = 'pqueue'  # turn on client message queue for backing up requests
    ok = sp.StartSocketPool(cc, 1)
    if not ok:
        print('Cannot connect to server with error message: ' + sp.Sockets[0].ErrMsg)
    else:
        hw = sp.Seek()

        # process requests one by one synchronously -- three round trips
        try:
            ms = CMyStruct.MakeOne()
            print(hw.say_hello(u'Jack', u'Smith'))
            hw.sleep(5000)
            res = hw.echo(ms)
        except OSError as ex:
            print(ex)
        except Exception as ex:
            print(ex)

        def cbAborted(ah, canceled):
            if canceled:
                print('Request canceled')
            elif not hw.Socket.ErrCode:
                print((2, 'Session closed after request is sent'))
            else:
                print((hw.Socket.ErrCode, hw.Socket.ErrMsg))

        def serverEx(ah, se):
            print(se)

        print('Going to send five requests with inline batching for better network efficiency and less round trips')
        with Sb() as sb:
            def cbSayHello(ar):
                print(ar.LoadString())
            try:
                if not hw.SendRequest(hwConst.idSayHelloHelloWorld, sb.SaveString('Hilary').SaveString('Clinton'),
                                      cbSayHello,
                                      cbAborted, serverEx):
                    raise OSError(1, 'Session already closed before sending the request say_hello')

                sb.Size = 0  # empty buffer
                if not hw.SendRequest(hwConst.idSleepHelloWorld, sb.SaveUInt(5000),
                                      None,
                                      cbAborted, serverEx):
                    raise OSError(1, 'Session already closed before sending the request sleep')

                sb.Size = 0
                if not hw.SendRequest(hwConst.idSayHelloHelloWorld, sb.SaveString('Donald').SaveString('Trump'),
                                      cbSayHello,
                                      cbAborted, serverEx):
                    raise OSError(1, 'Session already closed before sending the request say_hello')

                sb.Size = 0
                if not hw.SendRequest(hwConst.idSayHelloHelloWorld, sb.SaveString('Jack').SaveString('Smith'),
                                      cbSayHello,
                                      cbAborted, serverEx):
                    raise OSError(1, 'Session already closed before sending the request say_hello')

                sb.Size = 0
                f = Future()

                def cb_aborted(ah, canceled):
                    if canceled:
                        f.cancel()
                    else:
                        if hw.Socket.ErrCode:
                            f.set_exception(OSError(hw.Socket.ErrCode, hw.Socket.ErrMsg))
                        else:
                            f.set_exception(OSError(2, 'Session closed after request is sent'))

                if not hw.SendRequest(hwConst.idEchoHelloWorld, sb.Save(ms),
                                      lambda ar: f.set(ar.Load(CMyStruct())),
                                      cb_aborted,
                                      lambda ah, se: f.set_exception(se)):
                    raise OSError(1, 'Session already closed before sending the request echo')
                print('All five requests are sent to server for processing ......')
                res = f.get()  # wait and retrieve an expected complex structure
            except Exception as ex:
                print(ex)
    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()
