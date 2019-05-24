
from spa import CUQueue, BaseServiceID, CScopeUQueue
import threading
from spa.clientside.asynchandler import CAsyncServiceHandler
from spa.clientside.ccoreloader import CCoreLoader as ccl
from collections import deque
import io
import os

class CContext(object):
    def __init__(self, upload, flags):
        self.Uploading = upload
        self.FileSize = -1
        self.Flags = flags
        self.Sent = False
        self.LocalFile = ''
        self.FilePath = ''
        self.Download = None
        self.Transferring = None
        self.File = None
        self.ErrCode = 0
        self.ErrMsg = ''
        self.QueueOk = False
        self.InitSize = -1

    def _CloseFile(self):
        if self.File:
            if not self.Uploading and (self.ErrCode or self.ErrMsg):
                if self.InitSize == -1:
                    self.File.close()
                    os.remove(self.LocalFile)
                else:
                    self.File.flush()
                    self.File.seek(self.InitSize)
                    self.File.truncate(self.InitSize)
                    self.File.close()
            else:
                self.File.close()
            self.File = None

    def _OpenLocalRead(self):
        try:
            self.ErrCode = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_READING
            self.File = open(self.LocalFile, 'rb')
            self.ErrCode = 0
            self.File.seek(0, io.SEEK_END)
            self.FileSize = self.File.tell()
            self.File.seek(0, io.SEEK_SET)
        except IOError as e:
            self.ErrMsg = e.strerror
            self.ErrCode = e.errno
            if self.File:
                self.File.close()
                self.File = None

    def _OpenLocalWrite(self):
        existing = os.path.isfile(self.LocalFile);
        self.ErrCode = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
        mode = 'xb'
        if (self.Flags & CStreamingFile.FILE_OPEN_TRUNCACTED) == CStreamingFile.FILE_OPEN_TRUNCACTED:
            mode = 'wb'
        elif (sef.Flags & CStreamingFile.FILE_OPEN_APPENDED) == CStreamingFile.FILE_OPEN_APPENDED:
            mode = 'ab'
        try:
            self.File = open(self.LocalFile, mode)
            if existing:
                self.InitSize = self.File.tell()
            self.ErrCode = 0
        except IOError as e:
            self.ErrMsg = e.strerror
            if self.File:
                self.File.close()
                self.File = None

