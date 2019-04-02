package SPA.ClientSide;

import SPA.tagEncryptionMethod;
import java.util.HashMap;
import javax.json.*;

public final class CSpConfig {

    private String m_WorkingDir;

    public String getWorkingDir() {
        return m_WorkingDir;
    }

    private String m_CertStore;

    public String getCertStore() {
        return m_CertStore;
    }

    private int m_QueuePassword = 0;

    public int getQueuePassword() {
        return m_QueuePassword;
    }

    private HashMap<String, CConnectionContext> m_Hosts = new HashMap<>();

    public HashMap<String, CConnectionContext> getHosts() {
        return new HashMap<>(m_Hosts);
    }

    private HashMap<String, CPoolConfig> m_Pools = new HashMap<>();

    public HashMap<String, CPoolConfig> getPools() {
        return new HashMap<>(m_Pools);
    }

    private java.util.ArrayList<String> m_KeysAllowed = null;

    public CSpConfig(JsonObject root) {
        if (root.containsKey("WorkingDir")) {
            m_WorkingDir = root.getString("WorkingDir");
        }
        if (root.containsKey("QueuePassword")) {
            String qp = root.getString("QueuePassword");
            CClientSocket.QueueConfigure.setWorkDirectory(qp);
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
        joHosts.keySet().stream().forEach((key) -> {
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
                }
            }
            m_Hosts.put(key, cc);
        });

        JsonObject joPools = root.getJsonObject("Pools");
    }
}
