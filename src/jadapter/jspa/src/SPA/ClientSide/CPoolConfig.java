package SPA.ClientSide;

import SPA.CUQueue;
import SPA.tagOperationSystem;
import java.util.HashMap;
import javax.json.*;

public final class CPoolConfig {

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
        return new java.util.ArrayList<>(m_Hosts);
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

    private String Master = null;
    private Object Pool = null;

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
                m_Hosts.add(ja.getString(n));
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

    }

    void Normalize() throws Exception {
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
