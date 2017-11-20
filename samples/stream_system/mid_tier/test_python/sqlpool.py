
from spa import CDataSet, CTable, CMasterSlaveBase
from spa.clientside import CClientSocket, CAsyncDBHandler, tagSocketPoolEvent
from spa.udb import CDBColumnInfoArray

class CSqlMasterPool(CMasterSlaveBase):
    def __init__(self, clsAsyncHandler, defaultDB, midTier=False, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT):
        super(CSqlMasterPool, self).__init__(clsAsyncHandler, True, recvTimeout, CClientSocket.DEFAULT_CONN_TIMEOUT, 0)
        self._midTier_ = midTier
        self._msTool_ = CDataSet()
        self._m_cache_ = CDataSet()
        self._m_meta_ = CDBColumnInfoArray()
        self._m_handler_ = None

    @property
    def Cache(self):
        return self._msTool_

    def _SetInitialCache_(self):

        def rh(h, res, errMsg):
            if res == 0:
                self._m_cache_.DBServerName = ""
                self._m_cache_.Updater = ""
                self._m_cache_.Empty()
                ip, port = h.AttachedClientSocket.GetPeerName()
                ip += ":"
                ip += str(port)
                self._m_cache_.Set(ip, h.DBManagementSystem)

        # open default database and subscribe for table update events (update, delete and insert) by setting flag ClientSide.CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES
        ok = self._m_handler_.Open(self.DefaultDBName, rh, CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES)

        # bring all cached table data into m_cache first for initial cache, and exchange it with Cache if there is no error
        def sql_result(h, res, errMsg, affected, fail_ok, id):
            if res == 0:
                self._msTool_.Swap(self._m_cache_) # exchange between master Cache and this m_cache

        def sql_data(h, vData):
            meta = h.ColumnInfo
            self._m_cache_.AddRows(meta[0].DBPath, meta[0].TablePath, vData)

        def sql_meta(h):
            self._m_cache_.AddEmptyRowset(h.ColumnInfo)

        ok = self._m_handler_.ExecuteSql('', sql_result, sql_data, sql_meta)

    def OnSocketPoolEvent(self, spe, handler):

        super(CSqlMasterPool, self).OnSocketPoolEvent(spe, handler)


    class CSlavePool(CMasterSlaveBase):
        def __init__(self, clsAsyncHandler, defaultDB, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT):
            super(CSqlMasterPool.CSlavePool, self).__init__(clsAsyncHandler, defaultDB, recvTimeout)

        def OnSocketPoolEvent(self, spe, handler):
            if spe==tagSocketPoolEvent.speConnected:
                handler.Open(self.DefaultDBName, None)
            super(CSqlMasterPool.CSlavePool, self).OnSocketPoolEvent(spe, handler)
