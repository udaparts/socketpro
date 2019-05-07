using System;
using System.IO;

namespace SocketProAdapter.ClientSide {
    [ObsoleteAttribute]
    public class CStreamHelper {
        public delegate void DProgress(CStreamHelper sender, ulong pos);
        public event DProgress Progress;

        public CStreamHelper(CAsyncServiceHandler ash) {
            if (ash == null)
                throw new ArgumentNullException("A valid service handler required");
            m_ash = ash;
        }

        private ulong m_nDownloadingFileSize = ulong.MaxValue;

        /// <summary>
        /// Remote stream size in bytes. It will be -1 if not available.
        /// </summary>
        public ulong DownloadingStreamSize {
            get {
                return m_nDownloadingFileSize;
            }
        }

        /// <summary>
        /// Nullize stream
        /// </summary>
        public void Reset() {
            lock (m_cs) {
                m_s = null;
            }
        }

        private bool DataFromServerToClient(CAsyncServiceHandler sender, ushort reqId, CUQueue qData) {
            bool processed = false;
            switch (reqId) {
                case CStreamSerializationHelper.idReadDataFromServerToClient:
                    if (qData.GetSize() > 0) {
                        lock (m_cs) {
                            CStreamSerializationHelper.Write(m_s, qData);
                            if (Progress != null)
                                Progress.Invoke(this, (ulong)m_s.Position);
                        }
                        qData.SetSize(0);
                        processed = true;
                    }
                    break;
                default:
                    break;
            }
            return processed;
        }

        /// <summary>
        /// Get a stream from remote server onto a receiving stream at client side
        /// </summary>
        /// <param name="receiver">A stream at client side for receiving data from remote server</param>
        /// <param name="RemotePath">A string for finding an arbuturay file or other object</param>
        /// <returns>An empty string if successful. Otherwise, an error message if failed</returns>
        public string Download(Stream receiver, string RemotePath) {
            lock (m_cs) {
                if (m_s != null)
                    throw new InvalidOperationException("A stream during transaction");
                if (receiver == null || !receiver.CanWrite)
                    throw new InvalidOperationException("A writable target stream required");
                m_s = receiver;
            }
            //remove any exitsing DataFromServerToClient delegate
            m_ash.ResultReturned -= DataFromServerToClient;
            string res = "";
            m_ash.ResultReturned += DataFromServerToClient;
            bool ok = (m_ash.SendRequest(CStreamSerializationHelper.idStartDownloading, RemotePath, (ar) => {
                ar.Load(out m_nDownloadingFileSize).Load(out res);
            }) && m_ash.WaitAll());
            lock (m_cs) {
                if (res != null && res.Length > 0) {
                    m_s = null;
                    return res;
                } else if (res == null)
                    res = "";
                if (!ok && !m_ash.AttachedClientSocket.Sendable) {
                    m_s = null;
                    return m_ash.AttachedClientSocket.ErrorMsg;
                }
                if (Progress != null)
                    Progress.Invoke(this, (ulong)m_s.Position);
                if (!m_ash.SendRequest(CStreamSerializationHelper.idDownloadCompleted, (dc) => {
                    lock (m_cs) {
                        if (Progress != null)
                            Progress.Invoke(this, (ulong)m_s.Position);
                        m_s = null;
                    }
                    m_ash.ResultReturned -= DataFromServerToClient;
                })) {
                    m_s = null;
                    return m_ash.AttachedClientSocket.ErrorMsg;
                }
            }
            return res;
        }

