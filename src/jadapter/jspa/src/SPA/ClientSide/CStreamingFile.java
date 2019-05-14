package SPA.ClientSide;

import SPA.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayDeque;
import java.io.IOException;
import java.io.*;

public class CStreamingFile extends CAsyncServiceHandler {

    public final static int sidFile = SPA.BaseServiceID.sidFile; //asynchronous file streaming service id

    public CStreamingFile() {
        super(sidFile);
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

    //file open flags
    public final static int FILE_OPEN_TRUNCACTED = 1;
    public final static int FILE_OPEN_APPENDED = 2;
    public final static int FILE_OPEN_SHARE_READ = 4;
    public final static int FILE_OPEN_SHARE_WRITE = 8;

    //error code
    public final static int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1;
    public final static int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2;

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

        public boolean hasError() {
            return (ErrCode != 0 || (ErrMsg != null && ErrMsg.length() > 0));
        }
    };

    protected final Object m_csFile = new Object();
    final private ArrayDeque<CContext> m_vContext = new ArrayDeque<>(); //protected by m_csFile;

    @Override
    protected void OnMergeTo(CAsyncServiceHandler to) {
        CStreamingFile fTo = (CStreamingFile) to;
        synchronized (fTo.m_csFile) {
            synchronized (m_csFile) {
                fTo.m_vContext.addAll(m_vContext);
                m_vContext.clear();
            }
        }
    }

    @Override
    protected void OnBaseRequestProcessed(short reqId) {
        if (reqId == SPA.tagBaseRequestID.idDoEcho.getValue()) {
            return;
        }
        CClientSocket cs = getAttachedClientSocket();
        if (cs.getCountOfRequestsInQueue() <= 1 && cs.getClientQueue().getAvailable()) {
            synchronized (m_csFile) {
                if (m_vContext.size() > 0) {
                    ClientCoreLoader.PostProcessing(cs.getHandle(), 0, 0);
                }
            }
        }
    }

    /**
     * Get the number of files queued
     *
     * @return the number of files queued
     */
    public int getFilesQueued() {
        synchronized (m_csFile) {
            return m_vContext.size();
        }
    }

