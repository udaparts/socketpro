
from spa import CUQueue, CStreamSerializationHelper as ssh
import threading
from spa.clientside.asynchandler import CAsyncServiceHandler


class CStreamHelper(ssh):
    def __init__(self, ash):
        if not isinstance(ash, CAsyncServiceHandler):
            raise ValueError('A valid service handler required')
        self._m_ash = ash
        self._m_s = None
        self._m_nDownloadFileSize = -1
        self._m_nPos = 0
        self.Progress = None
        self._lock_ = threading.Lock()

    #<summary>
    #Hosting service handler
    #</summary>
    @property
    def AsyncServiceHandler(self):
        return self._m_ash

    @property
    def DownloadingStreamSize(self):
        return self._m_nDownloadFileSize

    def Reset(self):
        with self._lock_:
            self._m_s = None

    def _DataFromServerToClient(self, reqId, qData):
        processed = False
        if reqId == ssh.idReadDataFromServerToClient:
            if qData.GetSize() > 0:
                with self._lock_:
                    ssh.Write(self._m_s, qData)
                    if not self.Progress is None:
                        self.Progress(self, self._m_s.tell())
                qData.SetSize(0)
            processed = True
        return processed

    def Download(self, receiver, remotePath):
        remotePath = remotePath.strip()
        with self._lock_:
            if not self._m_s is None:
                raise Exception('A stream during transaction')
            if not hasattr(receiver, 'write'):
                raise ValueError('A writable target stream required')
            self._m_s = receiver
        self._m_ash.ResultReturned = self._DataFromServerToClient
        self._res = ''
        def callBack(ar):
            self._m_nDownloadFileSize = ar.LoadULong()
            self._res = ar.LoadString()
        ok = self._m_ash.SendRequest(ssh.idStartDownloading, CUQueue().SaveString(remotePath), callBack) and self._m_ash.WaitAll()
        with self._lock_:
            if not self._res is None and len(self._res) > 0:
                self._m_s = None
                return self._res
            elif self._res is None:
                self._res = ''
            if not ok and not self._m_ash.AttachedClientSocket.Sendable:
                self._m_s = None
                return self._m_ash.AttachedClientSocket.ErrorMsg
            if not self.Progress is None:
                self.Progress(self, self._m_s.tell())
            def dc(ar):
                with self._lock_:
                    if not self.Progress is None:
                        self.Progress(self, self._m_s.tell())
                    self._m_s = None
                self._m_ash.ResultReturned = None
            if not self._m_ash.SendRequest(ssh.idDownloadCompleted, None, dc):
                self._m_s = None
                return self._m_ash.AttachedClientSocket.ErrorMsg
        return self._res

    def _SendDataFromClientToServer(self):
        if self._m_ash.AttachedClientSocket.BytesInSendingBuffer > ssh.STREAM_CHUNK_SIZE:
            return 0
        if self._m_s is None:
            return 0
        send = 0
        bytes = ssh.Read(self._m_s)
        read = len(bytes)
        while read > 0:
            def callBack(ar):
                self._SendDataFromClientToServer()
            ok = self._m_ash.SendRequest(ssh.idWriteDataFromClientToServer, CUQueue(bytearray(bytes)), callBack)
            if not ok:
                self._m_s = None
                return send
            if not self.Progress is None:
                self.Progress(self, self._m_s.tell())
            send += read
            if self._m_ash.AttachedClientSocket.BytesInSendingBuffer > 10 * ssh.STREAM_CHUNK_SIZE:
                break
            bytes = ssh.Read(self._m_s)
            read = len(bytes)
            if read == 0:
                def dc(ar):
                    with self._lock_:
                        if not self.Progress is None:
                            self.Progress(self, self._m_s.tell())
                        self._m_s = None
                if not self._m_ash.SendRequest(ssh.idUploadCompleted, None, dc):
                    self._m_s = None
        return send

    def Upload(self, source, remotePath):
        remotePath = remotePath.strip()
        self._m_ash.ResultReturned = None
        with self._lock_:
            if not self._m_s is None:
                raise Exception('A stream during transaction')
        if not hasattr(source, 'read'):
                raise ValueError('A readable source stream required')
        self._res = ''
        def callBack(ar):
            self._res = ar.LoadString()
        ok = self._m_ash.SendRequest(ssh.idStartUploading, CUQueue().SaveString(remotePath), callBack) and self._m_ash.WaitAll()
        if not self._res is None and len(self._res) > 0:
            self._m_s = None
            return self._res
        elif self._res is None:
            self._res = ''
        if not ok and not self._m_ash.AttachedClientSocket.Sendable:
            return self._m_ash.AttachedClientSocket.ErrorMsg
        with self._lock_:
            if not self._m_s is None:
                raise Exception('A stream during transaction')
            self._m_s = source
            if not self.Progress is None:
                self.Progress(self, self._m_s.tell())
            if self._SendDataFromClientToServer() == 0:
                def dc(ar):
                    with self._lock_:
                        if not self.Progress is None and not self._m_s is None:
                            self.Progress(self, self._m_s.tell())
                        self._m_s = None
                if not self._m_ash.SendRequest(ssh.idUploadCompleted, None, dc):
                    self._m_s = None
                    if not self._m_ash.AttachedClientSocket.Sendable:
                        return self._m_ash.AttachedClientSocket.ErrorMsg
        return self._res
