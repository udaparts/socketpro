from webdemo.myhttppeer import CMyHttpPeer
from pub_sub.ps_server.hwpeer import CHelloWorldPeer
from spa.serverside import *
from consts import hwConst
import sys

with CSocketProServer() as server:
    def do_configuration():
        CSocketProServer.PushManager.AddAChatGroup(1, "R&D Department")
        CSocketProServer.PushManager.AddAChatGroup(2, "Sales Department")
        CSocketProServer.PushManager.AddAChatGroup(3, "Management Department")
        CSocketProServer.PushManager.AddAChatGroup(7, "HR Department")
        return True
    server.OnSettingServer = do_configuration

    # map request ids to their names and speeds so that SocketPro is able to map a request id
    # to its method and use main or worker thread at run time
    mapIdReq = {
        hwConst.idSayHello: 'sayHello',
        hwConst.idSleep: ['sleep', True],  # or ('sleep', True)
        hwConst.idEcho: 'echo'
    }
    server.HelloWorld = CSocketProService(CHelloWorldPeer, hwConst.sidHelloWorld, mapIdReq)

    #HTTP/WebSocket service
    server.HttpSvs = CSocketProService(CMyHttpPeer, BaseServiceID.sidHTTP, None)

    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
