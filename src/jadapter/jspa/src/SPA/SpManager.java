package SPA;

import javax.json.*;
import SPA.ClientSide.*;
import java.io.FileInputStream;

public final class SpManager {

    private static final Object m_cs = new Object();
    private static boolean m_Middle = false;
    private static CSpConfig m_sc = null;

    public static boolean getMidTier() {
        synchronized (m_cs) {
            return m_Middle;
        }
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

    public static void main(String[] args) {
        try {
            CSpConfig jc = SetConfig(false, "C:\\cyetest\\socketpro\\src\\njadapter\\sp_config.json");
            jc = null;
        } catch (Exception err) {
            System.out.println(err.getLocalizedMessage());
        }
    }
}
