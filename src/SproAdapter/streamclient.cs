using System;
using System.IO;

namespace SocketProAdapter.ClientSide
{
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
        public ulong DownloadingStreamSize
        {
            get
            {
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
                if (!ok && !m_ash.AttachedClientSocket.Sendable)
                {
                    m_s = null;
                    return m_ash.AttachedClientSocket.ErrorMsg;
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
                    return m_ash.AttachedClientSocket.ErrorMsg;
                }
            }
            return res;
        }

        private object m_cs = new object();
        private Stream m_s; //protected by m_cs
        private ulong SendDataFromClientToServer()
        {
            if (m_ash.AttachedClientSocket.BytesInSendingBuffer > CStreamSerializationHelper.STREAM_CHUNK_SIZE)
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
                    if (m_ash.AttachedClientSocket.BytesInSendingBuffer > 10 * CStreamSerializationHelper.STREAM_CHUNK_SIZE)
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
            if (!ok && !m_ash.AttachedClientSocket.Sendable)
                return m_ash.AttachedClientSocket.ErrorMsg;
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
        public CAsyncServiceHandler AsyncServiceHandler
        {
            get
            {
                return m_ash;
            }
        }
        private CAsyncServiceHandler m_ash;
    }
}
