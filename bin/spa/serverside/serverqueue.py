
from spa.serverside import IServerQueue
from ctypes import c_uint, c_ubyte
from spa.serverside.scoreloader import SCoreLoader as scl
from spa import tagQueueStatus, CUQueue
import time

class CServerQueue(IServerQueue):
    def __init__(self, qHandle=0):
        self._m_qHandle_ = qHandle

    @property
    def Handle(self):
        return self._m_qHandle_

    @Handle.setter
    def Handle(self, value):
        self._m_qHandle_ = value

    def AppendTo(self, queues):
        if queues is None:
            return True
        queues = list(queues)
        count = len(queues)
        if count == 0:
            return True
        index = 0
        handles = (c_uint * count)()
        for q in queues:
            if isinstance(q, IServerQueue):
                handles[index] = q.Handle
            else:
                handles[index] = q
            index += 1
        return scl.PushQueueTo(self._m_qHandle_, handles, count)

    def EnsureAppending(self, queues):
        if not self.Available:
            return False
        if self.QueueStatus != tagQueueStatus.qsMergePushing:
            return True
        if queues is None:
            return True
        queues = list(queues)
        if len(queues) == 0:
            return True
        handles = []
        for q in queues:
            h = 0
            if isinstance(q, IServerQueue):
                h = q.Handle
            else:
                h = q
            if scl.GetClientQueueStatus(h) != tagQueueStatus.qsMergeComplete:
                handles.append(h)
        if len(handles) > 0:
            return self.AppendTo(handles)
        self.Reset()
        return True

    def AbortJob(self):
        return scl.AbortJob(self._m_qHandle_)

    def StartJob(self):
        return scl.StartJob(self._m_qHandle_)

    def EndJob(self):
        return scl.EndJob(self._m_qHandle_)

    def Reset(self):
        scl.ResetQueue(self._m_qHandle_)
        
    def StopQueue(self, permanent=False):
        scl.StopQueue(self._m_qHandle_, permanent)

    def CancelQueuedMessages(self, startIndex, endIndex):
        return scl.CancelQueuedRequestsByIndex(self._m_qHandle_, startIndex, endIndex)

    def RemoveByTTL(self):
        return scl.RemoveQueuedRequestsByTTL(self._m_qHandle_)

    def Enqueue(self, reqId, q):
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        elif q is None:
            q = CUQueue()
        bytes = (c_ubyte * q.Size).from_buffer(q._m_bytes_, q._m_position_)
        return scl.Enqueue(self._m_qHandle_, reqId, bytes, q.Size)
    
    @property
    def LastMessageTime(self):
        seconds = scl.GetLastQueueMessageTime(self._m_qHandle_)
        temp = time.strptime('1 Jan 2013', '%d %b %Y')
        seconds += time.mktime(temp)
        return float(seconds)
    
    @property
    def MessagesInDequeuing(self):
        return scl.GetMessagesInDequeuing(self._m_qHandle_)

    @property
    def MessageCount(self):
        return scl.GetMessageCount(self._m_qHandle_)

    @property
    def QueueSize(self):
        return scl.GetQueueSize(self._m_qHandle_)

    @property
    def Available(self):
        return scl.IsQueueStartedByHandle(self._m_qHandle_)

    @property
    def Secure(self):
        return scl.IsQueueSecuredByHandle(self._m_qHandle_)

    @property
    def QueueFileName(self):
        return scl.GetQueueFileName(self._m_qHandle_).decode('latin-1')

    @property
    def QueueName(self):
        return scl.GetQueueName(self._m_qHandle_).decode('latin-1')

    @property
    def JobSize(self):
        return scl.GetJobSize(self._m_qHandle_)

    @property
    def DequeueShared(self):
        return scl.IsDequeueShared(self._m_qHandle_)

    @property
    def LastIndex(self):
        scl.GetQueueLastIndex(self._m_qHandle_)

    @property
    def QueueStatus(self):
        return scl.GetServerQueueStatus(self._m_qHandle_)

    @property
    def TTL(self):
        return scl.GetTTL(self._m_qHandle_)

    @property
    def Optimistic(self):
        return scl.GetOptimistic(self._m_qHandle_)

    @Optimistic.setter
    def Optimistic(self, value):
        scl.SetOptimistic(self._m_qHandle_, value)
