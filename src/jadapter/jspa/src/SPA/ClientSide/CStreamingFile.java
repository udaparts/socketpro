package SPA.ClientSide;

import SPA.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayDeque;
import java.io.IOException;
import java.io.*;

public class CStreamingFile extends CAsyncServiceHandler {

    public final static int sidFile = SPA.BaseServiceID.sidReserved + 0x6FFFFFF3; //asynchronous file streaming service id

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

    class CContext {

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
        public boolean Tried = false;
        public String ErrMsg = "";
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

    public long getFileSize() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return -1;
            }
            return m_vContext.getFirst().FileSize;
        }
    }

    public String getLocalFile() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return null;
            }
            return m_vContext.getFirst().LocalFile;
        }
    }

    public String getRemoteFile() {
        synchronized (m_csFile) {
            if (m_vContext.isEmpty()) {
                return null;
            }
            return m_vContext.getFirst().FilePath;
        }
    }

    @Override
    public int CleanCallbacks() {
        synchronized (m_csFile) {
            for (CContext c : m_vContext) {
                if (c.File != null) {
                    try {
                        if (c.File.isOpen()) {
                            c.File.close();
                        }
                        if (!c.Uploading) {
                            java.nio.file.Path path = java.nio.file.Paths.get(c.LocalFile);
                            java.nio.file.Files.deleteIfExists(path);
                        }
                    } catch (IOException err) {
                    } finally {
                    }
                }
            }
            m_vContext.clear();
        }
        return super.CleanCallbacks();
    }

    @Override
    protected void OnResultReturned(short reqId, CUQueue mc) {
        switch (reqId) {
            case idDownload: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                DDownload dl;
                synchronized (m_csFile) {
                    CContext context = m_vContext.getFirst();
                    if (context.File != null) {
                        try {
                            if (context.File.isOpen()) {
                                context.File.close();
                            }
                        } catch (IOException err) {
                        }
                    } else if (res == 0) {
                        res = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                        errMsg = context.ErrMsg;
                    }
                    dl = context.Download;
                }
                if (dl != null) {
                    dl.invoke(this, res, errMsg);
                }
                synchronized (m_csFile) {
                    m_vContext.removeFirst();
                }
            }
            break;
            case idStartDownloading:
                synchronized (m_csFile) {
                    CContext context = m_vContext.getFirst();
                    context.FileSize = mc.LoadLong();
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
                        if (context.File != null) {
                            try {
                                if (context.File.isOpen()) {
                                    context.File.close();
                                }
                            } catch (IOException ex) {
                            } finally {
                                context.File = null;
                            }
                        }
                    } finally {
                    }
                }
                break;
            case idDownloading: {
                long downloaded = -1;
                DTransferring trans;
                synchronized (m_cs) {
                    CContext context = m_vContext.getFirst();
                    trans = context.Transferring;
                    if (context.File != null) {
                        byte[] buffer = mc.getIntenalBuffer();
                        try {
                            ByteBuffer bytes = ByteBuffer.wrap(buffer, 0, mc.GetSize());
                            context.File.write(bytes);
                            downloaded = context.File.position();
                        } catch (IOException err) {
                        }
                    }
                }
                mc.SetSize(0);
                if (trans != null) {
                    trans.invoke(this, downloaded);
                }
            }
            break;
            case idUpload: {
                boolean removed = false;
                DUpload upl = null;
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                if (res != 0) {
                    removed = true;
                    synchronized (m_csFile) {
                        CContext context = m_vContext.getFirst();
                        upl = context.Upload;
                        if (context.File != null) {
                            try {
                                context.File.close();
                            } catch (IOException err) {
                            }
                        }
                    }
                }
                if (upl != null) {
                    upl.invoke(this, res, errMsg);
                }
                if (removed) {
                    synchronized (m_csFile) {
                        m_vContext.removeFirst();
                    }
                }
            }
            break;
            case idUploading: {
                DTransferring trans = null;
                long uploaded = mc.LoadLong();
                if (uploaded > 0) {
                    synchronized (m_csFile) {
                        CContext context = m_vContext.getFirst();
                        trans = context.Transferring;
                    }
                }
                if (trans != null) {
                    trans.invoke(this, uploaded);
                }
            }
            break;
            case idUploadCompleted: {
                DUpload upl;
                synchronized (m_csFile) {
                    CContext context = m_vContext.getFirst();
                    upl = context.Upload;
                    if (context.File != null) {
                        try {
                            context.File.close();
                        } catch (IOException err) {
                        }
                    }
                }
                if (upl != null) {
                    upl.invoke(this, 0, "");
                }
                synchronized (m_csFile) {
                    m_vContext.removeFirst();
                }
            }
            break;
            default:
                super.OnResultReturned(reqId, mc);
                break;
        }
        synchronized (m_csFile) {
            Transfer();
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
            return Transfer();
        }
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
            return Transfer();
        }
    }

    private boolean Transfer() {
        int index = 0;
        DAsyncResultHandler rh = null;
        DOnExceptionFromServer se = null;
        CClientSocket cs = getAttachedClientSocket();
        if (!cs.getSendable()) {
            return false;
        }
        int sent_buffer_size = cs.getBytesInSendingBuffer();
        if (sent_buffer_size > 3 * STREAM_CHUNK_SIZE) {
            return true;
        }
        CUQueue sb = CScopeUQueue.Lock();
        while (index < m_vContext.size()) {
            CContext context = (CContext) m_vContext.toArray()[index];
            if (context.Sent) {
                ++index;
                continue;
            }
            if (context.Uploading && context.Tried && context.File == null) {
                if (index == 0) {
                    if (context.Upload != null) {
                        context.Upload.invoke(this, CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                    }
                    m_vContext.removeFirst();
                } else {
                    ++index;
                }
                continue;
            }
            if (context.Uploading) {
                if (!context.Tried) {
                    context.Tried = true;
                    try {
                        File file = new File(context.LocalFile);
                        context.File = new RandomAccessFile(file, "r").getChannel();
                        context.FileSize = context.File.size();
                        sb.SetSize(0);
                        sb.Save(context.FilePath).Save(context.Flags).Save(context.FileSize);
                        IClientQueue cq = getAttachedClientSocket().getClientQueue();
                        if (cq.getAvailable()) {
                            if (!cq.StartJob()) {
                                context.File.close();
                                context.File = null;
                                throw new IOException("Cannot start queue job");
                            }
                        }
                        if (!SendRequest(idUpload, sb, rh, context.Discarded, se)) {
                            CScopeUQueue.Unlock(sb);
                            return false;
                        }
                    } catch (IOException err) {
                        context.ErrMsg = err.getLocalizedMessage();
                    }
                }
                if (context.File == null) {
                    if (index == 0) {
                        if (context.Upload != null) {
                            context.Upload.invoke(this, CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                        }
                        m_vContext.removeFirst();
                    } else {
                        ++index;
                    }
                    continue;
                } else {
                    if (sb.getMaxBufferSize() < STREAM_CHUNK_SIZE) {
                        sb.Realloc(STREAM_CHUNK_SIZE);
                    }
                    byte[] buffer = sb.getIntenalBuffer();
                    ByteBuffer bytes = ByteBuffer.wrap(buffer, 0, STREAM_CHUNK_SIZE);
                    try {
                        int ret = context.File.read(bytes);
                        while (ret > 0) {
                            if (!SendRequest(idUploading, buffer, ret, rh, context.Discarded, se)) {
                                CScopeUQueue.Unlock(sb);
                                return false;
                            }
                            sent_buffer_size = cs.getBytesInSendingBuffer();
                            if (ret < STREAM_CHUNK_SIZE) {
                                break;
                            }
                            if (sent_buffer_size >= 5 * STREAM_CHUNK_SIZE) {
                                break;
                            }
                            bytes = ByteBuffer.wrap(buffer, 0, STREAM_CHUNK_SIZE);
                            ret = context.File.read(bytes);
                        }
                        if (ret < STREAM_CHUNK_SIZE) {
                            context.Sent = true;
                            if (!SendRequest(idUploadCompleted, rh, context.Discarded, se)) {
                                CScopeUQueue.Unlock(sb);
                                return false;
                            }
                            IClientQueue cq = getAttachedClientSocket().getClientQueue();
                            if (cq.getAvailable()) {
                                cq.EndJob();
                            }
                        }
                        if (sent_buffer_size >= 4 * STREAM_CHUNK_SIZE) {
                            break;
                        }
                    } catch (IOException err) {
                    }
                }
            } else {
                sb.SetSize(0);
                sb.Save(context.FilePath).Save(context.Flags);
                if (!SendRequest(idDownload, sb, rh, context.Discarded, se)) {
                    CScopeUQueue.Unlock(sb);
                    return false;
                }
                context.Sent = true;
                sent_buffer_size = cs.getBytesInSendingBuffer();
                if (sent_buffer_size > 3 * STREAM_CHUNK_SIZE) {
                    break;
                }
            }
            ++index;
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }
}
