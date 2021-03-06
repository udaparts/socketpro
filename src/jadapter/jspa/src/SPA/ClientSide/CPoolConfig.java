package SPA.ClientSide;

import SPA.CUQueue;
import SPA.tagOperationSystem;
import java.util.HashMap;
import javax.json.*;

public final class CPoolConfig {

    static java.util.ArrayList<String> m_vH = new java.util.ArrayList<>();
    static java.util.ArrayList<String> m_vP = new java.util.ArrayList<>();
    static HashMap<String, CPoolConfig> m_mapPools = new HashMap<>();

    private HashMap<String, CPoolConfig> m_Slaves = null;

    public HashMap<String, CPoolConfig> getSlaves() {
        if (m_Slaves == null) {
            return null;
        }
        return new HashMap<>(m_Slaves);
    }

    private int m_SvsId;

    public int getSvsId() {
        return m_SvsId;
    }

    private java.util.ArrayList<String> m_Hosts = new java.util.ArrayList<>();

    public java.util.ArrayList<String> getHosts() {
        return m_Hosts;
    }

    private String m_Queue = null;

    /**
     *
     * @return The name of client queue which will be used to backup client
     * requests at client side
     */
    public String getQueue() {
        return m_Queue;
    }

    private String m_DefaultDb = null;

    public String getDefaultDb() {
        return m_DefaultDb;
    }

    private tagPoolType m_PoolType = tagPoolType.Regular;

    public tagPoolType getPoolType() {
        return m_PoolType;
    }

    private int m_ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;

    /**
     *
     * @return Socket connecting timeout in milliseconds
     */
    public int getConnTimeout() {
        return m_ConnTimeout;
    }

    JsonObject ToJsonObject() {
        JsonObjectBuilder job = Json.createObjectBuilder();
        job.add("SvsId", m_SvsId);
        JsonArrayBuilder jab = Json.createArrayBuilder();
        int size = m_Hosts.size();
        for (int n = 0; n < size; ++n) {
            jab.add(m_Hosts.get(n));
        }
        job.add("Hosts", jab.build());
        if (m_Queue != null) {
            job.add("Queue", m_Queue);
        }
        if (m_DefaultDb != null) {
            job.add("DefaultDb", m_DefaultDb);
        }
        job.add("PoolType", m_PoolType.getValue());
        job.add("ConnTimeout", m_ConnTimeout);
        job.add("RecvTimeout", m_RecvTimeout);
        job.add("AutoConn", getAutoConn());
        job.add("AutoMerge", m_AutoMerge);
        job.add("Threads", m_Threads);
        if (m_Slaves != null) {
            JsonObjectBuilder js = Json.createObjectBuilder();
            java.util.Set<String> set = m_Slaves.keySet();
            for (String skey : set) {
                CPoolConfig pool = m_Slaves.get(skey);
                js.add(skey, pool.ToJsonObject());
            }
            job.add("Slaves", js.build());
        }
        return job.build();
    }

    private int m_RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;

    /**
     *
     * @return Client request timeout in milliseconds
     */
    public int getRecvTimeout() {
        return m_RecvTimeout;
    }

    private boolean m_nac = false;

    public boolean getAutoConn() {
        return !m_nac;
    }

    private boolean m_AutoMerge = false;

    public boolean getAutoMerge() {
        return m_AutoMerge;
    }

    String Master = null;
    CSocketPool Pool = null;

    private int m_Threads = 1;

    public int getThreads() {
        return m_Threads;
    }

