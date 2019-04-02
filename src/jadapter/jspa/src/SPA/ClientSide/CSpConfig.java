package SPA.ClientSide;

import SPA.CUQueue;
import static SPA.ClientSide.CPoolConfig.m_vH;
import SPA.tagEncryptionMethod;
import SPA.tagOperationSystem;
import java.math.BigDecimal;
import java.util.HashMap;
import javax.json.*;

public final class CSpConfig {

    public String getWorkingDir() {
        return CClientSocket.QueueConfigure.getWorkDirectory();
    }

    private String m_CertStore;

    public String getCertStore() {
        return m_CertStore;
    }

    private int m_QueuePassword = 0;

    public int getQueuePassword() {
        return m_QueuePassword;
    }

    HashMap<String, CConnectionContext> m_Hosts = new HashMap<>();

    public HashMap<String, CConnectionContext> getHosts() {
        return new HashMap<>(m_Hosts);
    }

    HashMap<String, CPoolConfig> m_Pools = new HashMap<>();

    public HashMap<String, CPoolConfig> getPools() {
        return new HashMap<>(m_Pools);
    }

    private java.util.ArrayList<String> m_KeysAllowed = null;

    public String getConfig() {
        JsonObjectBuilder job = Json.createObjectBuilder();
        job.add("WorkingDir", getWorkingDir());
        if (m_CertStore != null && m_CertStore.length() > 0) {
            job.add("CertStore", m_CertStore);
        } else if (CUQueue.DEFAULT_OS == tagOperationSystem.osWin) {
            job.add("CertStore", "ROOT");
        }
        job.add("QueuePassword", m_QueuePassword);
        if (m_KeysAllowed != null) {
            JsonArrayBuilder jab = Json.createArrayBuilder();
            int count = m_KeysAllowed.size();
            for (int n = 0; n < count; ++n) {
                jab.add(m_KeysAllowed.get(n));
            }
            job.add("KeysAllowed", jab);
        }
        JsonObjectBuilder jh = Json.createObjectBuilder();
        java.util.Set<String> set = m_Hosts.keySet();
        for (String key : set) {
            CConnectionContext c = m_Hosts.get(key);
            jh.add(key, c.ToJsonObject());
        }
        job.add("Hosts", jh.build());
        JsonObjectBuilder jp = Json.createObjectBuilder();
        set = m_Pools.keySet();
        for (String key : set) {
            CPoolConfig pc = m_Pools.get(key);
            jp.add(key, pc.ToJsonObject());
        }
        job.add("Pools", jp.build());
        return job.build().toString();
    }

    private boolean CheckDuplicated(CConnectionContext cc) {
        int count = 0;
        java.util.Set<String> set = m_Hosts.keySet();
        for (String key : set) {
            CConnectionContext c = m_Hosts.get(key);
            count += (c.IsSame(cc) ? 1 : 0);
        }
        return (count > 1);
    }

    void Normalize() throws Exception {
        if (m_Hosts.isEmpty()) {
            throw new Exception("Host map cannot be empty");
        }
        java.util.Set<String> set = m_Hosts.keySet();
        for (String key : set) {
            if (key == null || key.length() == 0) {
                throw new Exception("Host key cannot be empty");
            }
            if (m_vH.indexOf(key) == -1) {
                throw new Exception("Host key " + key + " not found in hosts");
            }
            CConnectionContext cc = m_Hosts.get(key);
            cc.Normalize();
            if (CheckDuplicated(cc)) {
                throw new Exception("Connection context for host " + key + " duplicated");
            }
        }
        if (m_Pools.isEmpty()) {
            throw new Exception("Pool map cannot be empty");
        }
        set = m_Pools.keySet();
        for (String key : set) {
            if (key == null || key.length() == 0) {
                throw new Exception("Pool key cannot be empty");
            }
            CPoolConfig pc = m_Pools.get(key);
            pc.Normalize();
        }
    }

    CSpConfig(JsonObject root) {
        if (root.containsKey("WorkingDir")) {
            CClientSocket.QueueConfigure.setWorkDirectory(root.getString("WorkingDir"));
        }
        if (root.containsKey("QueuePassword")) {
            String qp = root.getString("QueuePassword");
            CClientSocket.QueueConfigure.setMessageQueuePassword(qp);
            m_QueuePassword = 1;
        }
        if (root.containsKey("CertStore")) {
            m_CertStore = root.getString("CertStore");
            CClientSocket.SSL.SetVerifyLocation(m_CertStore);
        }
        if (root.containsKey("KeysAllowed")) {
            JsonArray ja = root.getJsonArray("KeysAllowed");
            m_KeysAllowed = new java.util.ArrayList<>();
            for (int n = 0; n < ja.size(); ++n) {
                m_KeysAllowed.add(ja.getString(n));
            }
        }
        JsonObject joHosts = root.getJsonObject("Hosts");
        java.util.Set<String> set = joHosts.keySet();
        for (String key : set) {
            CPoolConfig.m_vH.add(key);
            JsonObject conn = joHosts.getJsonObject(key);
            CConnectionContext cc = new CConnectionContext();
            if (conn.containsKey("Host")) {
                cc.Host = conn.getString("Host");
            }
            if (conn.containsKey("Port")) {
                cc.Port = conn.getInt("Port");
            }
            if (conn.containsKey("UserId")) {
                cc.UserId = conn.getString("UserId");
            }
            if (conn.containsKey("Password")) {
                cc.Password = conn.getString("Password");
            }
            if (conn.containsKey("EncrytionMethod")) {
                cc.EncrytionMethod = tagEncryptionMethod.forValue(conn.getInt("EncrytionMethod"));
            }
            if (conn.containsKey("Zip")) {
                cc.Zip = conn.getBoolean("Zip");
            }
            if (conn.containsKey("V6")) {
                cc.V6 = conn.getBoolean("V6");
            }
            if (conn.containsKey("AnyData")) {
                JsonValue jv = conn.get("AnyData");
                JsonValue.ValueType vt = jv.getValueType();
                if (vt == JsonValue.ValueType.NULL) {
                } else if (vt == JsonValue.ValueType.FALSE) {
                    cc.AnyData = false;
                } else if (vt == JsonValue.ValueType.TRUE) {
                    cc.AnyData = true;
                } else if (vt == JsonValue.ValueType.STRING) {
                    cc.AnyData = conn.getString("AnyData");
                } else if (vt == JsonValue.ValueType.NUMBER) {
                    JsonNumber jn = conn.getJsonNumber("AnyData");
                    if (jn.isIntegral()) {
                        cc.AnyData = jn.longValue();
                    } else {
                        cc.AnyData = jn.doubleValue();
                    }
                } else if (vt == JsonValue.ValueType.ARRAY) {
                    cc.AnyData = conn.getJsonArray("AnyData");
                } else {
                    cc.AnyData = conn.getJsonObject("AnyData");
                }
            }
            m_Hosts.put(key, cc);
        }
        JsonObject joPools = root.getJsonObject("Pools");
        set = joPools.keySet();
        for (String key : set) {
            CPoolConfig pc = new CPoolConfig(joPools.getJsonObject(key), key);
            m_Pools.put(key, pc);
        }
    }
}
