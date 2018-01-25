using System;
using System.Collections.Generic;
using System.Reflection;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class CSocketPool<THandler> : IDisposable where THandler : CAsyncServiceHandler, new()
        {
            public delegate void DOnSocketPoolEvent(CSocketPool<THandler> sender, tagSocketPoolEvent spe, THandler AsyncServiceHandler);
            public delegate bool DDoSslServerAuthentication(CSocketPool<THandler> sender, CClientSocket cs);

            protected Dictionary<CClientSocket, THandler> m_dicSocketHandler = new Dictionary<CClientSocket, THandler>();
            protected object m_cs = new object();

            private uint m_nPoolId = 0;
            private bool m_autoConn = true;
            private uint m_recvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
            private uint m_connTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
            private SocketPoolCallback m_spc = null;
            private uint m_ServiceId = 0;
            private CConnectionContext[,] m_mcc;
            private string m_qName = "";
            public string QueueName
            {
                get
                {
                    lock (m_cs)
                    {
                        return m_qName;
                    }
                }
                set
                {
                    string s = value;
                    if (s != null)
                        s = s.Trim();
                    else
                        s = "";
#if WINCE
                    if (System.Environment.OSVersion.Platform != PlatformID.Unix)
#else
                    if (System.Environment.OSVersion.Platform != PlatformID.Unix && System.Environment.OSVersion.Platform != PlatformID.MacOSX)
#endif
                    {
                        s = s.ToLower();
                    }
                    lock (m_cs)
                    {
                        if (m_qName != s)
                        {
                            StopPoolQueue();
                            m_qName = s;
                            if (m_qName.Length > 0)
                            {
                                StartPoolQueue(m_qName);
                            }
                        }
                    }
                }
            }

            private void StopPoolQueue()
            {
                foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                {
                    if (cs.ClientQueue != null && cs.ClientQueue.Available)
                        cs.ClientQueue.StopQueue();
                }
            }

            private const uint DEFAULT_QUEUE_TIME_TO_LIVE = 240 * 3600;

            private void StartPoolQueue(string qName)
            {
                int index = 0;
                foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                {
                    bool ok = cs.ClientQueue.StartQueue(qName + index.ToString(), DEFAULT_QUEUE_TIME_TO_LIVE, cs.EncryptionMethod != tagEncryptionMethod.NoEncryption);
                    ++index;
                }
            }

            private void SetQueue(CClientSocket socket)
            {
                int index = 0;
                foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                {
                    if (cs == socket)
                    {
                        if (m_qName.Length > 0)
                        {
                            if (!cs.ClientQueue.Available)
                                cs.ClientQueue.StartQueue(m_qName + index.ToString(), DEFAULT_QUEUE_TIME_TO_LIVE, cs.EncryptionMethod != tagEncryptionMethod.NoEncryption);
                        }
                        break;
                    }
                    ++index;
                }
            }

            public static uint SocketPools
            {
                get
                {
                    return ClientCoreLoader.GetNumberOfSocketPools();
                }
            }

            public THandler[] AsyncHandlers
            {
                get
                {
                    lock (m_cs)
                    {
                        int n = 0;
                        THandler[] hs = new THandler[m_dicSocketHandler.Count];
                        ICollection<THandler> values = m_dicSocketHandler.Values;
                        foreach (THandler h in values)
                        {
                            hs[n] = h;
                            ++n;
                        }
                        return hs;
                    }
                }
            }

            public CClientSocket[] Sockets
            {
                get
                {
                    lock (m_cs)
                    {
                        int n = 0;
                        CClientSocket[] sockets = new CClientSocket[m_dicSocketHandler.Count];
                        ICollection<CClientSocket> values = m_dicSocketHandler.Keys;
                        foreach (CClientSocket s in values)
                        {
                            sockets[n] = s;
                            ++n;
                        }
                        return sockets;
                    }
                }
            }

            public uint PoolId
            {
                get
                {
                    lock (m_cs)
                    {
                        return m_nPoolId;
                    }
                }
            }

            /// <summary>
            /// Enable (true) or disable (false) automatically merging requests queued inside a local/client file to a connected session within a pool of sockets when a session is disconnected. The property defaults to false.
            /// </summary>
            public bool QueueAutoMerge
            {
                get
                {
                    lock (m_cs)
                    {
                        return ((ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId) > 0) ? true : false);
                    }
                }
                set
                {
                    lock (m_cs)
                    {
                        ClientCoreLoader.SetQueueAutoMergeByPool(m_nPoolId, (byte)(value ? 1 : 0));
                    }
                }
            }

            /// <summary>
            /// Seek an async handler by its associated queue file full path or raw name. 
            /// </summary>
            /// <param name="queueName">queue file full path or raw name</param>
            /// <returns>An async handler if found; and null or nothing if none is found</returns>
            public virtual THandler SeekByQueue(string queueName)
            {
                THandler h = null;
                if (queueName == null || queueName.Length == 0)
                    return null;
                char[] empty = { ' ', '\t', '\r', '\n' };
                queueName = queueName.Trim(empty);
                switch (System.Environment.OSVersion.Platform)
                {
                    case PlatformID.Win32S:
                    case PlatformID.Win32NT:
                    case PlatformID.Win32Windows:
                    case PlatformID.WinCE:
                    case PlatformID.Xbox:
                        queueName = queueName.ToLower();
                        break;
                    default:
                        break;
                }
                lock (m_cs)
                {
                    string rawName;
                    string appName;
                    unsafe
                    {
                        appName = new string((sbyte*)ClientCoreLoader.GetClientWorkDirectory());
                    }
                    foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                    {
                        if (!cs.ClientQueue.Available)
                            continue;
                        if (cs.ClientQueue.Secure)
                            rawName = queueName + "_" + appName + "_1.mqc";
                        else
                            rawName = queueName + "_" + appName + "_0.mqc";
                        string queueFileName = cs.ClientQueue.QueueFileName;

                        int len = queueFileName.Length;
                        int lenRaw = rawName.Length;
                        if (lenRaw > len)
                            continue;
                        int pos = queueFileName.LastIndexOf(rawName);

                        //queue file name with full path
                        if (pos == 0)
                            return m_dicSocketHandler[cs];

                        //queue raw name only
                        if ((pos + lenRaw) == len)
                            return m_dicSocketHandler[cs];
                    }
                }
                return h;
            }

            public uint Queues
            {
                get
                {
                    uint q = 0;
                    lock (m_cs)
                    {
                        foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                        {
                            q += (cs.ClientQueue.Available ? (uint)1 : 0);
                        }
                    }

                    return q;
                }
            }

            /// <summary>
            /// Seek an async handler on the min number of requests queued in memory and its associated socket connection
            /// </summary>
            /// <returns>An async handler if found; and null or nothing if no connection is found</returns>
            public virtual THandler Seek()
            {
                THandler h = null;
                lock (m_cs)
                {
                    foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                    {
                        if (cs.ConnectionState < tagConnectionState.csSwitched)
                            continue;
                        if (h == null)
                            h = m_dicSocketHandler[cs];
                        else
                        {
                            uint cs_coriq = cs.CountOfRequestsInQueue;
                            uint h_coriq = h.AttachedClientSocket.CountOfRequestsInQueue;
                            if (cs_coriq < h_coriq)
                                h = m_dicSocketHandler[cs];
                            else if (cs_coriq == h_coriq && cs.BytesSent < h.AttachedClientSocket.BytesSent)
                                h = m_dicSocketHandler[cs];
                        }
                    }
                }
                return h;
            }

            /// <summary>
            /// Seek an async handler on the min number of requests queued and its associated socket connection
            /// </summary>
            /// <returns>An async handler if found; and null or nothing if no proper queue is available</returns>
            public virtual THandler SeekByQueue()
            {
                THandler h = null;
                lock (m_cs)
                {
                    bool automerge = (ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId) > 0);
                    foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                    {
                        if (automerge && cs.ConnectionState < tagConnectionState.csSwitched)
                            continue;
                        IClientQueue cq = cs.ClientQueue;
                        if (!cq.Available || cq.JobSize > 0/*queue is in transaction at this time*/)
                            continue;
                        if (h == null)
                            h = m_dicSocketHandler[cs];
                        else if ((cq.MessageCount < h.AttachedClientSocket.ClientQueue.MessageCount) || (cs.Connected && !h.AttachedClientSocket.Connected))
                            h = m_dicSocketHandler[cs];
                    }
                }
                return h;
            }

            /// <summary>
            /// Lock an async handler infinitely
            /// </summary>
            /// <returns>An async handler if successful; and null or nothing if failed</returns>
            /// <remarks>You must also call the method Unlock to unlock the handler and its assocaited socket connection for reuse.</remarks>
            public THandler Lock()
            {
                return Lock(uint.MaxValue);
            }

            /// <summary>
            /// Lock an async handler on a given timeout.
            /// </summary>
            /// <param name="timeout">A timeout in milliseconds</param>
            /// <returns>An async handler if successful; and null or nothing if failed</returns>
            /// <remarks>You must also call the method Unlock to unlock the handler and its assocaited socket connection for reuse.</remarks>
            public virtual THandler Lock(uint timeout)
            {
                uint poolId;
                lock (m_cs)
                {
                    poolId = m_nPoolId;
                }
                return MapToHandler(ClientCoreLoader.LockASocket(poolId, timeout, IntPtr.Zero));
            }

            /// <summary>
            /// Lock an async handler on a given thread handle with infinite timeout
            /// </summary>
            /// <param name="sameThreadHandle">A handle to a thread</param>
            /// <returns>An async handler if successful; and null or nothing if failed</returns>
            /// <remarks>You must also call the method Unlock to unlock the handler and its associated socket connection for reuse.</remarks>
            public THandler Lock(IntPtr sameThreadHandle)
            {
                return Lock(sameThreadHandle, uint.MaxValue);
            }

            /// <summary>
            /// Lock an async handler on given thread handle and timeout.
            /// </summary>
            /// <param name="sameThreadHandle">A handle to a thread</param>
            /// <param name="timeout">A timeout in milliseconds</param>
            /// <returns>An async handler if successful; and null or nothing if failed</returns>
            /// <remarks>You must also call the method Unlock to unlock the handler and its associated socket connection for reuse.</remarks>
            public virtual THandler Lock(IntPtr sameThreadHandle, uint timeout)
            {
                uint poolId;
                lock (m_cs)
                {
                    poolId = m_nPoolId;
                }
                return MapToHandler(ClientCoreLoader.LockASocket(poolId, timeout, sameThreadHandle));
            }

            /// <summary>
            /// Unlock a previously locked async handler to pool for reuse.
            /// </summary>
            /// <param name="handler">A previously locked async handler</param>
            public virtual void Unlock(THandler handler)
            {
                if (handler == null)
                    return;
                Unlock(handler.AttachedClientSocket);
            }

            /// <summary>
            /// Unlock a previously locked socket to pool for reuse.
            /// </summary>
            /// <param name="handler">A previously locked socket</param>
            public virtual void Unlock(CClientSocket cs)
            {
                uint poolId;
                if (cs == null)
                    return;
                lock (m_cs)
                {
                    poolId = m_nPoolId;
                }
                ClientCoreLoader.UnlockASocket(poolId, cs.Handle);
            }

            /// <summary>
            /// Create an instance of socket pool with both request receiving and socket connecting timeout default to 30 seconds as well as auto connecting default to true.
            /// </summary>
            public CSocketPool()
            {
                m_spc += new SocketPoolCallback(OnSPEvent);
            }

            /// <summary>
            /// Create an instance of socket pool with both request receiving and socket connecting timeout default to 30 seconds.
            /// </summary>
            /// <param name="autoConn">All sockets will support auto connection if true; and not if false</param>
            public CSocketPool(bool autoConn)
            {
                m_autoConn = autoConn;
                m_spc += new SocketPoolCallback(OnSPEvent);
            }

            /// <summary>
            /// Create an instance of socket pool with socket connecting timeout default to 30 seconds.
            /// </summary>
            /// <param name="autoConn">All sockets will support auto connection if true; and not if false</param>
            /// <param name="recvTimeout">Request receiving timeout in milliseconds</param>
            public CSocketPool(bool autoConn, uint recvTimeout)
            {
                m_autoConn = autoConn;
                m_recvTimeout = recvTimeout;
                m_spc += new SocketPoolCallback(OnSPEvent);
            }

            /// <summary>
            /// Create an instance of socket pool
            /// </summary>
            /// <param name="autoConn">All sockets will support auto connection if true; and not if false</param>
            /// <param name="recvTimeout">Request receiving timeout in milliseconds</param>
            /// <param name="connTimeout">Socket connection timeout in milliseconds</param>
            public CSocketPool(bool autoConn, uint recvTimeout, uint connTimeout)
            {
                m_autoConn = autoConn;
                m_recvTimeout = recvTimeout;
                m_connTimeout = connTimeout;
                m_spc += new SocketPoolCallback(OnSPEvent);
            }

            /// <summary>
            /// Create an instance of socket pool
            /// </summary>
            /// <param name="autoConn">All sockets will support auto connection if true; and not if false</param>
            /// <param name="recvTimeout">Request receiving timeout in milliseconds</param>
            /// <param name="connTimeout">Socket connection timeout in milliseconds</param>
            /// <param name="svsId">Service id which defaults to 0</param>
            public CSocketPool(bool autoConn, uint recvTimeout, uint connTimeout, uint svsId)
            {
                m_autoConn = autoConn;
                m_recvTimeout = recvTimeout;
                m_connTimeout = connTimeout;
                m_ServiceId = svsId;
                m_spc += new SocketPoolCallback(OnSPEvent);
            }

            /// <summary>
            /// Shut down the socket pool after all connections are disconnected.
            /// </summary>
            public virtual void ShutdownPool()
            {
                uint poolId;
                lock (m_cs)
                {
                    poolId = m_nPoolId;
                    m_nPoolId = 0;
                    m_ServiceId = 0;
                }
                if (poolId != 0)
                    ClientCoreLoader.DestroySocketPool(poolId);
            }

            /// <summary>
            /// Disconnect all connections
            /// </summary>
            /// <returns>The method always returns true.</returns>
            public bool DisconnectAll()
            {
                uint poolId;
                lock (m_cs)
                {
                    poolId = m_nPoolId;
                }
                if (poolId != 0)
                    return ClientCoreLoader.DisconnectAll(poolId) != 0;
                return true;
            }

            public bool Started
            {
                get
                {
                    lock (m_cs)
                    {
                        return (m_nPoolId > 0 && ClientCoreLoader.GetThreadCount(m_nPoolId) > 0);
                    }
                }
            }

            public uint LockedSockets
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetLockedSockets(m_nPoolId);
                    }
                }
            }

            public uint IdleSockets
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetIdleSockets(m_nPoolId);
                    }
                }
            }

            public uint ConnectedSockets
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetConnectedSockets(m_nPoolId);
                    }
                }
            }

            public uint ThreadsCreated
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetThreadCount(m_nPoolId);
                    }
                }
            }

            public uint DisconnectedSockets
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetDisconnectedSockets(m_nPoolId);
                    }
                }
            }

            public uint SocketsPerThread
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
                    }

                }
            }

            public bool Avg
            {
                get
                {
                    lock (m_cs)
                    {
                        return ClientCoreLoader.IsAvg(m_nPoolId) != 0;
                    }
                }
            }

            bool StartSocketPool(uint socketsPerThread, uint threads, bool avg, tagThreadApartment ta)
            {
                if (Started)
                    return true;
                byte b = (byte)(avg ? 1 : 0);
                uint id = ClientCoreLoader.CreateSocketPool(m_spc, socketsPerThread, threads, b, ta);
                return id != 0;
            }

            void CopyCC(CConnectionContext[,] cc)
            {
                int threads = cc.GetLength(0);
                int socketsPerThread = cc.GetLength(1);
                if (socketsPerThread * threads == 0)
                    throw new InvalidOperationException("Must set connection context argument properly");

                lock (m_cs)
                {
                    m_mcc = new CConnectionContext[threads, socketsPerThread];
                    for (uint m = 0; m < socketsPerThread; ++m)
                    {
                        for (uint n = 0; n < threads; ++n)
                        {
                            if (cc[n, m] == null)
                                throw new ArgumentException("Must set connection context argument properly");
                            else
                            {
                                CConnectionContext c = new CConnectionContext();
                                CConnectionContext src = cc[n, m];
                                c.Host = src.Host;
                                c.Password = src.GetPassword();
                                c.Port = src.Port;
                                c.EncrytionMethod = src.EncrytionMethod;
                                c.UserId = src.UserId;
                                c.V6 = src.V6;
                                c.Zip = src.Zip;
                                m_mcc[n, m] = c;
                            }
                        }
                    }
                }
            }

            /// <summary>
            /// Start a pool of sockets with a given two-dimensional matrix of connection contexts
            /// </summary>
            /// <param name="cc">A given two-dimensional matrix of connection contexts. Its first dimension length represents the number of threads; and the second dimension length is the number of sockets per thread</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext[,] cc)
            {
                return StartSocketPool(cc, true, tagThreadApartment.taNone);
            }
            /// <summary>
            /// Start a pool of sockets with a given two-dimensional matrix of connection contexts
            /// </summary>
            /// <param name="cc">A given two-dimensional matrix of connection contexts. Its first dimension length represents the number of threads; and the second dimension length is the number of sockets per thread</param>
            /// <param name="avg">A boolean value for building internal socket pool, which defaults to true</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext[,] cc, bool avg)
            {
                return StartSocketPool(cc, avg, tagThreadApartment.taNone);
            }

            /// <summary>
            /// Start a pool of sockets with a given two-dimensional matrix of connection contexts
            /// </summary>
            /// <param name="cc">A given two-dimensional matrix of connection contexts. Its first dimension length represents the number of threads; and the second dimension length is the number of sockets per thread</param>
            /// <param name="avg">A boolean value for building internal socket pool, which defaults to true.</param>
            /// <param name="ta">A value for COM thread apartment if there is COM object involved. It is ignored on non-window platforms, and default to tagThreadApartment.taNone</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public virtual bool StartSocketPool(CConnectionContext[,] cc, bool avg, tagThreadApartment ta)
            {
                bool ok;
                char[] empty = { ' ', '\t', '\r', '\n' };
                if (cc == null || cc.Length == 0)
                    throw new ArgumentException("Must set connection context argument properly");
                if (Started)
                    ShutdownPool();
                CopyCC(cc);
                bool first = true;
                int threads = cc.GetLength(0);
                int socketsPerThread = cc.GetLength(1);
                if (!StartSocketPool((uint)socketsPerThread, (uint)threads, avg, ta))
                    return false;
                Dictionary<CClientSocket, THandler> temp = new Dictionary<CClientSocket, THandler>();
                lock (m_cs)
                {
                    int index = 0;
                    foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                    {
                        temp[cs] = m_dicSocketHandler[cs];
                        int m = index % threads;
                        int n = index / threads;
                        CConnectionContext c = m_mcc[m, n];
                        if (c.Host == null)
                            throw new InvalidOperationException("Host string can not be null");
                        c.Host = c.Host.Trim(empty);
                        if (c.Host.Length == 0)
                            throw new InvalidOperationException("Host string must be a valid string");
                        if (c.Port == 0)
                            throw new InvalidOperationException("Host port can't be zero");
                        cs.ConnectionContext = c;
                        ++index;
                    }
                }
                foreach (CClientSocket cs in temp.Keys)
                {
                    if (cs.Connected)
                    {
                        first = false;
                        continue;
                    }
                    CConnectionContext c = cs.ConnectionContext;
                    cs.UID = c.UserId;
                    cs.EncryptionMethod = c.EncrytionMethod;
                    cs.Zip = c.Zip;
                    IntPtr p = cs.Handle;
                    unsafe
                    {
                        fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(c.Host))
                        {
                            IntPtr host = new IntPtr(data);
                            if (first)
                            {
                                //we use sync connecting for the first socket connection
                                ok = ClientCoreLoader.Connect(p, host, c.Port, 1, (byte)(c.V6 ? 1 : 0)) != 0;
                                if (ok && ClientCoreLoader.WaitAll(cs.Handle, uint.MaxValue) != 0)
                                {
                                    first = false;
                                }
                            }
                            else
                            {
                                ok = ClientCoreLoader.Connect(p, host, c.Port, 0, (byte)(c.V6 ? 1 : 0)) != 0;
                            }
                        }
                    }
                }
                return (ConnectedSockets > 0);
            }

            /// <summary>
            ///  Start a pool of sockets with one connection context
            /// </summary>
            /// <param name="cc">A connection context structure</param>
            /// <param name="socketsPerThread">The number of socket connections per thread</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext cc, uint socketsPerThread)
            {
                return StartSocketPool(cc, socketsPerThread, 0, true, tagThreadApartment.taNone);
            }

            /// <summary>
            ///  Start a pool of sockets with one connection context
            /// </summary>
            /// <param name="cc">A connection context structure</param>
            /// <param name="socketsPerThread">The number of socket connections per thread</param>
            /// <param name="threads">The number of threads in a pool which defaults to the number of CPU cores</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext cc, uint socketsPerThread, uint threads)
            {
                return StartSocketPool(cc, socketsPerThread, threads, true, tagThreadApartment.taNone);
            }

            /// <summary>
            ///  Start a pool of sockets with one connection context
            /// </summary>
            /// <param name="cc">A connection context structure</param>
            /// <param name="socketsPerThread">The number of socket connections per thread</param>
            /// <param name="threads">The number of threads in a pool which defaults to the number of CPU cores</param>
            /// <param name="avg">A boolean value for building internal socket pool, which defaults to true.</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext cc, uint socketsPerThread, uint threads, bool avg)
            {
                return StartSocketPool(cc, socketsPerThread, threads, avg, tagThreadApartment.taNone);
            }

            /// <summary>
            ///  Start a pool of sockets with one connection context
            /// </summary>
            /// <param name="cc">A connection context structure</param>
            /// <param name="socketsPerThread">The number of socket connections per thread</param>
            /// <param name="threads">The number of threads in a pool which defaults to the number of CPU cores</param>
            /// <param name="avg">A boolean value for building internal socket pool, which defaults to true.</param>
            /// <param name="ta">A value for COM thread apartment if there is COM object involved. It is ignored on non-window platforms, and default to tagThreadApartment.taNone</param>
            /// <returns>False if there is no connection established; and true as long as there is one connection started</returns>
            public bool StartSocketPool(CConnectionContext cc, uint socketsPerThread, uint threads, bool avg, tagThreadApartment ta)
            {

                if (threads == 0)
#if WINCE
                    threads = 1;
#else
                    threads = (uint)Environment.ProcessorCount;
#endif
                CConnectionContext[,] mcc = new CConnectionContext[threads, socketsPerThread];
                for (uint m = 0; m < socketsPerThread; ++m)
                {
                    for (uint n = 0; n < threads; ++n)
                    {
                        mcc[n, m] = cc;
                    }
                }
                return StartSocketPool(mcc, avg, ta);
            }

            protected virtual void OnSocketPoolEvent(tagSocketPoolEvent spe, THandler AsyncServiceHandler)
            {

            }

            public event DOnSocketPoolEvent SocketPoolEvent;
            public event DDoSslServerAuthentication DoSslServerAuthentication;

            private THandler MapToHandler(IntPtr h)
            {
                lock (m_cs)
                {
                    foreach (CClientSocket cs in m_dicSocketHandler.Keys)
                    {
                        if (cs.Handle == h)
                            return m_dicSocketHandler[cs];
                    }
                    return null;
                }
            }

            private THandler m_pHFrom = null;

            private void OnSPEvent(uint poolId, tagSocketPoolEvent spe, IntPtr h)
            {
                THandler handler = MapToHandler(h);
                switch (spe)
                {
                    case tagSocketPoolEvent.speTimer:
                        if (CScopeUQueue.MemoryConsumed / 1024 > CScopeUQueue.SHARED_BUFFER_CLEAN_SIZE)
                            CScopeUQueue.DestroyUQueuePool();
                        break;
                    case tagSocketPoolEvent.speStarted:
                        lock (m_cs)
                        {
                            m_nPoolId = poolId;
                        }
                        break;
                    case tagSocketPoolEvent.speShutdown:
                        lock (m_cs)
                        {
                            m_dicSocketHandler.Clear();
                        }
                        break;
                    case tagSocketPoolEvent.speUSocketCreated:
                        {
                            CClientSocket cs = new CClientSocket();
                            cs.Set(h);
                            ClientCoreLoader.SetRecvTimeout(h, m_recvTimeout);
                            ClientCoreLoader.SetConnTimeout(h, m_connTimeout);
                            ClientCoreLoader.SetAutoConn(h, (byte)(m_autoConn ? 1 : 0));
                            handler = new THandler();
                            if (handler.SvsID == 0)
                                handler.m_nServiceId = m_ServiceId;
                            if (handler.SvsID <= SocketProAdapter.BaseServiceID.sidStartup)
                                throw new InvalidOperationException("Service id must be larger than SocketProAdapter.BaseServiceID.sidStartup");
                            handler.Attach(cs);
                            lock (m_cs)
                            {
                                m_dicSocketHandler[cs] = handler;
                            }
                        }
                        break;
                    case tagSocketPoolEvent.speUSocketKilled:
                        if (handler != null)
                        {
                            lock (m_cs)
                            {
                                m_dicSocketHandler.Remove(handler.AttachedClientSocket);
                            }
                        }
                        break;
                    case tagSocketPoolEvent.speConnecting:
                        break;
                    case tagSocketPoolEvent.speConnected:
                        if (ClientCoreLoader.IsOpened(h) != 0)
                        {
                            CClientSocket cs = handler.AttachedClientSocket;
                            if (DoSslServerAuthentication != null && cs.EncryptionMethod == tagEncryptionMethod.TLSv1 && !DoSslServerAuthentication.Invoke(this, cs))
                                return; //don't set password or call SwitchTo in case failure of ssl server authentication on certificate from server
                            ClientCoreLoader.SetSockOpt(h, tagSocketOption.soRcvBuf, 116800, tagSocketLevel.slSocket);
                            ClientCoreLoader.SetSockOpt(h, tagSocketOption.soSndBuf, 116800, tagSocketLevel.slSocket);
                            ClientCoreLoader.SetSockOpt(h, tagSocketOption.soTcpNoDelay, 1, tagSocketLevel.slTcp);
                            ClientCoreLoader.SetPassword(h, cs.ConnectionContext.GetPassword());
                            bool ok = ClientCoreLoader.StartBatching(h) != 0;
                            ok = ClientCoreLoader.SwitchTo(h, handler.SvsID) != 0;
                            ok = ClientCoreLoader.TurnOnZipAtSvr(h, (byte)(cs.ConnectionContext.Zip ? 1 : 0)) != 0;
                            ok = ClientCoreLoader.SetSockOptAtSvr(h, tagSocketOption.soRcvBuf, 116800, tagSocketLevel.slSocket) != 0;
                            ok = ClientCoreLoader.SetSockOptAtSvr(h, tagSocketOption.soSndBuf, 116800, tagSocketLevel.slSocket) != 0;
                            ok = ClientCoreLoader.SetSockOptAtSvr(h, tagSocketOption.soTcpNoDelay, 1, tagSocketLevel.slTcp) != 0;
                            SetQueue(cs);
                            ok = (ClientCoreLoader.CommitBatching(h, (byte)0) != 0);
                        }
                        break;
                    case tagSocketPoolEvent.speQueueMergedFrom:
                        m_pHFrom = MapToHandler(h);
#if DEBUG
                        IClientQueue cq = m_pHFrom.AttachedClientSocket.ClientQueue;
                        uint remaining = (uint)m_pHFrom.RequestsQueued;
                        if (cq.MessageCount != remaining)
                        {
                            Console.WriteLine("From: Messages = {0}, remaining requests = {1}", cq.MessageCount, remaining);
                        }
#endif
                        break;
                    case tagSocketPoolEvent.speQueueMergedTo:
                        {
                            THandler to = MapToHandler(h);
                            m_pHFrom.AppendTo(to);
                            m_pHFrom = null;
                        }
                        break;
                    default:
                        break;
                }
                lock (m_cs)
                {
                    if (SocketPoolEvent != null)
                        SocketPoolEvent.Invoke(this, spe, handler);
                }
                OnSocketPoolEvent(spe, handler);
            }

            private void CleanUp()
            {
                lock (m_cs)
                {
                    SocketPoolEvent = null;
                }
                DisconnectAll();
                ShutdownPool();
            }

            ~CSocketPool()
            {
                CleanUp();
            }

            #region IDisposable Members

            public void Dispose()
            {
                CleanUp();
            }

            #endregion
        }
    }
}
