
from spa.clientside.asyncdbhandler import CAsyncServiceHandler
from spa import BaseServiceID, tagBaseRequestID, CUQueue, tagOptimistic, CScopeUQueue
import threading


class CAsyncQueue(CAsyncServiceHandler):
    """
    A client side class for easy accessing remote persistent message queues by use of SocketPro communication framework
    """
    sidQueue = BaseServiceID.sidChat

    # queue-related request ids
    idEnqueue = tagBaseRequestID.idReservedTwo + 1
    idDequeue = tagBaseRequestID.idReservedTwo + 2
    idStartTrans = tagBaseRequestID.idReservedTwo + 3
    idEndTrans = tagBaseRequestID.idReservedTwo + 4
    idFlush = tagBaseRequestID.idReservedTwo + 5
    idClose = tagBaseRequestID.idReservedTwo + 6
    idGetKeys = tagBaseRequestID.idReservedTwo + 7

    # this id is designed for notifying dequeue batch size from server to client
    idBatchSizeNotified = tagBaseRequestID.idReservedTwo + 20

    # error code
    QUEUE_OK = 0
    QUEUE_TRANS_ALREADY_STARTED = 1
    QUEUE_TRANS_STARTING_FAILED = 2
    QUEUE_TRANS_NOT_STARTED_YET = 3
    QUEUE_TRANS_COMMITTING_FAILED = 4
    QUEUE_DEQUEUING = 5
    QUEUE_OTHER_WORKING_WITH_SAME_QUEUE = 6
    QUEUE_CLOSE_FAILED = 7
    QUEUE_ENQUEUING_FAILED = 8

    # callback definitions
    """
    public delegate void DQueueTrans(int errCode);
    public delegate void DGetKeys(string[] vKey);
    public delegate void DFlush(ulong messageCount, ulong fileSize);
    public delegate void DEnqueue(ulong indexMessage);
    public delegate void DClose(int errCode);
    public delegate void DDequeue(ulong messageCount, ulong fileSize, uint messagesDequeuedInBatch, uint bytesDequeuedInBatch);
    public delegate void DMessageQueued();
    """

    def __init__(self, sid = sidQueue):
        super(CAsyncQueue, self).__init__(sid)
        self.MessageQueued = None
        self._BatchSize_ = 0
        self._csQ_ = threading.Lock()
        self._keyQueue_ = '' #protected by _csQ_
        self._dDequeue_ = None #protected by _csQ_

    @property
    def DequeueBatchSize(self):
        """
        :return: dequeue batch size in bytes
        """
        return (self._BatchSize_ & 0xffffff)

    @property
    def EnqueueNotified(self):
        """
        Check if remote queue server is able to automatically notify a client when a message is enqueued at server side
        :return: true if remote queue server will automatically notify a client, and false if remote queue server will not
        """
        return ((self._BatchSize_ >> 24) == 0)

    @property
    def LastDequeueCallback(self):
        """
        :return: last dequeue callback
        """
        with self._csQ_:
            return self._dDequeue_

    def Enqueue(self, key, idMessage, q, e=None, discarded=None):
        """
        Enqueue a message into a queue file identified by a key
        :param key: An ASCII string for identifying a queue at server side
        :param idMessage: A unsigned short number to identify a message
        :param q: an instance of SPA.CUQueue containing a message
        :param e: A callback for tracking returning index
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not key:
            key = ''
        buffer = CScopeUQueue.Lock().SaveAString(key).SaveUShort(idMessage)
        if q:
            buffer.Push(q.IntenalBuffer, q.GetSize())
        ok = None
        if not e:
            ok = self.SendRequest(CAsyncQueue.idEnqueue, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idEnqueue, buffer, lambda ar: e(ar.AsyncServiceHandler, ar.LoadULong()), discarded)
        CScopeUQueue.Unlock(buffer)
        return ok

    def StartQueueTrans(self, key, qt=None, discarded=None):
        """
        Start enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
        :param key: An ASCII string for identifying a queue at server side
        :param qt: A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not key:
            key = ''
        buffer = CScopeUQueue.Lock().SaveAString(key)
        ok = None
        cq = self.AttachedClientSocket.ClientQueue
        if cq.Available:
            cq.StartJob()
        if not qt:
            ok = self.SendRequest(CAsyncQueue.idStartTrans, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idStartTrans, buffer, lambda ar: qt(ar.AsyncServiceHandler, ar.LoadInt()), discarded)
        CScopeUQueue.Unlock(buffer)
        return ok

    def EndQueueTrans(self, rollback=False, qt=None, discarded=None):
        """
        End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
        :param rollback: true for rollback, and false for committing
        :param qt: A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        buffer = CScopeUQueue.Lock().SaveBool(rollback)
        ok = None
        if not qt:
            ok = self.SendRequest(CAsyncQueue.idEndTrans, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idEndTrans, buffer, lambda ar: qt(ar.AsyncServiceHandler, ar.LoadInt()), discarded)
        cq = self.AttachedClientSocket.ClientQueue
        if cq.Available:
            if rollback:
                cq.AbortJob()
            else:
                cq.EndJob()
        CScopeUQueue.Unlock(buffer)
        return ok

    def GetKeys(self, gk, discarded=None):
        """
        Query queue keys opened at server side
        :param gk: A callback for tracking a list of key names
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not gk:
            return self.SendRequest(CAsyncQueue.idGetKeys, None, None, discarded)
        def cb(ar):
            v = []
            size = ar.LoadUInt()
            while size > 0:
                v.append(ar.LoadAString())
                size -= 1
            gk(ar.AsyncServiceHandler, v)
        return self.SendRequest(CAsyncQueue.idGetKeys, None, cb, discarded)

    def CloseQueue(self, key, c=None, discarded=None, permanent=False):
        """
        Try to close or delete a persistent queue opened at server side
        :param key: An ASCII string for identifying a queue at server side
        :param c: A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
        :param permanent: true for deleting a queue file, and false for closing a queue file
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not key:
            key = ''
        buffer = CScopeUQueue.Lock().SaveAString(key).SaveBool(permanent)
        ok = None
        if not c:
            ok = self.SendRequest(CAsyncQueue.idClose, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idClose, buffer, lambda ar: c(ar.AsyncServiceHandler, ar.LoadInt()), discarded)
        CScopeUQueue.Unlock(buffer)
        return ok

    def FlushQueue(self, key, f, option=tagOptimistic.oMemoryCached, discarded=None):
        """
        May flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
        :param key: An ASCII string for identifying a queue at server side
        :param f: A callback for tracking returning message count and queue file size in bytes
        :param option: one of tagOptimistic options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not key:
            key = ''
        buffer = CScopeUQueue.Lock().SaveAString(key).SaveInt(option)
        ok = None
        if not f:
            ok = self.SendRequest(CAsyncQueue.idFlush, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idFlush, buffer, lambda ar: f(ar.AsyncServiceHandler, ar.LoadULong(), ar.LoadULong()), discarded)
        CScopeUQueue.Unlock(buffer)
        return ok

    def Dequeue(self, key, d, timeout=0, discarded=None):
        """
        Dequeue messages from a persistent message queue file at server side in batch
        :param key: An ASCII string for identifying a queue at server side
        :param d: A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch
        :param timeout: A time-out number in milliseconds
        :param discarded A callback for tracking socket close or request cancel event
        :return: true for sending the request successfully, and false for failure
        """
        if not key:
            key = ''
        with self._csQ_:
            self._dDequeue_ = d
            self._keyQueue_ = key
        buffer = CScopeUQueue.Lock().SaveAString(key).SaveUInt(timeout)
        ok = None
        if not d:
            ok = self.SendRequest(CAsyncQueue.idDequeue, buffer, None, discarded)
        else:
            ok = self.SendRequest(CAsyncQueue.idDequeue, buffer, lambda ar: d(ar.AsyncServiceHandler, ar.LoadULong(), ar.LoadULong(), ar.LoadUInt(), ar.LoadUInt()), discarded)
        CScopeUQueue.Unlock(buffer)
        return ok

    def OnBaseRequestProcessed(self, reqId):
        if reqId == tagBaseRequestID.idMessageQueued:
            with self._csQ_:
                d = self._dDequeue_
                key = self._keyQueue_
            if d:
                self.Dequeue(key, d, 0)
            if self.MessageQueued:
                self.MessageQueued(self)

    def OnResultReturned(self, reqId, mc):
        if reqId == CAsyncQueue.idBatchSizeNotified:
            self._BatchSize_ = mc.LoadUInt()
