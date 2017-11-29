
from spa.clientside import CSocketPool, CClientSocket

class CMasterSlaveBase(CSocketPool):
    def __init__(self, clsAsyncHandler, defaultDB, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT):
        super(CMasterSlaveBase, self).__init__(clsAsyncHandler, True, recvTimeout, CClientSocket.DEFAULT_CONN_TIMEOUT, 0)
        self._defaultDB_ = defaultDB
        self._recvTimeout_ = recvTimeout

    @property
    def DefaultDBName(self):
        return self._defaultDB_

    @property
    def RecvTimeout(self):
        return self._recvTimeout_
    