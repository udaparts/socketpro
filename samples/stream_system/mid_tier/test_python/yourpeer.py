
from spa.serverside import CCacheBasePeer
from sharedstruct import *
from spa import CScopeUQueue
from spa.udb import *
from spa.clientside import UFuture
from config import CConfig
from spa.clientside import CAsyncDBHandler


class CYourPeer(CCacheBasePeer):
    Master = None
    Slave = None

    def OnFastRequestArrive(self, reqId, len):
        if reqId == idQueryMaxMinAvgs:
            pass
        elif reqId == idUploadEmployees:
            pass
        else:
            assert False  # not implemented

    def GetRentalDateTimes(self):
        index = self.UQueue.LoadLong()
        rental_id = self.UQueue.LoadLong()
        myDates = CRentalDateTimes(rental_id)
        res = 0
        errMsg = u''
        sql = u'SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=' + str(rental_id)
        redo = 1

        sb = CScopeUQueue()
        sb.SaveLong(index)
        while redo > 0:
            handler = CYourPeer.Slave.Seek()
            if not handler:
                res = -1
                errMsg = 'No connection to a slave database'
                break
            f = UFuture()
            def ares(h, r, err, affected, fail_ok, vtId):
                res = r
                errMsg = err
                f.set(1)

            def rows(h, vData):
                myDates.Rental = vData[1]
                myDates.Return = vData[2]
                myDates.LastUpdate = vData[3]

            def meta(h):
                assert len(h.ColumnInfo) == 4

            def closed():
                res = -2
                errMsg = 'Request canceled or socket closed'
                f.set(-1)

            if handler.ExecuteSql(sql, ares, rows, meta, True, True, closed):
                ret = f.get(20)
                if not ret:
                    # don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
                    res = -2
                    errMsg = 'Querying rental date times timed out'
                    redo = 0  # no redo because of timed - out
                elif ret > 0:
                    redo = 0  # disable redo after result returned successfully
                else:
                    # socket closed after sending
                    redo = 1
            else:
                # socket closed when sending SQL, re-seek a new handler and retry
                redo = 1
        myDates.SaveTo(sb.UQueue)
        sb.SaveInt(res).SaveString(errMsg)
        return sb

    def GetMasterSlaveConnectedSessions(self):
        index = self.UQueue.LoadLong()
        mc = CYourPeer.Master.ConnectedSockets
        sc = CYourPeer.Slave.ConnectedSockets
        sb = CScopeUQueue()
        sb.SaveLong(index).SaveUInt(mc).SaveUInt(sc)
        return sb

    def GetCachedTables(self):
        defaultDb = self.UQueue.LoadString()
        flags = self.UQueue.LoadUInt()
        index = self.UQueue.LoadLong()
        ms = tagManagementSystem.msUnknown
        res = 0
        errMsg = ''
        config = CConfig.getConfig()
        redo = 1
        while redo > 0:
            redo = 0  # disable redo
            if (flags & CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) == CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES:
                if not self.Push.Subscribe([CAsyncDBHandler.CACHE_UPDATE_CHAT_GROUP_ID, CAsyncDBHandler.STREAMING_SQL_CHAT_GROUP_ID]):
                    errMsg = 'Failed in subscribing for table events' # warning message
            if len(config.m_vFrontCachedTable) == 0:
                break
            sql = ''
            v = config.m_vFrontCachedTable
            for s in v:
                if (len(s)) > 0:
                    sql += ';'
                sql += ('SELECT * FROM ' + s)
            # use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            handler = CYourPeer.Master.Lock()
            if not handler:
                res = -1
                errMsg = 'No connection to a master database'
                break
            ms = handler.DBManagementSystem
            f = UFuture()

            def ares(h, r, err, affected, fail_ok, vtId):
                res = r
                errMsg = err
                f.set(1)

            def rows(h, vData):
                self.SendRows(vData)

            def meta(h):
                self.SendMeta(h.ColumnInfo, index)

            def closed():
                res = -2
                errMsg = 'Request canceled or socket closed'
                f.set(1)

            if not handler.ExecuteSql(sql, ares, rows, meta, True, True, lambda : f.set(-2)):
                res = handler.AttachedClientSocket.ErrorCode
                errMsg = handler.AttachedClientSocket.ErrorMsg
                break

            # put back locked handler and its socket back into pool for reuse as soon as possible
            CYourPeer.Master.Unlock(handler)
            ret = f.get(25.0)
            if ret != 1:
                res = -3
                errMsg = 'Querying cached table data timeout'

        sb = CScopeUQueue()
        sb.SaveInt(ms).SaveInt(res).SaveString(errMsg)
        return sb




