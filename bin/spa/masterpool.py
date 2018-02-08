
from spa import CDataSet, CTable, CMasterSlaveBase
from spa.clientside import CClientSocket, CAsyncDBHandler, tagSocketPoolEvent
from spa.udb import CDBColumnInfoArray, tagUpdateEvent, DB_CONSTS
from spa.serverside import CSocketProServer

class CMasterPool(CMasterSlaveBase):
    def __init__(self, clsAsyncHandler, defaultDB, midTier=False, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT):
        super(CMasterPool, self).__init__(clsAsyncHandler, defaultDB, recvTimeout)
        self._midTier_ = midTier
        self._msTool_ = CDataSet()
        self._m_cache_ = CDataSet()
        self._m_meta_ = None
        self._handler_ = None

    @property
    def Cache(self):
        return self._msTool_

    @property
    def MidTier(self):
        return self._midTier_

    def _SetInitialCache_(self):
        # bring all cached table data into m_cache first for initial cache, and exchange it with Cache if there is no error
        def sql_result(res, errMsg):
            ip, port = self._handler_.AttachedClientSocket.GetPeerName()
            ip += ":"
            ip += str(port)
            self._m_cache_.DBServerName = self._handler_.AttachedClientSocket.ConnectionContext.Host
            self._m_cache_.Set(ip, self._handler_.DBManagementSystem)
            if res == 0:
                self._msTool_.Swap(self._m_cache_) # exchange between master Cache and this m_cache

        def sql_data(vData):
            meta = self._m_meta_
            self._m_cache_.AddRows(meta[0].DBPath, meta[0].TablePath, vData)

        def sql_meta(meta):
            self._m_meta_ = meta
            self._m_cache_.AddEmptyRowset(meta)

        self._m_cache_.Updater = ''
        self._m_cache_.Empty()
        ok = self._handler_.GetCachedTables(self.DefaultDBName, sql_result, sql_data, sql_meta, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)

    def OnSocketPoolEvent(self, spe, handler):

        def OnPublish(sender, messageSender, group, msg):
            if group[0] == DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID:
                if self._midTier_:
                    CSocketProServer.PushManager.Publish(msg, [DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID])
                self._SetInitialCache_(self)
                return
            if self._midTier_:
                CSocketProServer.PushManager.Publish(msg, [DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID])

            # vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
            vData = msg
            eventType = vData[0]
            if len(self._msTool_.DBServerName) == 0:
                self._msTool_.DBServerName = vData[1]
            self._msTool_.Updater = vData[2]
            dbName = vData[3]
            tblName = vData[4]
            ret = 0
            if eventType == tagUpdateEvent.ueDelete:
                keys = vData[5:]
                key1 = None
                if len(keys) > 1:
                    key1 = keys[1]
                ret = self._msTool_.DeleteARow(dbName, tblName, keys[0], key1)
            elif eventType == tagUpdateEvent.ueInsert:
                ret = self._msTool_.AddRows(dbName, tblName, vData[5:])
            elif eventType == tagUpdateEvent.ueUpdate:
                ret = self._msTool_.UpdateARow(dbName, tblName, vData[5:])

        if spe == tagSocketPoolEvent.speUSocketCreated:
            if handler == self.AsyncHandlers[0]:
                self._handler_ = handler
                handler.AttachedClientSocket.Push.OnPublish = OnPublish
        elif spe == tagSocketPoolEvent.speConnected and handler.AttachedClientSocket.ErrorCode == 0:
            if handler == self.AsyncHandlers[0]:
                if self._midTier_:
                    CSocketProServer.PushManager.Publish(None, [DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID])
                self._SetInitialCache_()
            else:
                handler.GetCachedTables(self.DefaultDBName, None, None, None, 0)
        super(CMasterPool, self).OnSocketPoolEvent(spe, handler)

    class CSlavePool(CMasterSlaveBase):
        def __init__(self, clsAsyncHandler, defaultDB, recvTimeout=CClientSocket.DEFAULT_RECV_TIMEOUT):
            super(CMasterPool.CSlavePool, self).__init__(clsAsyncHandler, defaultDB, recvTimeout)
