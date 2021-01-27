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

        public string Config {
            get {
                DataContractJsonSerializer ser = new DataContractJsonSerializer(typeof(CSpConfig));
                MemoryStream ms = new MemoryStream();
                ser.WriteObject(ms, this);
                byte[] json = ms.ToArray();
                ms.Close();
                return System.Text.Encoding.UTF8.GetString(json, 0, json.Length);
            }
        }

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

        private static string ToStr(byte[] pk) {
            System.Text.StringBuilder hex = new System.Text.StringBuilder(pk.Length * 2);
            foreach (byte b in pk) {
                hex.AppendFormat("{0:x2}", b);
            }
            return hex.ToString();
        }

        internal bool Verify(CClientSocket cs) {
            int errCode;
            IUcert cert = cs.UCert;
            string res = cert.Verify(out errCode);
            //do ssl server certificate authentication here
            if (errCode == 0) return true;
            if (m_KeysAllowed != null) {
                return (m_KeysAllowed.IndexOf(ToStr(cert.PublicKey)) != -1);
            }
            return false;
        }

        private bool HasHostKey(string hkey) {
            foreach (var key in m_Hosts.Keys) {
                if (key == hkey) return true;
            }
            return false;
        }

        internal List<string> m_vPK = null;

        internal void CheckErrors() {
            if (m_KeysAllowed != null) {
                int count = m_KeysAllowed.Count;
                for (int n = 0; n < count; ++n) {
                    string s = m_KeysAllowed[n];
                    m_KeysAllowed[n] = s.Trim().ToLower();
                }
            }

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
                    throw new Exception("Connection context for host " + key + " duplicated");
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

    public static class SpManager {
        private static object m_cs = new object();
        private static CSpConfig m_sc = null;
        private static bool m_Middle = false;

        public static bool MidTier {
            get { return m_Middle; }
            internal set { m_Middle = value; }
        }

        /// <summary>
        /// Set socket pools configuration from a JSON text file
        /// </summary>
        /// <param name="midTier">True if calling from a middle tier; Otherwise, false</param>
        /// <param name="jsonConfig">A file path to a JSON configuration text file, which defaults to sp_config.json at current directory</param>
        /// <returns>An instance of CSpConfig</returns>
        public static CSpConfig SetConfig(bool midTier = false, string jsonConfig = null) {
            lock (m_cs) {
                if (m_sc != null)
                    return m_sc;
            }
            if (jsonConfig == null || jsonConfig.Length == 0) {
                jsonConfig = "sp_config.json";
            }
            using (StreamReader sr = File.OpenText(jsonConfig)) {
                string json = sr.ReadToEnd();
                MemoryStream ms = new MemoryStream(System.Text.Encoding.UTF8.GetBytes(json));
                DataContractJsonSerializer ser = new DataContractJsonSerializer(typeof(CSpConfig), new DataContractJsonSerializerSettings { UseSimpleDictionaryFormat = true });
                CSpConfig sc = ser.ReadObject(ms) as CSpConfig;
                sc.CheckErrors();
                lock (m_cs) {
                    m_sc = sc;
                    m_Middle = midTier;
                    return m_sc;
                }
            }
        }

        public static CSpConfig Config {
            get {
                lock (m_cs) {
                    if (m_sc == null)
                        return null;
                    m_sc.WorkingDir = CClientSocket.QueueConfigure.WorkDirectory;
                    return m_sc;
                }
            }
        }

        /// <summary>
        /// The number of running socket pools
        /// </summary>
        public static uint Pools {
            get {
                return ClientCoreLoader.GetNumberOfSocketPools();
            }
        }

        /// <summary>
        /// Client core library version string
        /// </summary>
        public static string Version {
            get {
                return CClientSocket.Version;
            }
        }

        public static dynamic GetPool(string poolKey) {
            if (poolKey == null || poolKey.Length == 0)
                throw new Exception("Pool key cannot be empty");
            CSpConfig sc = SetConfig();
            lock (m_cs) {
                if (sc.m_vPK.IndexOf(poolKey) == -1)
                    throw new Exception("Pool key " + poolKey + " cannot be found from configuaration");
                CPoolConfig pc = null;
                if (sc.m_Pools.ContainsKey(poolKey))
                    pc = sc.m_Pools[poolKey];
                else {
                    foreach (var key in sc.m_Pools.Keys) {
                        CPoolConfig p = sc.m_Pools[key];
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
                    case CMysql.sidMysql: {
                            CSocketPool<CMysql> mysql;
                            switch (pc.PoolType) {
                                case tagPoolType.Slave:
                                    mysql = new CSqlMasterPool<CMysql, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                case tagPoolType.Master:
                                    mysql = new CSqlMasterPool<CMysql, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                default:
                                    mysql = new CSocketPool<CMysql>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                                    break;
                            }
                            mysql.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = mysql;
                        }
                        break;
                    case BaseServiceID.sidODBC: {
                            CSocketPool<COdbc> odbc;
                            switch (pc.PoolType) {
                                case tagPoolType.Slave:
                                    odbc = new CSqlMasterPool<COdbc, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                case tagPoolType.Master:
                                    odbc = new CSqlMasterPool<COdbc, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                default:
                                    odbc = new CSocketPool<COdbc>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                                    break;
                            }
                            odbc.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = odbc;
                        }
                        break;
                    case CSqlite.sidSqlite: {
                            CSocketPool<CSqlite> sqlite;
                            switch (pc.PoolType) {
                                case tagPoolType.Slave:
                                    sqlite = new CSqlMasterPool<CSqlite, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                case tagPoolType.Master:
                                    sqlite = new CSqlMasterPool<CSqlite, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout);
                                    break;
                                default:
                                    sqlite = new CSocketPool<CSqlite>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                                    break;
                            }
                            sqlite.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = sqlite;
                        }
                        break;
                    case BaseServiceID.sidFile: {
                            CSocketPool<CStreamingFile> sf = new CSocketPool<CStreamingFile>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                            sf.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = sf;
                        }
                        break;
                    case BaseServiceID.sidQueue: {
                            var aq = new CSocketPool<CAsyncQueue>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout);
                            aq.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = aq;
                        }
                        break;
                    default: {
                            CSocketPool<CCachedBaseHandler> cbh;
                            switch (pc.PoolType) {
                                case tagPoolType.Slave:
                                    cbh = new CMasterPool<CCachedBaseHandler, CDataSet>.CSlavePool(pc.DefaultDb, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout, pc.SvsId);
                                    break;
                                case tagPoolType.Master:
                                    cbh = new CMasterPool<CCachedBaseHandler, CDataSet>(pc.DefaultDb, m_Middle, pc.RecvTimeout, pc.AutoConn, pc.ConnTimeout, pc.SvsId);
                                    break;
                                default:
                                    cbh = new CSocketPool<CCachedBaseHandler>(pc.AutoConn, pc.RecvTimeout, pc.ConnTimeout, pc.SvsId);
                                    break;
                            }
                            cbh.DoSslServerAuthentication += (sender, cs) => { return m_sc.Verify(cs); };
                            pool = cbh;
                        }
                        break;
                }
                pool.QueueName = pc.Queue;
                pc.Pool = pool;
                CConnectionContext[,] ppCC = new CConnectionContext[pc.Threads, pc.Hosts.Count];
                for (uint i = 0; i < pc.Threads; ++i) {
                    for (int j = 0; j < pc.Hosts.Count; ++j) {
                        ppCC[i, j] = sc.m_Hosts[pc.m_Hosts[j]];
                    }
                }
                pool.StartSocketPool(ppCC);
                pool.QueueAutoMerge = pc.AutoMerge;
                return pool;
            }
        }

        private static bool Mysql_DoSslServerAuthentication(CSocketPool<CMysql> sender, CClientSocket cs) {
            throw new NotImplementedException();
        }

        public static dynamic SeekHandler(string poolKey) {
            dynamic pool = GetPool(poolKey);
            return pool.Seek();
        }

        public static dynamic SeekHandlerByQueue(string poolKey) {
            dynamic pool = GetPool(poolKey);
            return pool.SeekByQueue();
        }
    }
}
