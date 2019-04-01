
using System;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Collections.Generic;

namespace SocketProAdapter.ClientSide {
    public enum tagPoolType {
        Regular = 0,
        Slave = 1,
        Master = 2
    }

    [DataContract]
    public sealed class CPoolConfig {
        internal CPoolConfig() { }
        private uint m_SvsId;
        [DataMember(IsRequired = false)]
        public uint SvsId {
            get {
                return m_SvsId;
            }
            internal set {
                m_SvsId = value;
            }
        }

        internal List<string> m_Hosts;
        [DataMember(IsRequired = true)]
        public List<string> Hosts {
            get {
                return new List<string>(m_Hosts);
            }
            internal set {
                m_Hosts = value;
            }
        }

        private string m_Queue;

        /// <summary>
        /// The name of client queue which will be used to backup client requests at client side
        /// </summary>
        [DataMember(IsRequired = false)]
        public string Queue {
            get { return m_Queue; }
            internal set { m_Queue = value; }
        }

        private string m_DefaultDb;
        [DataMember(IsRequired = false)]
        public string DefaultDb {
            get { return m_DefaultDb; }
            internal set { m_DefaultDb = value; }
        }

        private uint m_Threads;
        [DataMember(IsRequired = false)]
        public uint Threads {
            get {
                return m_Threads;
            }
            internal set {
                m_Threads = value;
            }
        }

        private tagPoolType m_PoolType = tagPoolType.Regular;
        [DataMember(IsRequired = false)]
        public tagPoolType PoolType {
            get {
                return m_PoolType;
            }
            internal set {
                m_PoolType = value;
            }
        }


        private uint m_ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;

        /// <summary>
        /// Socket connecting timeout in milliseconds
        /// </summary>
        [DataMember(IsRequired = false)]
        public uint ConnTimeout {
            get {
                return m_ConnTimeout;
            }
            internal set {
                if (value == 0)
                    m_ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
                else
                    m_ConnTimeout = value;
            }
        }

        private uint m_RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;

        /// <summary>
        /// Client request timeout in milliseconds
        /// </summary>
        [DataMember(IsRequired = false)]
        public uint RecvTimeout {
            get {
                return m_RecvTimeout;
            }
            internal set {
                if (value == 0)
                    m_RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
                else
                    m_RecvTimeout = value;
            }
        }

        /// <summary>
        /// Auto reconnecting
        /// </summary>
        /// <remarks>
        /// True for socket reconnecting. Otherwise, there is no auto reconnecting if socket is closed. The property shoud be true under most cases
        /// </remarks>
        [DataMember(IsRequired = false)]
        public bool AutoConn {
            get { return !m_nac; }
            internal set { m_nac = !value; }
        }
        private bool m_nac = false;

        private bool m_AutoMerge;

        /// <summary>
        /// True if requests, which are backed up inside a client queue, will be automatically merged into a new connected socket for processing when socket is closed.
        /// Otherwise, there are no requests auto merging
        /// </summary>
        /// <remarks>
        /// This feature is useful for fault auto recovery at client side. However, SocketPro client core may disable the auto merging if it detects that there is no new remote SocketPro server available
        /// </remarks>
        [DataMember(IsRequired = false)]
        public bool AutoMerge {
            get {
                return m_AutoMerge;
            }
            internal set {
                m_AutoMerge = value;
            }
        }

        internal Dictionary<string, CPoolConfig> m_Slaves;
        [DataMember(IsRequired = false)]
        public Dictionary<string, CPoolConfig> Slaves {
            get {
                if (m_Slaves == null) return null;
                return new Dictionary<string, CPoolConfig>(m_Slaves);
            }
            internal set { m_Slaves = value; }
        }

        internal string Master = null;

        internal dynamic Pool = null;

