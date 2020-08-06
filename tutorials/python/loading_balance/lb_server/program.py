from consts import piConst
from spa.serverside import CSocketProServer, CSocketProService, CClientPeer
import sys

with CSocketProServer() as server:
    mapIdReq = {}
    server.Pi = CSocketProService(CClientPeer, piConst.sidPi, mapIdReq)
    server.PiWorker = CSocketProService(CClientPeer, piConst.sidPiWorker, mapIdReq)
    ok = CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker)
    if not ok:
        print('Setting routing failed')
    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
