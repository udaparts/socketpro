from hello_world.server.helloworldpeer import CHelloWorldPeer
from spa.serverside import CSocketProServer, CSocketProService
from consts import hwConst
import sys

with CSocketProServer() as server:
    def OnClose(hSocket, errCode):
        bs = CSocketProService.SeekService(hSocket)
        if bs:
            sp = bs.Seek(hSocket)
            # ......
    server.OnClose = OnClose

    def OnIsPermitted(hSocket, userId, pwd, svsId):
        print('Ask for a service ' + str(svsId) + ' from user ' + userId + ' with password = ' + pwd)
        return True  # True == permitted and False == denied
    server.OnIsPermitted = OnIsPermitted

    mapIdMethod = {
        hwConst.idSayHello: 'sayHello',
        hwConst.idSleep: ['sleep', True],  # or ('sleep', True)
        hwConst.idEcho: 'echo'
    }
    server.hw = CSocketProService(CHelloWorldPeer, hwConst.sidHelloWorld, mapIdMethod)

    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