        internal void Normalize(string key) {
            Master = null;
            if (Queue != null) {
                Queue = Queue.Trim();
                if (Defines.OperationSystem == tagOperationSystem.osWin || Defines.OperationSystem == tagOperationSystem.osWinCE)
                    Queue = Queue.ToLower();
                if (Queue.Length == 0)
                    AutoMerge = false;
            } else
                AutoMerge = false;
            if (Threads == 0)
                Threads = 1;
            if (ConnTimeout == 0)
                ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
            if (RecvTimeout == 0)
                RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
            if (DefaultDb != null)
                DefaultDb = DefaultDb.Trim();
            if (DefaultDb != null && DefaultDb.Length > 0)
                PoolType = tagPoolType.Master;
            else {
                PoolType = tagPoolType.Regular;
                if (m_Slaves != null && m_Slaves.Count > 0)
                    throw new Exception("Regular pool cannot contain any slave pool");
            }
            switch (SvsId) {
                case BaseServiceID.sidHTTP:
                    throw new Exception("Client side does not support HTTP/websocket service");
                case BaseServiceID.sidQueue:
                    if ((DefaultDb != null && DefaultDb.Length > 0) || m_Slaves != null)
                        throw new Exception("Server queue service does not support master or slave pool");
                    break;
                case BaseServiceID.sidFile:
                    if ((DefaultDb != null && DefaultDb.Length > 0) || m_Slaves != null)
                        throw new Exception("Remote file service does not support master or slave pool");
                    break;
                case BaseServiceID.sidODBC:
                case CSqlite.sidSqlite:
                case CMysql.sidMysql:
                    break;
                default:
                    if (SvsId <= BaseServiceID.sidReserved)
                        throw new Exception("User defined service id must be larger than " + BaseServiceID.sidReserved);
                    break;
            }
            if ((DefaultDb == null || DefaultDb.Length == 0) && m_Slaves != null)
                throw new Exception("Slave array is not empty but DefaultDb string is empty");
            if (m_Slaves != null && PoolType == tagPoolType.Master) {
                NormalizeSlaves(SvsId, DefaultDb, key);
            }
        }
        private void NormalizeSlaves(uint svsId, string defalutDb, string mkey) {
            foreach (var key in m_Slaves.Keys) {
                if (key == null || key.Length == 0)
                    throw new Exception("Slave pool key cannot be empty");
                CPoolConfig pool = m_Slaves[key];
                pool.Master = mkey;
                pool.SvsId = svsId;
                if (pool.Queue != null) {
                    pool.Queue = pool.Queue.Trim();
                    if (Defines.OperationSystem == tagOperationSystem.osWin || Defines.OperationSystem == tagOperationSystem.osWinCE)
                        pool.Queue = pool.Queue.ToLower();
                    if (pool.Queue.Length == 0)
                        pool.AutoMerge = false;
                } else
                    pool.AutoMerge = false;
                if (pool.Threads == 0)
                    pool.Threads = 1;
                if (pool.ConnTimeout == 0)
                    pool.ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
                if (pool.RecvTimeout == 0)
                    pool.RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
                if (pool.DefaultDb != null) {
                    pool.DefaultDb = pool.DefaultDb.Trim();
                    if (pool.DefaultDb.Length == 0)
                        pool.DefaultDb = defalutDb;
                } else {
                    pool.DefaultDb = defalutDb;
                }
                pool.PoolType = tagPoolType.Slave;
                if (pool.m_Slaves != null)
                    throw new Exception("A slave pool cannot contain any new slave pool");
                if (pool.m_Hosts.Count == 0)
                    throw new Exception("Slave pool host array is empty");
            }
        }
    }

    [DataContract]
    public sealed class CSpConfig {
        internal CSpConfig() { }

        internal Dictionary<string, CConnectionContext> m_Hosts = new Dictionary<string, CConnectionContext>();
        [DataMember(IsRequired = true)]
        public Dictionary<string, CConnectionContext> Hosts {
            get { return new Dictionary<string, CConnectionContext>(m_Hosts); }
            internal set { m_Hosts = value; }
        }

        internal Dictionary<string, CPoolConfig> m_Pools = new Dictionary<string, CPoolConfig>();
        [DataMember(IsRequired = true)]
        public Dictionary<string, CPoolConfig> Pools {
            get { return new Dictionary<string, CPoolConfig>(m_Pools); }
            internal set { m_Pools = value; }
        }

        private string m_WorkingDir;
        [DataMember(IsRequired = false)]
        public string WorkingDir {
            get { return m_WorkingDir; }
            internal set { m_WorkingDir = value; }
        }

        private string m_CertStore;
        [DataMember(IsRequired = false)]
        public string CertStore {
            get { return m_CertStore; }
            set { m_CertStore = value; }
        }

        private object m_QueuePassword;
        [DataMember(IsRequired = false)]
        public object QueuePassword {
            get { return m_QueuePassword; }
            set { m_QueuePassword = value; }
        }

        private List<string> m_KeysAllowed;
        [DataMember(IsRequired = false)]
        public List<string> KeysAllowed {
            get {
                if (m_KeysAllowed == null) return null;
                return new List<string>(m_KeysAllowed);
            }
            internal set { m_KeysAllowed = value; }
        }

        private bool IsRepeated(CConnectionContext cc) {
            int count = 0;
            foreach (var c in m_Hosts.Values) {
                if (c != cc) {
                    count += cc.IsSame(c) ? 1 : 0;
                }
            }
            return count > 0;
        }

