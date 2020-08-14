package SPA.ClientSide;

import SPA.*;
import java.nio.channels.FileChannel;
import java.util.ArrayDeque;
import java.io.IOException;
import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class CStreamingFile extends CAsyncServiceHandler {

    public final static int sidFile = SPA.BaseServiceID.sidFile; //asynchronous file streaming service id

    public CStreamingFile() {
        super(sidFile);
    }

    @Override
    protected void finalize() throws Throwable {
        CleanCallbacks();
        super.finalize();
    }

    @Override
    public void close() {
        CleanCallbacks();
        super.CleanCallbacks();
    }

    /**
     * You may use the protected constructor when extending this class
     *
     * @param sid a service id
     */
    protected CStreamingFile(int sid) {
        super(sid);
    }

    public final static int STREAM_CHUNK_SIZE = 10240;

    //request ids
    public final static short idDownload = 0x7F70;
    public final static short idStartDownloading = 0x7F71;
    public final static short idDownloading = 0x7F72;
    public final static short idUpload = 0x7F73;
    public final static short idUploading = 0x7F74;
    public final static short idUploadCompleted = 0x7F75;
    public final static short idUploadBackup = 0x7F76;

    //file open flags
    public final static int FILE_OPEN_TRUNCACTED = 1;
    public final static int FILE_OPEN_APPENDED = 2;
    public final static int FILE_OPEN_SHARE_READ = 4;
    public final static int FILE_OPEN_SHARE_WRITE = 8;

    //error code
    public final static int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1;
    public final static int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2;
    public final static int FILE_BAD_OPERATION = -3;
    public final static int FILE_DOWNLOADING_INTERRUPTED = -4;

    public final static int MAX_FILES_STREAMED = 32;

    public interface DDownload {

        void invoke(CStreamingFile file, int res, String errMsg);
    }

    public interface DUpload {

        void invoke(CStreamingFile file, int res, String errMsg);
    }

    public interface DTransferring {

        void invoke(CStreamingFile file, long transferred);
    }

    private class CContext {

        public CContext(boolean uplaod, int flags) {
            Uploading = uplaod;
            Flags = flags;
        }
        public boolean Uploading;
        public long FileSize = -1;
        public int Flags;
        public boolean Sent = false;
        public String LocalFile = "";
        public String FilePath = "";
        public DDownload Download = null;
        public DUpload Upload = null;
        public DTransferring Transferring = null;
        public DDiscarded Discarded = null;
        public FileChannel File = null;
        public String ErrMsg = "";
        public boolean QueueOk = false;
        public int ErrCode = 0;
        public long InitSize = -1;
        public DOnExceptionFromServer Se = null;
        public UFuture<ErrInfo> Fut = null;

        public boolean hasError() {
            return (ErrCode != 0 || (ErrMsg != null && ErrMsg.length() > 0));
        }
    };

    private int m_MaxDownloading = 1;
    protected final Lock m_csFile = new ReentrantLock();
    final private ArrayDeque<CContext> m_vContext = new ArrayDeque<>(); //protected by m_csFile;

    @Override
    protected void OnMergeTo(CAsyncServiceHandler to) {
        CStreamingFile fTo = (CStreamingFile) to;
        ArrayDeque<CContext> vHead = new ArrayDeque<>();
        ArrayDeque<CContext> vTail = new ArrayDeque<>();
        fTo.m_csFile.lock();
        try {
            int count = fTo.m_vContext.size();
            for (CContext cxt : fTo.m_vContext) {
                if (cxt.ErrCode == 0 && cxt.ErrMsg.length() == 0 && cxt.File == null) {
                    vTail.add(cxt);
                } else {
                    vHead.add(cxt);
                }
            }
            m_csFile.lock();
            try {
                vHead.addAll(m_vContext);
            } finally {
                m_vContext.clear();
                m_csFile.unlock();
            }
            vHead.addAll(vTail);
            fTo.m_vContext.clear();
            fTo.m_vContext.addAll(vHead);
            if (count == 0 && fTo.m_vContext.size() > 0) {
                ClientCoreLoader.PostProcessing(fTo.getAttachedClientSocket().getHandle(), 0, 0);
                fTo.getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
            }
        } finally {
            fTo.m_csFile.unlock();
        }
    }

    /**
     * Get the number of files queued
     *
     * @return the number of files queued
     */
    public final int getFilesQueued() {
        int size = 0;
        m_csFile.lock();
        try {
            size = m_vContext.size();
        } finally {
            m_csFile.unlock();
        }
        return size;
    }

    /**
     * Get the number of files streamed
     *
     * @return the number of files streamed possibly
     */
    public final int getFilesStreamed() {
        int n = 0;
        m_csFile.lock();
        try {
            n = m_MaxDownloading;
        } finally {
            m_csFile.unlock();
        }
        return n;
    }

    /**
     * Set the max number of files streamed possibly
     *
     * @param max the max number of files streamed
     */
    public final void setFilesStreamed(int max) {
        m_csFile.lock();
        try {
            if (max <= 0) {
                m_MaxDownloading = 1;
            } else if (max > MAX_FILES_STREAMED) {
                m_MaxDownloading = MAX_FILES_STREAMED;
            } else {
                m_MaxDownloading = max;
            }
        } finally {
            m_csFile.unlock();
        }
    }

    /**
     * Cancel transferring files queued in memory
     *
     * @return the number of transferring files canceled
     */
    public int Cancel() {
        int canceled = 0;
        m_csFile.lock();
        try {
            while (m_vContext.size() > 0) {
                CContext back = m_vContext.getLast();
                if (back.File != null) {
                    //Send an interrupt request onto server to shut down downloading as earlier as possible
                    Interrupt(1);
                    break;
                }
                m_vContext.removeLast();
                ++canceled;
            }
        } finally {
            m_csFile.unlock();
        }
        return canceled;
    }

    /**
     * Get the file size in bytes for current file being in transaction
     *
     * @return file size in bytes
     */
    public final long getFileSize() {
        long n = -1;
        m_csFile.lock();
        try {
            if (!m_vContext.isEmpty()) {
                n = m_vContext.getFirst().FileSize;
            }
        } finally {
            m_csFile.unlock();
        }
        return n;
    }

    /**
     * Get local file name of current file being in transaction
     *
     * @return a string for local file name
     */
    public final String getLocalFile() {
        String s = null;
        m_csFile.lock();
        try {
            if (!m_vContext.isEmpty()) {
                s = m_vContext.getFirst().LocalFile;
            }
        } finally {
            m_csFile.unlock();
        }
        return s;
    }

    /**
     * Get remote file name of current file being in transaction
     *
     * @return a string for remote file name
     */
    public final String getRemoteFile() {
        String s = null;
        m_csFile.lock();
        try {
            if (!m_vContext.isEmpty()) {
                s = m_vContext.getFirst().FilePath;
            }
        } finally {
            m_csFile.unlock();
        }
        return s;
    }

    private int GetFilesOpened() {
        int opened = 0;
        for (CContext it : m_vContext) {
            if (it.File != null) {
                ++opened;
            } else if (!it.hasError()) {
                break;
            }
        }
        return opened;
    }

    @Override
    protected void OnPostProcessing(int hint, long data) {
        int d = 0;
        DAsyncResultHandler rh = null;
        m_csFile.lock();
        try {
            for (CContext it : m_vContext) {
                if (d >= m_MaxDownloading) {
                    break;
                }
                if (it.File != null) {
                    if (it.Uploading) {
                        break;
                    } else {
                        ++d;
                        continue;
                    }
                }
                if (it.hasError()) {
                    continue;
                }
                if (it.Uploading) {
                    OpenLocalRead(it);
                    if (!it.hasError()) {
                        try (CScopeUQueue sq = new CScopeUQueue()) {
                            CUQueue sb = sq.getUQueue();
                            sb.Save(it.FilePath).Save(it.Flags).Save(it.FileSize);
                            if (!SendRequest(idUpload, sb, rh, it.Discarded, it.Se)) {
                                CClientSocket cs = getSocket();
                                it.ErrCode = cs.getErrorCode();
                                if (it.ErrCode == 0) {
                                    it.ErrCode = SESSION_CLOSED_BEFORE;
                                    it.ErrMsg = "Session already closed before sending the request Upload";
                                } else {
                                    it.ErrMsg = cs.getErrorMsg();
                                }
                                if (it.Fut != null) {
                                    it.Fut.setException(new CSocketError(it.ErrCode, it.ErrMsg, idUpload, true));
                                }
                                continue;
                            }
                        }
                        break;
                    }
                } else {
                    OpenLocalWrite(it);
                    if (!it.hasError()) {
                        try (CScopeUQueue sq = new CScopeUQueue()) {
                            CUQueue sb = sq.getUQueue();
                            sb.Save(it.LocalFile).Save(it.FilePath).Save(it.Flags).Save(it.InitSize);
                            if (!SendRequest(idDownload, sb, rh, it.Discarded, it.Se)) {
                                CClientSocket cs = getSocket();
                                it.ErrCode = cs.getErrorCode();
                                if (it.ErrCode == 0) {
                                    it.ErrCode = SESSION_CLOSED_BEFORE;
                                    it.ErrMsg = "Session already closed before sending the request Download";
                                } else {
                                    it.ErrMsg = cs.getErrorMsg();
                                }
                                if (it.Fut != null) {
                                    it.Fut.setException(new CSocketError(it.ErrCode, it.ErrMsg, idDownload, true));
                                }
                                continue;
                            }
                        }
                        ++d;
                    }
                }
            }
            while (m_vContext.size() > 0) {
                CContext it = m_vContext.getFirst();
                if (it.hasError()) {
                    CloseFile(it);
                    if (it.Uploading) {
                        DUpload cb = it.Upload;
                        if (cb != null) {
                            int errCode = it.ErrCode;
                            String errMsg = it.ErrMsg;
                            m_csFile.unlock();
                            try {
                                cb.invoke(this, errCode, errMsg);
                            } finally {
                                m_csFile.lock();
                            }
                        }
                    } else {
                        DDownload cb = it.Download;
                        if (cb != null) {
                            int errCode = it.ErrCode;
                            String errMsg = it.ErrMsg;
                            m_csFile.unlock();
                            try {
                                cb.invoke(this, errCode, errMsg);
                            } finally {
                                m_csFile.lock();
                            }
                        }
                    }
                    m_vContext.removeFirst();
                } else {
                    break;
                }
            }
        } finally {
            m_csFile.unlock();
        }
    }

    @Override
    public int CleanCallbacks() {
        m_csFile.lock();
        try {
            for (CContext c : m_vContext) {
                if (c.File != null) {
                    c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                    c.ErrMsg = "Clean local writing file";
                    CloseFile(c);
                } else {
                    break;
                }
            }
            m_vContext.clear();
        } finally {
            m_csFile.unlock();
        }
        return super.CleanCallbacks();
    }

    private static void CloseFile(CContext c) {
        if (c.File != null) {
            try {
                if (!c.Uploading && c.hasError()) {
                    if (c.InitSize == -1) {
                        c.File.close();
                        java.nio.file.Path path = java.nio.file.Paths.get(c.LocalFile);
                        java.nio.file.Files.deleteIfExists(path);
                    } else {
                        c.File.force(false);
                        c.File.truncate(c.InitSize);
                        c.File.close();
                    }
                } else {
                    c.File.close();
                }
            } catch (Exception err) {
            }
            c.File = null;
        }
    }

    private void OpenLocalRead(CContext context) {
        try {
            File file = new File(context.LocalFile);
            context.File = new RandomAccessFile(file, "r").getChannel();
            context.FileSize = context.File.size();
        } catch (IOException err) {
            context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
            context.ErrMsg = err.getLocalizedMessage();
        }
    }

    private void OpenLocalWrite(CContext context) {
        try {
            File file = new File(context.LocalFile);
            boolean existing = file.exists();
            context.File = new RandomAccessFile(file, "rw").getChannel();
            if ((context.Flags & FILE_OPEN_SHARE_WRITE) == 0) {
                context.File.lock();
            }
            if (existing) {
                context.InitSize = 0;
                if ((context.Flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                    context.File.truncate(0);
                } else if ((context.Flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    context.File.position(context.File.size());
                    context.InitSize = context.File.position();
                }
            }
        } catch (IOException err) {
            context.ErrMsg = err.getLocalizedMessage();
            context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
        }
    }

    @Override
    protected void OnResultReturned(short reqId, CUQueue mc) {
        switch (reqId) {
            case idDownload: {
                DDownload dl = null;
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        CContext front = m_vContext.getFirst();
                        front.ErrCode = res;
                        front.ErrMsg = errMsg;
                        dl = front.Download;
                    }
                } finally {
                    m_csFile.unlock();
                }
                if (dl != null) {
                    dl.invoke(this, res, errMsg);
                }
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        CloseFile(m_vContext.removeFirst());
                    }
                } finally {
                    m_csFile.unlock();
                }
                OnPostProcessing(0, 0);
            }
            break;
            case idStartDownloading:
                long fileSize = mc.LoadLong();
                String localFile = mc.LoadString();
                String remoteFile = mc.LoadString();
                int flags = mc.LoadInt();
                long initSize = mc.LoadLong();
                m_csFile.lock();
                try {
                    if (m_vContext.isEmpty()) {
                        CContext ctx = new CContext(false, flags);
                        ctx.LocalFile = localFile;
                        ctx.FilePath = remoteFile;
                        OpenLocalWrite(ctx);
                        ctx.InitSize = initSize;
                        m_vContext.add(ctx);
                    }
                    CContext context = m_vContext.getFirst();
                    context.FileSize = fileSize;
                    initSize = (context.InitSize > 0) ? context.InitSize : 0;
                    try {
                        if (context.File.position() > initSize) {
                            context.File.position(initSize);
                            context.File.force(false);
                            context.File.truncate(initSize);
                        }
                    } catch (Exception err) {
                    }
                } finally {
                    m_csFile.unlock();
                }
                break;
            case idDownloading: {
                long downloaded = 0;
                DTransferring trans = null;
                CContext context = null;
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        context = m_vContext.getFirst();
                        trans = context.Transferring;
                        try {
                            context.File.write(mc.getIntenalBuffer());
                            initSize = (context.InitSize > 0) ? context.InitSize : 0;
                            downloaded = context.File.position() - initSize;
                        } catch (IOException err) {
                            context.ErrMsg = err.getLocalizedMessage();
                            context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                        }
                    }
                } finally {
                    m_csFile.unlock();
                }
                mc.SetSize(0);
                if (context != null && context.hasError()) {
                    if (context.Download != null) {
                        context.Download.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    m_csFile.lock();
                    try {
                        CloseFile(m_vContext.removeFirst());
                    } finally {
                        m_csFile.unlock();
                    }
                    OnPostProcessing(0, 0);
                } else if (trans != null) {
                    trans.invoke(this, downloaded);
                }
            }
            break;
            case idUploadBackup:
                break;
            case idUpload: {
                CContext context = null;
                DUpload upl = null;
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                if (res != 0 || (errMsg != null && errMsg.length() > 0)) {
                    m_csFile.lock();
                    try {
                        if (m_vContext.size() > 0) {
                            context = m_vContext.getFirst();
                            upl = context.Upload;
                            context.InitSize = mc.LoadLong();
                            context.ErrCode = res;
                            context.ErrMsg = errMsg;
                        }
                    } finally {
                        m_csFile.unlock();
                    }
                } else {
                    DAsyncResultHandler rh = null;
                    DOnExceptionFromServer se = null;
                    CClientSocket cs = getAttachedClientSocket();
                    m_csFile.lock();
                    try {
                        if (m_vContext.size() > 0) {
                            context = m_vContext.getFirst();
                            upl = context.Upload;
                            context.InitSize = mc.LoadLong();
                            try (CScopeUQueue sq = new CScopeUQueue()) {
                                CUQueue sb = sq.getUQueue();
                                if (sb.getMaxBufferSize() < STREAM_CHUNK_SIZE) {
                                    sb.Realloc(STREAM_CHUNK_SIZE);
                                } else {
                                    sb.Realloc(STREAM_CHUNK_SIZE);
                                }
                                context.QueueOk = cs.getClientQueue().StartJob();
                                boolean queue_enabled = cs.getClientQueue().getAvailable();
                                if (queue_enabled) {
                                    try (CScopeUQueue su = new CScopeUQueue()) {
                                        CUQueue q = su.getUQueue();
                                        q.Save(context.FilePath).Save(context.Flags).Save(context.FileSize).Save(context.InitSize);
                                        SendRequest(idUploadBackup, q, rh, context.Discarded, se);
                                    }
                                }
                                int ret = context.File.read(sb.getIntenalBuffer());
                                while (ret == sb.getMaxBufferSize()) {
                                    sb.SetSize(ret);
                                    if (!SendRequest(idUploading, sb.getIntenalBuffer(), ret, rh, context.Discarded, se)) {
                                        context.ErrCode = cs.getErrorCode();
                                        context.ErrMsg = cs.getErrorMsg();
                                        break;
                                    }
                                    sb.SetSize(0);
                                    ret = context.File.read(sb.getIntenalBuffer());
                                    if (queue_enabled) {
                                        //save file into client message queue
                                    } else if (cs.getBytesInSendingBuffer() > 40 * sb.getMaxBufferSize()) {
                                        break;
                                    }
                                }
                                if (ret > 0 && !context.hasError()) {
                                    sb.SetSize(ret);
                                    if (!SendRequest(idUploading, sb.getIntenalBuffer(), ret, rh, context.Discarded, se)) {
                                        context.ErrCode = cs.getErrorCode();
                                        context.ErrMsg = cs.getErrorMsg();
                                    }
                                }
                                if (ret < sb.getMaxBufferSize() && !context.hasError()) {
                                    context.Sent = true;
                                    if (!SendRequest(idUploadCompleted, rh, context.Discarded, se)) {
                                        context.ErrCode = cs.getErrorCode();
                                        context.ErrMsg = cs.getErrorMsg();
                                    } else if (context.QueueOk) {
                                        cs.getClientQueue().EndJob();
                                    }
                                }
                            } catch (IOException err) {
                                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                                context.ErrMsg = err.getLocalizedMessage();
                            }
                        }
                    } finally {
                        m_csFile.unlock();
                    }
                }
                if (context != null && context.hasError()) {
                    if (upl != null) {
                        upl.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    if (context.QueueOk) {
                        getAttachedClientSocket().getClientQueue().AbortJob();
                    }
                    m_csFile.lock();
                    try {
                        CloseFile(m_vContext.removeFirst());
                    } finally {
                        m_csFile.unlock();
                    }
                    OnPostProcessing(0, 0);
                }
            }
            break;
            case idUploading: {
                int errCode = 0;
                String errMsg = "";
                CContext context = null;
                DTransferring trans = null;
                DAsyncResultHandler rh = null;
                DOnExceptionFromServer se = null;
                CClientSocket cs = getAttachedClientSocket();
                long uploaded = mc.LoadLong();
                if (mc.getSize() >= 8) {
                    errCode = mc.LoadInt();
                    errMsg = mc.LoadString();
                }
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        context = m_vContext.getFirst();
                        trans = context.Transferring;
                        if (uploaded < 0 || errCode != 0 || errMsg.length() != 0) {
                            context.ErrCode = errCode;
                            context.ErrMsg = errMsg;
                            CloseFile(context);
                        } else if (!context.Sent) {
                            try (CScopeUQueue sq = new CScopeUQueue()) {
                                CUQueue sb = sq.getUQueue();
                                if (sb.getMaxBufferSize() < STREAM_CHUNK_SIZE) {
                                    sb.Realloc(STREAM_CHUNK_SIZE);
                                }
                                int ret = context.File.read(sb.getIntenalBuffer());
                                if (ret > 0) {
                                    sb.SetSize(ret);
                                    if (!SendRequest(idUploading, sb.getIntenalBuffer(), ret, rh, context.Discarded, se)) {
                                        context.ErrCode = cs.getErrorCode();
                                        context.ErrMsg = cs.getErrorMsg();
                                    }
                                }
                                if (ret < STREAM_CHUNK_SIZE) {
                                    context.Sent = true;
                                    if (!SendRequest(idUploadCompleted, rh, context.Discarded, se)) {
                                        context.ErrCode = cs.getErrorCode();
                                        context.ErrMsg = cs.getErrorMsg();
                                    }
                                }
                            } catch (IOException err) {
                                context.ErrMsg = err.getLocalizedMessage();
                                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                            }
                        }
                    }
                } finally {
                    m_csFile.unlock();
                }
                if (context != null && context.hasError()) {
                    if (context.Upload != null) {
                        context.Upload.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    m_csFile.lock();
                    try {
                        CloseFile(m_vContext.removeFirst());
                    } finally {
                        m_csFile.unlock();
                    }
                    OnPostProcessing(0, 0);
                } else if (trans != null) {
                    trans.invoke(this, uploaded);
                }
            }
            break;
            case idUploadCompleted: {
                DUpload upl = null;
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        CContext ctx = m_vContext.getFirst();
                        if (ctx.File != null) {
                            upl = m_vContext.getFirst().Upload;
                        } else {
                            ctx.QueueOk = false;
                            ctx.Sent = false;
                            CloseFile(ctx);
                        }
                    }
                } finally {
                    m_csFile.unlock();
                }
                if (upl != null) {
                    upl.invoke(this, 0, "");
                }
                m_csFile.lock();
                try {
                    if (m_vContext.size() > 0) {
                        CContext ctx = m_vContext.getFirst();
                        if (ctx.File != null) {
                            CloseFile(m_vContext.removeFirst());
                        }
                    }
                } finally {
                    m_csFile.unlock();
                }
                OnPostProcessing(0, 0);
            }
            break;
            default:
                super.OnResultReturned(reqId, mc);
                break;
        }
    }

    public final boolean Download(String localFile, String remoteFile) {
        return Download(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Download(String localFile, String remoteFile, DDownload dl) {
        return Download(localFile, remoteFile, dl, null, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans) {
        return Download(localFile, remoteFile, dl, trans, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded) {
        return Download(localFile, remoteFile, dl, trans, discarded, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, int flags) {
        return Download(localFile, remoteFile, dl, trans, discarded, flags, null);
    }

    public boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, int flags, DOnExceptionFromServer se) {
        if (localFile == null || localFile.length() == 0) {
            return false;
        }
        if (remoteFile == null || remoteFile.length() == 0) {
            return false;
        }
        CContext context = new CContext(false, flags);
        context.Download = dl;
        context.Transferring = trans;
        context.Discarded = discarded;
        context.FilePath = remoteFile;
        context.LocalFile = localFile;
        context.Se = se;
        m_csFile.lock();
        try {
            m_vContext.addLast(context);
            int filesOpened = GetFilesOpened();
            if (m_MaxDownloading > filesOpened) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                if (filesOpened == 0) {
                    getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
                }
            }
        } finally {
            m_csFile.unlock();
        }
        return true;
    }

    public final UFuture<ErrInfo> download(String localFile, String remoteFile) {
        return download(localFile, remoteFile, null, FILE_OPEN_TRUNCACTED);
    }

    public final UFuture<ErrInfo> download(String localFile, String remoteFile, DTransferring trans) {
        return download(localFile, remoteFile, trans, FILE_OPEN_TRUNCACTED);
    }

    public UFuture<ErrInfo> download(String localFile, String remoteFile, DTransferring trans, int flags) {
        if (localFile == null || localFile.length() == 0) {
            throw new IllegalArgumentException("localFile cannot be empty");
        }
        if (remoteFile == null || remoteFile.length() == 0) {
            throw new IllegalArgumentException("remoteFile cannot be empty");
        }
        final UFuture<ErrInfo> f = new UFuture<>("Download", idDownload, this);
        DDownload dl = new DDownload() {
            @Override
            public void invoke(CStreamingFile file, int res, String errMsg) {
                f.set(new ErrInfo(res, errMsg));
            }
        };
        CContext context = new CContext(false, flags);
        context.Download = dl;
        context.Transferring = trans;
        context.Discarded = getAborted(f);
        context.FilePath = remoteFile;
        context.LocalFile = localFile;
        context.Se = getSE(f);
        context.Fut = f;
        m_csFile.lock();
        try {
            m_vContext.addLast(context);
            int filesOpened = GetFilesOpened();
            if (m_MaxDownloading > filesOpened) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                if (filesOpened == 0) {
                    getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
                }
            }
        } finally {
            m_csFile.unlock();
        }
        return f;
    }

    public final boolean Upload(String localFile, String remoteFile) {
        return Upload(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Upload(String localFile, String remoteFile, DUpload up) {
        return Upload(localFile, remoteFile, up, null, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans) {
        return Upload(localFile, remoteFile, up, trans, null, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans, DDiscarded discarded) {
        return Upload(localFile, remoteFile, up, trans, discarded, FILE_OPEN_TRUNCACTED, null);
    }

    public final boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, int flags) {
        return Upload(localFile, remoteFile, up, trans, discarded, flags, null);
    }

    public boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, int flags, DOnExceptionFromServer se) {
        if (localFile == null || localFile.length() == 0) {
            return false;
        }
        if (remoteFile == null || remoteFile.length() == 0) {
            return false;
        }
        CContext context = new CContext(true, flags);
        context.Upload = up;
        context.Transferring = trans;
        context.Discarded = discarded;
        context.FilePath = remoteFile;
        context.LocalFile = localFile;
        context.Se = se;
        m_csFile.lock();
        try {
            m_vContext.addLast(context);
            int filesOpened = GetFilesOpened();
            if (m_MaxDownloading > filesOpened) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                if (filesOpened == 0) {
                    getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
                }
            }
        } finally {
            m_csFile.unlock();
        }
        return true;
    }

    public final UFuture<ErrInfo> upload(String localFile, String remoteFile) {
        return upload(localFile, remoteFile, null, FILE_OPEN_TRUNCACTED);
    }

    public final UFuture<ErrInfo> upload(String localFile, String remoteFile, DTransferring trans) {
        return upload(localFile, remoteFile, trans, FILE_OPEN_TRUNCACTED);
    }

    public UFuture<ErrInfo> upload(String localFile, String remoteFile, DTransferring trans, int flags) {
        if (localFile == null || localFile.length() == 0) {
            throw new IllegalArgumentException("localFile cannot be empty");
        }
        if (remoteFile == null || remoteFile.length() == 0) {
            throw new IllegalArgumentException("remoteFile cannot be empty");
        }
        final UFuture<ErrInfo> f = new UFuture<>("Upload", idUpload, this);
        DUpload up = new DUpload() {
            @Override
            public void invoke(CStreamingFile file, int res, String errMsg) {
                f.set(new ErrInfo(res, errMsg));
            }
        };
        CContext context = new CContext(true, flags);
        context.Upload = up;
        context.Transferring = trans;
        context.Discarded = getAborted(f);
        context.FilePath = remoteFile;
        context.LocalFile = localFile;
        context.Se = getSE(f);
        context.Fut = f;
        m_csFile.lock();
        try {
            m_vContext.addLast(context);
            int filesOpened = GetFilesOpened();
            if (m_MaxDownloading > filesOpened) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                if (filesOpened == 0) {
                    getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
                }
            }
        } finally {
            m_csFile.unlock();
        }
        return f;
    }
}
