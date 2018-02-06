package SPA.ClientSide;

import SPA.CScopeUQueue;

public class CSocketPool<THandler extends CAsyncServiceHandler> {
    
    private volatile java.lang.reflect.Constructor<THandler> m_AsyncHandlerCtor = null;
    
    public interface DOnSocketPoolEvent {
        
        void invoke(CSocketPool sender, tagSocketPoolEvent spe, CAsyncServiceHandler AsyncServiceHandler);
    }
    public DOnSocketPoolEvent SocketPoolEvent = null;
    
    public interface DDoSslServerAuthentication {
        
        boolean invoke(CSocketPool sender, CClientSocket cs);
    }
    public DDoSslServerAuthentication DoSslServerAuthentication = null;
    
    private THandler m_hFrom = null;
    
    private String m_qName = "";
    
    private void invoke(int poolId, int spc, long h) throws InstantiationException, IllegalAccessException, IllegalArgumentException, java.lang.reflect.InvocationTargetException {
        THandler handler = MapToHandler(h);
        tagSocketPoolEvent event = tagSocketPoolEvent.forValue(spc);
        switch (event) {
            case speTimer:
                if (CScopeUQueue.getMemoryConsumed() / 1024 > CScopeUQueue.getSHARED_BUFFER_CLEAN_SIZE()) {
                    CScopeUQueue.DestroyUQueuePool();
                }
                break;
            case speStarted:
                synchronized (m_cs) {
                    m_nPoolId = poolId;
                }
                break;
            case speShutdown:
                synchronized (m_cs) {
                    m_dicSocketHandler.clear();
                    m_nPoolId = 0;
                }
                break;
            case speUSocketCreated: {
                ClientCoreLoader.SetConnTimeout(h, m_connTimeout);
                ClientCoreLoader.SetRecvTimeout(h, m_recvTimeout);
                ClientCoreLoader.SetAutoConn(h, m_autoConn);
                CClientSocket cs = new CClientSocket(h);
                handler = m_AsyncHandlerCtor.newInstance();
                if (handler.getSvsID() == 0) {
                    handler.m_nServiceId = this.m_nServiceId;
                }
                if (handler.getSvsID() <= SPA.BaseServiceID.sidStartup) {
                    throw new UnsupportedOperationException("Service id must be larger than SocketProAdapter.BaseServiceID.sidStartup");
                }
                handler.Attach(cs);
                synchronized (m_cs) {
                    m_dicSocketHandler.put(cs, handler);
                }
            }
            break;
            case speUSocketKilled:
                if (handler != null) {
                    CClientSocket.Remove(handler.getAttachedClientSocket().getHandle());
                    synchronized (m_cs) {
                        m_dicSocketHandler.remove(handler.getAttachedClientSocket());
                    }
                }
                break;
            case speConnected:
                if (ClientCoreLoader.IsOpened(h)) {
                    CClientSocket cs = handler.getAttachedClientSocket();
                    if (cs.getEncryptionMethod() == SPA.tagEncryptionMethod.TLSv1 && DoSslServerAuthentication != null && !DoSslServerAuthentication.invoke(this, cs)) {
                        return;
                    }
                    ClientCoreLoader.SetPassword(h, cs.ConnectionContext.Password);
                    ClientCoreLoader.SetSockOpt(h, SPA.tagSocketOption.soRcvBuf.getValue(), 116800, SPA.tagSocketLevel.slSocket.getValue());
                    ClientCoreLoader.SetSockOpt(h, SPA.tagSocketOption.soSndBuf.getValue(), 116800, SPA.tagSocketLevel.slSocket.getValue());
                    ClientCoreLoader.SetSockOpt(h, SPA.tagSocketOption.soTcpNoDelay.getValue(), 1, SPA.tagSocketLevel.slTcp.getValue());
                    ClientCoreLoader.StartBatching(h);
                    boolean ok = ClientCoreLoader.SwitchTo(h, handler.getSvsID());
                    ok = ClientCoreLoader.TurnOnZipAtSvr(h, cs.ConnectionContext.Zip);
                    ok = ClientCoreLoader.SetSockOptAtSvr(h, SPA.tagSocketOption.soRcvBuf.getValue(), 116800, SPA.tagSocketLevel.slSocket.getValue());
                    ok = ClientCoreLoader.SetSockOptAtSvr(h, SPA.tagSocketOption.soSndBuf.getValue(), 116800, SPA.tagSocketLevel.slSocket.getValue());
                    ok = ClientCoreLoader.SetSockOptAtSvr(h, SPA.tagSocketOption.soTcpNoDelay.getValue(), 1, SPA.tagSocketLevel.slTcp.getValue());
                    ok = ClientCoreLoader.CommitBatching(h, false);
                }
                break;
            case speQueueMergedFrom:
                m_hFrom = MapToHandler(h);
                break;
            case speQueueMergedTo: {
                THandler to = MapToHandler(h);
                m_hFrom.AppendTo(to);
                m_hFrom = null;
            }
            break;
            default:
                break;
        }
        if (SocketPoolEvent != null) {
            SocketPoolEvent.invoke(this, event, handler);
        }
        OnSocketPoolEvent(event, handler);
        if (event == tagSocketPoolEvent.speConnected && handler.getAttachedClientSocket().getConnected()) {
            SetQueue(handler.getAttachedClientSocket());
        }
    }
    