        private bool HasHostKey(string hkey) {
            foreach (var key in m_Hosts.Keys) {
                if (key == hkey) return true;
            }
            return false;
        }

        internal List<string> m_vPK = null;

        internal void CheckErrors() {
            if (QueuePassword != null) {
                if (QueuePassword.GetType() == typeof(string)) {
                    CClientSocket.QueueConfigure.MessageQueuePassword = QueuePassword.ToString();
                    QueuePassword = 1;
                } else {
                    QueuePassword = 0;
                }
            } else {
                QueuePassword = 0;
            }
            if (WorkingDir != null && WorkingDir.Length > 0) {
                CClientSocket.QueueConfigure.WorkDirectory = WorkingDir;
            }
            if (CertStore != null && CertStore.Length > 0) {
                if (!CClientSocket.SSL.SetVerifyLocation(CertStore)) {
                    throw new Exception("Cannot set certificate store for verifying coming server certificates");
                }
            }
            if (m_Hosts.Count == 0) {
                throw new Exception("Host map cannot be empty");
            }
            foreach (var cc in m_Hosts.Values) {
                cc.Normalize();
            }

            foreach (var key in m_Hosts.Keys) {
                if (key == null || key.Length == 0)
                    throw new Exception("Host key cannot be empty");
                var cc = m_Hosts[key];
                if (cc.Host.Length == 0)
                    throw new Exception("Host " + key + " ip address cannot be empty");
                if (cc.Port == 0)
                    throw new Exception("Host " + key + " port number cannot be zero");
                if (IsRepeated(m_Hosts[key]))
                    throw new Exception("Host " + key + " duplicated");
            }
            if (m_Pools.Count == 0) {
                throw new Exception("Pool map cannot be empty");
            }
            m_vPK = new List<string>(m_Pools.Keys);
            foreach (var key in m_Pools.Keys) {
                if (key == null || key.Length == 0)
                    throw new Exception("Pool key cannot be empty");
                var p = m_Pools[key];
                p.Normalize(key);
                foreach (string hkey in p.m_Hosts) {
                    if (!HasHostKey(hkey))
                        throw new Exception("Host key " + hkey + " not found in hosts");
                }
                if (p.m_Slaves == null)
                    continue;
                foreach (var sk in p.m_Slaves.Keys) {
                    if (m_vPK.IndexOf(sk) >= 0)
                        throw new Exception("Slave pool key " + sk + " duplicated");
                    m_vPK.Add(sk);
                    foreach (string shkey in p.m_Slaves[sk].m_Hosts) {
                        if (!HasHostKey(shkey))
                            throw new Exception("Slave host key " + shkey + " not found in hosts");
                    }
                }

            }
        }
    }
}

namespace SocketProAdapter {
    public static class SpManager {
        private static object m_cs = new object();
        private static ClientSide.CSpConfig m_sc = null;
        private static bool m_Middle = false;

        public static bool MidTier {
            get { return m_Middle; }
            internal set { m_Middle = value; }
        }

        public static ClientSide.CSpConfig SetConfig(bool midTier = false, string jsConfig = null) {
            lock (m_cs) {
                if (m_sc != null)
                    return m_sc;
            }
            if (jsConfig == null || jsConfig.Length == 0) {
                jsConfig = "sp_config.json";
            }
            using (StreamReader sr = File.OpenText(jsConfig)) {
                string json = sr.ReadToEnd();
                MemoryStream ms = new MemoryStream(System.Text.Encoding.UTF8.GetBytes(json));
                DataContractJsonSerializer ser = new DataContractJsonSerializer(typeof(ClientSide.CSpConfig), new DataContractJsonSerializerSettings { UseSimpleDictionaryFormat = true });
                ClientSide.CSpConfig sc = ser.ReadObject(ms) as ClientSide.CSpConfig;
                sc.CheckErrors();
                lock (m_cs) {
                    m_sc = sc;
                    m_Middle = midTier;
                    return m_sc;
                }
            }
        }

        public static ClientSide.CSpConfig Config {
            get {
                lock (m_cs) {
                    if (m_sc == null)
                        return null;
                    m_sc.WorkingDir = ClientSide.CClientSocket.QueueConfigure.WorkDirectory;
                    return m_sc;
                }
            }
        }

        /// <summary>
        /// The number of running socket pools
        /// </summary>
        public static uint Pools {
            get {
                return ClientSide.ClientCoreLoader.GetNumberOfSocketPools();
            }
        }

