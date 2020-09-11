from spa.serverside import CCacheBasePeer
from spa.clientside import CSocketError
from shared.sharedstruct import *
from spa import CScopeUQueue
from spa.udb import *
from concurrent.futures import TimeoutError

class CYourPeer(CCacheBasePeer):
    Master = None
    Slave = None
    FrontCachedTables = ['sakila.actor', 'sakila.language', 'sakila.country', 'sakila.film_actor']

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
                sb.SaveInt(-2).SaveString('No connection to anyone of slave databases')
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
            # should be always true if pool has local queue for request backup
            assert ok

    def GetRentalDateTimes(self, q, reqIndex):
        rental_id = q.LoadLong()
        myDates = CRentalDateTimes(rental_id)
        sql = u'SELECT rental_date,return_date,last_update FROM rental where rental_id=' + str(rental_id)
        handler = CYourPeer.Slave.SeekByQueue()
        if not handler:
            with CScopeUQueue() as sb:
                myDates.SaveTo(sb.UQueue)
                sb.SaveInt(-2).SaveString('No connection to anyone of slave databases')
                self.SendResultIndex(reqIndex, sb, idGetRentalDateTimes)
        else:
            def ares(h, r, err, affected, fail_ok, vtId):
                with CScopeUQueue() as sb:
                    myDates.SaveTo(sb.UQueue)
                    sb.SaveInt(r).SaveString(err)
                    self.SendResultIndex(reqIndex, sb, idGetRentalDateTimes)
            def rows(h, vData):
                myDates.Rental = vData[0]
                myDates.Return = vData[1]
                myDates.LastUpdate = vData[2]
            ok = handler.Execute(sql, ares, rows)
            # should be always true if pool has local queue for request backup
            assert ok

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
        sb = CScopeUQueue()
        if len(CYourPeer.FrontCachedTables) == 0 or (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) != DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES:
            return sb.SaveInt(ms).SaveInt(0).SaveString('')
        if (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES:
            self.Push.Subscribe([DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID])
        sql = ''
        v = CYourPeer.FrontCachedTables
        for s in v:
            if (len(sql)) > 0:
                sql += ';'
            sql += ('SELECT * FROM ' + s)
        # use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
        handler = CYourPeer.Master.Lock()
        if not handler:
            return sb.SaveInt(ms).SaveInt(-2).SaveString('No connection to a master database')
        ms = handler.DBManagementSystem
        try:
            f = handler.execute(sql, lambda h, vData: self.SendRows(vData), lambda h : self.SendMeta(h.ColumnInfo, index))
            # put back locked handler and its socket back into pool for reuse as soon as possible
            CYourPeer.Master.Unlock(handler)
            try:
                res = f.result(30)
                return sb.SaveInt(ms).SaveInt(res.ec).SaveString(res.em)
            except TimeoutError as ex:
                return sb.SaveInt(ms).SaveInt(-3).SaveString(str(ex))
        except (CServerError, CSocketError) as ex:
            return sb.SaveInt(ms).SaveInt(ex[0]).SaveString(ex[1])
        except Exception as ex:
            return sb.SaveInt(ms).SaveInt(-1).SaveString(str(ex))

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
            cs = handler.Socket
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