import threading
from spa import tagBaseRequestID, CServerError as Se
from spa.clientside import CSocketError
from spa.memqueue import CUQueue, CScopeUQueue
from spa.clientside.ccoreloader import CCoreLoader as ccl
from ctypes import c_ubyte
from collections import deque
from concurrent.futures import Future as future

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
    SESSION_CLOSED_AFTER = -1000
    SESSION_CLOSED_BEFORE = -1001
    REQUEST_CANCELED = -1002
    SESSION_CLOSED_BEFORE_ERR_MSG = 'Session already closed before sending the request'
    SESSION_CLOSED_AFTER_ERR_MSG = 'Session closed after sending the request'
    REQUEST_CANCELED_ERR_MSG = 'Request canceled'

    def get_aborted(fut, reqId):
        if reqId <= 0:
            raise Exception('Request id must be larger than 0')
        fut.reqId = reqId
        def cb_aborted(ah, canceled):
            if fut.done(): return
            try:
                if canceled:
                    fut.set_exception(CSocketError(CAsyncServiceHandler.REQUEST_CANCELED,
                                                   CAsyncServiceHandler.REQUEST_CANCELED_ERR_MSG, reqId, False))
                else:
                    cs = ah.Socket
                    ec = cs.ErrCode
                    if ec:
                        fut.set_exception(CSocketError(ec, cs.ErrMsg, reqId, False))
                    else:
                        fut.set_exception(CSocketError(CAsyncServiceHandler.SESSION_CLOSED_AFTER,
                                                  CAsyncServiceHandler.SESSION_CLOSED_AFTER_ERR_MSG, reqId, False))
            except Exception as ex:
                pass
        return cb_aborted

    def get_se(fut):
        def server_ex(ah, se):  # an exception from remote server
            if fut.done(): return
            try:
                fut.set_exception(se)
            except Exception as ex:
                pass
        return server_ex

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

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.CleanCallbacks()

    def __del__(self):
        self.CleanCallbacks()

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
        """
        Send a result to the other routee
        :param q: An instance of CScopeUQueue or CUQueue or None
        :param reqId: An unique request id within a service handler
        :return: True if socket is connected, and False if socket is closed
        """
        delay = q
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        elif q is None:
            delay = CScopeUQueue()
            q = delay.UQueue
        elif not isinstance(q, CUQueue):
            raise ValueError('Bad input for parameter q')
        if reqId == 0:
            reqId = self._m_ClientSocket_.CurrentRequestID
        h = self._m_ClientSocket_.Handle
        bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        return ccl.SendRouteeResult(h, reqId, bytes, q.GetSize())

    def Interrupt(self, options):
        h = self._m_ClientSocket_.Handle
        return ccl.SendInterruptRequest(h, options)

    def throw(self, f):
        reqId = f
        if isinstance(f, future):
            reqId = f.reqId
        ec = self.Socket.ErrCode
        if ec:
            raise CSocketError(ec, self.Socket.ErrMsg, reqId, True)
        else:
            raise CSocketError(CAsyncServiceHandler.SESSION_CLOSED_BEFORE,
                          CAsyncServiceHandler.SESSION_CLOSED_BEFORE_ERR_MSG, reqId, True)

    def sendRequest(self, reqId, q):
        """
        Send a request onto a remote server for processing, and return a future immediately without blocking
        :param reqId: An unique request id within a service handler
        :param q: An instance of CScopeUQueue or CUQueue or None
        :return: A future for an instance of CScopeUQueue containing an expected result
        """
        f = future()
        def arh(ar):  # ar: an instance of CAsyncResult
            if f.done(): return
            sb = CScopeUQueue()
            sb.UQueue.Swap(ar.UQueue)
            f.set_result(sb)
        if not self.SendRequest(reqId, q, arh, CAsyncServiceHandler.get_aborted(f, reqId), CAsyncServiceHandler.get_se(f)):
            self.throw(f)
        return f

    def SendRequest(self, reqId, q, arh, discarded=None, efs=None):
        """
        Send a request onto a remote server for processing, and return immediately without blocking
        :param reqId: An unique request id within a service handler
        :param q: An instance of CScopeUQueue or CUQueue or None
        :param arh: A callback for tracking an instance of CAsyncResult containing an expected result
        :param discarded: A callback for tracking communication channel events, close and cancel
        :param efs: A callback for tracking an exception from server
        :return: True if communication channel is sendable, and False if communication channel is not sendable
        """
        if reqId <= tagBaseRequestID.idReservedTwo:
            raise ValueError('Request id must be larger than 0x2001')
        delay = q
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        elif q is None:
            delay = CScopeUQueue()
            q = delay.UQueue
        elif not isinstance(q, CUQueue):
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

    @property
    def Socket(self):
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

    def OnInterrupted(self, options):
        pass

    def OnPostProcessing(self, hint, data):
        pass

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
        kvBatching = 0
        kvCallback = 0
        with self._lock_:
            kvBatching = self._m_kvBatching_
            kvCallback = self._m_kvCallback_
            self._m_kvCallback_ = deque()
            self._m_kvBatching_ = deque()
        size = len(kvBatching) + len(kvCallback)
        for kv in kvBatching:
            if kv[1].Discarded:
                kv[1].Discarded(self, self.Socket.CurrentRequestID == tagBaseRequestID.idCancel)
        for kv in kvCallback:
            if kv[1].Discarded:
                kv[1].Discarded(self, self.Socket.CurrentRequestID == tagBaseRequestID.idCancel)
        return size

    def WaitAll(self, timeout = 0xffffffff):
        h = self._m_ClientSocket_.Handle
        if ccl.IsBatching(h):
            raise ValueError("Can't call the method WaitAll while batching requests")
        if ccl.IsQueueStarted(h) and ccl.GetJobSize(h) > 0:
            raise ValueError("Can't call the method WaitAll while enqueuing transactional requests")
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

    def OnExceptionFromServer(self, se):
        pass

    def _OnSE_(self, reqId, errMessage, errWhere, errCode):
        se = Se(errCode, errMessage, errWhere, reqId)
        rcb = self._GetAsyncResultHandler_(reqId)
        if rcb and rcb.ExceptionFromServer:
            rcb.ExceptionFromServer(self, se)
        self.OnExceptionFromServer(se)
        if self.ServerException:
            self.ServerException(self, se)

    def OnResultReturned(self, reqId, q):
        pass

    def OnAllProcessed(self):
        pass

    def OnBaseRequestProcessed(self, reqId):
        pass

    def _OnRR_(self, reqId, mc):
        if tagBaseRequestID.idInterrupt == reqId:
            options = mc.LoadULong()
            self.OnInterrupted(options)
            return
        rcb = self._GetAsyncResultHandler_(reqId)
        if rcb is None or rcb.AsyncResultHandler is None:
            if not (self.ResultReturned and self.ResultReturned(reqId, mc)):
                self.OnResultReturned(reqId, mc)
        else:
            ar = CAsyncResult(self, reqId, mc, rcb)
            rcb.AsyncResultHandler(ar)
