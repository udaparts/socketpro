from pub_sub.ps_server.hwpeer import CHelloWorldPeer
from spa.serverside import *
from consts import hwConst

with CSocketProServer() as server:
    def do_configuration():
        CSocketProServer.PushManager.AddAChatGroup(1, "R&D Department")
        CSocketProServer.PushManager.AddAChatGroup(2, "Sales Department")
        CSocketProServer.PushManager.AddAChatGroup(3, "Management Department")
        return True # True -- ok; False -- no listening server
    server.OnSettingServer = do_configuration

    def OnIsPermitted(hSocket, userId, pwd, svsId):
        print('Ask for a service ' + str(svsId) + ' from user ' + userId + ' with password = ' + pwd)
        return True  # True == permitted and False == denied
    server.OnIsPermitted = OnIsPermitted

    # map request ids to their names and speeds so that SocketPro is able to map a request id
    # to its method and use main or worker thread at run time
    mapIdReq = {
        hwConst.idSayHello : 'sayHello',
        hwConst.idSleep : ['sleep', True],  # or ('sleep', True)
        hwConst.idEcho : 'echo'
    }
    server.HelloWorld = CSocketProService(CHelloWorldPeer, hwConst.sidHelloWorld, mapIdReq)

    # test certificate and private key files are located at the directory ..\SocketProRoot\bin
    if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
        server.UseSSL("intermediate.pfx", "", "mypassword")

        # or load cert and private key from windows system cert store
        # server.UseSSL("root", "UDAParts Intermediate CA", "") # "my"
    else:
        server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword")

    if not server.Run(20901):
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
