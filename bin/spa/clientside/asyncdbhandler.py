
import threading
from spa.memqueue import CUQueue, CScopeUQueue
from spa.udb import *
from spa.clientside import *
import collections
import math, uuid

class CAsyncDBHandler(CAsyncServiceHandler):
    ONE_MEGA_BYTES = 0x100000
    BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0

    def __init__(self, serviceId):
        super(CAsyncDBHandler, self).__init__(serviceId)
        self._csDB = threading.Lock()
        self._vColInfo = CDBColumnInfoArray()
        self._strConnection = u''
        self._affected = -1
        self._dbError = 0
        self._dbErrorMsg = u''
        self._lastReqId = 0
        self._mapRowset = {}
        self._indexRowset = 0
        self._Blob = CUQueue()
        self._vData = []
        self._ms = tagManagementSystem.msUnknown
        self._flags = 0
        self._deqResult = collections.deque()
        self._parameters = 0
        self._indexProc = 0
        self._output = 0
        self._mapParameterCall = {}
        self._bCallReturn = False
        self._csOneSending = threading.Lock()
        self._queueOk = False

    def OnAllProcessed(self):
        with self._csDB:
            self._mapParameterCall = {}
            while len(self._mapRowset) > 16:
                self._mapRowset.popitem()

    def _GetResultHandler(self, reqId):
        if self.AttachedClientSocket.Random:
            with self._csDB:
                for one in self._deqResult:
                    if one.first == reqId:
                        self._deqResult.remove(one)
                        return one
        else:
            with self._csDB:
                if len(self._deqResult) > 0 and self._deqResult[0].first == reqId:
                    return self._deqResult.popleft()

    @property
    def Parameters(self):
        with self._csDB:
            return self._parameters

    @property
    def Outputs(self):
        with self._csDB:
            return self._output

    @property
    def LastDBErrorCode(self):
        with self._csDB:
            return self._dbError

    @property
    def DBManagementSystem(self):
        with self._csDB:
            return self._ms

    @property
    def Opened(self):
        with self._csDB:
            return self._strConnection != None and self._lastReqId > 0

    @property
    def ColumnInfo(self):
        with self._csDB:
            return self._vColInfo

    @property
    def LastAffected(self):
        with self._csDB:
            return self._affected

    @property
    def LastDBErrorMessage(self):
        with self._csDB:
            return self._dbErrMsg

    @property
    def CallReturn(self):
        with self._csDB:
            return self._bCallReturn

    def OnMergeTo(self, dbTo):
        with dbTo._csDB:
            with self._csDB:
                dbTo._mapRowset.update(self._mapRowset)
                self._mapRowset = {}
                dbTo._mapParameterCall.update(self._mapParameterCall)
                self._mapParameterCall = {}
                dbTo._deqResult.extend(self._deqResult)
                self._deqResult = collections.deque()

    def CleanCallbacks(self):
        with self._csDB:
            self._Clean()
        return super(CAsyncDBHandler, self).CleanCallbacks()

    def OnResultReturned(self, reqId, mc):
        if reqId == DB_CONSTS.idExecuteParameters or reqId == DB_CONSTS.idExecute:
            affected = mc.LoadLong()
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            vtId = mc.LoadObject()
            fail_ok = mc.LoadULong()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._lastReqId = reqId
                self._affected = affected
                self._dbError = res
                self._dbErrorMsg = errMsg
                if self._indexRowset in self._mapRowset:
                    self._mapRowset.pop(self._indexRowset)
                self._indexProc = 0
                if self._indexRowset in self._mapParameterCall:
                    self._mapParameterCall.pop(self._indexRowset)
                elif self.AttachedClientSocket.CountOfRequestsInQueue == 1:
                    self._mapParameterCall = {}
            if not t is None and not t.second is None:
                t.second(self, res, errMsg, affected, fail_ok, vtId)

        elif reqId == DB_CONSTS.idPrepare:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            parameters = mc.LoadInt()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._lastReqId = reqId
                self._dbError = res
                self._dbErrorMsg = errMsg
                self._indexProc = 0
                self._parameters = (parameters & 0xffff)
                self._output = (parameters >> 16)
                self._bCallReturn = False
            if not t is None and not t.second is None:
                t.second(self, res, errMsg)

        elif reqId == DB_CONSTS.idOpen:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            ms = mc.LoadInt()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._CleanRowset()
                self._dbError = res
                self._lastReqId = reqId
                if res == 0:
                    self._strConnection = errMsg
                    errMsg = u''
                else:
                    self._strConnection = u''
                self._dbErrorMsg = errMsg
                self._ms = ms
                self._indexProc = 0
                self._parameters = 0
                self._output = 0
                if self.AttachedClientSocket.CountOfRequestsInQueue == 1:
                    self._mapParameterCall = {}
            if not t is None and not t.second is None:
                t.second(self, res, errMsg)

        elif reqId == DB_CONSTS.idEndTrans:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._lastReqId = reqId
                self._dbError = res
                self._dbErrorMsg = errMsg
                self._CleanRowset()
                if self.AttachedClientSocket.CountOfRequestsInQueue == 1:
                    self._mapParameterCall = {}
            if not t is None and not t.second is None:
                t.second(self, res, errMsg)

        elif reqId == DB_CONSTS.idBeginTrans:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            ms = mc.LoadInt()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._CleanRowset()
                if res == 0:
                    self._strConnection = errMsg
                    errMsg = u''
                self._lastReqId = reqId
                self._dbError = res
                self._dbErrorMsg = errMsg
                self._ms = ms
            if not t is None and not t.second is None:
                t.second(self, res, errMsg)

        elif reqId == DB_CONSTS.idClose:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            t = self._GetResultHandler(reqId)
            with self._csDB:
                self._lastReqId = reqId
                self._dbError = res
                self._dbErrorMsg = errMsg
                self._CleanRowset()
                self._indexProc = 0
                self._parameters = 0
                self._output = 0
                if self.AttachedClientSocket.CountOfRequestsInQueue == 1:
                    self._mapParameterCall = {}
            if not t is None and not t.second is None:
                t.second(self, res, errMsg)

        elif reqId == DB_CONSTS.idRowsetHeader:
            self._Blob.SetSize(0)
            if self._Blob.MaxBufferSize > CAsyncDBHandler.ONE_MEGA_BYTES:
                self._Blob.Realloc(CAsyncDBHandler.ONE_MEGA_BYTES)
            self._vData = []
            header = None
            with self._csDB:
                self._vColInfo.LoadFrom(mc)
                self._indexRowset = mc.LoadULong()
                if mc.GetSize() > 0:
                    self._output = mc.LoadUInt()
                else:
                    self._output = 0
                if self._output == 0 and len(self._vColInfo) > 0:
                    if self._indexRowset in self._mapRowset:
                        header = self._mapRowset.get(self._indexRowset).first
            if not header is None:
                header(self)
        elif reqId == DB_CONSTS.idCallReturn:
            vt = mc.LoadObject()
            with self._csDB:
                if self._indexRowset in self._mapParameterCall:
                    vParam = self._mapParameterCall[self._indexRowset]
                    pos = self._parametersm * self._indexProc
                    vParam[pos] = vt
                self._bCallReturn = True

        elif reqId == DB_CONSTS.idBeginRows:
            self._Blob.SetSize(0)
            self._vData = []
            if mc.GetSize() > 0:
                with self._csDB:
                    self._indexRowset = mc.LoadULong()
        elif reqId == DB_CONSTS.idTransferring:
            while mc.GetSize() > 0:
                vt = mc.LoadObject()
                self._vData.append(vt)
        elif reqId == DB_CONSTS.idOutputParameter or reqId == DB_CONSTS.idEndRows:
            if mc.GetSize() > 0 or len(self._vData) > 0:
                while mc.GetSize() > 0:
                    vt = mc.LoadObject()
                    self._vData.append(vt)
                if reqId == DB_CONSTS.idOutputParameter:
                    with self._csDB:
                        self._output = len(self._vData)
                        if self._indexRowset in self._mapParameterCall:
                            vParam = self._mapParameterCall[self._indexRowset]
                            pos = self._parameters * self._indexProc + self._parameters - self._output
                            for obj in self._vData:
                                vParam[pos] = obj
                                pos += 1
                        self._indexProc += 1
                else:
                    row = None
                    with self._csDB:
                        if self._indexRowset in self._mapRowset:
                            row = self._mapRowset.get(self._indexRowset).second
                    if not row is None:
                        row(self, self._vData)
            self._vData = []
        elif reqId == DB_CONSTS.idStartBLOB:
            if mc.GetSize() > 0:
                self._Blob.SetSize(0)
                length = mc.LoadUInt()
                if length != -1 and length > self._Blob.MaxBufferSize:
                    self._Blob.Realloc(length)
                self._Blob.Push(mc.GetBuffer(), mc.GetSize())
                mc.SetSize(0)
        elif reqId == DB_CONSTS.idChunk:
            if mc.GetSize() > 0:
                self._Blob.Push(mc.GetBuffer(), mc.GetSize())
                mc.SetSize(0)
        elif reqId == DB_CONSTS.idEndBLOB:
            if mc.GetSize() > 0 or self._Blob.GetSize() > 0:
                self._Blob.Push(mc.GetBuffer(), mc.GetSize())
                mc.SetSize(0)
                content_len = self._Blob.PeakUInt(2)
                if content_len >= CAsyncDBHandler.BLOB_LENGTH_NOT_AVAILABLE:
                    content_len = self._Blob.GetSize() - 6 #sizeof(VARTYPE) + sizeof(unsigned int)
                    self._Blob.ResetUInt(content_len, 2)
                vt = self._Blob.LoadObject()
                self._vData.append(vt)
        else:
            super(CAsyncDBHandler, self).OnResultReturned(reqId, mc)

    def _Clean(self):
        self._strConnection = u''
        self._mapRowset = {}
        self._vColInfo = CDBColumnInfoArray()
        self._lastReqId = 0
        self._Blob.SetSize(0)
        if self._Blob.MaxBufferSize > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE:
            self._Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
        self._vData = []

    def _CleanRowset(self, size = 0):
        if ((len(self._mapRowset) > 0 or len(self._vColInfo) > 0) and self.AttachedClientSocket.Sendable and self.AttachedClientSocket.CountOfRequestsInQueue <= size and self.AttachedClientSocket.ClientQueue.MessageCount <= size):
            self._mapRowset = {}
            self._vColInfo = CDBColumnInfoArray()

    def Close(self, handler = None, discarded = None):
        """
        Notify connected remote server to close database connection string asynchronously
        :param handler: a callback for closing result, which should be OK always as long as there is network or queue available
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        cb = Pair(DB_CONSTS.idClose, handler)
        buffer = CScopeUQueue.Lock()
        with self._csOneSending:
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                self._deqResult.append(cb)
            ok = self.SendRequest(DB_CONSTS.idClose, buffer, None, discarded)
            if not ok:
                with self._csDB:
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(buffer)
        return ok

    def BeginTrans(self, isolation = tagTransactionIsolation.tiReadCommited, handler = None, discarded = None):
        """
        Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
        :param isolation: a value for transaction isolation. It defaults to tagTransactionIsolation.tiReadCommited
        :param handler: a callback for tracking its response result. It defaults to None
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        cb = Pair(DB_CONSTS.idBeginTrans, handler)
        q = CScopeUQueue.Lock()

        """
        make sure BeginTrans sending and underlying client persistent message queue as one combination sending
        to avoid possible request sending/client message writing overlapping within multiple threading environment
        """
        with self._csOneSending:
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                q.SaveInt(isolation).SaveString(self._strConnection).SaveUInt(self._flags)
                self._deqResult.append(cb)
            #associate begin transaction with underlying client persistent message queue
            self._queueOk = self.AttachedClientSocket.ClientQueue.StartJob()
            ok = self.SendRequest(DB_CONSTS.idBeginTrans, q, None, discarded)
            if not ok:
                with self._csDB:
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(q)
        return ok

    def EndTrans(self, plan = tagRollbackPlan.rpDefault, handler = None, discarded = None):
        """
        End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
        :param plan: a value for computing how included transactions should be rollback at server side. It defaults to tagRollbackPlan.rpDefault
        :param handler: a callback for tracking its response result
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        q = CScopeUQueue.Lock().SaveInt(plan)
        cb = Pair(DB_CONSTS.idEndTrans, handler)
        """
        make sure EndTrans sending and underlying client persistent message queue as one combination sending
        to avoid possible request sending/client message writing overlapping within multiple threading environment
        """
        with self._csOneSending:
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                self._deqResult.append(cb)
            ok = self.SendRequest(DB_CONSTS.idEndTrans, q, None, discarded)
            if ok:
                if self._queueOk:
                    #associate end transaction with underlying client persistent message queue
                    self.AttachedClientSocket.ClientQueue.EndJob()
                    self._queueOk = False
            else:
                with self._csDB:
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(q)
        return ok


    def Open(self, strConnection, handler = None, flags = 0, discarded = None):
        """
        Open a database connection at server side asynchronously
        :param strConnection: a database connection string. The database connection string can be an empty string if its server side supports global database connection string
        :param hander: a callback for database connecting result
        :param flags: a set of flags transferred to server to indicate how to build database connection at server side. It defaults to zero
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        s = ''
        q = CScopeUQueue.Lock().SaveString(strConnection).SaveUInt(flags)
        cb = Pair(DB_CONSTS.idOpen, handler)
        with self._csOneSending:
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                self._flags = flags
                self._deqResult.append(cb)
                if not strConnection is None:
                    s = self._strConnection
                    self._strConnection = strConnection
            ok = self.SendRequest(DB_CONSTS.idOpen, q, None, discarded)
            if not ok:
                with self._csDB:
                    if not strConnection is None:
                        self._strConnection = s
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(q)
        return ok

    def Prepare(self, sql, handler = None, lstParameterInfo = [], discarded = None):
        """
        Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
        :param sql: a parameterized SQL statement
        :param handler: a callback for SQL preparing result
        :param lstParameterInfo: a given array of parameter informations
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        q = CScopeUQueue.Lock().SaveString(sql)
        count = 0
        if not lstParameterInfo is None:
            count = len(lstParameterInfo)
        q.SaveUInt(count)
        if count > 0:
            for one in lstParameterInfo:
                one.SaveTo(q)
        cb = Pair(DB_CONSTS.idPrepare, handler)
        with self._csOneSending:
            with self._csDB:
                self._deqResult.append(cb)
            ok = self.SendRequest(DB_CONSTS.idPrepare, q, None, discarded)
            if not ok:
                with self._csDB:
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(q)
        return ok

    def Execute(self, sql_or_array, handler = None, row = None, rh = None, meta = True, lastInsertId = True, discarded = None):
        if isinstance(sql_or_array, list):
            return self.ExecuteParameters(sql_or_array, handler, row, rh, meta, lastInsertId, discarded)
        elif isinstance(sql_or_array, tuple):
            return self.ExecuteParameters(sql_or_array, handler, row, rh, meta, lastInsertId, discarded)
        return self.ExecuteSql(sql_or_array, handler, row, rh, meta, lastInsertId, discarded)

    def ExecuteSql(self, sql, handler = None, row = None, rh = None, meta = True, lastInsertId = True, discarded = None):
        """
        Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
        :param sql: a complex SQL statement which may be combined with multiple basic SQL statements
        :param handler: a callback for tracking final result
        :param row: a callback for tracking record or output parameter returned data
        :param rh: a callback for tracking row set of header column informations. Note that there will be NO row set data or its column informations returned if NO such a callback is set
        :param meta: a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true
        :param lastInsertId: a boolean value for last insert record identification number. It defaults to true
        :param discarded: a callback for tracking cancel or socket closed event
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        index = self.GetCallIndex()
        rowset = (rh or row)
        if not rowset:
            meta = False
        q = CScopeUQueue.Lock().SaveString(sql).SaveBool(rowset).SaveBool(meta).SaveBool(lastInsertId).SaveULong(index)
        cb = Pair(DB_CONSTS.idExecute, handler)
        with self._csOneSending:
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                if rowset:
                    self._mapRowset[index] = Pair(rh,row)
                self._deqResult.append(cb)
            ok = self.SendRequest(DB_CONSTS.idExecute, q, None, discarded)
            if not ok:
                with self._csDB:
                    self._deqResult.remove(cb)
                    if rowset:
                        self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def _SendParametersData(self, vParam):
        if vParam is None:
            return True
        size = len(vParam)
        self._firstRow = True
        def Send(sb):
            if sb.GetSize() > 0:
                if self._firstRow:
                    self._firstRow = False
                    if not self.SendRequest(DB_CONSTS.idBeginRows, sb, None):
                        return False
                else:
                    if not self.SendRequest(DB_CONSTS.idTransferring, sb, None):
                        return False
                sb.SetSize(0)
            elif self._firstRow:
                self._firstRow = False
                return self.SendRequest(DB_CONSTS.idBeginRows, None, None)
            return True
        def SendBlob(sb):
            start = True
            while sb.GetSize() > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE:
                bytes = sb.PopBytes(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                if start:
                    if not self.SendRequest(DB_CONSTS.idStartBLOB, CUQueue(bytes), None):
                        return False
                    start = False
                else:
                    if not self.SendRequest(DB_CONSTS.idChunk, CUQueue(bytes), None):
                        return False
            if not self.SendRequest(DB_CONSTS.idEndBLOB, sb, None):
                return False
            sb.SetSize(0)
            return True
        sb = CScopeUQueue.Lock()
        for vt in vParam:
            if isinstance(vt, str):
                #send string as unicode string
                length = len(vt) * 2
                if length < 2 * DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE:
                    sb.SaveObject(vt)
                else:
                    if not Send(sb):
                        CScopeUQueue.Unlock(sb)
                        return False
                    length += 6
                    sb.SaveUInt(length).SaveObject(vt)
                    if not SendBlob(sb):
                        CScopeUQueue.Unlock(sb)
                        return False
            elif isinstance(vt, bytearray):
                #send array of bytes
                length = len(vt)
                if length < 2 * DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE:
                    sb.SaveObject(vt)
                else:
                    if not Send(sb):
                        CScopeUQueue.Unlock(sb)
                        return False
                    length += 6
                    sb.SaveUInt(length).SaveObject(vt)
                    if not SendBlob(sb):
                        CScopeUQueue.Unlock(sb)
                        return False
            else:
                sb.SaveObject(vt)
            if sb.GetSize() >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE and not Send(sb):
                CScopeUQueue.Unlock(sb)
                return False
        ok = Send(sb)
        CScopeUQueue.Unlock(sb)
        return ok

    def ExecuteParameters(self, vParam, handler = None, row = None, rh = None, meta = True, lastInsertId = True, discarded = None):
        """
        Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
        :param vParam: an array of parameter data which will be bounded to previously prepared parameters
        :param handler: a callback for tracking final result
        :param row: a callback for tracking record or output parameter returned data
        :param rh: a callback for tracking row set of header column informations. Note that there will be NO row set data or its column informations returned if NO such a callback is set
        :param meta: a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true
        :param lastInsertId: a boolean value for last insert record identification number. It defaults to true
        :param discarded: a callback for tracking cancel or socket closed event. It defaults to None
        :return: true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
        """
        ok = True
        index = self.GetCallIndex()
        rowset = (rh or row)
        cb = Pair(DB_CONSTS.idExecuteParameters, handler)
        if not rowset:
            meta = False
        q = CScopeUQueue.Lock().SaveBool(rowset).SaveBool(meta).SaveBool(lastInsertId).SaveULong(index)
        """
        make sure all parameter data sendings and ExecuteParameters sending as one combination sending
        to avoid possible request sending overlapping within multiple threading environment
        """
        with self._csOneSending:
            if not self._SendParametersData(vParam):
                CScopeUQueue.Unlock(q)
                return False
            #don't make self._csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
            with self._csDB:
                self._deqResult.append(cb)
                self._mapParameterCall[index] = vParam
                if rowset:
                    self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(DB_CONSTS.idExecuteParameters, q, None, discarded)
            if not ok:
                with self._csDB:
                    if rowset:
                        self._mapRowset.pop(index)
                    self._mapParameterCall.pop(index)
                    self._deqResult.remove(cb)
        CScopeUQueue.Unlock(q)
        return ok