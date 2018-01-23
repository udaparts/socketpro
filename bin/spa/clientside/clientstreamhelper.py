
from spa import CUQueue, BaseServiceID, CScopeUQueue
import threading
from spa.clientside.asynchandler import CAsyncServiceHandler
from collections import deque
import io

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
        self.Tried = False
        self.ErrMsg = ''

class CStreamingFile(CAsyncServiceHandler):
    sidFile = BaseServiceID.sidReserved + 0x6FFFFFF3 # asynchronous file streaming service id
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
        with self._csFile:
            for c in self._vContext:
                if c.File:
                    c.File.close()
                    c.File = None
            self._vContext = deque()
        return super(CStreamingFile, self).CleanCallbacks()

    @property
    def FileSize(self):
        with self._csFile:
            if len(self._vContext) == 0:
                return -1
            return self._vContext[0].FileSize


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
            return self._Transfer()

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
            return self._Transfer()

    def OnResultReturned(self, reqId, mc):
        if reqId == CStreamingFile.idDownload:
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            dl = None
            with self._csFile:
                context = self._vContext[0]
                if context.File:
                    context.File.close()
                elif res==0:
                    res = CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
                    errMsg = context.ErrMsg
                dl = context.Download
            if dl:
                dl(self, res, errMsg)
            with self._csFile:
                self._vContext.popleft()
        elif reqId == CStreamingFile.idStartDownloading:
            with self._csFile:
                context = self._vContext[0]
                context.FileSize = mc.LoadULong()
                mode = 'xb'
                if (context.Flags & CStreamingFile.FILE_OPEN_TRUNCACTED) == CStreamingFile.FILE_OPEN_TRUNCACTED:
                    mode = 'wb'
                elif (context.Flags & CStreamingFile.FILE_OPEN_APPENDED) == CStreamingFile.FILE_OPEN_APPENDED:
                    mode = 'ab'
                try:
                    context.File = open(context.LocalFile, mode)
                except IOError as e:
                    context.ErrMsg = e.strerror
                    context.File = None
        elif reqId == CStreamingFile.idDownloading:
            downloaded = -1
            trans = None
            with self._csFile:
                context = self._vContext[0]
                trans = context.Transferring
                if context.File:
                    context.File.write(mc.GetBuffer())
                    downloaded = context.File.tell()
            mc.SetSize(0)
            if trans:
                trans(self, downloaded)
        elif reqId == CStreamingFile.idUpload:
            removed = False
            upl = None
            res = mc.LoadInt()
            errMsg = mc.LoadString()
            if res != 0:
                with self._csFile:
                    context = self._vContext[0]
                    removed = True
                    upl = context.Download
                    if context.File:
                        context.File.close()
            if upl:
                upl(self, res, errMsg)
            if removed:
                with self._csFile:
                    self._vContext.popleft()
        elif reqId == CStreamingFile.idUploading:
            trans = None
            uploaded = mc.LoadLong()
            if uploaded > 0:
                with self._csFile:
                    context = self._vContext[0]
                    trans = context.Transferring
            if trans:
                trans(self, uploaded)
        elif reqId == CStreamingFile.idUploadCompleted:
            upl = None
            with self._csFile:
                context = self._vContext[0]
                upl = context.Download
                if context.File:
                    context.File.close()
            if upl:
                upl(self, 0, '')
            with self._csFile:
                self._vContext.popleft()
        else:
            super(CStreamingFile, self).OnResultReturned(reqId, mc)
        with self._csFile:
            self._Transfer()

    def _Transfer(self):
        index = 0
        rh = None
        se = None
        cs = self.AttachedClientSocket
        if not cs.Sendable:
            return False
        sent_buffer_size = cs.BytesInSendingBuffer
        if sent_buffer_size > 3 * CStreamingFile.STREAM_CHUNK_SIZE:
            return True
        while index < len(self._vContext):
            context = self._vContext[index]
            if context.Sent:
                index += 1
                continue
            if context.Uploading and context.Tried and not context.File:
                if index == 0:
                    if context.Download:
                        context.Download(self, CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg)
                    self._vContext.popleft()
                else:
                    index += 1
                continue
            if context.Uploading:
                if not context.Tried:
                    context.Tried = True
                    try:
                        context.File = open(context.LocalFile, 'rb')
                        context.File.seek(0, io.SEEK_END)
                        context.FileSize = context.File.tell()
                        context.File.seek(0, io.SEEK_SET)
                        cq = self.AttachedClientSocket.ClientQueue
                        if cq.Available:
                            cq.StartJob()
                        with CScopeUQueue() as q:
                            q.SaveString(context.FilePath).SaveUInt(context.Flags).SaveULong(context.FileSize)
                            if not self.SendRequest(CStreamingFile.idUpload, q, rh, context.Discarded, se):
                                return False
                    except IOError as e:
                        context.ErrMsg = e.strerror
                        context.File = None
                if not context.File:
                    if index == 0:
                        if context.Download:
                            context.Download(self, CStreamingFile.CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg)
                        self._vContext.popleft()
                    else:
                        index += 1
                    continue
                else:
                    ret = bytearray(context.File.read(CStreamingFile.STREAM_CHUNK_SIZE))
                    while len(ret) > 0:
                        if not self.SendRequest(CStreamingFile.idUploading, CUQueue(ret), rh, context.Discarded, se):
                            return False
                        sent_buffer_size = cs.BytesInSendingBuffer;
                        if len(ret) < CStreamingFile.STREAM_CHUNK_SIZE:
                            break
                        if sent_buffer_size >= 5 * CStreamingFile.STREAM_CHUNK_SIZE:
                            break;
                        ret = bytearray(context.File.read(CStreamingFile.STREAM_CHUNK_SIZE))
                    if len(ret) < CStreamingFile.STREAM_CHUNK_SIZE:
                        context.Sent = True
                        if not self.SendRequest(CStreamingFile.idUploadCompleted, None, rh, context.Discarded, se):
                            return False
                        cq = self.AttachedClientSocket.ClientQueue
                        if cq.Available:
                            cq.EndJob()
                    if sent_buffer_size >= 4 * CStreamingFile.STREAM_CHUNK_SIZE:
                        break
            else:
                with CScopeUQueue() as q:
                    q.SaveString(context.FilePath).SaveUInt(context.Flags)
                    if not self.SendRequest(CStreamingFile.idDownload, q, rh, context.Discarded, se):
                        return False
                    context.Sent = True
                    sent_buffer_size = cs.BytesInSendingBuffer
                    if sent_buffer_size > 3 * CStreamingFile.STREAM_CHUNK_SIZE:
                        return True
            index += 1
        return True
