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

    public static CAsyncServiceHandler SeekHandlerByQueue(String poolKey) throws Exception {
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
            return pc.Pool.SeekByQueue();
        }
    }

    public static CAsyncServiceHandler SeekHandler(String poolKey) throws Exception {
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
            return pc.Pool.Seek();
        }
    }

    public static CSocketPool GetPool(String poolKey) throws Exception {
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
                case CMysql.sidMysql:
                    switch (pc.getPoolType()) {
                        case Master:
                            pc.Pool = new SPA.CSqlMasterPool<>(CMysql.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<CMysql> master = new SPA.CSqlMasterPool<>(CMysql.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            pc.Pool = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            pc.Pool = new CSocketPool<>(CMysql.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    break;
                case COdbc.sidOdbc:
                    switch (pc.getPoolType()) {
                        case Master:
                            pc.Pool = new SPA.CSqlMasterPool<>(COdbc.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<COdbc> master = new SPA.CSqlMasterPool<>(COdbc.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            pc.Pool = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            pc.Pool = new CSocketPool<>(COdbc.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    break;
                case CSqlite.sidSqlite:
                    switch (pc.getPoolType()) {
                        case Master:
                            pc.Pool = new SPA.CSqlMasterPool<>(CSqlite.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CSqlMasterPool<CSqlite> master = new SPA.CSqlMasterPool<>(CSqlite.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            pc.Pool = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            pc.Pool = new CSocketPool<>(CSqlite.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    break;
                case CAsyncQueue.sidQueue:
                    pc.Pool = new CSocketPool<>(CAsyncQueue.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                    break;
                case CStreamingFile.sidFile:
                    pc.Pool = new CSocketPool<>(CStreamingFile.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                    break;
                default:
                    switch (pc.getPoolType()) {
                        case Master:
                            pc.Pool = new SPA.CMasterPool<>(CCachedBaseHandler.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            break;
                        case Slave: {
                            SPA.CMasterPool<CCachedBaseHandler> master = new SPA.CMasterPool<>(CCachedBaseHandler.class, pc.getDefaultDb(), m_Middle, pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                            pc.Pool = master.new CSlavePool(pc.getDefaultDb(), pc.getRecvTimeout(), pc.getAutoConn(), pc.getConnTimeout());
                        }
                        break;
                        default:
                            pc.Pool = new CSocketPool<>(CCachedBaseHandler.class, pc.getAutoConn(), pc.getRecvTimeout(), pc.getConnTimeout());
                            break;
                    }
                    break;
            }
            pc.Pool.setQueueName(pc.getQueue());
            pc.Pool.DoSslServerAuthentication = new CSocketPool.DDoSslServerAuthentication() {
                @Override
                public boolean invoke(CSocketPool sender, CClientSocket cs) {
                    return m_sc.Verify(cs);
                }
            };
            pc.Pool.StartSocketPool(ppCC);
            pc.Pool.setQueueAutoMerge(pc.getAutoMerge());
            return pc.Pool;
        }
    }
}
