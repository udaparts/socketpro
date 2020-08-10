from spa import CUQueue, BaseServiceID, CScopeUQueue
import threading
from spa.clientside.asynchandler import CAsyncServiceHandler
from spa.clientside.ccoreloader import CCoreLoader as ccl
from collections import deque
from concurrent.futures import Future as future
import itertools
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
        self.Se = None

    def _HasError(self):
        return self.ErrCode or self.ErrMsg

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
            self.File = open(self.LocalFile, 'rb')
            self.ErrCode = 0
            self.File.seek(0, io.SEEK_END)
            self.FileSize = self.File.tell()
            self.File.seek(0, io.SEEK_SET)
        except IOError as e:
            self.ErrMsg = e.strerror
            self.ErrCode = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_READING
            if self.File:
                self.File.close()
                self.File = None

    def _OpenLocalWrite(self):
        existing = os.path.isfile(self.LocalFile)
        mode = 'wb'
        if (self.Flags & CStreamingFile.FILE_OPEN_APPENDED) == CStreamingFile.FILE_OPEN_APPENDED:
            mode = 'ab'
        try:
            self.File = open(self.LocalFile, mode)
            if mode != 'ab':
                self.File.truncate(0)
            if existing:
                self.InitSize = self.File.tell()
            self.ErrCode = 0
        except IOError as e:
            self.ErrCode = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
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
    idUploadBackup = 0x7F76

    # file open flags
    FILE_OPEN_TRUNCACTED = 1
    FILE_OPEN_APPENDED = 2
    FILE_OPEN_SHARE_READ = 4
    FILE_OPEN_SHARE_WRITE = 8

    # error code
    CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1
    CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2
    FILE_BAD_OPERATION = -3
    FILE_DOWNLOADING_INTERRUPTED = -4

    MAX_FILES_STREAMED = 32

    def __init__(self, sid=sidFile):
        super(CStreamingFile, self).__init__(sid)
        self._csFile = threading.Lock()
        self._vContext = deque() # protected by self._csFile
        self._MaxDownloading = 1

    def CleanCallbacks(self):
        with self._csFile:
            for c in self._vContext:
                if c.File:
                    c.ErrCode = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
                    c.ErrMsg = "Clean local writing file"
                    c._CloseFile()
                else:
                    break
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

    @property
    def FilesStreamed(self):
        with self._csFile:
            return self._MaxDownloading

    @FilesStreamed.setter
    def FilesStreamed(self, value):
        with self._csFile:
            if value <= 0:
                self._MaxDownloading = 1
            elif value > CStreamingFile.MAX_FILES_STREAMED:
                self._MaxDownloading = CStreamingFile.MAX_FILES_STREAMED
            else:
                self._MaxDownloading = value

    def Cancel(self):
        canceled = 0
        with self._csFile:
            count = len(self._vContext)
            while count > 0:
                if self._vContext[count-1].File:
                    # Send an interrupt request onto server to shut down downloading as earlier as possible
                    self.Interrupt(1)
                    break
                self._vContext.popleft()
                count -= 1
                canceled += 1
        return canceled

    def _GetFilesOpened(self):
        opened = 0
        for it in self._vContext:
            if it.File:
                opened += 1
            elif not it._HasError():
                break
        return opened

    def OnPostProcessing(self, hint, data):
        d = 0
        with self._csFile:
            for it in self._vContext:
                if d >= self._MaxDownloading:
                    break
                if it.File:
                    if it.Uploading:
                        break
                    else:
                        d += 1
                        continue
                if it._HasError():
                    continue
                if it.Uploading:
                    it._OpenLocalRead()
                    if not it._HasError():
                        with CScopeUQueue() as q:
                            q.SaveString(it.FilePath).SaveUInt(it.Flags).SaveULong(it.FileSize)
                            if not self.SendRequest(CStreamingFile.idUpload, q, None, it.Discarded, it.Se):
                                it.ErrCode = CStreamingFile.SESSION_CLOSED_BEFORE
                                it.ErrMsg = 'Session already closed before sending the request Upload'
                                continue
                        break
                else:
                    it._OpenLocalWrite()
                    if not it._HasError():
                        with CScopeUQueue() as q:
                            q.SaveString(it.LocalFile).SaveString(it.FilePath).SaveUInt(it.Flags).SaveLong(it.InitSize)
                            if not self.SendRequest(CStreamingFile.idDownload, q, None, it.Discarded, it.Se):
                                it.ErrCode = CStreamingFile.SESSION_CLOSED_BEFORE
                                it.ErrMsg = 'Session already closed before sending the request Download'
                                continue
                        d += 1

            while len(self._vContext):
                it = self._vContext[0]
                if it._HasError():
                    cb = it.Download
                    if cb:
                        try:
                            self._csFile.release()
                            cb(self, it.ErrCode, it.ErrMsg)
                        finally:
                            self._csFile.acquire()
                    self._vContext.popleft()
                else:
                    break

    def OnMergeTo(self, to):
        with to._csFile:
            pos = 0
            count = len(to._vContext)
            for it in to._vContext:
                if not it._HasError() and not it.File:
                    break
                pos += 1
            left = deque(itertools.islice(to._vContext, pos))
            right = deque(itertools.islice(to._vContext, pos, len(to._vContext)))
            with self._csFile:
                left.extend(self._vContext)
                self._vContext = deque()
            left.extend(right)
            to._vContext = left
            if count == 0 and len(to._vContext):
                ccl.PostProcessing(to.AttachedClientSocket.Handle, 0, 0)
                if not to.AttachedClientSocket.CountOfRequestsInQueue:
                    to.AttachedClientSocket.DoEcho() #make sure WaitAll works correctly

    def Upload(self, localFile, remoteFile, up=None, trans=None, discarded=None, flags=FILE_OPEN_TRUNCACTED, se=None):
        """
        Post a context to upload a local file onto a remote server
        :param localFile: A path to a local file at client side for uploading
        :param remoteFile: A path to a remote file at server side
        :param up: A callback for tracking a final result of uploading, which contains an int and an error message
        :param trans: A callback for tracking uploading progress
        :param discarded: A callback for tracking communication channel events, close and cancel
        :param flags: An integer bit-wise option flags for one or more options such as
        FILE_OPEN_TRUNCACTED|FILE_OPEN_APPENDED and FILE_OPEN_SHARE_WRITE
        :param se: A callback for tracking an exception (CServerError) from server
        :return: True if successful and False if failed when localFile or remoteFile is empty
        """
        if not localFile or str(localFile) == 0:
            return False
        if not remoteFile or str(remoteFile) == 0:
            return False
        context = CContext(True, flags)
        context.Download = up
        context.Transferring = trans
        context.Discarded = discarded
        context.FilePath = remoteFile
        context.LocalFile = localFile
        context.Se = se
        with self._csFile:
            self._vContext.append(context)
            filesOpened = self._GetFilesOpened()
            if self._MaxDownloading > filesOpened:
                ccl.PostProcessing(self.AttachedClientSocket.Handle, 0, 0)
                if not filesOpened:
                    self.AttachedClientSocket.DoEcho() #make sure WaitAll works correctly
        return True

    def upload(self, localFile, remoteFile, trans=None, flags=FILE_OPEN_TRUNCACTED):
        """
        Post a context to upload a local file onto a remote server
        :param localFile: A path to a local file at client side for uploading
        :param remoteFile: A path to a remote file at server side
        :param trans: A callback for tracking uploading progress
        :param flags: An integer bit-wise option flags for one or more options such as
        FILE_OPEN_TRUNCACTED|FILE_OPEN_APPENDED and FILE_OPEN_SHARE_WRITE
        :return: A future for a final result of uploading, which contains an int and an error message
        """
        if not localFile or str(localFile) == 0:
            raise ValueError('localFile cannot be empty')
        if not remoteFile or str(remoteFile) == 0:
            raise ValueError('remoteFile cannot be empty')
        f = future()
        def cb_aborted(ah, canceled):
            if canceled:
                f.cancel()
            else:
                f.set_exception(OSError(CStreamingFile.SESSION_CLOSED_AFTER, 'Session closed after sending the request Upload'))
        def cb_upload(file, res, errmsg):
            f.set_result({'ec':res, 'em':errmsg})
        def server_ex(ah, se):  # an exception from remote server
            f.set_exception(se)
        ok = self.Upload(localFile, remoteFile, cb_upload, trans, cb_aborted, flags, server_ex)
        return f

    def Download(self, localFile, remoteFile, dl=None, trans=None, discarded=None, flags=FILE_OPEN_TRUNCACTED, se=None):
        """
        Post a context to download a remote file at server side to a local file at client side
        :param localFile: A path to a local file at client side for downloading
        :param remoteFile: A path to a remote file at server side
        :param up: A callback for tracking a final result of downloading, which contains an int and an error message
        :param trans: A callback for tracking downloading progress
        :param discarded: A callback for tracking communication channel events, close and cancel
        :param flags: An integer bit-wise option flags for one or more options such as
        FILE_OPEN_TRUNCACTED|FILE_OPEN_APPENDED and FILE_OPEN_SHARE_WRITE
        :param se: A callback for tracking an exception (CServerError) from server
        :return: True if successful and False if failed when localFile or remoteFile is empty
        """
        if not localFile or str(localFile) == 0:
            return False
        if not remoteFile or str(remoteFile) == 0:
            return False
        context = CContext(False, flags)
        context.Download = dl
        context.Transferring = trans
        context.Discarded = discarded
        context.FilePath = remoteFile
        context.LocalFile = localFile
        context.Se = se
        with self._csFile:
            self._vContext.append(context)
            filesOpened = self._GetFilesOpened()
            if self._MaxDownloading > filesOpened:
                ccl.PostProcessing(self.AttachedClientSocket.Handle, 0, 0)
                if not filesOpened:
                    self.AttachedClientSocket.DoEcho()  # make sure WaitAll works correctly
        return True

    def download(self, localFile, remoteFile, trans=None, flags=FILE_OPEN_TRUNCACTED):
        """
        Post a context to download a remote file at server side to a local file at client side
        :param localFile: A path to a local file at client side for downloading
        :param remoteFile: A path to a remote file at server side
        :param trans: A callback for tracking downloading progress
        :param flags: An integer bit-wise option flags for one or more options such as
        FILE_OPEN_TRUNCACTED|FILE_OPEN_APPENDED and FILE_OPEN_SHARE_WRITE
        :return: A future for a final result ({'ec':res, 'em':errmsg}) of downloading, which contains an int and an error message
        """
        if not localFile or str(localFile) == 0:
            raise ValueError('localFile cannot be empty')
        if not remoteFile or str(remoteFile) == 0:
            raise ValueError('remoteFile cannot be empty')
        f = future()
        def cb_aborted(file, canceled):
            if canceled:
                f.cancel()
            else:
                f.set_exception(OSError(CStreamingFile.SESSION_CLOSED_AFTER, 'Session closed after sending the request Download'))
        def cb_download(file, res, errmsg):
            f.set_result({'ec':res, 'em':errmsg})
        def server_ex(ah, se):  # an exception from remote server
            f.set_exception(se)
        ok = self.Download(localFile, remoteFile, cb_download, trans, cb_aborted, flags, server_ex)
        return f

    def OnResultReturned(self, reqId, mc):
        if reqId == CStreamingFile.idDownload:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            dl = None
            with self._csFile:
                if len(self._vContext):
                    context = self._vContext[0]
                    context.ErrCode = res
                    context.ErrMsg = errMsg
                    dl = context.Download
            if dl:
                dl(self, res, errMsg)
            with self._csFile:
                if len(self._vContext):
                    self._vContext.popleft()._CloseFile()
            self.OnPostProcessing(0, 0)
        elif reqId == CStreamingFile.idStartDownloading:
            with self._csFile:
                fileSize = mc.LoadULong()
                localFile = mc.LoadString()
                remoteFile = mc.LoadString()
                flags = mc.LoadUInt()
                initSize = mc.LoadLong()
                if len(self._vContext) == 0:
                    ctx = CContext(False, flags)
                    ctx.LocalFile = localFile
                    ctx.FilePath = remoteFile
                    ctx._OpenLocalWrite()
                    ctx.InitSize = initSize
                    self._vContext.append(ctx)
                front = self._vContext[0]
                front.FileSize = fileSize
                initSize = 0
                if front.InitSize > 0:
                    initSize = front.InitSize
                if front.File.tell() > initSize:
                    front.File.flush()
                    front.File.seek(initSize)
                    front.File.truncate(initSize)
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
        elif reqId == CStreamingFile.idUploadBackup:
            pass
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
                        if mc.GetSize() > 0:
                            context.InitSize = mc.LoadLong()
                        context.ErrCode = res
                        context.ErrMsg = errMsg
                        ctx = context
            else:
                with self._csFile:
                    if len(self._vContext) > 0:
                        context = self._vContext[0]
                        if mc.GetSize() > 0:
                            context.InitSize = mc.LoadLong()
                        context.QueueOk = cs.ClientQueue.StartJob()
                        queue_enabled = cs.ClientQueue.Available
                        if queue_enabled:
                            with CScopeUQueue() as q:
                                q.SaveString(context.FilePath).SaveUInt(context.Flags).SaveULong(context.FileSize).SaveLong(context.InitSize)
                                self.SendRequest(CStreamingFile.idUploadBackup, q, None, context.Discarded, None)
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
                            if not queue_enabled:
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
            errCode = 0
            errMsg = ''
            cs = self.AttachedClientSocket
            ctx = CContext(False, 0)
            trans = None
            uploaded = mc.LoadLong()
            if mc.GetSize() >= 8:
                errCode = mc.LoadInt()
                errMsg = mc.LoadString()
            with self._csFile:
                if len(self._vContext) > 0:
                    context = self._vContext[0]
                    trans = context.Transferring
                    if uploaded < 0 or errCode or errMsg:
                        context.ErrMsg = errMsg
                        context.ErrCode = errCode
                        ctx = context
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
