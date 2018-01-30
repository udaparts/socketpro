
from spa.serverside.socketpeer import CSocketPeer
from spa import CUQueue, IPushEx, CScopeUQueue
from ctypes import c_ubyte, c_bool, byref, c_uint
from spa.serverside.scoreloader import SCoreLoader as scl

class CClientPeer(CSocketPeer):
    class CPushImpl(IPushEx):
        def __init__(self, p):
            self._m_p = p

        def __del__(self):
            self._m_p = None

        def Publish(self, message, groups, hint=''):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return scl.Speak(self._m_p.Handle, bytes, q.GetSize(), arr, size)

        def SendUserMessage(self, message, userId, hint=''):
            if userId is None:
                userId = u''
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return scl.SendUserMessage(self._m_p.Handle, userId, bytes, q.GetSize())

        def Subscribe(self, groups):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            return scl.Enter(self._m_p.Handle, arr, size)

        def Unsubscribe(self):
            scl.Exit(self._m_p.Handle)

        def PublishEx(self, message, groups):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            if message is None:
                message = ()
            arrMessage = (c_ubyte * len(message))(*message)
            return scl.SpeakEx(self._m_p.Handle, arrMessage, len(message), arr, size)

        def SendUserMessageEx(self, userId, message):
            if userId is None:
                userId = u''
            if message is None:
                message = ()
            arrMessage = (c_ubyte * len(message))(*message)
            return scl.SendUserMessageEx(self._m_p.Handle, userId, arrMessage, len(message))

    def __init__(self):
        super(CClientPeer, self).__init__()
        self._m_push = CClientPeer.CPushImpl(self)

    @property
    def Push(self):
        return self._m_push

    def SendResult(self, q, reqId=0):
        if reqId == 0:
            reqId = self.CurrentRequestID
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        if q is None:
            q = CUQueue()
        buffer = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        if self.Random:
            return scl.SendReturnData(self.Handle, self.CurrentRequestIndex, reqId, q.GetSize(), buffer)
        return scl.SendReturnData(self.Handle, reqId, q.GetSize(), buffer)

    def SendResultIndex(self, reqIndex, q, reqId):
        if reqId == 0:
            reqId = self.CurrentRequestID
        if isinstance(q, CScopeUQueue):
            q = q.UQueue
        if q is None:
            q = CUQueue()
        buffer = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        return scl.SendReturnDataIndex(self.Handle, reqIndex, reqId, q.GetSize(), buffer)

    def MakeRequest(self, reqId, q):
        if q is None:
            q = CUQueue()
        buffer = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_, q._m_position_)
        return scl.MakeRequest(self.Handle, reqId, buffer, q.GetSize())

    def Dequeue(self, qHandle, messageCount, bNotifiedWhenAvailable, waitTime=0):
        return scl.Dequeue(qHandle, self.Handle, messageCount, bNotifiedWhenAvailable, waitTime)

    def Dequeue2(self, qHandle, bNotifiedWhenAvailable, maxBytes=8*1024, waitTime=0):
        return scl.Dequeue2(qHandle, self.Handle, maxBytes, bNotifiedWhenAvailable, waitTime)

    def EnableClientDequeue(self, enable):
        scl.EnableClientDequeue(self.Handle, enable)

    def StartBatching(self):
        return scl.StartBatching(self.Handle)

    def CommitBatching(self):
        return scl.CommitBatching(self.Handle)

    def AbortBatching(self):
        return scl.AbortBatching(self.Handle)

    def AbortDequeuedMessage(self):
        scl.AbortDequeuedMessage(self.Handle)

    def GetPeerOs(self):
        b = c_bool()
        os = scl.GetPeerOs(self.Handle, byref(b))
        return os, b.value

    @property
    def ZipLevel(self):
        return scl.GetZipLevel(self.Handle)

    @ZipLevel.setter
    def ZipLevel(self, value):
        scl.SetZipLevel(self.Handle, value)

    @property
    def Zip(self):
        return scl.GetZip(self.Handle)

    @Zip.setter
    def Zip(self, value):
        scl.SetZip(self.Handle, value)

    @property
    def BytesBatched(self):
        return scl.GetBytesBatched(self.Handle)

    @property
    def DequeuedMessageAborted(self):
        return scl.IsDequeuedMessageAborted(self.Handle)

    @property
    def IsDequeueRequest(self):
        return scl.IsDequeueRequest(self.Handle)