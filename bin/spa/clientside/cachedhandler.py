
from spa.clientside import CAsyncServiceHandler
import threading
from spa import CUQueue, CScopeUQueue, Pair
from spa.udb import *
import collections

class CCachedBaseHandler(CAsyncServiceHandler):
    ONE_MEGA_BYTES = 0x100000
    BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0

    def __init__(self, serviceId):
        super(CCachedBaseHandler, self).__init__(serviceId)
        self._csCache = threading.Lock()
        self._mapRowset = {}
        self._indexRowset = 0
        self._Blob = CUQueue()
        self._vData = []
        self._ms = tagManagementSystem.msUnknown
        self._mapHandler = {}

    @property
    def DBManagementSystem(self):
        with self._csCache:
            return self._ms

    def CleanCallbacks(self):
        with self._csCache:
            self._mapRowset = {}
            self._mapHandler = {}
        return super(CCachedBaseHandler, self).CleanCallbacks()

    def OnMergeTo(self, dbTo):
        with dbTo._csDB:
            with self._csDB:
                dbTo._mapRowset.update(self._mapRowset)
                self._mapRowset = {}
                dbTo._mapHandler.update(self._mapHandler)
                self._mapHandler = {}

    def GetCachedTables(self, defaultDb, handler, row, rh, flags=DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES):
        q = CScopeUQueue.Lock()
        index = self.GetCallIndex();
        with self._csCache:
            self._mapRowset[index] = Pair(rh, row)
            self._mapHandler[index] = handler
        q.SaveString(defaultDb).SaveUInt(flags).SaveULong(index)
        ok = self.SendRequest(DB_CONSTS.idGetCachedTables, q, None)
        CScopeUQueue.Unlock(q)
        if not ok:
            with self._csCache:
                self._mapHandler.pop(index)
                self._mapRowset.pop(index)
        return ok

    def OnResultReturned(self, reqId, mc):
        if reqId == DB_CONSTS.idRowsetHeader:
            self._Blob.SetSize(0)
            if self._Blob.MaxBufferSize > CCachedBaseHandler.ONE_MEGA_BYTES:
                self._Blob.Realloc(CCachedBaseHandler.ONE_MEGA_BYTES)
            self._vData = []
            vColInfo = CDBColumnInfoArray()
            vColInfo.LoadFrom(mc)
            header = None
            with self._csCache:
                self._indexRowset = mc.LoadULong()
                if len(vColInfo.list) > 0:
                    if self._indexRowset in self._mapRowset:
                        header = self._mapRowset.get(self._indexRowset).first
            if not header is None:
                header(vColInfo)
        elif reqId == DB_CONSTS.idBeginRows:
            self._Blob.SetSize(0)
            self._vData = []
            if mc.GetSize() > 0:
                with self._csCache:
                    self._indexRowset = mc.LoadULong()
        elif reqId == DB_CONSTS.idTransferring:
            while mc.GetSize() > 0:
                vt = mc.LoadObject()
                self._vData.append(vt)
        elif reqId == DB_CONSTS.idEndRows:
            if mc.GetSize() > 0 or len(self._vData) > 0:
                while mc.GetSize() > 0:
                    vt = mc.LoadObject()
                    self._vData.append(vt)
                row = None
                with self._csCache:
                    if self._indexRowset in self._mapRowset:
                        row = self._mapRowset.get(self._indexRowset).second
                if not row is None:
                    row(self._vData)
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
                if content_len >= CCachedBaseHandler.BLOB_LENGTH_NOT_AVAILABLE:
                    content_len = self._Blob.GetSize() - 6 #sizeof(VARTYPE) + sizeof(unsigned int)
                    self._Blob.ResetUInt(content_len, 2)
                vt = self._Blob.LoadObject()
                self._vData.append(vt)
        elif reqId == DB_CONSTS.idGetCachedTables:
            res = mc.LoadInt()
            self._ms = mc.LoadInt()
            err_msg = mc.LoadString()
            r = None
            with self._csCache:
                if self._indexRowset in self._mapHandler:
                    r = self._mapHandler.get(self._indexRowset)
                    self._mapHandler.pop(self._indexRowset)
                if self._indexRowset in self._mapRowset:
                    self._mapRowset.pop(self._indexRowset)
            if r:
                r(res, err_msg)
        else:
            super(CCachedBaseHandler, self).OnResultReturned(reqId, mc)
