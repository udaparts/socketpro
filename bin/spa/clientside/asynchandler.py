
import threading
from spa import tagBaseRequestID
from spa.memqueue import CUQueue, CScopeUQueue
from spa.clientside.ccoreloader import CCoreLoader as ccl
from ctypes import c_ubyte
from collections import deque


class CAsyncResult(object):
    def __init__(self, ash, reqId, q, arh):
        self.AsyncServiceHandler = ash
        self.RequestId = reqId
        self.UQueue = q
        self.CurrentAsyncResultHandler = arh

    def LoadUUID(self):
        return self.UQueue.LoadUUID()

    def LoadBytes(self):
        return self.UQueue.LoadBytes()

    def LoadInt(self):
        return self.UQueue.LoadInt()

    def LoadUInt(self):
        return self.UQueue.LoadUInt()

    def LoadShort(self):
        return self.UQueue.LoadShort()

    def LoadUShort(self):
        return self.UQueue.LoadUShort()

    def LoadByte(self):
        return self.UQueue.LoadByte()

    def LoadLong(self):
        return self.UQueue.LoadLong()

    def LoadULong(self):
        return self.UQueue.LoadULong()

    def LoadBoolean(self):
        return self.UQueue.LoadBoolean()

    def LoadFloat(self):
        return self.UQueue.LoadFloat()

    def LoadDouble(self):
        return self.UQueue.LoadDouble()

    def LoadString(self):
        return self.UQueue.LoadString()

    def LoadChar(self):
        return self.UQueue.LoadChar()

    def LoadAString(self):
        return self.UQueue.LoadAString()

    def LoadAChar(self):
        return self.UQueue.LoadAChar()

    def LoadObject(self, func=None):
        return self.UQueue.LoadObject(func)

    def LoadDecimal(self):
        return self.UQueue.LoadDecimal()

    def LoadDate(self):
        return self.UQueue.LoadDate()

    def Load(self, obj):
        return self.UQueue.Load(obj)

    def LoadByClass(self, cls):
        return self.UQueue.LoadByClass(cls)

class CResultCb(object):
    def __init__(self, arh = None, discarded = None, efs = None):
        self.AsyncResultHandler = arh
        self.Discarded = discarded
        self.ExceptionFromServer = efs

