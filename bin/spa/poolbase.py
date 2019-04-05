
from spa.clientside import CSocketPool, CClientSocket

class CMasterSlaveBase(CSocketPool):
    def __init__(self, clsAsyncHandler, defaultDB, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT, autoConn=True, connTimeout=CClientSocket.DEFAULT_CONN_TIMEOUT, svsId=0):
        super(CMasterSlaveBase, self).__init__(clsAsyncHandler, autoConn, recvTimeout, connTimeout, svsId)
        self._defaultDB_ = defaultDB

    @property
    def DefaultDBName(self):
        return self._defaultDB_
