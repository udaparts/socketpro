

from consts import piConst
from spa.serverside import CSocketProServer, CSocketProService, CClientPeer
import sys

with CSocketProServer() as server:
    map = {}
    server.Pi = CSocketProService(CClientPeer, piConst.sidPi, map)
    server.PiWorker = CSocketProService(CClientPeer, piConst.sidPiWorker, map)
    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    else:
        ok = CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
