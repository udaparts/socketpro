
from spa.serverside.scoreloader import SCoreLoader as scl
from spa import classproperty, BaseServiceID, tagChatRequestID
from ctypes import c_char, c_uint, byref, c_bool
from spa.serverside.spserver import CSocketProServer

class CSocketPeer(object):
    SOCKET_NOT_FOUND = 0xffffffff
    REQUEST_CANCELED = SOCKET_NOT_FOUND - 1
    BAD_OPERATION = SOCKET_NOT_FOUND - 2
    RESULT_SENDING_FAILED = SOCKET_NOT_FOUND - 3

    def __init__(self):
        self._m_qBuffer = None
        self._m_sh = 0
        self._m_Service = None
        self._m_random = False

    @property
    def Random(self):
        return self._m_random

    @property
    def CurrentRequestIndex(self):
        return scl.GetCurrentRequestIndex(self._m_sh)

    @property
    def CurrentRequestID(self):
        return scl.GetCurrentRequestID(self._m_sh)

    @property
    def CurrentRequestLen(self):
        return scl.GetCurrentRequestLen(self._m_sh)

    @property
    def BytesReceived(self):
        return scl.GetBytesReceived(self._m_sh)

    @property
    def BytesSent(self):
        return scl.GetBytesSent(self._m_sh)

    @property
    def CountOfJoinedChatGroups(self):
        return scl.GetCountOfJoinedChatGroups(self._m_sh)

    @property
    def ErrorCode(self):
        return scl.GetServerSocketErrorCode(self._m_sh)

    @property
    def ErrorMessage(self):
        str = (c_char * 4097)()
        res = scl.GetServerSocketErrorMessage(self._m_sh, str, 4097)
        return str.value.decode('latin-1')

    def GetPeerName(self):
        port = c_uint()
        addr = (c_char * 256)()
        res = scl.GetPeerName(self._m_sh, byref(port), addr, 256)
        return addr.value.decode('latin-1'), port.value

    @property
    def BytesInReceivingBuffer(self):
        return scl.GetRcvBytesInQueue(self._m_sh)

    @property
    def BytesInSendingBuffer(self):
        return scl.GetSndBytesInQueue(self._m_sh)

    @classproperty
    def RequestCount(cls):
        return scl.GetRequestCount()

    @property
    def SocketNativeHandle(self):
        return scl.GetSocketNativeHandle(self._m_sh)

    @property
    def BaseService(self):
         return self._m_Service

    @property
    def UQueue(self):
        return self._m_qBuffer

    @property
    def Handle(self):
        return self._m_sh

    @property
    def SSL(self):
        return scl.GetSSL(self._m_sh)

    @property
    def UID(self):
        return CSocketProServer.CredentialManager.GetUserID(self._m_sh)

    @UID.setter
    def UID(self, value):
        scl.SetUserID(self._m_sh, value)

    @property
    def SvsID(self):
        return scl.GetSvsID(self._m_sh)

    @property
    def Batching(self):
        return scl.IsBatching(self._m_sh)

    @property
    def IsCanceled(self):
        return scl.IsCanceled(self._m_sh)

    @property
    def IsFakeRequest(self):
        return scl.IsFakeRequest(self._m_sh)

    @classproperty
    def IsMainThread(cls):
        return scl.IsMainThread()

    @property
    def Connected(self):
        return scl.IsOpened(self._m_sh)

    def DropCurrentSlowRequest(self):
        scl.DropCurrentSlowRequest(self._m_sh)

    def GetOS(self):
        b = c_bool()
        os = scl.GetPeerOs(self._m_sh, byref(b))
        return os, b.value

    def Close(self):
        scl.Close(self._m_sh)

    def PostClose(self, errCode=0):
        scl.PostClose(self._m_sh, errCode)

    def RequestsInQueue(self):
        return scl.QueryRequestsInQueue(self._m_sh)

    def SendExceptionResult(self, errMessage, errWhere, errCode=0, reqId=0, reqIndex=-1):
        if reqIndex == -1:
            if self.Random:
                return scl.SendExceptionResultIndex(self._m_sh, self.CurrentRequestIndex, errMessage, errWhere, reqId, errCode)
            return scl.SendExceptionResult(self._m_sh, errMessage, errWhere, reqId, errCode)
        return scl.SendExceptionResultIndex(self._m_sh, reqIndex, errMessage, errWhere, reqId, errCode)

    def OnSwitchFrom(self, oldServiceId):
        pass

    def OnChatRequestCame(self, reqId):
        pass

    def OnSlowRequestProcessed(self, reqId):
        pass

    def OnReleaseResource(self, bClosing, info):
        pass

    def OnBaseRequestCame(self, chatReqId):
        pass

    def OnRequestArrive(self, reqId, len):
        pass

    def OnFastRequestArrive(self, reqId, len):
        pass

    def OnSlowRequestArrive(self, reqId, len):
        return 0

    def _OnChatComing(self, chatReqId):
        ok = True
        q = self._m_qBuffer
        if chatReqId == tagChatRequestID.idEnter:
            groups = q.LoadObject()
            self.OnSubscribe(groups)
        elif chatReqId == tagChatRequestID.idExit:
            self.OnUnsubscribe(self.ChatGroups)
        elif chatReqId == tagChatRequestID.idSendUserMessage:
            receiver = q.LoadString()
            msg = q.LoadObject()
            self.OnSendUserMessage(receiver, msg)
        elif chatReqId == tagChatRequestID.idSpeak:
            groups = q.LoadObject()
            msg = q.LoadObject()
            self.OnPublish(msg, groups)
        elif chatReqId == tagChatRequestID.idSendUserMessageEx:
            svsId = scl.GetSvsID(self._m_sh)
            if svsId != BaseServiceID.sidHTTP:
                receiver = q.LoadString()
                msg = q.GetBuffer()
                self.OnSendUserMessageEx(receiver, msg)
            else:
                ok = False
        elif chatReqId == tagChatRequestID.idSpeakEx:
            svsId = scl.GetSvsID(self._m_sh)
            if svsId != BaseServiceID.sidHTTP:
                bytes = q.LoadBytes()
                groups = []
                while q.GetSize() >= 4:
                    groups.append(q.LoadUInt())
                self.OnPublishEx(groups, bytes)
            else:
                ok = False
        else:
            ok = False
        if not ok:
            self.SendExceptionResult('Unexpected chat request', 'CSocketPeer._OnChatComing', chatReqId, 0)

    def OnSubscribe(self, groups):
        pass

    def OnUnsubscribe(self, groups):
        pass

    def OnPublish(self, objMessage, groups):
        pass

    def OnSendUserMessage(self, receiver, objMessage):
        pass

    def OnResultsSent(self):
        pass

    def OnSendUserMessageEx(self, receiver, message):
        pass

    def OnPublishEx(self, groups, message):
        pass

    @property
    def ChatGroups(self):
        res = scl.GetCountOfJoinedChatGroups(self._m_sh)
        groups = (c_uint * res)()
        scl.GetJoinedGroupIds(self._m_sh, groups, res)
        return list(groups)
