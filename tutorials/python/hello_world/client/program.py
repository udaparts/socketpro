from hello_world.client.asynchelloworld import CHelloWorld
from spa import CScopeUQueue as Sb, CServerError as Se
from spa.clientside import CSocketPool, CConnectionContext, CSocketError
from consts import hwConst
from msstruct import CMyStruct
import sys

with CSocketPool(CHelloWorld) as sp:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    # sp.QueueName = 'pqueue'  # turn on client message queue for backing up requests
    if not sp.StartSocketPool(cc, 1):
        print('Cannot connect to server with error message: ' + sp.Sockets[0].ErrMsg)
    else:
        hw = sp.Seek()
        ms = CMyStruct.MakeOne()  # make a complex structure
        print(ms)
        # process requests one by one synchronously -- three round trips
        try:
            print(hw.say_hello(u'Jack', u'Smith'))
            hw.sleep(4000)
            print(hw.echo(ms))
        except Se as ex:  # an exception from remote server
            print(ex)
        except CSocketError as ex:  # a communication error
            print(ex)
        except Exception as ex:
            print('Unexpected error: ' + str(ex))  # invalid parameter, bad de-serialization, and so on

        print('')
        print('Going to send requests with inline batching for better network efficiency and less round trips')
        try:
            fut0 = hw.sendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString('Hilary').SaveString('Clinton'))
            fut1 = hw.sendRequest(hwConst.idSleepHelloWorld, Sb().SaveUInt(15000))
            fut2 = hw.sendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString('Donald').SaveString('Trump'))
            fut3 = hw.sendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString('Jack').SaveString('Smith'))
            # save a complex object that has interface IUSerializer implemented
            fut4 = hw.sendRequest(hwConst.idEchoHelloWorld, Sb().Save(ms))
            print('All requests are sent to server for processing ......')

            print(fut0.result().LoadString())
            print('Buffer size: ' + str(fut1.result().Size))  # sleep returns an empty buffer
            # bad_operation = fut1.result().LoadString()
            print(fut2.result().LoadString())
            print(fut3.result().LoadString())
            # load a complex object that has interface IUSerializer implemented
            print(fut4.result().Load(CMyStruct()))
        except Se as ex:  # an exception from remote server
            print(ex)
        except CSocketError as ex:  # a communication error
            print(ex)
        except Exception as ex:
            print('Unexpected error: ' + str(ex))  # invalid parameter, bad de-serialization, and so on

        def cb_aborted(ah, canceled):
            if canceled:
                print('Request canceled')
            else:
                cs = ah.Socket
                ec = cs.ErrCode
                if ec:
                    print({'ec':ec, 'em': cs.ErrMsg})
                else:
                    print({'ec': CHelloWorld.SESSION_CLOSED_AFTER, 'em': 'Session closed after sending the request SendRequest'})
        if not hw.SendRequest(hwConst.idSayHelloHelloWorld, Sb().SaveString('SocketPro').SaveString('UDAParts'),
                lambda ar: print(ar.LoadString()), cb_aborted, lambda ah, se: print(se)):
            print({'ec': CHelloWorld.SESSION_CLOSED_BEFORE, 'em': 'Session already closed before sending the request SendRequest'})

    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()