class CAsyncServiceHandler(object):
    _csCallIndex_ = threading.Lock()
    _CallIndex_ = 0

    def GetCallIndex():
        with CAsyncServiceHandler._csCallIndex_:
            CAsyncServiceHandler._CallIndex_ += 1
            return CAsyncServiceHandler._CallIndex_

    def __init__(self, serviceId):
        self._m_nServiceId_ = serviceId
        self._lock_ = threading.Lock()
        self._m_ClientSocket_ = None
        self._m_kvCallback_ = deque()
        self._m_kvBatching_ = deque()
        self.ServerException = None
        self.ResultReturned = None
        self._lock_Send_ = threading.Lock()


    def GetCallIndex(self):
        with CAsyncServiceHandler._csCallIndex_:
            CAsyncServiceHandler._CallIndex_ += 1
            return CAsyncServiceHandler._CallIndex_

    def _GetAsyncResultHandler_(self, reqId):
        if self._m_ClientSocket_._m_random_:
            for kv in self._m_kvCallback_:
                if kv[0] == reqId:
                    self._m_kvCallback_.remove(kv)
                    return kv[1]
        else:
            if len(self._m_kvCallback_) > 0 and self._m_kvCallback_[0][0] == reqId:
                return self._m_kvCallback_.popleft()[1]
        return None

    def SendRouteeResult(self, q, reqId=0):
        if q is None:
            q = CUQueue()
        if reqId == 0:
            reqId = self._m_ClientSocket_.CurrentRequestID
        h = self._m_ClientSocket_.Handle
        bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        return ccl.SendRouteeResult(h, reqId, bytes, q.GetSize())

    def SendRequest(self, reqId, q, arh, discarded=None, efs=None):
        if q is None:
            q = CUQueue(bytearray(0))
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        if not isinstance(q, CUQueue):
            raise ValueError('Bad input for parameter q')
        #http://stackoverflow.com/questions/21483482/efficient-way-to-convert-string-to-ctypes-c-ubyte-array-in-python
        bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        h = self._m_ClientSocket_.Handle
        if h == 0:
            return False
        kv = None
        batching = False
        if arh or discarded or efs:
            kv = (reqId, CResultCb(arh, discarded, efs))
            batching = ccl.IsBatching(h)
            with self._lock_Send_:
                with self._lock_:
                    if batching:
                        self._m_kvBatching_.append(kv)
                    else:
                        self._m_kvCallback_.append(kv)
                if ccl.SendRequest(h, reqId, bytes, q.GetSize()):
                    return True
        else:
            if ccl.SendRequest(h, reqId, bytes, q.GetSize()):
                return True
        if kv:
            with self._lock_:
                if batching:
                    self._m_kvBatching_.pop()
                else:
                    self._m_kvCallback_.pop()
        return False

    @property
    def RequestsQueued(self):
        with self._lock_:
           return len(self._m_kvCallback_)

    @property
    def Batching(self):
        h = self._m_ClientSocket_.Handle
        return ccl.IsBatching(h)

    @property
    def RouteeRequest(self):
        h = self._m_ClientSocket_.Handle
        return ccl.IsRouteeRequest(h)

    @property
    def DequeuedResult(self):
        h = self._m_ClientSocket_.Handle
        return ccl.DequeuedResult(h)

    @property
    def DequeuedMessageAborted(self):
        h = self._m_ClientSocket_.Handle
        return ccl.IsDequeuedMessageAborted(h)

    @property
    def SvsID(self):
        return self._m_nServiceId_

    @property
    def AttachedClientSocket(self):
        return self._m_ClientSocket_

    def StartBatching(self):
        with self._lock_:
            self._m_kvBatching_.clear()
        h = self._m_ClientSocket_.Handle
        return ccl.StartBatching(h)

    def AbortBatching(self):
        with self._lock_:
            for kv in self._m_kvBatching_:
                if kv[1].Discarded:
                    kv[1].Discarded(self, True)
            self._m_kvBatching_.clear()
        h = self._m_ClientSocket_.Handle
        return ccl.AbortBatching(h)

    def AbortDequeuedMessage(self):
        h = self._m_ClientSocket_.Handle
        ccl.AbortDequeuedMessage(h)

    def OnMergeTo(self, to):
        pass

    def _AppendTo_(self, to):
        with to._lock_:
            with self._lock_:
                self.OnMergeTo(to)
                to._m_kvCallback_ += self._m_kvCallback_
                self._m_kvCallback_.clear()

    def CommitBatching(self, bBatchingAtServerSide=False):
        with self._lock_:
            self._m_kvCallback_ += self._m_kvBatching_
            self._m_kvBatching_.clear()
        h = self._m_ClientSocket_.Handle
        return ccl.CommitBatching(h, bBatchingAtServerSide)

    def CleanCallbacks(self):
        with self._lock_:
            size = len(self._m_kvBatching_) + len(self._m_kvCallback_)
            for kv in self._m_kvBatching_:
                if kv[1].Discarded:
                    kv[1].Discarded(self, self.AttachedClientSocket.CurrentRequestID == tagBaseRequestID.idCancel)
            self._m_kvBatching_.clear()
            for kv in self._m_kvCallback_:
                if kv[1].Discarded:
                    kv[1].Discarded(self, self.AttachedClientSocket.CurrentRequestID == tagBaseRequestID.idCancel)
            self._m_kvCallback_.clear()
            return size

    def WaitAll(self, timeout = 0xffffffff):
        h = self._m_ClientSocket_.Handle
        if ccl.IsBatching(h):
            raise ValueError("Can't call the method WaitAll during batching requests")
        if ccl.IsQueueStarted(h) and ccl.GetJobSize(h) > 0:
            raise ValueError("Can't call the method WaitAll during enqueuing transactional requests")
        return ccl.WaitAll(h, timeout)

    def _Attach_(self, cs):
        self._Detach_()
        if cs is None:
            return ok
        ok = cs._Attach_(self)
        self._m_ClientSocket_ = cs
        return ok

    def _Detach_(self):
        if self._m_ClientSocket_ is None:
            return
        cs = self._m_ClientSocket_
        self._m_ClientSocket_ = None
        cs._Detach_(self)

    def _SetNull_(self):
        self._m_ClientSocket_ = None

    def OnExceptionFromServer(self, reqId, errMessage, errWhere, errCode):
        pass

    def _OnSE_(self, reqId, errMessage, errWhere, errCode):
        #print 'reqId = ' + str(reqId) + ', errWhere = ' + errWhere + ', errCode = ' + str(errCode)
        #print u'errMessage = ' + errMessage
        rcb = self._GetAsyncResultHandler_(reqId)
        if rcb and rcb.ExceptionFromServer:
            rcb.ExceptionFromServer(self, reqId, errMessage, errWhere, errCode)
        self.OnExceptionFromServer(reqId, errMessage, errWhere, errCode)
        if not self.ServerException is None:
            self.ServerException(self, reqId, errMessage, errWhere, errCode)

    def OnResultReturned(self, reqId, q):
        pass

    def OnAllProcessed(self):
        pass

    def OnBaseRequestProcessed(self, reqId):
        pass

    def _OnRR_(self, reqId, mc):
        rcb = self._GetAsyncResultHandler_(reqId)
        if rcb is None or rcb.AsyncResultHandler is None:
            if not (self.ResultReturned and self.ResultReturned(reqId, mc)):
                self.OnResultReturned(reqId, mc)
        else:
            ar = CAsyncResult(self, reqId, mc, rcb)
            rcb.AsyncResultHandler(ar)
