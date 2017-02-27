
from myhttppeer import CMyHttpPeer
from spa.serverside import *
from consts import SQueueConst
import sys

CSocketProServer.QueueManager.MessageQueuePassword = 'MyPasswordForMsgQueue'
if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
    CSocketProServer.QueueManager.WorkDirectory = 'c:\\sp_test'
else:
    CSocketProServer.QueueManager.WorkDirectory = '/home/yye/sp_test/'

with CSocketProServer() as server:
    def doConfiguration():
        CSocketProServer.PushManager.AddAChatGroup(1, "R&D Department")
        CSocketProServer.PushManager.AddAChatGroup(2, "Sales Department")
        CSocketProServer.PushManager.AddAChatGroup(3, "Management Department")
        CSocketProServer.PushManager.AddAChatGroup(7, "HR Department")
        return True
    server.OnSettingServer = doConfiguration

    #HTTP/WebSocket service
    server.HttpSvs = CSocketProService(CMyHttpPeer, BaseServiceID.sidHTTP, None)

    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()