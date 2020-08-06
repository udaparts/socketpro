from webdemo.myhttppeer import CMyHttpPeer
from spa.serverside import *
import sys

with CSocketProServer() as server:
    def do_configuration():
        CSocketProServer.PushManager.AddAChatGroup(1, "R&D Department")
        CSocketProServer.PushManager.AddAChatGroup(2, "Sales Department")
        CSocketProServer.PushManager.AddAChatGroup(3, "Management Department")
        CSocketProServer.PushManager.AddAChatGroup(7, "HR Department")
        return True
    server.OnSettingServer = do_configuration

    #HTTP/WebSocket service
    server.HttpSvs = CSocketProService(CMyHttpPeer, BaseServiceID.sidHTTP, None)

    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
