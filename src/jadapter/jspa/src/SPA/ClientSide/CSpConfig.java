package SPA.ClientSide;

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
        return m_Hosts;
    }
    
    private HashMap<String, CPoolConfig> m_Pools = new HashMap<>();
    public HashMap<String, CPoolConfig> getPools() {
        return m_Pools;
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
            for (int n =0; n < ja.size(); ++n) {
                m_KeysAllowed.add(ja.getString(n));
            }
        }
        JsonObject joHosts = root.getJsonObject("Hosts");
        
        JsonObject joPools = root.getJsonObject("Pools");
    }
}
