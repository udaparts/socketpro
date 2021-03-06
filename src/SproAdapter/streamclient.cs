﻿using System;
using System.IO;
#if TASKS_ENABLED
using System.Threading.Tasks;
#endif

namespace SocketProAdapter.ClientSide
{
    [ObsoleteAttribute]
    public class CStreamHelper
    {
        public delegate void DProgress(CStreamHelper sender, ulong pos);
        public event DProgress Progress;

        public CStreamHelper(CAsyncServiceHandler ash)
        {
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
        public void Reset()
        {
            lock (m_cs)
            {
                m_s = null;
            }
        }

        private bool DataFromServerToClient(CAsyncServiceHandler sender, ushort reqId, CUQueue qData)
        {
            bool processed = false;
            switch (reqId)
            {
                case CStreamSerializationHelper.idReadDataFromServerToClient:
                    if (qData.GetSize() > 0)
                    {
                        lock (m_cs)
                        {
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
        public string Download(Stream receiver, string RemotePath)
        {
            lock (m_cs)
            {
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
            bool ok = (m_ash.SendRequest(CStreamSerializationHelper.idStartDownloading, RemotePath, (ar) =>
            {
                ar.Load(out m_nDownloadingFileSize).Load(out res);
            }) && m_ash.WaitAll());
            lock (m_cs)
            {
                if (res != null && res.Length > 0)
                {
                    m_s = null;
                    return res;
                }
                else if (res == null)
                    res = "";
                if (!ok && !m_ash.Socket.Sendable)
                {
                    m_s = null;
                    return m_ash.Socket.ErrorMsg;
                }
                if (Progress != null)
                    Progress.Invoke(this, (ulong)m_s.Position);
                if (!m_ash.SendRequest(CStreamSerializationHelper.idDownloadCompleted, (dc) =>
                {
                    lock (m_cs)
                    {
                        if (Progress != null)
                            Progress.Invoke(this, (ulong)m_s.Position);
                        m_s = null;
                    }
                    m_ash.ResultReturned -= DataFromServerToClient;
                }))
                {
                    m_s = null;
                    return m_ash.Socket.ErrorMsg;
                }
            }
            return res;
        }

        private object m_cs = new object();
        private Stream m_s; //protected by m_cs
        private ulong SendDataFromClientToServer()
        {
            if (m_ash.Socket.BytesInSendingBuffer > CStreamSerializationHelper.STREAM_CHUNK_SIZE)
                return 0;
            ulong send = 0;
            using (CScopeUQueue su = new CScopeUQueue())
            {
                if (m_s == null)
                    return 0;
                uint read = CStreamSerializationHelper.Read(m_s, su.UQueue);
                while (read > 0)
                {
                    bool ok = m_ash.SendRequest(CStreamSerializationHelper.idWriteDataFromClientToServer, su.UQueue.m_bytes, read, (ar) =>
                    {
                        SendDataFromClientToServer();
                    });
                    if (Progress != null)
                        Progress.Invoke(this, (ulong)m_s.Position);

                    if (!ok)
                    {
                        m_s = null;
                        break;
                    }
                    send += read;
                    if (m_ash.Socket.BytesInSendingBuffer > 10 * CStreamSerializationHelper.STREAM_CHUNK_SIZE)
                        break;
                    read = CStreamSerializationHelper.Read(m_s, su.UQueue);
                    if (read == 0)
                    {
                        if (!m_ash.SendRequest(CStreamSerializationHelper.idUploadCompleted, (dc) =>
                        {
                            lock (m_cs)
                            {
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
        public string Upload(Stream source, string RemotePath)
        {
            //remove any exitsing DataFromServerToClient delegate
            m_ash.ResultReturned -= DataFromServerToClient;
            lock (m_cs)
            {
                if (m_s != null)
                    throw new InvalidOperationException("A stream during transaction");
            }
            if (source == null || !source.CanRead)
                throw new InvalidOperationException("A readable source stream required");
            string res = "";
            bool ok = (m_ash.SendRequest(CStreamSerializationHelper.idStartUploading, RemotePath, (ar) =>
            {
                ar.Load(out res);
            }) && m_ash.WaitAll());
            if (res != null && res.Length > 0)
                return res;
            if (!ok && !m_ash.Socket.Sendable)
                return m_ash.Socket.ErrorMsg;
            lock (m_cs)
            {
                if (m_s != null)
                    throw new InvalidOperationException("A stream during transaction");
                m_s = source;
                if (Progress != null)
                    Progress.Invoke(this, (ulong)m_s.Position);
                if (SendDataFromClientToServer() == 0)
                {
                    if (!m_ash.SendRequest(CStreamSerializationHelper.idUploadCompleted, (dc) =>
                    {
                        lock (m_cs)
                        {
                            if (Progress != null && m_s != null)
                                Progress.Invoke(this, (ulong)m_s.Position);
                            m_s = null;
                        }
                    }))
                    {
                        m_s = null;
                        if (!m_ash.Socket.Sendable)
                            return m_ash.Socket.ErrorMsg;
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

    public class CStreamingFile : CAsyncServiceHandler
    {
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
        public const ushort idUploadBackup = 0x7F76;

        //file open flags
        public const uint FILE_OPEN_TRUNCACTED = 1;
        public const uint FILE_OPEN_APPENDED = 2;
        public const uint FILE_OPEN_SHARE_READ = 4;
        public const uint FILE_OPEN_SHARE_WRITE = 8;

        //error code
        public const int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1;
        public const int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2;
        public const int FILE_BAD_OPERATION = -3;
        public const int FILE_DOWNLOADING_INTERRUPTED = -4;

        public CStreamingFile()
            : base(sidFile)
        {
        }

        /// <summary>
        /// You may use the protected constructor when extending this class
        /// </summary>
        /// <param name="sid">A service id</param>
        protected CStreamingFile(uint sid)
            : base(sid)
        {
        }

        private class CContext
        {
            public CContext(bool uplaod, uint flags)
            {
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
            public long InitSize = -1;
            public DOnExceptionFromServer Se = null;
#if TASKS_ENABLED
            public TaskCompletionSource<ErrInfo> Fut = null;
#endif
            public bool HasError {
                get {
                    return (ErrCode != 0 || ErrMsg.Length > 0);
                }
            }
        };

        protected object m_csFile = new object();
        private Deque<CContext> m_vContext = new Deque<CContext>(); //protected by m_csFile;

        /// <summary>
        /// Cancel transferring files queued in memory
        /// </summary>
        /// <returns>the number of transferring files canceled</returns>
        public uint Cancel()
        {
            uint canceled = 0;
            lock (m_csFile)
            {
                while (m_vContext.Count > 0)
                {
                    var back = m_vContext[m_vContext.Count - 1];
                    if (back.File != null)
                    {
                        //Send an interrupt request onto server to shut down downloading as earlier as possible
                        Interrupt(CStreamingFile.DEFAULT_INTERRUPT_OPTION);
                        break;
                    }
                    if (back.Discarded != null)
                    {
                        try
                        {
                            System.Threading.Monitor.Exit(m_csFile);
                            back.Discarded(this, true);
                        }
                        finally
                        {
                            System.Threading.Monitor.Enter(m_csFile);
                        }
                    }
                    m_vContext.RemoveFromBack();
                    ++canceled;
                }
            }
            return canceled;
        }

        private static void CloseFile(CContext c)
        {
            if (c.File != null)
            {
                if (!c.Uploading && c.HasError)
                {
                    if (c.InitSize == -1)
                    {
                        c.File.Close();
                        try
                        {
                            System.IO.File.Delete(c.LocalFile);
                        }
                        catch { }
                    }
                    else
                    {
                        try
                        {
                            c.File.Flush();
                            c.File.SetLength(c.InitSize);
                        }
                        catch { }
                        finally
                        {
                            c.File.Close();
                        }
                    }
                }
                else
                {
                    c.File.Close();
                }
                c.File = null;
            }
        }

        private void OpenLocalRead(CContext context)
        {
            try
            {
                FileShare fs = FileShare.None;
                if ((context.Flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ)
                    fs = FileShare.Read;
                context.File = new FileStream(context.LocalFile, FileMode.Open, FileAccess.Read, fs);
                context.FileSize = context.File.Length;
            }
            catch (Exception err)
            {
                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                context.ErrMsg = err.Message;
            }
        }

        private void OpenLocalWrite(CContext context)
        {
            bool existing = File.Exists(context.LocalFile);
            if (existing)
            {
                context.InitSize = 0;
            }
            else
            {
                context.InitSize = -1;
            }
            try
            {
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
                if ((context.Flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED)
                {
                    context.InitSize = context.File.Position;
                }
            }
            catch (Exception err)
            {
                context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                context.ErrMsg = err.Message;
            }
        }

        public override uint CleanCallbacks()
        {
            lock (m_csFile)
            {
                foreach (CContext c in m_vContext)
                {
                    if (c.File != null)
                    {
                        c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                        c.ErrMsg = "Clean local writing file";
                        CloseFile(c);
                    }
                    else
                    {
                        break;
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
                lock (m_csFile)
                {
                    return (uint)m_vContext.Count;
                }
            }
        }

        private uint m_MaxDownloading = 1;

        public readonly uint MAX_FILES_STREAMED = 32;

        /// <summary>
        /// The max number of files streamed
        /// </summary>
        public uint FilesStreamed {
            get {
                lock (m_csFile)
                {
                    return m_MaxDownloading;
                }
            }
            set {
                lock (m_csFile)
                {
                    if (value == 0)
                    {
                        m_MaxDownloading = 1;
                    }
                    else if (value > MAX_FILES_STREAMED)
                    {
                        m_MaxDownloading = MAX_FILES_STREAMED;
                    }
                    else
                    {
                        m_MaxDownloading = value;
                    }
                }
            }
        }

        /// <summary>
        /// The file size in bytes for current file being in transaction
        /// </summary>
        public long FileSize {
            get {
                lock (m_csFile)
                {
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
                lock (m_csFile)
                {
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
                lock (m_csFile)
                {
                    if (m_vContext.Count == 0)
                        return null;
                    return m_vContext[0].FilePath;
                }
            }
        }

        protected override void OnMergeTo(CAsyncServiceHandler to)
        {
            CStreamingFile fTo = (CStreamingFile)to;
            lock (fTo.m_csFile)
            {
                int pos = 0;
                int count = fTo.m_vContext.Count;
                foreach (CContext cxt in fTo.m_vContext)
                {
                    if (cxt.ErrCode == 0 && cxt.ErrMsg.Length == 0 && cxt.File == null)
                    {
                        break;
                    }
                    ++pos;
                }
                lock (m_csFile)
                {
                    fTo.m_vContext.InsertRange(pos, m_vContext);
                    m_vContext.Clear();
                }
                if (count == 0 && fTo.m_vContext.Count > 0)
                {
                    ClientCoreLoader.PostProcessing(fTo.Socket.Handle, 0, 0);
                    fTo.Socket.DoEcho(); //make sure WaitAll works correctly
                }
            }
        }

        public bool Upload(string localFile, string remoteFile)
        {
            return Upload(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up)
        {
            return Upload(localFile, remoteFile, up, null, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans)
        {
            return Upload(localFile, remoteFile, up, trans, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans, DDiscarded discarded)
        {
            return Upload(localFile, remoteFile, up, trans, discarded, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, uint flags)
        {
            return Upload(localFile, remoteFile, up, trans, discarded, flags, null);
        }

        public virtual bool Upload(string localFile, string remoteFile, DUpload up, DTransferring trans, DDiscarded discarded, uint flags, DOnExceptionFromServer se)
        {
            if (localFile == null || localFile.Length == 0)
                throw new ArgumentException("localFile cannot be empty");
            if (remoteFile == null || remoteFile.Length == 0)
                throw new ArgumentException("remoteFile cannot be empty");
            CContext context = new CContext(true, flags);
            context.Upload = up;
            context.Transferring = trans;
            context.Discarded = discarded;
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            context.Se = se;
            lock (m_csFile)
            {
                m_vContext.AddToBack(context);
                uint filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened)
                {
                    ClientCoreLoader.PostProcessing(Socket.Handle, 0, 0);
                    if (filesOpened == 0)
                    {
                        Socket.DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }
            return true;
        }

#if TASKS_ENABLED
        public virtual Task<ErrInfo> download(string localFile, string remoteFile, DTransferring trans = null, uint flags = FILE_OPEN_TRUNCACTED)
        {
            if (localFile == null || localFile.Length == 0)
                throw new ArgumentException("localFile cannot be empty");
            if (remoteFile == null || remoteFile.Length == 0)
                throw new ArgumentException("remoteFile cannot be empty");
            TaskCompletionSource<ErrInfo> tcs = new TaskCompletionSource<ErrInfo>();
            CContext context = new CContext(false, flags);
            context.Download = (file, res, em) =>
            {
                tcs.TrySetResult(new ErrInfo(res, em));
            };
            context.Transferring = trans;
            context.Discarded = get_aborted(tcs, idDownload);
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            context.Se = get_se(tcs);
            context.Fut = tcs;
            lock (m_csFile)
            {
                m_vContext.AddToBack(context);
                uint filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened)
                {
                    ClientCoreLoader.PostProcessing(Socket.Handle, 0, 0);
                    if (filesOpened == 0)
                    {
                        Socket.DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }
            return tcs.Task;
        }

        public virtual Task<ErrInfo> upload(string localFile, string remoteFile, DTransferring trans = null, uint flags = FILE_OPEN_TRUNCACTED)
        {
            if (localFile == null || localFile.Length == 0)
                throw new ArgumentException("localFile cannot be empty");
            if (remoteFile == null || remoteFile.Length == 0)
                throw new ArgumentException("remoteFile cannot be empty");
            TaskCompletionSource<ErrInfo> tcs = new TaskCompletionSource<ErrInfo>();
            CContext context = new CContext(false, flags);
            context.Download = (file, res, em) =>
            {
                tcs.TrySetResult(new ErrInfo(res, em));
            };
            context.Transferring = trans;
            context.Discarded = get_aborted(tcs, idUpload);
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            context.Se = get_se(tcs);
            context.Fut = tcs;
            lock (m_csFile)
            {
                m_vContext.AddToBack(context);
                uint filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened)
                {
                    ClientCoreLoader.PostProcessing(Socket.Handle, 0, 0);
                    if (filesOpened == 0)
                    {
                        Socket.DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }
            return tcs.Task;
        }
#endif

        public bool Download(string localFile, string remoteFile)
        {
            return Download(localFile, remoteFile, null, null, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl)
        {
            return Download(localFile, remoteFile, dl, null, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans)
        {
            return Download(localFile, remoteFile, dl, trans, null, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded)
        {
            return Download(localFile, remoteFile, dl, trans, discarded, FILE_OPEN_TRUNCACTED, null);
        }

        public bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, uint flags)
        {
            return Download(localFile, remoteFile, dl, trans, discarded, flags, null);
        }

        private uint GetFilesOpened()
        {
            uint opened = 0;
            foreach (CContext it in m_vContext)
            {
                if (it.File != null)
                {
                    ++opened;
                }
                else if (!it.HasError)
                {
                    break;
                }
            }
            return opened;
        }

        public virtual bool Download(string localFile, string remoteFile, DDownload dl, DTransferring trans, DDiscarded discarded, uint flags, DOnExceptionFromServer se)
        {
            if (localFile == null || localFile.Length == 0)
                throw new ArgumentException("localFile cannot be empty");
            if (remoteFile == null || remoteFile.Length == 0)
                throw new ArgumentException("remoteFile cannot be empty");
            CContext context = new CContext(false, flags);
            context.Download = dl;
            context.Transferring = trans;
            context.Discarded = discarded;
            context.FilePath = remoteFile;
            context.LocalFile = localFile;
            context.Se = se;
            lock (m_csFile)
            {
                m_vContext.AddToBack(context);
                uint filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened)
                {
                    ClientCoreLoader.PostProcessing(Socket.Handle, 0, 0);
                    if (filesOpened == 0)
                    {
                        Socket.DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }
            return true;
        }

        protected override void OnPostProcessing(uint hint, ulong data)
        {
            uint d = 0;
            DAsyncResultHandler rh = null;
            System.Threading.Monitor.Enter(m_csFile);
            foreach (CContext it in m_vContext)
            {
                if (d >= m_MaxDownloading)
                {
                    break;
                }
                if (it.File != null)
                {
                    if (it.Uploading)
                    {
                        break;
                    }
                    else
                    {
                        ++d;
                        continue;
                    }
                }
                if (it.HasError)
                {
                    continue;
                }
                if (it.Uploading)
                {
                    OpenLocalRead(it);
                    if (!it.HasError)
                    {
                        if (!SendRequest(idUpload, it.FilePath, it.Flags, it.FileSize, rh, it.Discarded, it.Se))
                        {
                            CClientSocket cs = Socket;
                            it.ErrCode = cs.ErrorCode;
                            if (it.ErrCode == 0)
                            {
                                it.ErrCode = SESSION_CLOSED_BEFORE;
                                it.ErrMsg = SESSION_CLOSED_BEFORE_ERR_MSG;
                            }
                            else
                            {
                                it.ErrMsg = cs.ErrorMsg;
                            }
#if TASKS_ENABLED
                            if (it.Fut != null)
                            {
                                it.Fut.TrySetException(new CSocketError(it.ErrCode, it.ErrMsg, idUpload, true));
                                it.Discarded = null;
                                it.Se = null;
                            }
#endif
                            continue;
                        }
                        break;
                    }
                }
                else
                {
                    OpenLocalWrite(it);
                    if (!it.HasError)
                    {
                        if (!SendRequest(idDownload, it.LocalFile, it.FilePath, it.Flags, it.InitSize, rh, it.Discarded, it.Se))
                        {
                            CClientSocket cs = Socket;
                            it.ErrCode = cs.ErrorCode;
                            if (it.ErrCode == 0)
                            {
                                it.ErrCode = SESSION_CLOSED_BEFORE;
                                it.ErrMsg = SESSION_CLOSED_BEFORE_ERR_MSG;
                            }
                            else
                            {
                                it.ErrMsg = cs.ErrorMsg;
                            }
#if TASKS_ENABLED
                            if (it.Fut != null)
                            {
                                it.Fut.TrySetException(new CSocketError(it.ErrCode, it.ErrMsg, idDownload, true));
                                it.Discarded = null;
                                it.Se = null;
                            }
#endif
                        }
                        ++d;
                    }
                }
            }
            while (m_vContext.Count > 0)
            {
                CContext it = m_vContext[0];
                if (it.HasError)
                {
                    CloseFile(it);
                    if (it.Uploading)
                    {
                        DUpload cb = it.Upload;
                        if (cb != null)
                        {
                            int errCode = it.ErrCode;
                            string errMsg = it.ErrMsg;
                            try
                            {
                                System.Threading.Monitor.Exit(m_csFile);
                                cb.Invoke(this, errCode, errMsg);
                            }
                            finally
                            {
                                System.Threading.Monitor.Enter(m_csFile);
                            }
                        }
                    }
                    else
                    {
                        DDownload cb = it.Download;
                        if (cb != null)
                        {
                            int errCode = it.ErrCode;
                            string errMsg = it.ErrMsg;
                            try
                            {
                                System.Threading.Monitor.Exit(m_csFile);
                                cb.Invoke(this, errCode, errMsg);
                            }
                            finally
                            {
                                System.Threading.Monitor.Enter(m_csFile);
                            }
                        }
                    }
                    m_vContext.RemoveFromFront();
                }
                else
                {
                    break;
                }
            }
            System.Threading.Monitor.Exit(m_csFile);
        }

        protected override void OnResultReturned(ushort reqId, CUQueue mc)
        {
            switch (reqId)
            {
                case idDownload:
                    {
                        int res;
                        string errMsg;
                        mc.Load(out res).Load(out errMsg);
                        DDownload dl = null;
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                CContext ctx = m_vContext[0];
                                ctx.ErrCode = res;
                                ctx.ErrMsg = errMsg;
                                dl = ctx.Download;
                            }
                        }
                        if (dl != null)
                        {
                            dl.Invoke(this, res, errMsg);
                        }
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                CloseFile(m_vContext.RemoveFromFront());
                            }
                        }
                        OnPostProcessing(0, 0);
                    }
                    break;
                case idStartDownloading:
                    lock (m_csFile)
                    {
                        long fileSize;
                        string localFile, remoteFile;
                        uint flags;
                        long initSize;
                        mc.Load(out fileSize).Load(out localFile).Load(out remoteFile).Load(out flags).Load(out initSize);
                        lock (m_csFile)
                        {
                            if (m_vContext.Count == 0)
                            {
                                CContext ctx = new CContext(false, flags);
                                ctx.LocalFile = localFile;
                                ctx.FilePath = remoteFile;
                                OpenLocalWrite(ctx);
                                ctx.InitSize = initSize;
                                m_vContext.AddToBack(ctx);
                            }
                            CContext context = m_vContext[0];
                            context.FileSize = fileSize;
                            initSize = (context.InitSize > 0) ? context.InitSize : 0;
                            if (context.File.Position > initSize)
                            {
                                context.File.SetLength(initSize);
                            }
                        }
                    }
                    break;
                case idDownloading:
                    {
                        long downloaded = 0;
                        DTransferring trans = null;
                        CContext context = null;
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                context = m_vContext[0];
                                trans = context.Transferring;
                                byte[] buffer = mc.IntenalBuffer;
                                try
                                {
                                    context.File.Write(buffer, 0, (int)mc.GetSize());
                                    long initSize = (context.InitSize > 0) ? context.InitSize : 0;
                                    downloaded = context.File.Position - initSize;
                                }
                                catch (System.IO.IOException err)
                                {
                                    context.ErrMsg = err.Message;
#if NO_HRESULT
                                    context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
#else
                                    context.ErrCode = err.HResult;
#endif
                                }
                            }
                        }
                        mc.SetSize(0);
                        if (context != null && context.HasError)
                        {
                            if (context.Download != null)
                            {
                                context.Download.Invoke(this, context.ErrCode, context.ErrMsg);
                            }
                            CloseFile(m_vContext.RemoveFromFront());
                            OnPostProcessing(0, 0);
                        }
                        else if (trans != null)
                        {
                            trans.Invoke(this, downloaded);
                        }
                    }
                    break;
                case idUploadBackup:
                    break;
                case idUpload:
                    {
                        CContext context = null;
                        int res;
                        string errMsg;
                        mc.Load(out res).Load(out errMsg);
                        if (res != 0 || (errMsg != null && errMsg.Length > 0))
                        {
                            lock (m_csFile)
                            {
                                if (m_vContext.Count > 0)
                                {
                                    context = m_vContext[0];
                                    mc.Load(out context.InitSize);
                                    context.ErrCode = res;
                                    context.ErrMsg = errMsg;
                                }
                            }
                        }
                        else
                        {
                            CClientSocket cs = Socket;
                            lock (m_csFile)
                            {
                                if (m_vContext.Count > 0)
                                {
                                    context = m_vContext[0];
                                    mc.Load(out context.InitSize);
                                    using (CScopeUQueue sb = new CScopeUQueue())
                                    {
                                        DAsyncResultHandler rh = null;
                                        DOnExceptionFromServer se = null;
                                        if (sb.UQueue.MaxBufferSize < STREAM_CHUNK_SIZE)
                                            sb.UQueue.Realloc(STREAM_CHUNK_SIZE);
                                        byte[] buffer = sb.UQueue.IntenalBuffer;
                                        try
                                        {
                                            context.QueueOk = cs.ClientQueue.StartJob();
                                            bool queue_enabled = cs.ClientQueue.Available;
                                            if (queue_enabled)
                                            {
                                                SendRequest(idUploadBackup, context.FilePath, context.Flags, context.FileSize, context.InitSize, rh, context.Discarded, se);
                                            }
                                            int ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                            while (ret == STREAM_CHUNK_SIZE)
                                            {
                                                if (!SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se))
                                                {
                                                    context.ErrCode = cs.ErrorCode;
                                                    context.ErrMsg = cs.ErrorMsg;
                                                    break;
                                                }
                                                ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                                if (queue_enabled)
                                                {
                                                    //save file into client message queue
                                                }
                                                else if (cs.BytesInSendingBuffer > 40 * STREAM_CHUNK_SIZE)
                                                {
                                                    break;
                                                }
                                            }
                                            if (ret > 0 && !context.HasError)
                                            {
                                                if (!SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se))
                                                {
                                                    context.ErrCode = cs.ErrorCode;
                                                    context.ErrMsg = cs.ErrorMsg;
                                                }
                                            }
                                            if (ret < STREAM_CHUNK_SIZE && !context.HasError)
                                            {
                                                context.Sent = true;
                                                SendRequest(idUploadCompleted, rh, context.Discarded, se);
                                                if (context.QueueOk)
                                                {
                                                    Socket.ClientQueue.EndJob();
                                                }
                                            }
                                        }
                                        catch (System.IO.IOException err)
                                        {
                                            errMsg = err.Message;
#if NO_HRESULT
                                            res = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
#else
                                            res = err.HResult;
#endif
                                            context.ErrCode = res;
                                            context.ErrMsg = errMsg;
                                        }
                                    }
                                }
                            }
                        }
                        if (context != null && context.HasError)
                        {
                            if (context.Upload != null)
                            {
                                context.Upload.Invoke(this, context.ErrCode, context.ErrMsg);
                            }
                            lock (m_csFile)
                            {
                                CloseFile(m_vContext.RemoveFromFront());
                            }
                            if (context.QueueOk)
                            {
                                Socket.ClientQueue.AbortJob();
                            }
                            OnPostProcessing(0, 0);
                        }
                    }
                    break;
                case idUploading:
                    {
                        int errCode = 0;
                        string errMsg = "";
                        CContext context = null;
                        DTransferring trans = null;
                        long uploaded;
                        mc.Load(out uploaded);
                        if (mc.GetSize() >= 8)
                        {
                            mc.Load(out errCode).Load(out errMsg);
                        }
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                context = m_vContext[0];
                                trans = context.Transferring;
                                if (uploaded < 0 || errCode != 0 || errMsg.Length != 0)
                                {
                                    context.ErrCode = errCode;
                                    context.ErrMsg = errMsg;
                                    CloseFile(context);
                                }
                                else if (!context.Sent)
                                {
                                    using (CScopeUQueue sb = new CScopeUQueue())
                                    {
                                        DAsyncResultHandler rh = null;
                                        DOnExceptionFromServer se = null;
                                        if (sb.UQueue.MaxBufferSize < STREAM_CHUNK_SIZE)
                                            sb.UQueue.Realloc(STREAM_CHUNK_SIZE);
                                        byte[] buffer = sb.UQueue.IntenalBuffer;
                                        try
                                        {
                                            int ret = context.File.Read(buffer, 0, (int)STREAM_CHUNK_SIZE);
                                            if (ret > 0)
                                            {
                                                SendRequest(idUploading, buffer, (uint)ret, rh, context.Discarded, se);
                                            }
                                            if (ret < STREAM_CHUNK_SIZE)
                                            {
                                                context.Sent = true;
                                                SendRequest(idUploadCompleted, rh, context.Discarded, se);
                                            }
                                        }
                                        catch (System.IO.IOException err)
                                        {
                                            context.ErrMsg = err.Message;
#if NO_HRESULT
                                            context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
#else
                                            context.ErrCode = err.HResult;
#endif
                                        }
                                    }
                                }
                            }
                        }
                        if (context != null && context.HasError)
                        {
                            if (context.Upload != null)
                            {
                                context.Upload.Invoke(this, context.ErrCode, context.ErrMsg);
                            }
                            lock (m_csFile)
                            {
                                CloseFile(m_vContext.RemoveFromFront());
                            }
                            OnPostProcessing(0, 0);
                        }
                        else if (trans != null)
                        {
                            trans.Invoke(this, uploaded);
                        }
                    }
                    break;
                case idUploadCompleted:
                    {
                        DUpload upl = null;
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                if (m_vContext[0].File != null)
                                {
                                    upl = m_vContext[0].Upload;
                                }
                                else
                                {
                                    m_vContext[0].QueueOk = false;
                                    m_vContext[0].Sent = false;
                                    CloseFile(m_vContext[0]);
                                }
                            }
                        }
                        if (upl != null)
                        {
                            upl.Invoke(this, 0, "");
                        }
                        lock (m_csFile)
                        {
                            if (m_vContext.Count > 0)
                            {
                                if (m_vContext[0].File != null)
                                {
                                    CloseFile(m_vContext.RemoveFromFront());
                                }
                            }
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
