package SPA;

import SPA.UDB.*;

public class CDataSet {

    public final static int INVALID_VALUE = -1;
    protected final Object m_cs = new Object();
    protected java.util.ArrayList<CTable> m_ds = new java.util.ArrayList<>();

    public void setDBNameCaseSensitive(boolean bCaseSensitive) {
        synchronized (m_cs) {
            m_bDBNameCaseSensitive = bCaseSensitive;
        }
    }

    public void setTableNameCaseSensitive(boolean bCaseSensitive) {
        synchronized (m_cs) {
            m_bTableNameCaseSensitive = bCaseSensitive;
        }
    }

    public void setFieldNameCaseSensitive(boolean bCaseSensitive) {
        synchronized (m_cs) {
            m_bFieldNameCaseSensitive = bCaseSensitive;
        }
    }

    public void setDataCaseSensitive(boolean bCaseSensitive) {
        synchronized (m_cs) {
            m_bDataCaseSensitive = bCaseSensitive;
        }
    }

    public boolean getDBNameCaseSensitive() {
        synchronized (m_cs) {
            return m_bDBNameCaseSensitive;
        }
    }

    public boolean getTableNameCaseSensitive() {
        synchronized (m_cs) {
            return m_bTableNameCaseSensitive;
        }
    }

    public boolean getFieldNameCaseSensitive() {
        synchronized (m_cs) {
            return m_bFieldNameCaseSensitive;
        }
    }

    public boolean getDataCaseSensitive() {
        synchronized (m_cs) {
            return m_bDataCaseSensitive;
        }
    }

    public String getDBServerIp() {
        synchronized (m_cs) {
            return m_strIp;
        }
    }

    public String getDBServerName() {
        synchronized (m_cs) {
            return m_strHostName;
        }
    }

    public String getUpdater() {
        synchronized (m_cs) {
            return m_strUpdater;
        }
    }

    public tagManagementSystem getDBManagementSystem() {
        synchronized (m_cs) {
            return m_ms;
        }
    }

    public boolean isEmpty() {
        synchronized (m_cs) {
            return m_ds.isEmpty();
        }
    }

    public void Empty() {
        synchronized (m_cs) {
            m_ds.clear();
        }
    }

    public void Swap(CDataSet tc) {
        if (tc == null) {
            return;
        }
        synchronized (m_cs) {
            java.util.ArrayList<CTable> ds = m_ds;
            m_ds = tc.m_ds;
            tc.m_ds = ds;
            String s = m_strIp;
            m_strIp = tc.m_strIp;
            tc.m_strIp = s;
            s = m_strHostName;
            m_strHostName = tc.m_strHostName;
            tc.m_strHostName = s;
            s = m_strUpdater;
            m_strUpdater = tc.m_strUpdater;
            tc.m_strUpdater = s;
            tagManagementSystem ms = m_ms;
            m_ms = tc.m_ms;
            tc.m_ms = ms;
        }
    }

    public void AddEmptyRowset(CDBColumnInfoArray meta) {
        if (meta == null || meta.isEmpty()) {
            return;
        }
        synchronized (m_cs) {
            m_ds.add(new CTable(meta, m_bFieldNameCaseSensitive, m_bDataCaseSensitive));
        }
    }

    private String m_strIp = "";
    private String m_strHostName = "";
    private String m_strUpdater = "";
    private tagManagementSystem m_ms = tagManagementSystem.msUnknown;
    private boolean m_bDBNameCaseSensitive = false;
    private boolean m_bTableNameCaseSensitive = false;
    private boolean m_bFieldNameCaseSensitive = false;
    private boolean m_bDataCaseSensitive = false;
}