class CStreamingFile(CAsyncServiceHandler):
    sidFile = BaseServiceID.sidFile # asynchronous file streaming service id
    STREAM_CHUNK_SIZE = 10240

    # request ids
    idDownload = 0x7F70
    idStartDownloading = 0x7F71
    idDownloading = 0x7F72
    idUpload = 0x7F73
    idUploading = 0x7F74
    idUploadCompleted = 0x7F75

    # file open flags
    FILE_OPEN_TRUNCACTED = 1
    FILE_OPEN_APPENDED = 2
    FILE_OPEN_SHARE_READ = 4
    FILE_OPEN_SHARE_WRITE = 8

    # error code
    CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1
    CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2

    def __init__(self, sid=sidFile):
        super(CStreamingFile, self).__init__(sid)
        self._csFile = threading.Lock()
        self._vContext = deque() # protected by self._csFile

    def CleanCallbacks(self):
        errCode = self.AttachedClientSocket.ErrorCode
        errMsg = self.AttachedClientSocket.ErrorMessage
        with self._csFile:
            for c in self._vContext:
                if c.File:
                    c.ErrCode = -1
                    c.ErrMsg = "Socket closed"
                c._CloseFile()
            self._vContext = deque()
        return super(CStreamingFile, self).CleanCallbacks()

    @property
    def FileSize(self):
        with self._csFile:
            if len(self._vContext) == 0:
                return -1
            return self._vContext[0].FileSize

    @property
    def FilesQueued(self):
        with self._csFile:
            return len(self._vContext)

    @property
    def LocalFile(self):
        with self._csFile:
            if len(self._vContext) == 0:
                return None
            return self._vContext[0].LocalFile

    @property
    def RemoteFile(self):
        with self._csFile:
            if len(self._vContext) == 0:
                return None
            return self._vContext[0].FilePath

    def Cancel(self):
        canceled = 0
        with self._csFile:
            count = len(self._vContext)
            while count > 0:
                if self._vContext[count-1].File:
                    break
                self._vContext.popleft()
                canceled += 1
        return canceled

    def OnPostProcessing(self, hint, data):
        cs = self.AttachedClientSocket
        ctx = CContext(False, 0)
        with self._csFile:
            count = len(self._vContext)
            if count > 0:
                front = self._vContext[0]
                if front.Uploading:
                    front._OpenLocalRead()
                else:
                    front._OpenLocalWrite()
                if front.ErrCode or front.ErrMsg:
                    ctx = front
                elif front.Uploading:
                    with CScopeUQueue() as q:
                        q.SaveString(front.FilePath).SaveUInt(front.Flags).SaveULong(front.FileSize)
                        if not self.SendRequest(CStreamingFile.idUpload, q, None, front.Discarded, None):
                            front.ErrCode = cs.ErrorCode
                            front.ErrMsg = cs.ErrorMessage
                            ctx = front
                else:
                    with CScopeUQueue() as q:
                        q.SaveString(front.FilePath).SaveUInt(front.Flags)
                        if not self.SendRequest(CStreamingFile.idDownload, q, None, front.Discarded, None):
                            front.ErrCode = cs.ErrorCode
                            front.ErrMsg = cs.ErrorMessage
                            ctx = front
        if ctx.ErrCode or ctx.ErrMsg:
            ctx._CloseFile()
            if ctx.Download:
                ctx.Download(self, ctx.ErrCode, ctx.ErrMsg)
            with self._csFile:
                self._vContext.popleft()
            if len(self._vContext) > 0:
                #post processing the next one
                ccl.PostProcessing(self.AttachedClientSocket.Handle, 0, 0)
                self.AttachedClientSocket.DoEcho #make sure WaitAll works correctly

    def OnMergeTo(self, to):
        with to._csFile:
            with self._csFile:
                to._vContext.extend(self._vContext)
                self._vContext = deque()

    def Upload(self, localFile, remoteFile, up=None, trans=None, discarded=None, flags=FILE_OPEN_TRUNCACTED):
        if not localFile:
            return False
        if not remoteFile:
            return False
        context = CContext(True, flags)
        context.Download = up
        context.Transferring = trans
        context.Discarded = discarded
        context.FilePath = remoteFile
        context.LocalFile = localFile
        with self._csFile:
            self._vContext.append(context)
            if len(self._vContext) == 1:
                ccl.PostProcessing(self.AttachedClientSocket.Handle, 0, 0)
                self.AttachedClientSocket.DoEcho #make sure WaitAll works correctly
        return True

    def Download(self, localFile, remoteFile, dl=None, trans=None, discarded=None, flags=FILE_OPEN_TRUNCACTED):
        if not localFile:
            return False
        if not remoteFile:
            return False
        context = CContext(False, flags)
        context.Download = dl
        context.Transferring = trans
        context.Discarded = discarded
        context.FilePath = remoteFile
        context.LocalFile = localFile
        with self._csFile:
            self._vContext.append(context)
            if len(self._vContext) == 1:
                ccl.PostProcessing(self.AttachedClientSocket.Handle, 0, 0)
                self.AttachedClientSocket.DoEcho #make sure WaitAll works correctly
        return True

    def OnResultReturned(self, reqId, mc):
        if reqId == CStreamingFile.idDownload:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            dl = None
            with self._csFile:
                if len(self._vContext):
                    context = self._vContext[0]
                    dl = context.Download
            if dl:
                dl(self, res, errMsg)
            with self._csFile:
                if len(self._vContext):
                    self._vContext.popleft()._CloseFile()
            self.OnPostProcessing(0, 0)
        elif reqId == CStreamingFile.idStartDownloading:
            with self._csFile:
                if len(self._vContext):
                    front = self._vContext[0]
                    front.FileSize = mc.LoadULong()
                    initSize = 0
                    if front.InitSize > 0:
                        initSize = front.InitSize
                    if front.File.tell() > initSize:
                        front.File.flush()
                        front.File.seek(initSize)
                        front.File.truncate(initSize)
                else:
                    mc.SetSize(0)
        elif reqId == CStreamingFile.idDownloading:
            downloaded = 0
            trans = None
            with self._csFile:
                if len(self._vContext):
                    context = self._vContext[0]
                    trans = context.Transferring
                    context.File.write(mc.GetBuffer())
                    initSize = 0
                    if context.InitSize > 0:
                        initSize = context.InitSize
                    downloaded = context.File.tell() - initSize
            mc.SetSize(0)
            if trans:
                trans(self, downloaded)
        elif reqId == CStreamingFile.idUpload:
            cs = self.AttachedClientSocket
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            ctx = CContext(False, 0)
            if res or errMsg:
                ctx.ErrCode = res
                ctx.ErrMsg = errMsg
                with self._csFile:
                    if len(self._vContext) > 0:
                        context = self._vContext[0]
                        context.ErrCode = res
                        context.ErrMsg = errMsg
                        ctx = context
            else:
                with self._csFile:
                    if len(self._vContext) > 0:
                        context = self._vContext[0]
                        context.QueueOk = cs.ClientQueue.StartJob()
                        ret = bytearray(context.File.read(CStreamingFile.STREAM_CHUNK_SIZE))
                        while len(ret) == CStreamingFile.STREAM_CHUNK_SIZE:
                            if not self.SendRequest(CStreamingFile.idUploading, CUQueue(ret), None, context.Discarded, None):
                                context.ErrCode = cs.ErrorCode
                                context.ErrMsg = cs.ErrorMessage
                                ctx = context
                                break
                            ret = bytearray(context.File.read(CStreamingFile.STREAM_CHUNK_SIZE))
                            if len(ret) < CStreamingFile.STREAM_CHUNK_SIZE:
                                break
                            if not context.QueueOk:
                                sent_buffer_size = cs.BytesInSendingBuffer
                                if sent_buffer_size >= 40 * CStreamingFile.STREAM_CHUNK_SIZE:
                                    break
                        if ctx.ErrCode or ctx.ErrMsg:
                            pass
                        elif len(ret) > 0:
                            if not self.SendRequest(CStreamingFile.idUploading, CUQueue(ret), None, context.Discarded, None):
                                context.ErrCode = cs.ErrorCode
                                context.ErrMsg = cs.ErrorMessage
                                ctx = context
                        if not (ctx.ErrCode or ctx.ErrMsg) and len(ret) < CStreamingFile.STREAM_CHUNK_SIZE:
                            context.Sent = True
                            if not self.SendRequest(CStreamingFile.idUploadCompleted, None, None, context.Discarded, None):
                                context.ErrCode = cs.ErrorCode
                                context.ErrMsg = cs.ErrorMessage
                                ctx = context
                            if context.QueueOk:
                                cs.ClientQueue.EndJob()
            if ctx.ErrCode or ctx.ErrMsg:
                ctx._CloseFile()
                if ctx.Download:
                    ctx.Download(self, ctx.ErrCode, ctx.ErrMsg)
                with self._csFile:
                    self._vContext.popleft()
                if ctx.QueueOk:
                    cs.ClientQueue.AbortJob()
                self.OnPostProcessing(0, 0)
        elif reqId == CStreamingFile.idUploading:
            cs = self.AttachedClientSocket
            ctx = CContext(False, 0)
            trans = None
            uploaded = mc.LoadLong()
            with self._csFile:
                if len(self._vContext) > 0:
                    context = self._vContext[0]
                    trans = context.Transferring
                    if uploaded < 0:
                        context._CloseFile()
                    elif not context.Sent:
                        ret = bytearray(context.File.read(CStreamingFile.STREAM_CHUNK_SIZE))
                        if len(ret) > 0:
                            if not self.SendRequest(CStreamingFile.idUploading, CUQueue(ret), None, context.Discarded, None):
                                context.ErrCode = cs.ErrorCode
                                context.ErrMsg = cs.ErrorMessage
                                ctx = context
                        if not (ctx.ErrCode or ctx.ErrMsg) and len(ret) < CStreamingFile.STREAM_CHUNK_SIZE:
                            context.Sent = True
                            if not self.SendRequest(CStreamingFile.idUploadCompleted, None, None, context.Discarded, None):
                                context.ErrCode = cs.ErrorCode
                                context.ErrMsg = cs.ErrorMessage
                                ctx = context
            if ctx.ErrCode or ctx.ErrMsg:
                ctx._CloseFile()
                if ctx.Download:
                    ctx.Download(self, ctx.ErrCode, ctx.ErrMsg)
                with self._csFile:
                    self._vContext.popleft()
                self.OnPostProcessing(0, 0)
            elif trans:
                trans(self, uploaded)
        elif reqId == CStreamingFile.idUploadCompleted:
            upl = None
            with self._csFile:
                if len(self._vContext):
                    context = self._vContext[0]
                    if context.File:
                        upl = context.Download
                    else:
                        context.Sent = False
                        context.QueueOk = False
            if upl:
                upl(self, 0, '')
            with self._csFile:
                if len(self._vContext):
                    context = self._vContext[0]
                    if context.File:
                        context._CloseFile()
                        self._vContext.popleft()
            self.OnPostProcessing(0, 0)
        else:
            super(CStreamingFile, self).OnResultReturned(reqId, mc)
