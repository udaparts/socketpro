package SPA.ClientSide;

import javax.json.*;
import java.io.FileInputStream;
import java.util.HashMap;

public final class SpManager {

    private static final Object m_cs = new Object();
    private static boolean m_Middle = false;
    private static CSpConfig m_sc = null;

    public static boolean getMidTier() {
        synchronized (m_cs) {
            return m_Middle;
        }
    }

    public static CSpConfig SetConfig() throws Exception {
        return SetConfig(false, null);
    }

    public static CSpConfig SetConfig(boolean midTier) throws Exception {
        return SetConfig(midTier, null);
    }

    public static CSpConfig SetConfig(boolean midTier, String jcFile) throws Exception {
        synchronized (m_cs) {
            if (m_sc != null) {
                return m_sc;
            }
        }
        if (jcFile == null || jcFile.length() == 0) {
            jcFile = "sp_config.json";
        }
        CSpConfig sc = new CSpConfig(Json.createReader(new FileInputStream(jcFile)).readObject());
        sc.Normalize();
        synchronized (m_cs) {
            m_sc = sc;
            m_Middle = midTier;
        }
        return sc;
    }

    public static CSpConfig getConfig() {
        synchronized (m_cs) {
            return m_sc;
        }
    }

    /**
     *
     * @return The number of running socket pools
     */
    public static int getPools() {
        return SPA.ClientSide.CSocketPool.getSocketPools();
    }

    /**
     *
     * @return Client core library version string
     */
    public static String getVersion() {
        return SPA.ClientSide.CClientSocket.getVersion();
    }

    @SuppressWarnings("unchecked")
    public static Object SeekHandlerByQueue(String poolKey) throws Exception {
        if (poolKey == null || poolKey.length() == 0) {
            throw new Exception("Pool key cannot be empty");
        }
        synchronized (m_cs) {
            if (CPoolConfig.m_vP.indexOf(poolKey) == -1) {
                throw new Exception("Pool key cannot be found from configuration file");
            }
            CPoolConfig pc = CPoolConfig.m_mapPools.get(poolKey);
            if (pc.Pool == null) {
                GetPool(poolKey);
            }
            switch (pc.getSvsId()) {
                case CMysql.sidMysql:
                    return ((CSocketPool<CMysql>) pc.Pool).SeekByQueue();
                case COdbc.sidOdbc:
                    return ((CSocketPool<COdbc>) pc.Pool).SeekByQueue();
                case CSqlite.sidSqlite:
                    return ((CSocketPool<CSqlite>) pc.Pool).SeekByQueue();
                case CAsyncQueue.sidQueue:
                    return ((CSocketPool<CAsyncQueue>) pc.Pool).SeekByQueue();
                case CStreamingFile.sidFile:
                    return ((CSocketPool<CStreamingFile>) pc.Pool).SeekByQueue();
                default:
                    return ((CSocketPool<CCachedBaseHandler>) pc.Pool).SeekByQueue();
            }
        }
    }

    public static Object LockHandler(String poolKey) throws Exception {
        return LockHandler(poolKey, -1);
    }

    @SuppressWarnings("unchecked")
    public static Object LockHandler(String poolKey, int timeout) throws Exception {
        if (poolKey == null || poolKey.length() == 0) {
            throw new Exception("Pool key cannot be empty");
        }
        synchronized (m_cs) {
            if (CPoolConfig.m_vP.indexOf(poolKey) == -1) {
                throw new Exception("Pool key cannot be found from configuration file");
            }
            CPoolConfig pc = CPoolConfig.m_mapPools.get(poolKey);
            if (pc.Pool == null) {
                GetPool(poolKey);
            }
            switch (pc.getSvsId()) {
                case CMysql.sidMysql:
                    return ((CSocketPool<CMysql>) pc.Pool).Lock(timeout);
                case COdbc.sidOdbc:
                    return ((CSocketPool<COdbc>) pc.Pool).Lock(timeout);
                case CSqlite.sidSqlite:
                    return ((CSocketPool<CSqlite>) pc.Pool).Lock(timeout);
                case CAsyncQueue.sidQueue:
                    return ((CSocketPool<CAsyncQueue>) pc.Pool).Lock(timeout);
                case CStreamingFile.sidFile:
                    return ((CSocketPool<CStreamingFile>) pc.Pool).Lock(timeout);
                default:
                    return ((CSocketPool<CCachedBaseHandler>) pc.Pool).Lock(timeout);
            }
        }
    }

    @SuppressWarnings("unchecked")
    public static Object SeekHandler(String poolKey) throws Exception {
        if (poolKey == null || poolKey.length() == 0) {
            throw new Exception("Pool key cannot be empty");
        }
        synchronized (m_cs) {
            if (CPoolConfig.m_vP.indexOf(poolKey) == -1) {
                throw new Exception("Pool key cannot be found from configuration file");
            }
            CPoolConfig pc = CPoolConfig.m_mapPools.get(poolKey);
            if (pc.Pool == null) {
                GetPool(poolKey);
            }
            switch (pc.getSvsId()) {
                case CMysql.sidMysql:
                    return ((CSocketPool<CMysql>) pc.Pool).Seek();
                case COdbc.sidOdbc:
                    return ((CSocketPool<COdbc>) pc.Pool).Seek();
                case CSqlite.sidSqlite:
                    return ((CSocketPool<CSqlite>) pc.Pool).Seek();
                case CAsyncQueue.sidQueue:
                    return ((CSocketPool<CAsyncQueue>) pc.Pool).Seek();
                case CStreamingFile.sidFile:
                    return ((CSocketPool<CStreamingFile>) pc.Pool).Seek();
                default:
                    return ((CSocketPool<CCachedBaseHandler>) pc.Pool).Seek();
            }
        }
    }

