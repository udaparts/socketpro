package SPA;

import SPA.UDB.*;

public class CDataSet {

    public final static int INVALID_VALUE = -1;
    protected final Object m_cs = new Object();
    protected java.util.ArrayList<CTable> m_ds = new java.util.ArrayList<>();

    public CDBColumnInfoArray GetColumMeta(String dbName, String tblName) {
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                return meta;
            }
        }
        return new CDBColumnInfoArray();
    }

    public int GetRowCount(String dbName, String tblName) {
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                return tbl.getDataMatrix().size();
            }
        }
        return INVALID_VALUE;
    }

    public int GetColumnCount(String dbName, String tblName) {
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                return meta.size();
            }
        }
        return 0;
    }

    private boolean Is(CTable tbl, String dbName, String tblName) {
        boolean eq;
        CDBColumnInfoArray meta = tbl.getMeta();
        CDBColumnInfo col = meta.get(0);
        if (m_bDBNameCaseSensitive) {
            eq = col.DBPath.equals(dbName);
        } else {
            eq = col.DBPath.equalsIgnoreCase(dbName);
        }
        if (!eq) {
            return false;
        }
        if (m_bTableNameCaseSensitive) {
            eq = col.TablePath.equals(dbName);
        } else {
            eq = col.TablePath.equalsIgnoreCase(dbName);
        }
        return eq;
    }

    public int FindOrdinal(String dbName, String tblName, String colName) {
        if (tblName == null || colName == null) {
            return CTable.INVALID_ORDINAL;
        }
        if (dbName == null) {
            dbName = "";
        }
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                return tbl.FindOrdinal(colName);
            }
        }
        return CTable.INVALID_ORDINAL;
    }

    public java.util.HashMap<Integer, CDBColumnInfo> FindKeys(String dbName, String tblName) {
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                return tbl.getKeys();
            }
        }
        return new java.util.HashMap<>();
    }

    public java.util.ArrayList<Pair<String, String>> getDBTablePair() {
        java.util.ArrayList<Pair<String, String>> v = new java.util.ArrayList<>();
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                CDBColumnInfoArray meta = tbl.getMeta();
                CDBColumnInfo col = meta.get(0);
                Pair<String, String> p = new Pair<>(col.DBPath, col.TablePath);
                v.add(p);
            }
        }
        return v;
    }

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
