using System;
using System.Collections.Generic;
using System.Text;

namespace SocketProAdapter.ClientSide
{
    public class ReplicationSetting
    {
        public const uint DEAFULT_TTL = (uint)30 * 24 * 3600; //30 days

        /// <summary>
        /// An absolute path to a directory containing message queue files.
        /// </summary>
        public string QueueDir = CClientSocket.QueueConfigure.WorkDirectory;

        /// <summary>
        /// False for auto socket connecting. Otherwise, there is no auto connection.
        /// </summary>
        public bool NoAutoConn = false;

        /// <summary>
        /// Time-to-live in seconds. It is ignored if persistent message queue feature is not used. If the value is not set or zero, the value will default to DEFAULT_TTL (30 days).
        /// </summary>
        public uint TTL = DEAFULT_TTL;

        /// <summary>
        /// A timeout for receiving result from remote SocketPro server. If the value is not set or it is zero, the value will default to CClientSocket.DEFAULT_RECV_TIMEOUT (30 seconds).
        /// </summary>
        public uint RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;

        /// <summary>
        /// A timeout for connecting to remote SocketPro server. If the value is not set or it is zero, the value will default to CClientSocket.DEFAULT_CONN_TIMEOUT (30 seconds).
        /// </summary>
        public uint ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;


        internal ReplicationSetting Copy()
        {
            ReplicationSetting rs = new ReplicationSetting();
            rs.ConnTimeout = ConnTimeout;
            rs.NoAutoConn = NoAutoConn;
            rs.QueueDir = QueueDir;
            rs.RecvTimeout = RecvTimeout;
            rs.TTL = TTL;
            return rs;
        }
    }