        private object m_cs = new object();
        private Stream m_s; //protected by m_cs
        private ulong SendDataFromClientToServer() {
            if (m_ash.AttachedClientSocket.BytesInSendingBuffer > CStreamSerializationHelper.STREAM_CHUNK_SIZE)
                return 0;
            ulong send = 0;
            using (CScopeUQueue su = new CScopeUQueue()) {
                if (m_s == null)
                    return 0;
                uint read = CStreamSerializationHelper.Read(m_s, su.UQueue);
                while (read > 0) {
                    bool ok = m_ash.SendRequest(CStreamSerializationHelper.idWriteDataFromClientToServer, su.UQueue.m_bytes, read, (ar) => {
                        SendDataFromClientToServer();
                    });
                    if (Progress != null)
                        Progress.Invoke(this, (ulong)m_s.Position);

                    if (!ok) {
                        m_s = null;
                        break;
                    }
                    send += read;
                    if (m_ash.AttachedClientSocket.BytesInSendingBuffer > 10 * CStreamSerializationHelper.STREAM_CHUNK_SIZE)
                        break;
                    read = CStreamSerializationHelper.Read(m_s, su.UQueue);
                    if (read == 0) {
                        if (!m_ash.SendRequest(CStreamSerializationHelper.idUploadCompleted, (dc) => {
                            lock (m_cs) {
                                if (Progress != null)
                                    Progress.Invoke(this, (ulong)m_s.Position);
                                m_s = null;
                            }
                        }))
                            m_s = null;
                    }
                }
            }
            return send;
        }

        /// <summary>
        /// Send a stream from client to remote server
        /// </summary>
        /// <param name="source">A source stream at client side</param>
        /// <param name="RemotePath">A string sent to server for a file name or other object which will receive this stream data</param>
        /// <returns>An empty string if successful. Otherwise, an error message if failed</returns>
        public string Upload(Stream source, string RemotePath) {
            //remove any exitsing DataFromServerToClient delegate
            m_ash.ResultReturned -= DataFromServerToClient;
            lock (m_cs) {
                if (m_s != null)
                    throw new InvalidOperationException("A stream during transaction");
            }
            if (source == null || !source.CanRead)
                throw new InvalidOperationException("A readable source stream required");
            string res = "";
            bool ok = (m_ash.SendRequest(CStreamSerializationHelper.idStartUploading, RemotePath, (ar) => {
                ar.Load(out res);
            }) && m_ash.WaitAll());
            if (res != null && res.Length > 0)
                return res;
            if (!ok && !m_ash.AttachedClientSocket.Sendable)
                return m_ash.AttachedClientSocket.ErrorMsg;
            lock (m_cs) {
                if (m_s != null)
                    throw new InvalidOperationException("A stream during transaction");
                m_s = source;
                if (Progress != null)
                    Progress.Invoke(this, (ulong)m_s.Position);
                if (SendDataFromClientToServer() == 0) {
                    if (!m_ash.SendRequest(CStreamSerializationHelper.idUploadCompleted, (dc) => {
                        lock (m_cs) {
                            if (Progress != null && m_s != null)
                                Progress.Invoke(this, (ulong)m_s.Position);
                            m_s = null;
                        }
                    })) {
                        m_s = null;
                        if (!m_ash.AttachedClientSocket.Sendable)
                            return m_ash.AttachedClientSocket.ErrorMsg;
                    }
                }
            }
            return res;
        }

        /// <summary>
        /// Hosting service handler
        /// </summary>
        public CAsyncServiceHandler AsyncServiceHandler {
            get {
                return m_ash;
            }
        }
        private CAsyncServiceHandler m_ash;
    }

    public class CStreamingFile : CAsyncServiceHandler {
        public const uint sidFile = BaseServiceID.sidFile; //asynchronous file streaming service id

        public delegate void DDownload(CStreamingFile file, int res, string errMsg);
        public delegate void DUpload(CStreamingFile file, int res, string errMsg);
        public delegate void DTransferring(CStreamingFile file, long transferred);

        public const uint STREAM_CHUNK_SIZE = 10240;

        //request ids
        public const ushort idDownload = 0x7F70;
        public const ushort idStartDownloading = 0x7F71;
        public const ushort idDownloading = 0x7F72;
        public const ushort idUpload = 0x7F73;
        public const ushort idUploading = 0x7F74;
        public const ushort idUploadCompleted = 0x7F75;

