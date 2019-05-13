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
            public string ErrMsg = "";
            public bool QueueOk = false;
            public int ErrCode = 0;
        };

        protected object m_csFile = new object();
        private Deque<CContext> m_vContext = new Deque<CContext>(); //protected by m_csFile;

        private static void CloseFile(CContext c) {
            if (c.File != null) {
                c.File.Close();
                if (!c.Uploading && (c.ErrCode != 0 || (c.ErrMsg != null && c.ErrMsg.Length > 0))) {
                    try {
                        System.IO.File.Delete(c.LocalFile);
                    } catch { } finally { }
                }
            }
        }

        private void OpenLocalRead(CContext context) {
            try {
                FileShare fs = FileShare.None;
                if ((context.Flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ)
                    fs = FileShare.Read;
                context.File = new FileStream(context.LocalFile, FileMode.Open, FileAccess.Read, fs);
                context.FileSize = context.File.Length;
            } catch (Exception err) {
                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                context.ErrMsg = err.Message;
            } finally { }
        }

        private void OpenLocalWrite(CContext context) {
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
                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                context.ErrMsg = err.Message;
            } finally { }
        }

        public override uint CleanCallbacks() {
            lock (m_csFile) {
                foreach (CContext c in m_vContext) {
                    CloseFile(c);
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
                if (m_vContext.Count == 1) {
                    ClientCoreLoader.PostProcessing(AttachedClientSocket.Handle, 0, 0);
                    AttachedClientSocket.DoEcho(); //make sure WaitAll works correctly
                }
            }
            return true;
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
                if (m_vContext.Count == 1) {
                    ClientCoreLoader.PostProcessing(AttachedClientSocket.Handle, 0, 0);
                    AttachedClientSocket.DoEcho(); //make sure WaitAll works correctly
                }
            }
            return true;
        }

        protected override void OnPostProcessing(uint hint, ulong data) {
            CContext ctx = new CContext(false, 0);
            lock (m_csFile) {
                if (m_vContext.Count > 0) {
                    CContext context = m_vContext[0];
                    if (context.Uploading) {
                        OpenLocalRead(context);
                    } else {
                        OpenLocalWrite(context);
                    }
                    DAsyncResultHandler rh = null;
                    DOnExceptionFromServer se = null;
                    if (context.ErrCode != 0 || (context.ErrMsg != null && context.ErrMsg.Length > 0)) {
                        ctx = m_vContext.RemoveFromFront();
                        if (m_vContext.Count > 0) {
                            ClientCoreLoader.PostProcessing(AttachedClientSocket.Handle, 0, 0);
                            AttachedClientSocket.DoEcho(); //make sure WaitAll works correctly
                        }
                    } else if (context.Uploading) {
                        SendRequest(idUpload, context.FilePath, context.Flags, context.FileSize, rh, context.Discarded, se);
                    } else {
                        SendRequest(idDownload, context.FilePath, context.Flags, rh, context.Discarded, se);
                    }
                }
            }
            if (ctx.ErrCode != 0 || (ctx.ErrMsg != null && ctx.ErrMsg.Length > 0)) {
                CloseFile(ctx);
                if (ctx.Download != null) {
                    ctx.Download(this, ctx.ErrCode, ctx.ErrMsg);
                }
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
                            dl = m_vContext[0].Download;
                        }
                        if (dl != null) {
                            dl.Invoke(this, res, errMsg);
                        }
                        lock (m_cs) {
                            CContext context = m_vContext.RemoveFromFront();
                            CloseFile(context);
                        }
                        OnPostProcessing(0, 0);
                    }
                    break;
                case idStartDownloading:
                    lock (m_csFile) {
                        CContext context = m_vContext[0];
                        mc.Load(out context.FileSize);
                    }
                    break;
                case idDownloading: {
                        long downloaded = 0;
                        DTransferring trans = null;
                        CContext context = null;
                        lock (m_cs) {
                            context = m_vContext[0];
                            trans = context.Transferring;
                            byte[] buffer = mc.IntenalBuffer;
                            try {
                                context.File.Write(buffer, 0, (int)mc.GetSize());
                                downloaded = context.File.Position;
                            } catch (System.IO.IOException err) {
                                context.ErrMsg = err.Message;
#if SP_MANAGER
                                context.ErrCode = err.HResult;
#else
                                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
#endif
                            }

                        }
                        mc.SetSize(0);
                        if (context.ErrCode != 0 || (context.ErrMsg != null && context.ErrMsg.Length > 0)) {
                            CloseFile(context);
                            if (context.Download != null) {
                                context.Download.Invoke(this, context.ErrCode, context.ErrMsg);
                            }
                            m_vContext.RemoveFromFront();
                            OnPostProcessing(0, 0);
                        } else if (trans != null) {
                            trans.Invoke(this, downloaded);
                        }
                    }
                    break;
                case idUpload: {
                        CContext context = null;
                        int res;
                        string errMsg;
                        mc.Load(out res).Load(out errMsg);
                        if (res != 0 || (errMsg != null && errMsg.Length > 0)) {
                            lock (m_csFile) {
                                context = m_vContext[0];
                                context.ErrCode = res;
                                context.ErrMsg = errMsg;
                            }
                        } else {
                            lock (m_csFile) {
                                bool automerge = (ClientCoreLoader.GetQueueAutoMergeByPool(AttachedClientSocket.PoolId) != 0);
                                context = m_vContext[0];
                                using (CScopeUQueue sb = new CScopeUQueue()) {
                                    DAsyncResultHandler rh = null;
                                    DOnExceptionFromServer se = null;
                                    if (sb.UQueue.MaxBufferSize < STREAM_CHUNK_SIZE)
                                        sb.UQueue.Realloc(STREAM_CHUNK_SIZE);
                                    byte[] buffer = sb.UQueue.IntenalBuffer;
                                    context.QueueOk = AttachedClientSocket.ClientQueue.StartJob();
                                    try {
                                        int ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                        while (ret == STREAM_CHUNK_SIZE) {
                                            SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se);
                                            ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                            if (context.QueueOk) {
                                                if (automerge) {
                                                } else if (AttachedClientSocket.ConnectionState > tagConnectionState.csConnected) {
                                                    uint pending = AttachedClientSocket.ClientQueue.MessagesInDequeuing;
                                                    ulong jobsize = AttachedClientSocket.ClientQueue.JobSize;
                                                    ulong msg = AttachedClientSocket.ClientQueue.MessageCount;
                                                    if (msg + jobsize - pending > 80) {
                                                        break;
                                                    }
                                                }
                                            } else if (AttachedClientSocket.BytesInSendingBuffer > 40 * STREAM_CHUNK_SIZE || AttachedClientSocket.ConnectionState < tagConnectionState.csConnected) {
                                                break;
                                            }
                                        }
                                        if (ret > 0) {
                                            SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se);
                                        }
                                        if (ret < STREAM_CHUNK_SIZE) {
                                            context.Sent = true;
                                            SendRequest(idUploadCompleted, rh, context.Discarded, se);
                                            if (context.QueueOk) {
                                                AttachedClientSocket.ClientQueue.EndJob();
                                            }
                                        }
                                    } catch (System.IO.IOException err) {
                                        errMsg = err.Message;
#if SP_MANAGER
                                        res = err.HResult;
#else
                                        res = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
#endif
                                        context.ErrCode = res;
                                        context.ErrMsg = errMsg;
                                    }
                                }
                            }
                        }
                        if (res != 0 || (errMsg != null && errMsg.Length > 0)) {
                            CloseFile(context);
                            if (context.Upload != null) {
                                context.Upload.Invoke(this, res, errMsg);
                            }
                            lock (m_csFile) {
                                context = m_vContext.RemoveFromFront();
                            }
                            OnPostProcessing(0, 0);
                        }
                    }
                    break;
                case idUploading: {
                        CContext context = null;
                        DTransferring trans = null;
                        long uploaded;
                        mc.Load(out uploaded);
                        lock (m_csFile) {
                            context = m_vContext[0];
                            trans = context.Transferring;
                            if (!context.Sent) {
                                using (CScopeUQueue sb = new CScopeUQueue()) {
                                    DAsyncResultHandler rh = null;
                                    DOnExceptionFromServer se = null;
                                    if (sb.UQueue.MaxBufferSize < STREAM_CHUNK_SIZE)
                                        sb.UQueue.Realloc(STREAM_CHUNK_SIZE);
                                    byte[] buffer = sb.UQueue.IntenalBuffer;
                                    try {
                                        int ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                        if (ret > 0) {
                                            SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se);
                                        }
                                        if (ret < STREAM_CHUNK_SIZE) {
                                            context.Sent = true;
                                            SendRequest(idUploadCompleted, rh, context.Discarded, se);
                                            if (context.QueueOk) {
                                                AttachedClientSocket.ClientQueue.EndJob();
                                            }
                                        }
                                    } catch (System.IO.IOException err) {
                                        context.ErrMsg = err.Message;
#if SP_MANAGER
                                        context.ErrCode = err.HResult;
#else
                                        context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
#endif
                                    }

                                }
                            }
                        }
                        if (context.ErrCode != 0 || (context.ErrMsg != null && context.ErrMsg.Length > 0)) {
                            CloseFile(context);
                            if (context.Upload != null) {
                                context.Upload.Invoke(this, context.ErrCode, context.ErrMsg);
                            }
                            lock (m_csFile) {
                                m_vContext.RemoveFromFront();
                            }
                            OnPostProcessing(0, 0);
                        } else if (trans != null) {
                            trans.Invoke(this, uploaded);
                        }
                    }
                    break;
                case idUploadCompleted: {
                        DUpload upl = null;
                        lock (m_csFile) {
                            upl = m_vContext[0].Upload;
                        }
                        if (upl != null) {
                            upl.Invoke(this, 0, "");
                        }
                        lock (m_csFile) {
                            CContext context = m_vContext.RemoveFromFront();
                            CloseFile(context);
                        }
                        OnPostProcessing(0, 0);
                    }
                    break;
                default:
                    base.OnResultReturned(reqId, mc);
                    break;
            }
        }


    }
}

