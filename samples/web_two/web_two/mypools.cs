using SocketProAdapter.ClientSide;
namespace SPA {
    using CSql = CMysql; //point to one of CMysql, COdbc+MSSQL and CSQLite
    public class CMyMaster : SocketProAdapter.CSqlMasterPool<CSql, SocketProAdapter.CDataSet> {
        public CMyMaster(string defaultDB) : base(defaultDB) { }
        public CMyMaster(string defaultDB, bool midTier) : base(defaultDB, midTier) { }
        public CMyMaster(string defaultDB, bool midTier, uint recvTimeout) : base(defaultDB, midTier, recvTimeout) { }

        /// <summary>
        /// Lock a handler from socket pool. You must call this method with UnlockByMyAlgorithm in pair
        /// </summary>
        /// <param name="timeout">The max time for locking a handler in ms</param>
        /// <returns>A SQL handler from socket pool</returns>
        public CSql LockByMyAlgorithm(int timeout) {
            CSql sql = null;
            System.Threading.Monitor.Enter(m_cs);
            while (timeout >= 0) {
                CClientSocket ret = null;
                foreach (CClientSocket cs in m_dicSocketHandler.Keys) {
                    IClientQueue cq = cs.ClientQueue;
                    if (!cq.Available || cq.JobSize > 0/*queue is in transaction at this time*/)
                        continue; //A null handler may return if local message queue is in transaction
                    if (ret == null) ret = cs;
                    else if (cs.Connected) {
                        if (!ret.Connected)
                            ret = cs;
                        else if (cq.MessageCount < ret.ClientQueue.MessageCount)
                            ret = cs;
                    } else if (!ret.Connected && cq.MessageCount < ret.ClientQueue.MessageCount)
                        ret = cs;
                }
                if (ret != null) {
                    sql = m_dicSocketHandler[ret];
                    m_dicSocketHandler.Remove(ret);
                }
                if (sql != null) break;
                System.DateTime dtNow = System.DateTime.Now;
                bool ok = System.Threading.Monitor.Wait(m_cs, timeout);
                int ts = (int)(System.DateTime.Now - dtNow).TotalMilliseconds;
                if (!ok || timeout <= ts) break;
                timeout -= ts;
            }
            System.Threading.Monitor.Exit(m_cs);
            return sql; //A null handler may return in case time-out
        }
        /// <summary>
        /// Put original handler back into socket pool for reuse. You must call this method after calling LockByMyAlgorithm
        /// </summary>
        /// <param name="sql">A handler originated from LockByMyAlgorithm</param>
        public void UnlockByMyAlgorithm(CSql sql) {
            if (sql == null) return;
            CClientSocket cs = sql.AttachedClientSocket;
            lock (m_cs) {
                if (!m_dicSocketHandler.ContainsKey(cs)) {
                    m_dicSocketHandler[cs] = sql;
                    System.Threading.Monitor.PulseAll(m_cs);
                }
            }
        }
    }
}