    public class CReplication<THandler> : IDisposable
        where THandler : CAsyncServiceHandler, new()
    {
        private CSocketPool<THandler> m_pool = null;
        private Dictionary<string, CConnectionContext> m_mapQueueConn = new Dictionary<string, CConnectionContext>();
        private ReplicationSetting m_rs;
        public static readonly char DIR_SEP;

        static CReplication()
        {
            switch (System.Environment.OSVersion.Platform)
            {
                case PlatformID.Win32S:
                case PlatformID.Win32NT:
                case PlatformID.Win32Windows:
                case PlatformID.WinCE:
                case PlatformID.Xbox:
                    DIR_SEP = '\\';
                    break;
                default:
                    DIR_SEP = '/';
                    break;
            }
        }

        public uint Connections
        {
            get
            {
                if (m_pool == null)
                    return 0;
                return m_pool.ConnectedSockets;
            }
        }

        private void Cleanup()
        {
            if (m_pool != null)
            {
                m_pool.Dispose();
                m_pool = null;
            }
        }

        ~CReplication()
        {
            Cleanup();
        }

        #region IDisposable Members
        public void Dispose()
        {
            Cleanup();
        }
        #endregion

        protected virtual bool DoSslServerAuthentication(CClientSocket cs)
        {
            return true;
        }

        private void CheckReplicationSetting(ReplicationSetting qms)
        {
            if (qms.TTL == 0)
                qms.TTL = ReplicationSetting.DEAFULT_TTL;
            if (qms.ConnTimeout == 0)
                qms.ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
            if (qms.RecvTimeout == 0)
                qms.RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
#if WINCE

#else
            if (qms.QueueDir == null)
                throw new InvalidOperationException("An absolute path required for working directory");
            qms.QueueDir = qms.QueueDir.Trim();
            if (qms.QueueDir.Length == 0)
                throw new InvalidOperationException("An absolute path required for working directory");
            if (qms.QueueDir.LastIndexOf(DIR_SEP) != qms.QueueDir.Length - 1)
                throw new InvalidOperationException("An absolute path must be ended with the char " + DIR_SEP);
            //make sure directories existing
            if (!System.IO.Directory.Exists(qms.QueueDir))
                System.IO.Directory.CreateDirectory(qms.QueueDir);
#endif
            m_rs = qms.Copy();
        }

        void EndProcess(string[] vDbFullName, bool secure, CConnectionContext[,] mcc, tagThreadApartment ta)
        {
            m_pool = new CSocketPool<THandler>(!m_rs.NoAutoConn, m_rs.RecvTimeout, m_rs.ConnTimeout);
            if (secure)
            {
                m_pool.DoSslServerAuthentication += (sender, cs) =>
                {
                    if (cs.ConnectionContext.Port == ushort.MaxValue)
                        return true;
                    return DoSslServerAuthentication(cs);
                };
            }

            m_pool.SocketPoolEvent += (sender, spe, handler) =>
            {
                switch (spe)
                {
                    case tagSocketPoolEvent.speSocketClosed:
                        handler.CleanCallbacks();
                        break;
                    case tagSocketPoolEvent.speConnecting:
                        if (handler.AttachedClientSocket.ConnectionContext.Port == ushort.MaxValue)
                            handler.AttachedClientSocket.ConnectingTimeout = 500;
                        break;
                    default:
                        break;
                }
            };

            bool ok = m_pool.StartSocketPool(mcc, true, ta);
            int n = 0;
            foreach (CClientSocket s in m_pool.Sockets)
            {
                string key = vDbFullName[n];
                ok = s.ClientQueue.StartQueue(key, m_rs.TTL, secure);
                ++n;
            }
            if (Replicable)
                SourceQueue.EnsureAppending(TargetQueues);
            THandler[] targetHandlers = TargetHandlers;
            foreach (THandler h in targetHandlers)
            {
                ok = h.AttachedClientSocket.DoEcho();
            }
        }

        private void CheckConnQueueName(Dictionary<string, CConnectionContext> mapQueueConn)
        {
            m_mapQueueConn.Clear();
            if (mapQueueConn == null || mapQueueConn.Count == 0)
                throw new ArgumentException("One middle server required at least");
            foreach (string key in mapQueueConn.Keys)
            {
                CConnectionContext cc = mapQueueConn[key];
                if (cc == null)
                    throw new InvalidOperationException("An invalid host found");
                string v = key;
                if (v == null)
                    throw new InvalidOperationException("A non-empty string for persistent queue name required for each of hosts");
                v = v.Trim();
                v = v.Trim(DIR_SEP);
                if (v.Length == 0)
                    throw new InvalidOperationException("A non-empty string for persistent queue name required for each of hosts");
                if (DoesQueueExist(v))
                    throw new InvalidOperationException("Queue name duplicated -- " + v);
                m_mapQueueConn[v] = cc;
            }
        }

        /// <summary>
        /// Start a socket pool for replication
        /// </summary>
        /// <param name="mapQueueConn">A dictionary for message queue name and connecting context. a unique name must be specified for each of connecting contexts</param>
        /// <returns>True if there is at least one connection established; and false if there is no connection</returns>
        public bool Start(Dictionary<string, CConnectionContext> mapQueueConn)
        {
            return Start(mapQueueConn, null, tagThreadApartment.taNone);
        }

        /// <summary>
        /// Start a socket pool for replication
        /// </summary>
        /// <param name="mapQueueConn">A dictionary for message queue name and connecting context. a unique name must be specified for each of connecting contexts</param>
        /// <param name="rootQueueName">A string for root replication queue name. It is ignored if it is not replicable</param>
        /// <returns>True if there is at least one connection established; and false if there is no connection</returns>
        public bool Start(Dictionary<string, CConnectionContext> mapQueueConn, string rootQueueName)
        {
            return Start(mapQueueConn, rootQueueName, tagThreadApartment.taNone);
        }

        /// <summary>
        /// Start a socket pool for replication
        /// </summary>
        /// <param name="mapQueueConn">A dictionary for message queue name and connecting context. a unique name must be specified for each of connecting contexts</param>
        /// <param name="rootQueueName">A string for root replication queue name. It is ignored if it is not replicable</param>
        /// <param name="ta">COM thread apartment; and it defaults to taNone. It is ignored on non-window platforms</param>
        /// <returns>True if there is at least one connection established; and false if there is no connection</returns>
        public virtual bool Start(Dictionary<string, CConnectionContext> mapQueueConn, string rootQueueName, tagThreadApartment ta)
        {
            CheckConnQueueName(mapQueueConn);
            uint n = 0;
            bool secure = false;
            int all = m_mapQueueConn.Count;
            if (all > 1)
                ++all;
            if (rootQueueName == null)
                rootQueueName = "";
            rootQueueName = rootQueueName.Trim();
            rootQueueName = rootQueueName.Trim(DIR_SEP);
            if (rootQueueName.Length == 0)
            {
                string appName = System.AppDomain.CurrentDomain.FriendlyName;
                int dot = appName.LastIndexOf('.');
                if (dot == -1)
                    rootQueueName = appName;
                else
                    rootQueueName = appName.Substring(0, dot);
            }
            string[] vDbFullName = new string[all];
            CConnectionContext[,] mcc = new CConnectionContext[1, all];
            foreach (string key in m_mapQueueConn.Keys)
            {
                mcc[0, n] = m_mapQueueConn[key];
                if (!secure && mcc[0, n].EncrytionMethod == tagEncryptionMethod.TLSv1)
                    secure = true;
                vDbFullName[n] = m_rs.QueueDir + key;
                ++n;
            }
            if (all > 1)
            {
                CConnectionContext last = new CConnectionContext("127.0.0.1", ushort.MaxValue, "UReplication", "");
                last.EncrytionMethod = secure ? tagEncryptionMethod.TLSv1 : tagEncryptionMethod.NoEncryption;
                mcc[0, n] = last;
                vDbFullName[n] = m_rs.QueueDir + rootQueueName;
            }
            EndProcess(vDbFullName, secure, mcc, ta);
            return (m_pool.ConnectedSockets > 0);
        }

        /// <summary>
        /// Construct a CSqlReplication instance
        /// </summary>
        /// <param name="qms">A structure for setting its underlying socket pool and message queue directory as well as password for source queue</param>
        public CReplication(ReplicationSetting qms)
        {
            CheckReplicationSetting(qms);
        }

        /// <summary>
        /// Check if it is replicable.
        /// </summary>
        public bool Replicable
        {
            get
            {
                return (m_mapQueueConn.Count > 1);
            }
        }

        public THandler SourceHandler
        {
            get
            {
                THandler one = null;
                if (m_pool != null)
                {
                    foreach (THandler h in m_pool.AsyncHandlers)
                    {
                        one = h;
                    }
                }
                return one;
            }
        }

        public IClientQueue SourceQueue
        {
            get
            {
                THandler src = SourceHandler;
                if (src == null)
                    return null;
                return src.AttachedClientSocket.ClientQueue;
            }
        }

        public IClientQueue[] TargetQueues
        {
            get
            {
                int n = 0;
                THandler[] handlers = TargetHandlers;
                if (handlers == null)
                    return null;
                IClientQueue[] tq = new IClientQueue[handlers.Length];
                foreach (THandler h in handlers)
                {
                    tq[n] = h.AttachedClientSocket.ClientQueue;
                    ++n;
                }
                return tq;
            }
        }

        public THandler[] TargetHandlers
        {
            get
            {
                int n = 0;
                if (m_pool == null)
                    return null;
                THandler[] tq = new THandler[m_mapQueueConn.Count];
                foreach (THandler h in m_pool.AsyncHandlers)
                {
                    tq[n] = h;
                    if (n >= (tq.Length - 1))
                        break;
                    ++n;
                }
                return tq;
            }
        }

        public uint Hosts
        {
            get
            {
                return (uint)m_mapQueueConn.Count;
            }
        }

        public uint Queues
        {
            get
            {
                if (m_pool == null)
                    return 0;
                return m_pool.Queues;
            }
        }

        public ReplicationSetting ReplicationSetting
        {
            get
            {
                return m_rs;
            }
        }

        /// <summary>
        /// Make a replication. An invalid operation exception will be thrown if not replicable.
        /// </summary>
        /// <returns>True for success; and false for failure</returns>
        public bool DoReplication()
        {
            if (m_mapQueueConn.Count == 1)
                throw new InvalidOperationException("No replication is allowed because the number of target message queues less than two");
            IClientQueue src = SourceQueue;
            if (src == null)
                return false;
            return src.AppendTo(TargetQueues);
        }

        private bool DoesQueueExist(string qName)
        {
            bool ignoreCase;
            switch (System.Environment.OSVersion.Platform)
            {
                case PlatformID.Win32S:
                case PlatformID.Win32NT:
                case PlatformID.Win32Windows:
                case PlatformID.WinCE:
                case PlatformID.Xbox:
                    ignoreCase = true;
                    break;
                default:
                    ignoreCase = false;
                    break;
            }
            foreach (string name in m_mapQueueConn.Keys)
            {
                if (string.Compare(name, qName, ignoreCase) == 0)
                    return true;
            }
            return false;
        }

        virtual public bool Send(ushort reqId, byte[] data, uint len)
        {
            CAsyncServiceHandler.DAsyncResultHandler ash = null;
            THandler src = SourceHandler;
            if (src == null)
                return false;
            IClientQueue cq = src.AttachedClientSocket.ClientQueue;
            if (!cq.Available)
                return false;
            bool ok = src.SendRequest(reqId, data, len, ash);
            if (Replicable && cq.JobSize == 0)
                ok = cq.AppendTo(TargetQueues);
            return ok;
        }

        public bool EndJob()
        {
            IClientQueue src = SourceQueue;
            if (src == null || !src.Available)
                return false;
            bool ok = src.EndJob();
            if (ok && Replicable)
                ok = src.AppendTo(TargetQueues);
            return ok;
        }

        public bool StartJob()
        {
            IClientQueue src = SourceQueue;
            if (src == null || !src.Available)
                return false;
            return src.StartJob();
        }

        public bool AbortJob()
        {
            IClientQueue src = SourceQueue;
            if (src == null || !src.Available)
                return false;
            return src.AbortJob();
        }

        public bool Send(ushort reqId)
        {
            return Send(reqId, (byte[])null, (uint)0);
        }

        public bool Send(ushort reqId, CUQueue q)
        {
            if (q == null || q.GetSize() == 0)
                return Send(reqId);
            if (q.HeadPosition > 0)
                return Send(reqId, q.GetBuffer(), q.GetSize());
            return Send(reqId, q.m_bytes, q.GetSize());
        }

        public bool Send(ushort reqId, CScopeUQueue q)
        {
            if (q == null)
                return Send(reqId);
            return Send(reqId, q.UQueue);
        }

        public bool Send<T0>(ushort reqId, T0 t0)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1>(ushort reqId, T0 t0, T1 t1)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

        public bool Send<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
        {
            CUQueue su = CScopeUQueue.Lock();
            su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
            bool ok = Send(reqId, su);
            CScopeUQueue.Unlock(su);
            return ok;
        }

    }
}