    public static Object GetPool(String poolKey) throws Exception {
        if (poolKey == null || poolKey.length() == 0) {
            throw new Exception("Pool key cannot be empty");
        }
        synchronized (m_cs) {
            if (CPoolConfig.m_vP.indexOf(poolKey) == -1) {
                throw new Exception("Pool key cannot be found from configuration file");
            }
            CPoolConfig pc = CPoolConfig.m_mapPools.get(poolKey);
            if (pc.Pool != null) {
                return pc.Pool;
            }
            boolean ok = false;
            int threads = pc.getThreads();
            java.util.ArrayList<String> hosts = pc.getHosts();
            int sockets_per_thread = hosts.size();
            HashMap<String, CConnectionContext> mapHost = m_sc.getHosts();
            CConnectionContext[][] ppCC = new CConnectionContext[threads][sockets_per_thread];
            for (int i = 0; i < threads; ++i) {
                for (int j = 0; j < sockets_per_thread; ++j) {
                    ppCC[i][j] = mapHost.get(hosts.get(j));
                }
            }
            switch (pc.getSvsId()) {
                case CMysql.sidMysql: {
                    CSocketPool<CMysql> mysql;
                    switch (pc.getPoolType()) {
                        case Master:
                            mysql = new SPA.CSqlMasterPool<>(CMysql.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<CMysql> master = new SPA.CSqlMasterPool<>(CMysql.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            mysql = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            mysql = new CSocketPool<>(CMysql.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    mysql.setQueueName(pc.getQueue());
                    mysql.setQueueAutoMerge(pc.getAutoMerge());
                    if (!mysql.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = mysql;
                }
                break;
                case COdbc.sidOdbc: {
                    CSocketPool<COdbc> odbc;
                    switch (pc.getPoolType()) {
                        case Master:
                            odbc = new SPA.CSqlMasterPool<>(COdbc.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<COdbc> master = new SPA.CSqlMasterPool<>(COdbc.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            odbc = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            odbc = new CSocketPool<>(COdbc.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    odbc.setQueueName(pc.getQueue());
                    odbc.setQueueAutoMerge(pc.getAutoMerge());
                    if (!odbc.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = odbc;
                }
                break;
                case CSqlite.sidSqlite: {
                    CSocketPool<CSqlite> sqlite;
                    switch (pc.getPoolType()) {
                        case Master:
                            sqlite = new SPA.CSqlMasterPool<>(CSqlite.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<CSqlite> master = new SPA.CSqlMasterPool<>(CSqlite.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            sqlite = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            sqlite = new CSocketPool<>(CSqlite.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    sqlite.setQueueName(pc.getQueue());
                    sqlite.setQueueAutoMerge(pc.getAutoMerge());
                    if (!sqlite.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = sqlite;
                }
                break;
                case CAsyncQueue.sidQueue: {
                    CSocketPool<CAsyncQueue> aq = new CSocketPool<>(CAsyncQueue.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                    aq.setQueueName(pc.getQueue());
                    aq.setQueueAutoMerge(pc.getAutoMerge());
                    ok = aq.StartSocketPool(ppCC);
                    if (!aq.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = aq;
                }
                break;
                case CStreamingFile.sidFile: {
                    CSocketPool<CStreamingFile> sf = new CSocketPool<>(CStreamingFile.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                    sf.setQueueName(pc.getQueue());
                    sf.setQueueAutoMerge(pc.getAutoMerge());
                    if (!sf.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = sf;
                }
                break;
                default: {
                    CSocketPool<CCachedBaseHandler> ah;
                    switch (pc.getPoolType()) {
                        case Master:
                            ah = new SPA.CMasterPool<>(CCachedBaseHandler.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CMasterPool<CCachedBaseHandler> master = new SPA.CMasterPool<>(CCachedBaseHandler.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            ah = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            ah = new CSocketPool<>(CCachedBaseHandler.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    ah.setQueueName(pc.getQueue());
                    ah.setQueueAutoMerge(pc.getAutoMerge());
                    if (!ah.StartSocketPool(ppCC)) {
                        throw new Exception("There is no connection establised for pool " + poolKey);
                    }
                    pc.Pool = ah;
                }
                break;
            }
            return pc.Pool;
        }
    }
/*
    @SuppressWarnings("unchecked")
    public static void main(String[] args) {
        try {
            CSpConfig jc = SetConfig(false, "c:\\cyetest\\socketpro\\src\\njadapter\\sp_config.json");
            String s = jc.getConfig();
            CMysql mysql = (CMysql) SeekHandler("masterdb");
            mysql = null;
        } catch (Exception err) {
            System.out.println(err.toString());
        }
    }
*/
}
