
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
            self.QueryMaxMinAvgs(self.UQueue)
        elif reqId == idUploadEmployees:
            self.UploadEmployees(self.UQueue)
        else:
            assert False  # not implemented

    def QueryMaxMinAvgs(self, q):
        index = q.LoadULong()
        filter = q.LoadString()
        pmma = CMaxMinAvg()
        sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment"
        if filter and len(filter) > 0:
            sql += (' WHERE ' + filter)
        res = 0
        errMsg = ''
        while True:
            handler = CYourPeer.Slave.Seek()
            if not handler:
                res = -1
                errMsg = 'No connection to a slave database'
                break
            peer_handle = self.Handle

            def ares(h, r, err, affected, fail_ok, vtId):
                # send result if front peer not closed yet
                if peer_handle == self.Handle:
                    with CScopeUQueue() as sb0:
                        sb0.SaveULong(index).SaveInt(res).SaveString(errMsg)
                        pmma.SaveTo(sb0.UQueue)
                        self.SendResult(sb0, idQueryMaxMinAvgs)

            def rows(h, vData):
                pmma.Max = float(vData[0])
                pmma.Min = float(vData[1])
                pmma.Avg = float(vData[2])

            def meta(h):
                pass

            def closed():
                # retry if front peer not closed yet
                if peer_handle == self.Handle:
                    with CScopeUQueue() as sb0:
                        sb0.SaveULong(index).SaveInt(filter)
                        self.QueryMaxMinAvgs(sb0.UQueue)

            if handler.Execute(sql, ares, rows, meta, True, True, closed):
                # disable redo once request is put on wire
                return

            # re-seek a handler and retry as socket is closed when sending the request

        with CScopeUQueue() as sb:
            sb.SaveULong(index).SaveInt(res).SaveString(errMsg)
            pmma.SaveTo(sb.UQueue)
            self.SendResult(sb, idQueryMaxMinAvgs)


    def UploadEmployees(self, q):
        index = q.LoadULong()
        vData = []
        count = q.LoadUInt()
        while count > 0:
            vData.append(q.LoadObject())
            count -= 1
        res = 0
        errMsg = ''
        vId = CLongArray()
        while True:
            if len(vData) == 0:
                break  # no retry
            elif len(vData) % 3 != 0:
                res = -1
                errMsg = 'Data array size is wrong'
                break  # no retry
            # use master for insert, update and delete
            # use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            handler = CYourPeer.Master.Lock()
            if not handler:
                res = -2
                errMsg = 'No connection to a master database'
                break  # no retry
            while True:
                if not handler.BeginTrans():
                    break  # re-seek a handler and retry as socket is closed when sending request
                if not handler.Prepare('INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)'):
                    break  # re-seek a handler and retry as socket is closed when sending request
                rows = len(vData) / 3
                r = 0
                ok = False
                while r < rows:
                    v = []
                    v.append(vData[r * 3 + 0])
                    v.append(vData[r * 3 + 1])
                    v.append(vData[r * 3 + 2])
                    r += 1

                    def ares(h, r, err, affected, fail_ok, vtId):
                        if r != 0:
                            res = r
                            errMsg = err
                            vId.list.append(-1)
                        else:
                            vId.list.append(vtId)

                    ok = handler.Execute(v, ares)
                    if not ok:
                        break
                if not ok:
                    break  # re-seek a handler and retry as socket is closed when sending request

                def et(h, r, err):
                    if r == 0:
                        res = r
                        errMsg = err

                    # send result if front peer not closed yet
                    if peer_handle == self.Handle:
                        with CScopeUQueue() as sb0:
                            sb0.SaveULong(index).SaveInt(res).SaveString(errMsg)
                            vId.SaveTo(sb0.UQueue)
                            self.SendResult(sb0, idUploadEmployees)

                def closed():
                    # retry if front peer not closed yet
                    if peer_handle == self.Handle:
                        with CScopeUQueue() as sb0:
                            # repack original data
                            sb0.SaveULong(index)
                            count = len(vData)
                            for d in vData:
                                sb0.SaveObject(d)
                            self.UploadEmployees(sb0.UQueue)

                peer_handle = self.Handle
                if handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, et, closed):
                    CYourPeer.Master.Unlock(handler)  # put handler back into pool for reuse as soon as possible
                    return  # disable redo if requests are put on wire
                else:
                    pass  # re-seek a handler and retry as socket is closed when sending request

        with CScopeUQueue() as sb:
            sb.SaveULong(index).SaveInt(res).SaveString(errMsg)
            vId.SaveTo(sb.UQueue)
            self.SendResult(sb, idUploadEmployees)

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

            if handler.Execute(sql, ares, rows, meta, True, True, closed):
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
            if (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES:
                if not self.Push.Subscribe([DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID]):
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

            if not handler.Execute(sql, ares, rows, meta, True, True, lambda : f.set(-2)):
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