        //file open flags
        public const uint FILE_OPEN_TRUNCACTED = 1;
        public const uint FILE_OPEN_APPENDED = 2;
        public const uint FILE_OPEN_SHARE_READ = 4;
        public const uint FILE_OPEN_SHARE_WRITE = 8;

        //error code
        public const int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1;
        public const int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2;

        public CStreamingFile()
            : base(sidFile) {
        }

        /// <summary>
        /// You may use the protected constructor when extending this class
        /// </summary>
        /// <param name="sid">A service id</param>
        protected CStreamingFile(uint sid)
            : base(sid) {
        }

        private class CContext {
            public CContext(bool uplaod, uint flags) {
                Uploading = uplaod;
                Flags = flags;
            }
            public bool Uploading;
            public long FileSize = -1;
            public uint Flags;
            public bool Sent = false;
            public string LocalFile = "";
            public string FilePath = "";
            public DDownload Download = null;
            public DUpload Upload = null;
            public DTransferring Transferring = null;
            public DDiscarded Discarded = null;
            public FileStream File = null;
            public bool Tried = false;
            public string ErrMsg = "";
            public bool QueueOk = false;
        };

        protected object m_csFile = new object();
        private Deque<CContext> m_vContext = new Deque<CContext>(); //protected by m_csFile;

        public override uint CleanCallbacks() {
            lock (m_csFile) {
                foreach (CContext c in m_vContext) {
                    if (c.File != null) {
                        c.File.Close();
                        if (!c.Uploading) {
                            try {
                                System.IO.File.Delete(c.LocalFile);
                            } finally { }
                        }
                    }
                }
                m_vContext.Clear();
            }
            return base.CleanCallbacks();
        }

        /// <summary>
        /// The number of files queued
        /// </summary>
        public uint FilesQueued {
            get {
                lock (m_csFile) {
                    return (uint)m_vContext.Count;
                }
            }
        }

        /// <summary>
        /// The file size in bytes for current file being in transaction
        /// </summary>
        public long FileSize {
            get {
                lock (m_csFile) {
                    if (m_vContext.Count == 0)
                        return -1;
                    return m_vContext[0].FileSize;
                }
            }
        }

        /// <summary>
        /// Local file name of current file being in transaction
        /// </summary>
        public string LocalFile {
            get {
                lock (m_csFile) {
                    if (m_vContext.Count == 0)
                        return null;
                    return m_vContext[0].LocalFile;
                }
            }
        }

        /// <summary>
        /// Remote file name of current file being in transaction
        /// </summary>
        public string RemoteFile {
            get {
                lock (m_csFile) {
                    if (m_vContext.Count == 0)
                        return null;
                    return m_vContext[0].FilePath;
                }
            }
        }

        protected override void OnMergeTo(CAsyncServiceHandler to) {
            CStreamingFile fTo = (CStreamingFile)to;
            lock (fTo.m_csFile) {
                lock (m_csFile) {
                    fTo.m_vContext.InsertRange(fTo.m_vContext.Count, m_vContext);
                    m_vContext.Clear();
                }
            }
        }