    CPoolConfig(JsonObject pc, String key) {
        if (pc.containsKey("SvsId")) {
            m_SvsId = pc.getInt("SvsId");
        }
        if (pc.containsKey("AutoMerge")) {
            m_AutoMerge = pc.getBoolean("AutoMerge");
        }
        if (pc.containsKey("Threads")) {
            m_Threads = pc.getInt("Threads");
        }
        if (pc.containsKey("AutoConn")) {
            m_nac = !pc.getBoolean("AutoConn");
        }
        if (pc.containsKey("RecvTimeout")) {
            m_RecvTimeout = pc.getInt("RecvTimeout");
        }
        if (pc.containsKey("ConnTimeout")) {
            m_ConnTimeout = pc.getInt("ConnTimeout");
        }
        if (pc.containsKey("DefaultDb")) {
            m_DefaultDb = pc.getString("DefaultDb").trim();
            if (m_DefaultDb.length() > 0) {
                m_PoolType = tagPoolType.Master;
            }
        }
        if (pc.containsKey("Queue")) {
            m_Queue = pc.getString("Queue").trim();
            if (CUQueue.DEFAULT_OS == tagOperationSystem.osWin || CUQueue.DEFAULT_OS == tagOperationSystem.osWinCE) {
                m_Queue = m_Queue.toLowerCase();
            }
        }
        if (m_Queue == null || m_Queue.length() == 0) {
            m_AutoMerge = false;
        }
        if (pc.containsKey("Hosts")) {
            JsonArray ja = pc.getJsonArray("Hosts");
            for (int n = 0; n < ja.size(); ++n) {
                String k = ja.getString(n);
                m_Hosts.add(k);
            }
        }
        if (pc.containsKey("Slaves")) {
            m_Slaves = new HashMap<>();
            JsonObject jos = pc.getJsonObject("Slaves");
            java.util.Set<String> set = jos.keySet();
            for (String skey : set) {
                CPoolConfig ps = new CPoolConfig(jos.getJsonObject(skey), null);
                ps.m_SvsId = m_SvsId;
                ps.Master = key;
                ps.m_PoolType = tagPoolType.Slave;
                m_Slaves.put(skey, ps);
            }
        }
    }

    private void NormalizeSlaves(String defalutDb) throws Exception {
        java.util.Set<String> set = m_Slaves.keySet();
        for (String skey : set) {
            if (skey == null || skey.length() == 0) {
                throw new Exception("Slave pool key cannot be empty");
            }
            if (m_vP.indexOf(skey) != -1) {
                throw new Exception("Slave pool key are duplicated");
            }
            CPoolConfig pool = m_Slaves.get(skey);
            if (pool.m_Slaves != null) {
                throw new Exception("A slave pool cannot contain any new slave pool");
            }
            pool.Normalize();
            if (pool.m_DefaultDb == null || pool.m_DefaultDb.length() == 0) {
                pool.m_DefaultDb = defalutDb;
            }
            m_vP.add(skey);
            CPoolConfig.m_mapPools.put(skey, pool);
        }
    }

    void Normalize() throws Exception {
        if (m_Hosts.isEmpty()) {
            throw new Exception("Pool host array is empty");
        }
        for (int n = 0; n < m_Hosts.size(); ++n) {
            String s = m_Hosts.get(n);
            if (m_vH.indexOf(s) == -1) {
                throw new Exception("Host " + s + " not found in Hosts");
            }
        }
        switch (m_SvsId) {
            case SPA.BaseServiceID.sidHTTP:
                throw new Exception("Client side does not support HTTP/websocket service");
            case SPA.BaseServiceID.sidQueue:
                if ((m_DefaultDb != null && m_DefaultDb.length() > 0) || m_Slaves != null) {
                    throw new Exception("Server queue service does not support master or slave pool");
                }
                break;
            case SPA.BaseServiceID.sidFile:
                if ((m_DefaultDb != null && m_DefaultDb.length() > 0) || m_Slaves != null) {
                    throw new Exception("Remote file service does not support master or slave pool");
                }
                break;
            case SPA.BaseServiceID.sidODBC:
            case CSqlite.sidSqlite:
            case CMysql.sidMysql:
                break;
            default:
                if (m_SvsId <= SPA.BaseServiceID.sidReserved) {
                    throw new Exception("User defined service id must be larger than " + SPA.BaseServiceID.sidReserved);
                }
                break;
        }
        if ((m_DefaultDb == null || m_DefaultDb.length() == 0) && m_Slaves != null) {
            throw new Exception("Slave array is not empty but DefaultDb string is empty");
        }
        if (m_Slaves != null && m_PoolType == tagPoolType.Master) {
            NormalizeSlaves(m_DefaultDb);
        }
    }
}
