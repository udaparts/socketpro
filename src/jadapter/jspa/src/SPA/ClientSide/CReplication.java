package SPA.ClientSide;

public class CReplication<THandler extends CAsyncServiceHandler> {

    private volatile CSocketPool<THandler> m_pool = null;
    private final java.util.HashMap<String, CConnectionContext> m_mapQueueConn = new java.util.HashMap<>();
    private volatile ReplicationSetting m_rs;
    public final static String DIR_SEP = System.getProperty("file.separator");

    public final int getConnections() {
        if (m_pool == null) {
            return 0;
        }
        return m_pool.getConnectedSockets();
    }
    
    /**
     * Shutdown its internal socket pool and dispose this object. Don't use the object after calling this method
     */
    public void Dispose() {
        Cleanup();
    }

    private void Cleanup() {
        if (m_pool != null) {
            m_pool.ShutdownPool();
            m_pool = null;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        Cleanup();
        super.finalize();
    }

    protected boolean DoSslServerAuthentication(CClientSocket cs) {
        return true;
    }

    private void CheckReplicationSetting(ReplicationSetting qms) {
        if (qms.TTL == 0) {
            qms.TTL = ReplicationSetting.DEAFULT_TTL;
        }
        if (qms.ConnTimeout == 0) {
            qms.ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
        }
        if (qms.RecvTimeout == 0) {
            qms.RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
        }
        if (qms.QueueDir == null) {
            throw new UnsupportedOperationException("An absolute path required for working directory");
        }
        qms.QueueDir = qms.QueueDir.trim();
        if (qms.QueueDir.length() == 0) {
            throw new UnsupportedOperationException("An absolute path required for working directory");
        }
        if (qms.QueueDir.lastIndexOf(DIR_SEP) != qms.QueueDir.length() - 1) {
            throw new UnsupportedOperationException("An absolute path must be ended with the char " + DIR_SEP);
        }
        //make sure directories existing
        if (!(new java.io.File(qms.QueueDir)).isDirectory()) {
            (new java.io.File(qms.QueueDir)).mkdir();
        }
        m_rs = qms.Copy();
    }

    private void EndProcess(String[] vDbFullName, boolean secure, CConnectionContext[][] mcc, SPA.tagThreadApartment ta) {
        if (secure) {
            m_pool.DoSslServerAuthentication = new CSocketPool.DDoSslServerAuthentication() {
                @Override
                public boolean invoke(CSocketPool sender, CClientSocket cs) {
                    if (cs.ConnectionContext.Port == -1) {
                        return true;
                    }
                    return DoSslServerAuthentication(cs);
                }
            };
        }
        m_pool.SocketPoolEvent = new CSocketPool.DOnSocketPoolEvent() {
            @Override
            public void invoke(CSocketPool sender, tagSocketPoolEvent spe, CAsyncServiceHandler handler) {
                switch (spe) {
                    case speSocketClosed:
                        handler.CleanCallbacks();
                        break;
                    case speConnecting:
                        if (handler.getAttachedClientSocket().ConnectionContext.Port == -1) {
                            handler.getAttachedClientSocket().setConnectingTimeout(500);
                        }
                        break;
                    default:
                        break;
                }
            }
        };
        boolean ok = m_pool.StartSocketPool(mcc, true, ta);
        int n = 0;
        for (CClientSocket s : m_pool.getSockets()) {
            String key = vDbFullName[n];
            ok = s.getClientQueue().StartQueue(key, m_rs.TTL, secure);
            ++n;
        }
        if (getReplicable()) {
            getSourceQueue().EnsureAppending(getTargetQueues());
        }
        THandler[] targetHandlers = getTargetHandlers();
        for (THandler h : targetHandlers) {
            ok = h.getAttachedClientSocket().DoEcho();
        }
    }

    private void CheckConnQueueName(java.util.HashMap<String, CConnectionContext> mapQueueConn) {
        m_mapQueueConn.clear();
        if (mapQueueConn == null || mapQueueConn.isEmpty()) {
            throw new IllegalArgumentException("One middle server required at least");
        }
        for (String key : mapQueueConn.keySet()) {
            CConnectionContext cc = mapQueueConn.get(key);
            if (cc == null) {
                throw new UnsupportedOperationException("An invalid host found");
            }
            String v = key;
            if (v == null) {
                throw new UnsupportedOperationException("A non-empty string for persistent queue name required for each of hosts");
            }
            v = v.trim();
            if (v.length() == 0) {
                throw new UnsupportedOperationException("A non-empty string for persistent queue name required for each of hosts");
            }
            if (DoesQueueExist(v)) {
                throw new UnsupportedOperationException("Queue name duplicated -- " + v);
            }
            m_mapQueueConn.put(v, cc);
        }
    }

    /**
     * Start a socket pool for replication
     *
     * @param mapQueueConn A dictionary for message queue name and connecting
     * context. a unique name must be specified for each of connecting contexts
     * @return True if there is at least one connection established; and false
     * if there is no connection
     */
    public final boolean Start(java.util.HashMap<String, CConnectionContext> mapQueueConn) {
        return Start(mapQueueConn, null, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a socket pool for replication
     *
     * @param mapQueueConn A dictionary for message queue name and connecting
     * context. a unique name must be specified for each of connecting contexts
     * @param rootQueueName A string for root replication queue name. It is
     * ignored if it is not replicable
     * @return True if there is at least one connection established; and false
     * if there is no connection
     */
    public final boolean Start(java.util.HashMap<String, CConnectionContext> mapQueueConn, String rootQueueName) {
        return Start(mapQueueConn, rootQueueName, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a socket pool for replication
     *
     * @param mapQueueConn A dictionary for message queue name and connecting
     * context. a unique name must be specified for each of connecting contexts
     * @param rootQueueName A string for root replication queue name. It is
     * ignored if it is not replicable
     * @param ta COM thread apartment; and it defaults to taNone. It is ignored
     * on non-window platforms
     * @return True if there is at least one connection established; and false
     * if there is no connection
     */
    public final boolean Start(java.util.HashMap<String, CConnectionContext> mapQueueConn, String rootQueueName, SPA.tagThreadApartment ta) {
        CheckConnQueueName(mapQueueConn);
        int n = 0;
        boolean secure = false;
        int all = m_mapQueueConn.size();
        if (all > 1) {
            ++all;
        }
        if (rootQueueName == null) {
            rootQueueName = "";
        }
        rootQueueName = rootQueueName.trim();
        if (rootQueueName.length() == 0) {
            String appName = "java";//System.AppDomain.CurrentDomain.FriendlyName;
            int dot = appName.lastIndexOf('.');
            if (dot == -1) {
                rootQueueName = appName;
            } else {
                rootQueueName = appName.substring(0, dot);
            }
        }
        String[] vDbFullName = new String[all];
        CConnectionContext[][] mcc = new CConnectionContext[1][all];
        for (String key : m_mapQueueConn.keySet()) {
            mcc[0][n] = m_mapQueueConn.get(key);
            if (!secure && mcc[0][n].EncrytionMethod == SPA.tagEncryptionMethod.TLSv1) {
                secure = true;
            }
            vDbFullName[n] = m_rs.QueueDir + key;
            ++n;
        }
        if (all > 1) {
            CConnectionContext last = new CConnectionContext("127.0.0.1", -1, "UReplication", "");
            last.EncrytionMethod = secure ? SPA.tagEncryptionMethod.TLSv1 : SPA.tagEncryptionMethod.NoEncryption;
            mcc[0][n] = last;
            vDbFullName[n] = m_rs.QueueDir + rootQueueName;
        }
        EndProcess(vDbFullName, secure, mcc, ta);
        return (m_pool.getConnectedSockets() > 0);
    }

    /**
     * Construct a CSqlReplication instance
     *
     * @param impl Class for generic type THandler
     * @param qms A structure for setting its underlying socket pool and message
     * queue directory as well as password for source queue
     */
    public CReplication(Class<THandler> impl, ReplicationSetting qms) {
        CheckReplicationSetting(qms);
        m_pool = new CSocketPool<>(impl, !m_rs.NoAutoConn, m_rs.RecvTimeout, m_rs.ConnTimeout);
    }

    public final boolean getReplicable() {
        return (m_mapQueueConn.size() > 1);
    }

    public final THandler getSourceHandler() {
        THandler one = null;
        if (m_pool != null) {
            for (THandler h : m_pool.getAsyncHandlers()) {
                one = h;
            }
        }
        return one;
    }

    public final IClientQueue getSourceQueue() {
        THandler src = getSourceHandler();
        if (src == null) {
            return null;
        }
        return src.getAttachedClientSocket().getClientQueue();
    }

    public final IClientQueue[] getTargetQueues() {
        int n = 0;
        THandler[] handlers = getTargetHandlers();
        if (handlers == null) {
            return null;
        }
        IClientQueue[] tq = new IClientQueue[handlers.length];
        for (THandler h : handlers) {
            tq[n] = h.getAttachedClientSocket().getClientQueue();
            ++n;
        }
        return tq;
    }
    private volatile THandler[] m_TargetHandlers = null;

    @SuppressWarnings("unchecked")
    public final THandler[] getTargetHandlers() {
        if (m_pool == null) {
            return null;
        }
        if (m_TargetHandlers != null) {
            return m_TargetHandlers;
        }
        THandler[] handlers = m_pool.getAsyncHandlers();
        if (handlers.length == 0) {
            return null;
        }
        if (handlers.length == 1) {
            m_TargetHandlers = (THandler[]) java.lang.reflect.Array.newInstance(handlers[0].getClass(), 1);
        } else {
            m_TargetHandlers = (THandler[]) java.lang.reflect.Array.newInstance(handlers[0].getClass(), handlers.length - 1);
        }
        int n = 0;
        for (THandler h : handlers) {
            m_TargetHandlers[n] = h;
            ++n;
            if (n >= (handlers.length - 1)) {
                break;
            }
        }
        return m_TargetHandlers;
    }

    public final int getHosts() {
        return (int) m_mapQueueConn.size();
    }

    public final int getQueues() {
        if (m_pool == null) {
            return 0;
        }
        return m_pool.getQueues();
    }

    public final ReplicationSetting getReplicationSetting() {
        return m_rs;
    }

    /**
     * Make a replication. An invalid operation exception will be thrown if not
     * replicable.
     *
     * @return True for success; and false for failure
     */
    public final boolean DoReplication() {
        if (m_mapQueueConn.size() == 1) {
            throw new UnsupportedOperationException("No replication is allowed because the number of target message queues less than two");
        }
        IClientQueue src = getSourceQueue();
        if (src == null) {
            return false;
        }
        return src.AppendTo(getTargetQueues());
    }

    private boolean DoesQueueExist(String qName) {
        boolean ignoreCase;
        switch (SPA.CUQueue.DEFAULT_OS) {
            case osWin:
            case osWinCE:
                ignoreCase = true;
                break;
            default:
                ignoreCase = false;
                break;
        }
        for (String name : m_mapQueueConn.keySet()) {
            if (ignoreCase) {
                if (name.equalsIgnoreCase(qName)) {
                    return true;
                } else if (name.equals(qName)) {
                    return true;
                }
            }
        }
        return false;
    }

    public boolean Send(short reqId, byte[] data, int len) {
        CAsyncServiceHandler.DAsyncResultHandler ash = null;
        THandler src = getSourceHandler();
        if (src == null) {
            return false;
        }
        IClientQueue cq = src.getAttachedClientSocket().getClientQueue();
        if (!cq.getAvailable()) {
            return false;
        }
        boolean ok = src.SendRequest(reqId, data, len, ash);
        if (getReplicable() && cq.getJobSize() == 0) {
            ok = cq.AppendTo(getTargetQueues());
        }
        return ok;
    }

    public final boolean EndJob() {
        IClientQueue src = getSourceQueue();
        if (src == null || !src.getAvailable()) {
            return false;
        }
        boolean ok = src.EndJob();
        if (ok && getReplicable()) {
            ok = src.AppendTo(getTargetQueues());
        }
        return ok;
    }

    public final boolean StartJob() {
        IClientQueue src = getSourceQueue();
        if (src == null || !src.getAvailable()) {
            return false;
        }
        return src.StartJob();
    }

    public final boolean AbortJob() {
        IClientQueue src = getSourceQueue();
        if (src == null || !src.getAvailable()) {
            return false;
        }
        return src.AbortJob();
    }

    public final boolean Send(short reqId) {
        return Send(reqId, (byte[]) null, (int) 0);
    }

    public final boolean Send(short reqId, SPA.CUQueue q) {
        if (q == null || q.GetSize() == 0) {
            return Send(reqId);
        }
        if (q.getHeadPosition() > 0) {
            return Send(reqId, q.GetBuffer(), q.GetSize());
        }
        return Send(reqId, q.getIntenalBuffer(), q.GetSize());
    }

    public final boolean Send(short reqId, SPA.CScopeUQueue q) {
        if (q == null) {
            return Send(reqId);
        }
        return Send(reqId, q.getUQueue());
    }
}