        /// <summary>
        /// Client core library version string
        /// </summary>
        public static string Version {
            get {
                return ClientSide.CClientSocket.Version;
            }
        }

        public static dynamic GetPool(string poolKey) {
            if (poolKey == null || poolKey.Length == 0)
                throw new Exception("Pool key cannot be empty");
            ClientSide.CSpConfig sc = SetConfig();
            if (sc.m_vPK.IndexOf(poolKey) == -1)
                throw new Exception("Pool key cannot be found from configuaration");
            ClientSide.CPoolConfig pc = null;
            if (sc.m_Pools.ContainsKey(poolKey))
                pc = sc.m_Pools[poolKey];
            else {
                foreach (var key in sc.m_Pools.Keys) {
                    ClientSide.CPoolConfig p = sc.m_Pools[key];
                    if (p.m_Slaves == null)
                        continue;
                    if (p.m_Slaves.ContainsKey(poolKey)) {
                        pc = p.m_Slaves[poolKey];
                        break;
                    }
                }
            }
            if (pc.Pool != null)
                return pc.Pool;
            dynamic pool;
            switch (pc.SvsId) {
                case ClientSide.CMysql.sidMysql:
                    switch (pc.PoolType) {
                        case ClientSide.tagPoolType.Slave:
                            pool = new CSqlMasterPool<ClientSide.CMysql, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        case ClientSide.tagPoolType.Master:
                            pool = new CSqlMasterPool<ClientSide.CMysql, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        default:
                            pool = new ClientSide.CSocketPool<ClientSide.CMysql>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout, pc.SvsId);
                            break;
                    }
                    break;
                case BaseServiceID.sidODBC:
                    switch (pc.PoolType) {
                        case ClientSide.tagPoolType.Slave:
                            pool = new CSqlMasterPool<ClientSide.COdbc, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        case ClientSide.tagPoolType.Master:
                            pool = new CSqlMasterPool<ClientSide.COdbc, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        default:
                            pool = new ClientSide.CSocketPool<ClientSide.COdbc>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout, pc.SvsId);
                            break;
                    }
                    break;
                case ClientSide.CSqlite.sidSqlite:
                    switch (pc.PoolType) {
                        case ClientSide.tagPoolType.Slave:
                            pool = new CSqlMasterPool<ClientSide.CSqlite, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        case ClientSide.tagPoolType.Master:
                            pool = new CSqlMasterPool<ClientSide.CSqlite, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                            break;
                        default:
                            pool = new ClientSide.CSocketPool<ClientSide.CSqlite>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                            break;
                    }
                    break;
                case BaseServiceID.sidFile:
                    pool = new ClientSide.CSocketPool<ClientSide.CStreamingFile>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                    break;
                case BaseServiceID.sidQueue:
                    pool = new ClientSide.CSocketPool<ClientSide.CAsyncQueue>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                    break;
                default:
                    switch (pc.PoolType) {
                        case ClientSide.tagPoolType.Slave:
                            pool = new CMasterPool<ClientSide.CCachedBaseHandler, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout, pc.SvsId);
                            break;
                        case ClientSide.tagPoolType.Master:
                            pool = new CMasterPool<ClientSide.CCachedBaseHandler, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout, pc.SvsId);
                            break;
                        default:
                            pool = new ClientSide.CSocketPool<ClientSide.CCachedBaseHandler>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout, pc.SvsId);
                            break;
                    }
                    break;
            }
            pool.QueueName = pc.Queue;
            pool.QueueAutoMerge = pc.AutoMerge;
            pc.Pool = pool;
            ClientSide.CConnectionContext[,] ppCC = new ClientSide.CConnectionContext[pc.Threads, pc.Hosts.Count];
            for (uint i = 0; i < pc.Threads; ++i) {
                for (int j = 0; j < pc.Hosts.Count; ++j) {
                    ppCC[i, j] = sc.m_Hosts[pc.m_Hosts[j]];
                }
            }
            if (!pool.StartSocketPool(ppCC)) {
                throw new Exception("There is no connection establised for pool " + poolKey);
            }
            return pool;
        }

        public static dynamic SeekHandler(string poolKey) {
            dynamic pool = GetPool(poolKey);
            return pool.Seek();
        }

        public static dynamic SeekHandlerByQueue(string poolKey) {
            dynamic pool = GetPool(poolKey);
            return pool.SeekByQueue();
        }

        public static dynamic LockHandler(string poolKey, uint timeout = uint.MaxValue) {
            dynamic pool = GetPool(poolKey);
            return pool.Lock(timeout);
        }
    }
}
