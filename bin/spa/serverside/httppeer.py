from spa.serverside.socketpeer import CSocketPeer
from spa import IPush, CScopeUQueue
from ctypes import c_uint, c_ubyte
from spa.serverside.scoreloader import SCoreLoader as scl, CHttpHV
from spa.serverside import tagHttpRequestID


class CHttpPeerBase(CSocketPeer):

    class CPushImpl(IPush):
        def __init__(self, p):
            self._m_p = p

        def __del__(self):
            self._m_p = None

        def Publish(self, message, groups, hint=''):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            sb = CScopeUQueue()
            q = sb.UQueue.SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return scl.Speak(self._m_p.Handle, bytes, q.GetSize(), arr, size)

        def SendUserMessage(self, message, userId, hint=''):
            if userId is None:
                userId = u''
            sb = CScopeUQueue()
            q = sb.UQueue.SaveObject(message, hint)
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

    def __init__(self):
        super(CHttpPeerBase,self).__init__()
        self._m_push = CHttpPeerBase.CPushImpl(self)
        self._m_bHttpOk = False
        self._m_WebRequestName = None
        self._m_vArg = []
        self._m_ReqHeaders = None

    def __del__(self):
        self._m_push = None

    def _Clear(self):
        if self._m_ReqHeaders is None:
            return
        if not scl.IsWebSocket(self.Handle):
            self._m_ReqHeaders = None

    def DownloadFile(self, filePath):
        self._Clear()
        return scl.DownloadFile(self.Handle, filePath.encode('latin-1'))

    """
    <summary>
    Send the last chunk of data and indicate the HTTP response is ended
    </summary>
    <param name="buffer">The last chunk of data</param>
    <returns>The data size in byte</returns>
    <remarks>You must call the method StartChunkResponse before calling this method</remarks>
    """
    def EndChunkResponse(self, buffer):
        if buffer is None:
            buffer = bytearray(0)
        len = len(buffer)
        self._Clear()
        p = (c_ubyte * len).from_buffer(buffer)
        return scl.EndHTTPChunkResponse(self.Handle, p, len)

    """
    <summary>
    Send a chunk of data after calling the method StartChunkResponse
    </summary>
    <param name="buffer">A buffer data</param>
    <returns>The data size in byte</returns>
    <remarks>You must call the method StartChunkResponse before calling this method</remarks>
    """
    def SendChunk(self, buffer):
        if buffer is None:
            return 0
        len = len(buffer)
        if len ==0:
            return 0
        p = (c_ubyte * len).from_buffer(buffer)
        return scl.SendHTTPChunk(self.Handle, p, len)

    def SendResult(self, s):
        size = 0
        self._Clear()
        if s is None:
            s = ''
        s = s.encode('utf-8')
        size = len(s)
        return scl.SendHTTPReturnDataA(self.Handle, s, size)

    def SetResponseCode(self, httpCode):
        return scl.SetHTTPResponseCode(self.Handle, httpCode)

    def SetResponseHeader(self, uft8Header, utf8Value):
        return scl.SetHTTPResponseHeader(self.Handle, uft8Header, utf8Value)

    """
    <summary>
    Begin to send HTTP result in chunk
    </summary>
    <returns>The data size in bytes</returns>
    <remarks>The method EndChunkResponse should be called at the end after this method is called</remarks>
    """
    def StartChunkResponse(self):
        return scl.StartHTTPChunkResponse(self.Handle)

    def OnGet(self):
        self.SetResponseCode(501)
        self.SendResult('No support to GET')

    def OnPost(self):
        self.SetResponseCode(501)
        self.SendResult('No support to POST')

    def OnUserRequest(self):
        self.SetResponseCode(501)
        self.SendResult('No support to ' + self.RequestName)

    def OnDelete(self):
        self.SetResponseCode(501)
        self.SendResult('No support to DELETE')

    def OnHead(self):
        self.SetResponseCode(501)
        self.SendResult('No support to HEAD')

    def OnMultiPart(self):
        self.SetResponseCode(501)
        self.SendResult('No support to MultiPart')

    def OnOptions(self):
        self.SetResponseCode(501)
        self.SendResult('No support to OPTIONS')

    def OnPut(self):
        self.SetResponseCode(501)
        self.SendResult('No support to PUT')

    def OnTrace(self):
        self.SetResponseCode(501)
        self.SendResult('No support to TRACE')

    def OnConnect(self):
        self.SetResponseCode(501)
        self.SendResult('No support to CONNECT')

    def _OnHttpRequestArrive(self, hId, len):
        if hId == tagHttpRequestID.idGet:
            self.OnGet()
        elif hId == tagHttpRequestID.idPost:
            self.OnPost()
        elif hId == tagHttpRequestID.idUserRequest:
            self.OnUserRequest()
        elif hId == tagHttpRequestID.idHead:
            self.OnHead()
        elif hId == tagHttpRequestID.idDelete:
            self.OnDelete()
        elif hId == tagHttpRequestID.idPut:
            self.OnPut()
        elif hId == tagHttpRequestID.idMultiPart:
            self.OnMultiPart()
        elif hId == tagHttpRequestID.idOptions:
            self.OnOptions()
        elif hId == tagHttpRequestID.idTrace:
            self.OnTrace()
        elif hId == tagHttpRequestID.idConnect:
            self.OnConnect()
        else:
            self.SetResponseCode(501)
            self.SendResult('Method not implemented')
        return 0

    @property
    def Push(self):
        return self._m_push

    @property
    def Args(self):
        return self._m_vArg

    @property
    def RequestName(self):
        return self._m_WebRequestName

    @property
    def Authenticated(self):
        return self._m_bHttpOk

    @property
    def Path(self):
        return scl.GetHTTPPath(self.Handle).decode('utf-8')

    @property
    def ContentLength(self):
        return scl.GetHTTPContentLength(self.Handle)

    @property
    def Query(self):
        return scl.GetHTTPQuery(self.Handle).decode('utf-8')

    @property
    def HttpMethod(self):
        return scl.GetHTTPMethod(self.Handle)

    @property
    def IsWebSocket(self):
        return scl.IsWebSocket(self.Handle)

    @property
    def IsCrossDomain(self):
        return scl.IsCrossDomain(self.Handle)

    @property
    def Version(self):
        return scl.GetHTTPVersion(self.Handle)

    @property
    def Host(self):
        return scl.GetHTTPHost(self.Handle).decode('utf-8')

    @property
    def Transport(self):
        return scl.GetHTTPTransport(self.Handle)

    @property
    def TransferEncoding(self):
        return scl.GetHTTPTransferEncoding(self.Handle)

    @property
    def ContentMultiplax(self):
        return scl.GetHTTPContentMultiplax(self.Handle)

    @property
    def Id(self):
        return scl.GetHTTPId(self.Handle).decode('utf-8')

    @property
    def CurrentMultiplaxHeaders(self):
        mapHeaders = {}
        hv = (CHttpHV * 64)()
        res = scl.GetHTTPCurrentMultiplaxHeaders(self.Handle, hv, 64)
        n = 0
        while n < res:
            mapHeaders[hv.Header] = hv.Value.decode('utf-8')
            n += 1
        return mapHeaders

    @property
    def RequestHeaders(self):
        if not self._m_ReqHeaders is None:
            return self._m_ReqHeaders
        self._m_ReqHeaders = {}
        hv = (CHttpHV * 64)()
        res = scl.GetHTTPRequestHeaders(self.Handle, hv, 64)
        n = 0
        while n < res:
            self._m_ReqHeaders[hv.Header] = hv.Value.decode('utf-8')
            n += 1
        return self._m_ReqHeaders

    def DoAuthentication(self, userId, password):
        return True