        public bool Upload(string localFile, string remoteFile) {
            return Upload(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up) {
            return Upload(localFile, remoteFile, up, null, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans) {
            return Upload(localFile, remoteFile, up, trans, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans, DDiscarded discarded) {
            return Upload(localFile, remoteFile, up, trans, discarded, FILE_OPEN_TRUNCACTED);
        }

        public virtual bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, uint flags) {
            if (localFile == null || localFile.Length == 0)
                return false;
            if (remoteFile == null || remoteFile.Length == 0)
                return false;
            CContext context = new CContext(true, flags);
            context.Upload = up;
            context.Transferring = trans;
            context.Discarded = discarded;
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            lock (m_csFile) {
                m_vContext.AddToBack(context);
                return Transfer();
            }
        }

        public bool Download(string localFile, string remoteFile) {
            return Download(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl) {
            return Download(localFile, remoteFile, dl, null, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans) {
            return Download(localFile, remoteFile, dl, trans, null, FILE_OPEN_TRUNCACTED);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded) {
            return Download(localFile, remoteFile, dl, trans, discarded, FILE_OPEN_TRUNCACTED);
        }

        public virtual bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, uint flags) {
            if (localFile == null || localFile.Length == 0)
                return false;
            if (remoteFile == null || remoteFile.Length == 0)
                return false;
            CContext context = new CContext(false, flags);
            context.Download = dl;
            context.Transferring = trans;
            context.Discarded = discarded;
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            lock (m_csFile) {
                m_vContext.AddToBack(context);
                return Transfer();
            }
        }

        protected override void OnResultReturned(ushort reqId, CUQueue mc) {
            switch (reqId) {
                case idDownload: {
                        int res;
                        string errMsg;
                        mc.Load(out res).Load(out errMsg);
                        DDownload dl;
                        lock (m_csFile) {
                            CContext context = m_vContext[0];
                            if (context.File != null) {
                                context.File.Close();
                                context.File = null;
                            } else if (res == 0) {
                                res = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                                errMsg = context.ErrMsg;
                            }
                            dl = context.Download;
                        }
                        if (dl != null)
                            dl(this, res, errMsg);
                        lock (m_csFile) {
                            m_vContext.RemoveFromFront();
                        }
                    }
                    break;
                case idStartDownloading:
                    lock (m_csFile) {
                        CContext context = m_vContext[0];
                        mc.Load(out context.FileSize);
                        try {
                            FileMode fm;
                            if ((context.Flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED)
                                fm = FileMode.Create;
                            else if ((context.Flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED)
                                fm = FileMode.Append;
                            else
                                fm = FileMode.OpenOrCreate;
                            FileShare fs = FileShare.None;
                            if ((context.Flags & FILE_OPEN_SHARE_WRITE) == FILE_OPEN_SHARE_WRITE)
                                fs = FileShare.Write;
                            context.File = new FileStream(context.LocalFile, fm, FileAccess.Write, fs);
                        } catch (Exception err) {
                            context.ErrMsg = err.Message;
                        } finally { }
                    }
                    break;
                case idDownloading: {
                        long downloaded = -1;
                        DTransferring trans = null;
                        lock (m_cs) {
                            CContext context = m_vContext[0];
                            trans = context.Transferring;
                            if (context.File != null) {
                                byte[] buffer = mc.IntenalBuffer;
                                context.File.Write(buffer, 0, (int)mc.GetSize());
                                downloaded = context.File.Position;
                            }
                        }
                        mc.SetSize(0);
                        if (trans != null)
                            trans(this, downloaded);
                    }
                    break;
                case idUpload: {
                        bool removed = false;
                        DUpload upl = null;
                        int res;
                        string errMsg;
                        mc.Load(out res).Load(out errMsg);
                        if (res != 0) {
                            lock (m_csFile) {
                                CContext context = m_vContext[0];
                                removed = true;
                                upl = context.Upload;
                                if (context.File != null) {
                                    context.File.Close();
                                }
                            }
                        }
                        if (upl != null)
                            upl(this, res, errMsg);
                        if (removed) {
                            lock (m_csFile) {
                                m_vContext.RemoveFromFront();
                            }
                        }
                    }
                    break;
                case idUploading: {
                        DTransferring trans = null;
                        long uploaded;
                        mc.Load(out uploaded);
                        if (uploaded > 0) {
                            lock (m_csFile) {
                                CContext context = m_vContext[0];
                                trans = context.Transferring;
                            }
                        }
                        if (trans != null)
                            trans(this, uploaded);
                    }
                    break;
                case idUploadCompleted: {
                        DUpload upl = null;
                        lock (m_csFile) {
                            CContext context = m_vContext[0];
                            upl = context.Upload;
                            if (context.File != null) {
                                context.File.Close();
                                context.File = null;
                            }
                        }
                        if (upl != null)
                            upl(this, 0, "");
                        lock (m_csFile) {
                            m_vContext.RemoveFromFront();
                        }
                    }
                    break;
                default:
                    base.OnResultReturned(reqId, mc);
                    break;
            }
            lock (m_csFile) {
                Transfer();
            }
        }

        private bool Transfer() {
            int index = 0;
            DAsyncResultHandler rh = null;
            DOnExceptionFromServer se = null;
            CClientSocket cs = AttachedClientSocket;
            if (!cs.Sendable)
                return false;
            uint sent_buffer_size = cs.BytesInSendingBuffer;
            if (sent_buffer_size > 3 * STREAM_CHUNK_SIZE)
                return true;
            while (index < m_vContext.Count) {
                CContext context = m_vContext[index];
                if (context.Sent) {
                    ++index;
                    return true;
                }
                if (context.Uploading && context.Tried && context.File == null) {
                    if (index == 0) {
                        if (context.Upload != null) {
                            context.Upload(this, CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                        }
                        m_vContext.RemoveFromFront();
                    } else {
                        ++index;
                    }
                    continue;
                }
                if (context.Uploading) {
                    if (!context.Tried) {
                        context.Tried = true;
                        try {
                            FileShare fs = FileShare.None;
                            if ((context.Flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ)
                                fs = FileShare.Read;
                            context.File = new FileStream(context.LocalFile, FileMode.Open, FileAccess.Read, fs);
                            context.FileSize = context.File.Length;
                            context.QueueOk = AttachedClientSocket.ClientQueue.StartJob();
                            if (!SendRequest(idUpload, context.FilePath, context.Flags, context.FileSize, rh, context.Discarded, se))
                                return false;
                        } catch (Exception err) {
                            context.ErrMsg = err.Message;
                        } finally { }
                    }
                    if (context.File == null) {
                        if (index == 0) {
                            if (context.Upload != null) {
                                context.Upload(this, CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                            }
                            m_vContext.RemoveFromFront();
                        } else
                            ++index;
                        continue;
                    } else {
                        using (CScopeUQueue sb = new CScopeUQueue()) {
                            if (sb.UQueue.MaxBufferSize < STREAM_CHUNK_SIZE)
                                sb.UQueue.Realloc(STREAM_CHUNK_SIZE);
                            byte[] buffer = sb.UQueue.IntenalBuffer;
                            int ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                            while (ret > 0) {
                                if (!SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se))
                                    return false;
                                sent_buffer_size = cs.BytesInSendingBuffer;
                                if (ret < (int)STREAM_CHUNK_SIZE)
                                    break;
                                if (sent_buffer_size >= 5 * STREAM_CHUNK_SIZE)
                                    break;
                                ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                            }
                            if (ret < (int)STREAM_CHUNK_SIZE) {
                                context.Sent = true;
                                if (!SendRequest(idUploadCompleted, rh, context.Discarded, se))
                                    return false;
                                if (context.QueueOk) {
                                    AttachedClientSocket.ClientQueue.EndJob();
                                    context.QueueOk = false;
                                }
                            }
                            if (sent_buffer_size >= 4 * STREAM_CHUNK_SIZE)
                                break;
                        }
                    }
                } else {
                    if (!SendRequest(idDownload, context.FilePath, context.Flags, rh, context.Discarded, se))
                        return false;
                    context.Sent = true;
                    context.Tried = true;
                    sent_buffer_size = cs.BytesInSendingBuffer;
                    if (sent_buffer_size > 3 * STREAM_CHUNK_SIZE)
                        break;
                }
                ++index;
            }
            return true;
        }
    }
}

