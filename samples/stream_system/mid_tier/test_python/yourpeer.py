
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
            self.QueryMaxMinAvgs(self.UQueue, self.CurrentRequestIndex)
        elif reqId == idUploadEmployees:
            self.UploadEmployees(self.UQueue, self.CurrentRequestIndex)
        elif reqId == idGetRentalDateTimes:
            self.GetRentalDateTimes(self.UQueue, self.CurrentRequestIndex)
        else:
            assert False  # not implemented

    def QueryMaxMinAvgs(self, q, reqIndex):
        filter = q.LoadString()
        pmma = CMaxMinAvg()
        sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment"
        if filter and len(filter) > 0:
            sql += (' WHERE ' + filter)
        handler = CYourPeer.Slave.SeekByQueue()
        if not handler:
            with CScopeUQueue() as sb:
                sb.SaveInt(-1).SaveString('No connection to a slave database')
                pmma.SaveTo(sb.UQueue)
                self.SendResultIndex(reqIndex, sb, idQueryMaxMinAvgs)
        else:
            peer_handle = self.Handle
            def ares(h, r, err, affected, fail_ok, vtId):
                # send result if front peer not closed yet
                if peer_handle == self.Handle:
                    with CScopeUQueue() as sb:
                        sb.SaveInt(r).SaveString(err)
                        pmma.SaveTo(sb.UQueue)
                        self.SendResultIndex(reqIndex, sb, idQueryMaxMinAvgs)
            def rows(h, vData):
                pmma.Max = float(vData[0])
                pmma.Min = float(vData[1])
                pmma.Avg = float(vData[2])
            ok = handler.Execute(sql, ares, rows)
            assert ok # should be always true if pool has local queue for request backup

    def UploadEmployees(self, q, reqIndex):
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
                break
            handler = CYourPeer.Master.Lock()
            if not handler:
                res = -2
                errMsg = 'No connection to a master database'
                break
            cs = handler.AttachedClientSocket
            if not handler.BeginTrans() or not handler.Prepare('INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)'):
                res = cs.ErrorCode
                errMsg = cs.ErrorMsg
                break
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
                res = cs.ErrorCode
                errMsg = cs.ErrorMsg
                break
            peer_handle = self.Handle

            def et(h, r, err):
                if r == 0:
                    res = r
                    errMsg = err
                # send result if front peer not closed yet
                if peer_handle == self.Handle:
                    with CScopeUQueue() as sb0:
                        sb0.SaveInt(res).SaveString(errMsg)
                        vId.SaveTo(sb0.UQueue)
                        self.SendResultIndex(reqIndex, sb0, idUploadEmployees)

            def closed(h, canceled):
                # retry if front peer not closed yet
                if peer_handle == self.Handle:
                    res = cs.ErrorCode
                    errMsg = cs.ErrorMsg
                    with CScopeUQueue() as sb0:
                        sb0.SaveInt(res).SaveString(errMsg)
                        vId.SaveTo(sb0.UQueue)
                        self.SendResultIndex(reqIndex, sb0, idUploadEmployees)
            if not handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, et, closed):
                res = cs.ErrorCode
                errMsg = cs.ErrorMsg
                break
            # put handler back into pool for reuse as soon as possible, as long as socket is not closed yet
            CYourPeer.Master.Unlock(handler)
            # all requests are successfully put on wire and don't use the below to send result
            return

        with CScopeUQueue() as sb:
            sb.SaveInt(res).SaveString(errMsg)
            vId.SaveTo(sb.UQueue)
            self.SendResultIndex(reqIndex, sb, idUploadEmployees)

    """
    # manual retry for better fault tolerance
    def UploadEmployees(self, q, reqIndex):
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
                peer_handle = self.Handle
                def et(h, r, err):
                    if r == 0:
                        res = r
                        errMsg = err

                    # send result if front peer not closed yet
                    if peer_handle == self.Handle:
                        with CScopeUQueue() as sb0:
                            sb0.SaveInt(res).SaveString(errMsg)
                            vId.SaveTo(sb0.UQueue)
                            self.SendResultIndex(reqIndex, sb0, idUploadEmployees)
                def closed(h, canceled):
                    # retry if front peer not closed yet
                    if peer_handle == self.Handle:
                        with CScopeUQueue() as sb0:
                            # repack original data
                            count = len(vData)
                            for d in vData:
                                sb0.SaveObject(d)
                            self.UploadEmployees(sb0.UQueue, reqIndex)
                if handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, et, closed):
                    CYourPeer.Master.Unlock(handler)  # put handler back into pool for reuse as soon as possible
                    return  # disable redo if requests are put on wire
                else:
                    pass  # re-seek a handler and retry as socket is closed when sending request

        with CScopeUQueue() as sb:
            sb.SaveInt(res).SaveString(errMsg)
            vId.SaveTo(sb.UQueue)
            self.SendResultIndex(reqIndex, sb, idUploadEmployees)
    """

    def GetRentalDateTimes(self, q, reqIndex):
        rental_id = q.LoadLong()
        myDates = CRentalDateTimes()
        sql = u'SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=' + str(rental_id)
        handler = CYourPeer.Slave.SeekByQueue()
        if not handler:
            with CScopeUQueue() as sb:
                myDates.SaveTo(sb.UQueue)
                sb.SaveInt(-1).SaveString('No connection to a slave database')
                self.SendResultIndex(reqIndex, sb, idGetRentalDateTimes)
        else:
            def ares(h, r, err, affected, fail_ok, vtId):
                with CScopeUQueue() as sb:
                    myDates.SaveTo(sb.UQueue)
                    sb.SaveInt(r).SaveString(err)
                    self.SendResultIndex(reqIndex, sb, idGetRentalDateTimes)
            def rows(h, vData):
                myDates.rental_id = vData[0]
                myDates.Rental = vData[1]
                myDates.Return = vData[2]
                myDates.LastUpdate = vData[3]
            ok = handler.Execute(sql, ares, rows)
            assert ok # should be always true if pool has local queue for request backup

    def GetMasterSlaveConnectedSessions(self):
        mc = CYourPeer.Master.ConnectedSockets
        sc = CYourPeer.Slave.ConnectedSockets
        sb = CScopeUQueue()
        sb.SaveUInt(mc).SaveUInt(sc)
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
            if len(config.m_vFrontCachedTable) == 0 or flags == 0:
                break
            sql = ''
            v = config.m_vFrontCachedTable
            for s in v:
                if (len(sql)) > 0:
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