    /**
     * Get the file size in bytes for current file being in transaction
     *
     * @return file size in bytes
     */
    public long getFileSize() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return -1;
            }
            return m_vContext.getFirst().FileSize;
        }
    }

    /**
     * Get local file name of current file being in transaction
     *
     * @return a string for local file name
     */
    public String getLocalFile() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return null;
            }
            return m_vContext.getFirst().LocalFile;
        }
    }

    /**
     * Get remote file name of current file being in transaction
     *
     * @return a string for remote file name
     */
    public String getRemoteFile() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return null;
            }
            return m_vContext.getFirst().FilePath;
        }
    }

    @Override
    protected void OnPostProcessing(int hint, long data) {
        CContext ctx = null;
        CClientSocket cs = getAttachedClientSocket();
        synchronized (m_csFile) {
            if (m_vContext.size() > 0) {
                CContext context = m_vContext.getFirst();
                if (context.Uploading) {
                    OpenLocalRead(context);
                } else {
                    OpenLocalWrite(context);
                }
                DAsyncResultHandler rh = null;
                DOnExceptionFromServer se = null;
                if (context.ErrCode != 0 || (context.ErrMsg != null && context.ErrMsg.length() > 0)) {
                    ctx = context;
                } else if (context.Uploading) {
                    try (CScopeUQueue sq = new CScopeUQueue()) {
                        CUQueue sb = sq.getUQueue();
                        sb.Save(context.FilePath).Save(context.Flags).Save(context.FileSize);
                        if (!SendRequest(idUpload, sb, rh, context.Discarded, se)) {
                            ctx = context;
                            context.ErrCode = cs.getErrorCode();
                            context.ErrMsg = cs.getErrorMsg();
                        }
                    }
                } else {
                    try (CScopeUQueue sq = new CScopeUQueue()) {
                        CUQueue sb = sq.getUQueue();
                        sb.Save(context.FilePath).Save(context.Flags).Save(context.FileSize);
                        if (!SendRequest(idDownload, sb, rh, context.Discarded, se)) {
                            ctx = context;
                            context.ErrCode = cs.getErrorCode();
                            context.ErrMsg = cs.getErrorMsg();
                        }
                    }
                }
            }
        }
        if (ctx == null) {
            return;
        }
        if (ctx.Uploading) {
            if (ctx.Upload != null) {
                ctx.Upload.invoke(this, ctx.ErrCode, ctx.ErrMsg);
            }
        } else {
            if (ctx.Download != null) {
                ctx.Download.invoke(this, ctx.ErrCode, ctx.ErrMsg);
            }
        }
        synchronized (m_csFile) {
            CloseFile(m_vContext.removeFirst());
            if (m_vContext.size() > 0) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
            }
        }
    }

    @Override
    public int CleanCallbacks() {
        synchronized (m_csFile) {
            for (CContext c : m_vContext) {
                CloseFile(c);
            }
            m_vContext.clear();
        }
        return super.CleanCallbacks();
    }

    private static void CloseFile(CContext c) {
        try {
            if (c.File != null) {
                if (c.File.isOpen()) {
                    c.File.close();
                }
                if (!c.Uploading) {
                    java.nio.file.Path path = java.nio.file.Paths.get(c.LocalFile);
                    java.nio.file.Files.deleteIfExists(path);
                }
            }
        } catch (Exception err) {
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
            context.File = new RandomAccessFile(file, "rw").getChannel();
            if ((context.Flags & FILE_OPEN_SHARE_WRITE) == 0) {
                context.File.lock();
            }
            if ((context.Flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                context.File.truncate(0);
            } else if ((context.Flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                context.File.position(context.File.size());
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
                DDownload dl;
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                synchronized (m_csFile) {
                    dl = m_vContext.getFirst().Download;
                }
                if (dl != null) {
                    dl.invoke(this, res, errMsg);
                }
                synchronized (m_csFile) {
                    CloseFile(m_vContext.removeFirst());
                }
                OnPostProcessing(0, 0);
            }
            break;
            case idStartDownloading:
                synchronized (m_csFile) {
                    CContext context = m_vContext.getFirst();
                    context.FileSize = mc.LoadLong();
                }
                break;
            case idDownloading: {
                long downloaded = 0;
                DTransferring trans;
                CContext context;
                synchronized (m_csFile) {
                    context = m_vContext.getFirst();
                    trans = context.Transferring;
                    byte[] buffer = mc.getIntenalBuffer();
                    try {
                        ByteBuffer bytes = ByteBuffer.wrap(buffer, 0, mc.GetSize());
                        context.File.write(bytes);
                        downloaded = context.File.position();
                    } catch (IOException err) {
                        context.ErrMsg = err.getLocalizedMessage();
                        context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                    }
                }
                mc.SetSize(0);
                if (context.hasError()) {
                    if (context.Download != null) {
                        context.Download.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    synchronized (m_csFile) {
                        CloseFile(m_vContext.removeFirst());
                    }
                    OnPostProcessing(0, 0);
                } else if (trans != null) {
                    trans.invoke(this, downloaded);
                }
            }
            break;
            case idUpload: {
                CContext context;
                DUpload upl;
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                if (res != 0 || (errMsg != null && errMsg.length() > 0)) {
                    synchronized (m_csFile) {
                        context = m_vContext.getFirst();
                        upl = context.Upload;
                        context.ErrCode = res;
                        context.ErrMsg = errMsg;
                    }
                } else {
                    DAsyncResultHandler rh = null;
                    DOnExceptionFromServer se = null;
                    CClientSocket cs = getAttachedClientSocket();
                    synchronized (m_csFile) {
                        context = m_vContext.getFirst();
                        upl = context.Upload;
                        try (CScopeUQueue sq = new CScopeUQueue()) {
                            CUQueue sb = sq.getUQueue();
                            if (sb.getMaxBufferSize() < STREAM_CHUNK_SIZE) {
                                sb.Realloc(STREAM_CHUNK_SIZE);
                            }
                            byte[] buffer = sb.getIntenalBuffer();
                            ByteBuffer bytes = ByteBuffer.wrap(buffer, 0, STREAM_CHUNK_SIZE);
                            context.QueueOk = cs.getClientQueue().StartJob();
                            int ret = context.File.read(bytes);
                            while (ret == STREAM_CHUNK_SIZE) {
                                if (!SendRequest(idUploading, buffer, ret, rh, context.Discarded, se)) {
                                    context.ErrCode = cs.getErrorCode();
                                    context.ErrMsg = cs.getErrorMsg();
                                    break;
                                }
                                ret = context.File.read(bytes);
                                if (context.QueueOk) {
                                    //save file into client message queue
                                } else if (cs.getBytesInSendingBuffer() > 40 * STREAM_CHUNK_SIZE) {
                                    break;
                                }
                            }
                            if (ret > 0 && !context.hasError()) {
                                if (!SendRequest(idUploading, buffer, ret, rh, context.Discarded, se)) {
                                    context.ErrCode = cs.getErrorCode();
                                    context.ErrMsg = cs.getErrorMsg();
                                }
                            }
                            if (ret < STREAM_CHUNK_SIZE && !context.hasError()) {
                                context.Sent = true;
                                if (!SendRequest(idUploadCompleted, rh, context.Discarded, se)) {
                                    context.ErrCode = cs.getErrorCode();
                                    context.ErrMsg = cs.getErrorMsg();
                                }
                            }
                        } catch (IOException err) {
                            context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                            context.ErrMsg = err.getLocalizedMessage();
                        }
                    }
                }
                if (context.hasError()) {
                    if (upl != null) {
                        upl.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    if (context.QueueOk) {
                        getAttachedClientSocket().getClientQueue().AbortJob();
                    }
                    synchronized (m_csFile) {
                        CloseFile(m_vContext.removeFirst());
                    }
                    OnPostProcessing(0, 0);
                }
            }
            break;
            case idUploading: {
                CContext context;
                DTransferring trans;
                DAsyncResultHandler rh = null;
                DOnExceptionFromServer se = null;
                CClientSocket cs = getAttachedClientSocket();
                long uploaded = mc.LoadLong();
                synchronized (m_csFile) {
                    context = m_vContext.getFirst();
                    trans = context.Transferring;
                    if (!context.Sent) {
                        try (CScopeUQueue sq = new CScopeUQueue()) {
                            CUQueue sb = sq.getUQueue();
                            if (sb.getMaxBufferSize() < STREAM_CHUNK_SIZE) {
                                sb.Realloc(STREAM_CHUNK_SIZE);
                            }
                            byte[] buffer = sb.getIntenalBuffer();
                            ByteBuffer bytes = ByteBuffer.wrap(buffer, 0, STREAM_CHUNK_SIZE);
                            int ret = context.File.read(bytes);
                            if (ret > 0) {
                                if (!SendRequest(idUploading, buffer, ret, rh, context.Discarded, se)) {
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
                if (context.hasError()) {
                    if (context.Upload != null) {
                        context.Upload.invoke(this, context.ErrCode, context.ErrMsg);
                    }
                    synchronized (m_csFile) {
                        CloseFile(m_vContext.removeFirst());
                    }
                    OnPostProcessing(0, 0);
                } else if (trans != null) {
                    trans.invoke(this, uploaded);
                }
            }
            break;
            case idUploadCompleted: {
                DUpload upl;
                synchronized (m_csFile) {
                    upl = m_vContext.getFirst().Upload;
                }
                if (upl != null) {
                    upl.invoke(this, 0, "");
                }
                synchronized (m_csFile) {
                    CloseFile(m_vContext.removeFirst());
                }
                OnPostProcessing(0, 0);
            }
            break;
            default:
                super.OnResultReturned(reqId, mc);
                break;
        }
    }

    public boolean Download(String localFile, String remoteFile) {
        return Download(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Download(String localFile, String remoteFile, DDownload dl) {
        return Download(localFile, remoteFile, dl, null, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans) {
        return Download(localFile, remoteFile, dl, trans, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded) {
        return Download(localFile, remoteFile, dl, trans, discarded, FILE_OPEN_TRUNCACTED);
    }

    public boolean Download(String localFile, String remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, int flags) {
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
        synchronized (m_csFile) {
            m_vContext.addLast(context);
            if (m_vContext.size() == 1) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
            }
        }
        return true;
    }

    public boolean Upload(String localFile, String remoteFile) {
        return Upload(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Upload(String localFile, String remoteFile, DUpload up) {
        return Upload(localFile, remoteFile, up, null, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans) {
        return Upload(localFile, remoteFile, up, trans, null, FILE_OPEN_TRUNCACTED);
    }

    public boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans, DDiscarded discarded) {
        return Upload(localFile, remoteFile, up, trans, discarded, FILE_OPEN_TRUNCACTED);
    }

    public boolean Upload(String localFile, String remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, int flags) {
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
        synchronized (m_csFile) {
            m_vContext.addLast(context);
            if (m_vContext.size() == 1) {
                ClientCoreLoader.PostProcessing(getAttachedClientSocket().getHandle(), 0, 0);
                getAttachedClientSocket().DoEcho(); //make sure WaitAll works correctly
            }
        }
        return true;
    }
}