    protected void OnSocketPoolEvent(tagSocketPoolEvent spe, THandler h) {
        
    }
    
    @Override
    protected void finalize() throws Throwable {
        Clean();
        super.finalize();
    }
    
    void Clean() {
        int poolId = 0;
        synchronized (m_cs) {
            poolId = m_nPoolId;
            m_nPoolId = 0;
        }
        if (poolId != 0) {
            ClientCoreLoader.DestroySocketPool(poolId);
        }
    }
    
    public final boolean DisconnectAll() {
        int poolId;
        synchronized (m_cs) {
            poolId = m_nPoolId;
        }
        if (poolId != 0) {
            return ClientCoreLoader.DisconnectAll(poolId);
        }
        return true;
    }
    
    public final boolean getStarted() {
        synchronized (m_cs) {
            return (ClientCoreLoader.GetThreadCount(m_nPoolId) != 0);
        }
    }
    
    public final boolean getAvg() {
        synchronized (m_cs) {
            return ClientCoreLoader.IsAvg(m_nPoolId);
        }
    }
    
    public static int getSocketPools() {
        return ClientCoreLoader.GetNumberOfSocketPools();
    }
    
    public final int getPoolId() {
        synchronized (m_cs) {
            return m_nPoolId;
        }
    }
    
    public final boolean getQueueAutoMerge() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId);
        }
    }

    /**
     * Enable or disable automatically merging requests queued inside
     * client/local files within a pool of sockets.
     *
     * @param merge A boolean value to enable (true) or disable (false)
     * automatically merging requests queued inside client/local files
     */
    public final void setQueueAutoMerge(boolean merge) {
        synchronized (m_cs) {
            ClientCoreLoader.SetQueueAutoMergeByPool(m_nPoolId, merge);
        }
    }
    
    public final void ShutdownPool() {
        Clean();
    }
    
    public final int getThreadsCreated() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetThreadCount(m_nPoolId);
        }
    }
    
    public final int getDisconnectedSockets() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetDisconnectedSockets(m_nPoolId);
        }
    }
    
    public final int getSocketsPerThread() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
        }
    }
    
    public final int getLockedSockets() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetLockedSockets(m_nPoolId);
        }
    }
    
    public final int getIdleSockets() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetIdleSockets(m_nPoolId);
        }
    }
    
    public final int getConnectedSockets() {
        synchronized (m_cs) {
            return ClientCoreLoader.GetConnectedSockets(m_nPoolId);
        }
    }

    /**
     * Create an instance of socket pool
     *
     * @param impl Class for generic type THandler
     * @param autoConn All sockets will support auto connection if true; and not
     * if false
     * @param recvTimeout Request receiving timeout in milliseconds
     * @param connTimeout Socket connection timeout in milliseconds
     * @param nServiceId Service id which defaults to 0
     */
    public CSocketPool(Class<THandler> impl, boolean autoConn, int recvTimeout, int connTimeout, int nServiceId) {
        try {
            m_AsyncHandlerCtor = impl.getConstructor();
        } catch (NoSuchMethodException | SecurityException err) {
        }
        m_autoConn = autoConn;
        m_recvTimeout = recvTimeout;
        m_connTimeout = connTimeout;
        m_nServiceId = nServiceId;
    }

    /**
     * Create an instance of socket pool
     *
     * @param impl Class for generic type THandler
     * @param autoConn All sockets will support auto connection if true; and not
     * if false
     * @param recvTimeout Request receiving timeout in milliseconds
     * @param connTimeout Socket connection timeout in milliseconds
     */
    public CSocketPool(Class<THandler> impl, boolean autoConn, int recvTimeout, int connTimeout) {
        try {
            m_AsyncHandlerCtor = impl.getConstructor();
        } catch (NoSuchMethodException | SecurityException err) {
        }
        m_autoConn = autoConn;
        m_recvTimeout = recvTimeout;
        m_connTimeout = connTimeout;
    }

    /**
     * Create an instance of socket pool with socket connecting timeout default
     * to 30 seconds.
     *
     * @param impl Class for generic type THandler
     * @param autoConn All sockets will support auto connection if true; and not
     * if false
     * @param recvTimeout Request receiving timeout in milliseconds
     */
    public CSocketPool(Class<THandler> impl, boolean autoConn, int recvTimeout) {
        try {
            m_AsyncHandlerCtor = impl.getConstructor();
        } catch (NoSuchMethodException | SecurityException err) {
        }
        m_autoConn = autoConn;
        m_recvTimeout = recvTimeout;
    }

    /**
     * Create an instance of socket pool with both request receiving and socket
     * connecting timeout default to 30 seconds.
     *
     * @param impl Class for generic type THandler
     * @param autoConn All sockets will support auto connection if true; and not
     * if false
     */
    public CSocketPool(Class<THandler> impl, boolean autoConn) {
        try {
            m_AsyncHandlerCtor = impl.getConstructor();
        } catch (NoSuchMethodException | SecurityException err) {
        }
        m_autoConn = autoConn;
    }

    /**
     * Create an instance of socket pool with both request receiving and socket
     * connecting timeout default to 30 seconds as well as auto connecting
     * default to true.
     *
     * @param impl Class for generic type THandler
     */
    public CSocketPool(Class<THandler> impl) {
        try {
            m_AsyncHandlerCtor = impl.getConstructor();
        } catch (NoSuchMethodException | SecurityException err) {
        }
    }
    
    private CConnectionContext[][] m_mcc;
    
    private void CopyCC(CConnectionContext[][] cc) {
        int threads = cc.length;
        int socketsPerThread = cc[0].length;
        if (socketsPerThread * threads == 0) {
            throw new UnsupportedOperationException("Must set connection context argument properly");
        }
        synchronized (m_cs) {
            m_mcc = new CConnectionContext[threads][socketsPerThread];
            for (int m = 0; m < socketsPerThread; ++m) {
                for (int n = 0; n < threads; ++n) {
                    if (cc[n][m] == null) {
                        throw new IllegalArgumentException("Must set connection context argument properly");
                    } else {
                        CConnectionContext c = new CConnectionContext();
                        CConnectionContext src = cc[n][m];
                        c.Host = src.Host;
                        c.Password = src.Password;
                        c.Port = src.Port;
                        c.EncrytionMethod = src.EncrytionMethod;
                        c.UserId = src.UserId;
                        c.V6 = src.V6;
                        c.Zip = src.Zip;
                        m_mcc[n][m] = c;
                    }
                }
            }
        }
    }
    
    public String getQueueName() {
        synchronized (m_cs) {
            return m_qName;
        }
    }
    
    public void setQueueName(String qName) {
        String s = qName;
        if (s != null) {
            s = s.trim();
        } else {
            s = "";
        }
        if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin || SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWinCE) {
            s = s.toLowerCase();
        }
        synchronized (m_cs) {
            if (!m_qName.equals(s)) {
                StopPoolQueue();
                m_qName = s;
                if (m_qName.length() > 0) {
                    StartPoolQueue(m_qName);
                }
            }
        }
    }
    
    private void StopPoolQueue() {
        for (CClientSocket cs : m_dicSocketHandler.keySet()) {
            if (cs.getClientQueue() != null && cs.getClientQueue().getAvailable()) {
                cs.getClientQueue().StopQueue();
            }
        }
    }
    
    private static final int DEFAULT_QUEUE_TIME_TO_LIVE = 240 * 3600;
    
    private void StartPoolQueue(String qName) {
        int index = 0;
        for (CClientSocket cs : m_dicSocketHandler.keySet()) {
            final boolean ok = cs.getClientQueue().StartQueue(qName + index, DEFAULT_QUEUE_TIME_TO_LIVE, cs.getEncryptionMethod() != SPA.tagEncryptionMethod.NoEncryption);
            ++index;
        }
    }
    
    private void SetQueue(CClientSocket socket) {
        int index = 0;
        for (CClientSocket cs : m_dicSocketHandler.keySet()) {
            if (cs == socket) {
                if (m_qName.length() > 0) {
                    if (!cs.getClientQueue().getAvailable()) {
                        cs.getClientQueue().StartQueue(m_qName + index, DEFAULT_QUEUE_TIME_TO_LIVE, cs.getEncryptionMethod() != SPA.tagEncryptionMethod.NoEncryption);
                    }
                } else {
                    if (cs.getClientQueue().getAvailable()) {
                        cs.getClientQueue().StopQueue();
                    }
                }
                break;
            }
            ++index;
        }
    }
    
    private THandler MapToHandler(long h) {
        synchronized (m_cs) {
            for (CClientSocket cs : m_dicSocketHandler.keySet()) {
                if (cs.getHandle() == h) {
                    return m_dicSocketHandler.get(cs);
                }
            }
            return null;
        }
    }

    /**
     * Start a pool of sockets with one connection context
     *
     * @param cc A connection context structure
     * @param socketsPerThread The number of socket connections per thread
     * @param threads The number of threads in a pool which defaults to the
     * number of CPU cores
     * @param avg A boolean value for building internal socket pool, which
     * defaults to true.
     * @param ta A value for COM thread apartment if there is COM object
     * involved. It is ignored on non-window platforms, and default to
     * tagThreadApartment.taNone
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext cc, int socketsPerThread, int threads, boolean avg, SPA.tagThreadApartment ta) {
        if (threads == 0) {
            threads = Runtime.getRuntime().availableProcessors();
        }
        CConnectionContext[][] mcc = new CConnectionContext[threads][socketsPerThread];
        for (int m = 0; m < socketsPerThread; ++m) {
            for (int n = 0; n < threads; ++n) {
                mcc[n][m] = cc;
            }
        }
        return StartSocketPool(mcc, avg, ta);
    }

    /**
     * Start a pool of sockets with one connection context
     *
     * @param cc A connection context structure
     * @param socketsPerThread The number of socket connections per thread
     * @param threads The number of threads in a pool which defaults to the
     * number of CPU cores
     * @param avg A boolean value for building internal socket pool, which
     * defaults to true.
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext cc, int socketsPerThread, int threads, boolean avg) {
        return StartSocketPool(cc, socketsPerThread, threads, avg, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a pool of sockets with one connection context
     *
     * @param cc A connection context structure
     * @param socketsPerThread The number of socket connections per thread
     * @param threads The number of threads in a pool which defaults to the
     * number of CPU cores
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext cc, int socketsPerThread, int threads) {
        return StartSocketPool(cc, socketsPerThread, threads, true, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a pool of sockets with one connection context
     *
     * @param cc A connection context structure
     * @param socketsPerThread The number of socket connections per thread
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext cc, int socketsPerThread) {
        return StartSocketPool(cc, socketsPerThread, 0, true, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a pool of sockets with a given two-dimensional matrix of connection
     * contexts
     *
     * @param cc A given two-dimensional matrix of connection contexts. Its
     * first dimension length represents the number of threads; and the second
     * dimension length is the number of sockets per thread
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext[][] cc) {
        return StartSocketPool(cc, true, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a pool of sockets with a given two-dimensional matrix of connection
     * contexts
     *
     * @param cc A given two-dimensional matrix of connection contexts. Its
     * first dimension length represents the number of threads; and the second
     * dimension length is the number of sockets per thread
     * @param avg A boolean value for building internal socket pool, which
     * defaults to true
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public final boolean StartSocketPool(CConnectionContext[][] cc, boolean avg) {
        return StartSocketPool(cc, avg, SPA.tagThreadApartment.taNone);
    }

    /**
     * Start a pool of sockets with a given two-dimensional matrix of connection
     * contexts
     *
     * @param cc A given two-dimensional matrix of connection contexts. Its
     * first dimension length represents the number of threads; and the second
     * dimension length is the number of sockets per thread
     * @param avg A boolean value for building internal socket pool, which
     * defaults to true.
     * @param ta A value for COM thread apartment if there is COM object
     * involved. It is ignored on non-window platforms, and default to
     * tagThreadApartment.taNone
     * @return False if there is no connection established; and true as long as
     * there is one connection started
     */
    public boolean StartSocketPool(CConnectionContext[][] cc, boolean avg, SPA.tagThreadApartment ta) {
        boolean ok;
        java.util.HashMap<CClientSocket, THandler> temp = new java.util.HashMap<>();
        if (cc == null || cc.length == 0 || ta == null) {
            throw new IllegalArgumentException("Must set connection context argument properly");
        }
        if (getStarted()) {
            ShutdownPool();
        }
        CopyCC(cc);
        boolean first = true;
        int threads = cc.length;
        int socketsPerThread = cc[0].length;
        if (!StartSocketPool((int) socketsPerThread, (int) threads, avg, ta.getValue())) {
            return false;
        }
        
        synchronized (m_cs) {
            int index = 0;
            for (CClientSocket cs : m_dicSocketHandler.keySet()) {
                temp.put(cs, m_dicSocketHandler.get(cs));
                int m = index % threads;
                int n = index / threads;
                CConnectionContext c = m_mcc[m][n];
                if (c.Host == null) {
                    throw new UnsupportedOperationException("Host string can not be null");
                }
                c.Host = c.Host.trim();
                if (c.Host.length() == 0) {
                    throw new UnsupportedOperationException("Host string must be a valid string");
                }
                if (c.Port == 0) {
                    throw new UnsupportedOperationException("Host port can't be zero");
                }
                cs.ConnectionContext = c;
                ++index;
            }
        }
        
        for (CClientSocket cs : temp.keySet()) {
            if (cs.getConnected()) {
                first = false;
                continue;
            }
            CConnectionContext c = cs.ConnectionContext;
            cs.setUID(c.UserId);
            cs.setEncryptionMethod(c.EncrytionMethod);
            cs.setZip(c.Zip);
            byte[] host = c.Host.getBytes();
            if (first) {
                //we use sync connecting for the first socket connection
                ok = ClientCoreLoader.Connect(cs.getHandle(), host, host.length, c.Port, true, c.V6);
                if (ok && ClientCoreLoader.WaitAll(cs.getHandle(), -1)) {
                    first = false;
                }
            } else {
                ClientCoreLoader.Connect(cs.getHandle(), host, host.length, c.Port, false, c.V6);
            }
        }
        return (getConnectedSockets() > 0);
    }
    
    private boolean StartSocketPool(int socketsPerThread, int threads, boolean avg, int ta) {
        if (getStarted()) {
            return true;
        }
        int id = ClientCoreLoader.CreateSocketPool(this, socketsPerThread, threads, avg, ta);
        return (id != 0);
    }
    
    private volatile THandler[] m_Handlers = null;
    
    @SuppressWarnings("unchecked")
    public final THandler[] getAsyncHandlers() {
        synchronized (m_cs) {
            if (m_Handlers != null) {
                return m_Handlers;
            }
            int n = 0;
            java.util.Collection<THandler> values = m_dicSocketHandler.values();
            int len = values.size();
            for (THandler h : values) {
                if (m_Handlers == null) {
                    m_Handlers = (THandler[]) java.lang.reflect.Array.newInstance(h.getClass(), len);
                }
                m_Handlers[n] = h;
                ++n;
            }
            return m_Handlers;
        }
    }
    
    public final CClientSocket[] getSockets() {
        synchronized (m_cs) {
            int n = 0;
            CClientSocket[] sockets = new CClientSocket[m_dicSocketHandler.size()];
            java.util.Collection<CClientSocket> values = m_dicSocketHandler.keySet();
            for (CClientSocket s : values) {
                sockets[n] = s;
                ++n;
            }
            return sockets;
        }
    }

    /**
     * Seek an async handler by its associated queue file full path or raw name.
     *
     *
     * @param queueName queue file full path or raw name
     * @return An async handler if found; and null or nothing if none is found
     */
    public final THandler SeekByQueue(String queueName) {
        THandler h = null;
        if (queueName == null || queueName.length() == 0) {
            return null;
        }
        switch (SPA.CUQueue.DEFAULT_OS) {
            case osWin:
            case osWinCE:
                queueName = queueName.toLowerCase();
                break;
            default:
                break;
        }
        synchronized (m_cs) {
            String rawName;
            String appName = ClientCoreLoader.GetClientWorkDirectory();
            for (CClientSocket cs : m_dicSocketHandler.keySet()) {
                if (!cs.getClientQueue().getAvailable()) {
                    continue;
                }
                if (cs.getClientQueue().getSecure()) {
                    rawName = queueName + "_" + appName + "_1.mqc";
                } else {
                    rawName = queueName + "_" + appName + "_0.mqc";
                }
                String queueFileName = cs.getClientQueue().getQueueFileName();
                
                int len = queueFileName.length();
                int lenRaw = rawName.length();
                if (lenRaw > len) {
                    continue;
                }
                int pos = queueFileName.lastIndexOf(rawName);

                //queue file name with full path
                if (pos == 0) {
                    return m_dicSocketHandler.get(cs);
                }

                //queue raw name only
                if ((pos + lenRaw) == len) {
                    return m_dicSocketHandler.get(cs);
                }
            }
        }
        return h;
    }
    
    public final int getQueues() {
        int q = 0;
        synchronized (m_cs) {
            java.util.Set<CClientSocket> set = m_dicSocketHandler.keySet();
            for (CClientSocket cs : set) {
                q += (cs.getClientQueue().getAvailable() ? 1 : 0);
            }
        }
        return q;
    }

    /**
     * Seek an async handler on the min number of requests queued in memory and
     * its associated socket connection
     *
     * @return An async handler if found; and null or nothing if no connection
     * is found
     */
    public final THandler Seek() {
        THandler h = null;
        synchronized (m_cs) {
            for (CClientSocket cs : m_dicSocketHandler.keySet()) {
                if (cs.getConnectionState() != tagConnectionState.csSwitched) {
                    continue;
                }
                if (h == null) {
                    h = m_dicSocketHandler.get(cs);
                } else {
                    int cs_coriq = cs.getCountOfRequestsInQueue();
                    int h_coriq = h.getAttachedClientSocket().getCountOfRequestsInQueue();
                    if (cs_coriq < h_coriq) {
                        h = m_dicSocketHandler.get(cs);
                    } else if (cs_coriq == h_coriq && cs.getBytesSent() < h.getAttachedClientSocket().getBytesSent()) {
                        h = m_dicSocketHandler.get(cs);
                    }
                }
            }
        }
        return h;
    }

    /**
     * Seek an async handler on the min number of requests queued and its
     * associated socket connection
     *
     * @return An async handler if found; and null or nothing if no proper queue
     * is available
     */
    public final THandler SeekByQueue() {
        THandler h = null;
        synchronized (m_cs) {
            boolean automerge = ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId);
            for (CClientSocket cs : m_dicSocketHandler.keySet()) {
                if (automerge && cs.getConnectionState().getValue() < tagConnectionState.csSwitched.getValue()) {
                    continue;
                }
                IClientQueue cq = cs.getClientQueue();
                if (!cq.getAvailable() || cq.getJobSize() > 0/*queue is in transaction at this time*/) {
                    continue;
                }
                if (h == null) {
                    h = m_dicSocketHandler.get(cs);
                } else if ((cq.getMessageCount() < h.getAttachedClientSocket().getClientQueue().getMessageCount()) || (cs.getConnected() && !h.getAttachedClientSocket().getConnected())) {
                    h = m_dicSocketHandler.get(cs);
                }
            }
        }
        return h;
    }

    /**
     * Lock an async handler infinitely
     *
     * @return An async handler if successful; and null or nothing if failed You
     * must also call the method Unlock to unlock the handler and its associated
     * socket connection for reuse.
     */
    public final THandler Lock() {
        return Lock(-1);
    }

    /**
     * Lock an async handler on a given timeout.
     *
     * @param timeout A timeout in milliseconds
     * @return An async handler if successful; and null or nothing if failed You
     * must also call the method Unlock to unlock the handler and its associated
     * socket connection for reuse.
     */
    public final THandler Lock(int timeout) {
        int poolId;
        synchronized (m_cs) {
            poolId = m_nPoolId;
        }
        return MapToHandler(ClientCoreLoader.LockASocket(poolId, timeout, 0));
    }

    /**
     * Lock an async handler on a given thread handle with infinite timeout
     *
     * @param sameThreadHandle A handle to a thread
     * @return An async handler if successful; and null or nothing if failed You
     * must also call the method Unlock to unlock the handler and its associated
     * socket connection for reuse.
     */
    public final THandler Lock(long sameThreadHandle) {
        return Lock(sameThreadHandle, -1);
    }

    /**
     * Lock an async handler on given thread handle and timeout.
     *
     * @param sameThreadHandle A handle to a thread
     * @param timeout A timeout in milliseconds
     * @return An async handler if successful; and null or nothing if failed You
     * must also call the method Unlock to unlock the handler and its associated
     * socket connection for reuse.
     */
    public final THandler Lock(long sameThreadHandle, int timeout) {
        int poolId;
        synchronized (m_cs) {
            poolId = m_nPoolId;
        }
        return MapToHandler(ClientCoreLoader.LockASocket(poolId, timeout, sameThreadHandle));
    }

    /**
     * Unlock a previously locked async handler to pool for reuse.
     *
     * @param handler A previously locked async handler
     */
    public final void Unlock(THandler handler) {
        if (handler == null) {
            return;
        }
        Unlock(handler.getAttachedClientSocket());
    }

    /**
     * Unlock a previously locked socket to pool for reuse.
     *
     * @param cs A previously locked socket
     */
    public final void Unlock(CClientSocket cs) {
        int poolId;
        if (cs == null) {
            return;
        }
        synchronized (m_cs) {
            poolId = m_nPoolId;
        }
        ClientCoreLoader.UnlockASocket(poolId, cs.getHandle());
    }
    
    private volatile int m_nPoolId; //locked by m_cs
    private volatile boolean m_autoConn = true;
    private volatile int m_recvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;
    private volatile int m_connTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;
    private volatile int m_nServiceId = 0;
    protected final Object m_cs = new Object();
    protected volatile java.util.HashMap<CClientSocket, THandler> m_dicSocketHandler = new java.util.HashMap<>(); //locked by m_cs
}
