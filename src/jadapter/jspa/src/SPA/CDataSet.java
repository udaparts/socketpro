package SPA;

import SPA.UDB.*;
import java.util.ArrayList;

public class CDataSet {

    public final static int INVALID_VALUE = -1;
    protected final Object m_cs = new Object();
    java.util.ArrayList<CTable> m_ds = new java.util.ArrayList<>();

    private Object Convert(CTable tbl, Object data, short vtTarget) {
        return tbl.ChangeType(data, vtTarget);
    }

    public int In(String dbName, String tblName, int ordinal, ArrayList<Object> v, CTable tbl) {
        if (v == null || v.isEmpty()) {
            return 0;
        }
        if (dbName == null) {
            tblName = "";
        }
        if (tblName == null || tblName.length() == 0) {
            return CTable.NO_TABLE_NAME_GIVEN;
        }
        synchronized (m_cs) {
            for (CTable t : m_ds) {
                if (!Is(t, dbName, tblName)) {
                    continue;
                }
                return t.In(ordinal, v, tbl, true);
            }
        }
        return CTable.NO_TABLE_FOUND;
    }

    public int NotIn(String dbName, String tblName, int ordinal, ArrayList<Object> v, CTable tbl) {
        if (v == null || v.isEmpty()) {
            return 0;
        }
        if (dbName == null) {
            tblName = "";
        }
        if (tblName == null || tblName.length() == 0) {
            return CTable.NO_TABLE_NAME_GIVEN;
        }
        synchronized (m_cs) {
            for (CTable t : m_ds) {
                if (!Is(t, dbName, tblName)) {
                    continue;
                }
                return t.NotIn(ordinal, v, tbl, true);
            }
        }
        return CTable.NO_TABLE_FOUND;
    }

    public void Set(String strIp, tagManagementSystem ms) {
        if (strIp == null) {
            strIp = "";
        }
        synchronized (m_cs) {
            m_strIp = strIp;
            m_ms = ms;
        }
    }

    public int AddRows(String dbName, String tblName, java.util.ArrayList<Object> v) {
        if (v == null || v.isEmpty()) {
            return 0;
        }
        if (dbName == null || tblName == null) {
            return INVALID_VALUE;
        }
        int count = v.size();
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                int col_count = meta.size();
                if ((count % col_count) > 0) {
                    return INVALID_VALUE;
                }
                CDBVariantArray prow = null;
                for (int n = 0; n < count; ++n) {
                    if ((n % col_count) == 0) {
                        prow = new CDBVariantArray();
                        tbl.getDataMatrix().add(prow);
                    }
                    short vtTarget = meta.get(n % col_count).DataType;
                    Object obj = Convert(tbl, v.get(n), vtTarget);
                    boolean added = prow.add(obj);
                }
                return count / col_count;
            }
        }
        return 0;
    }

    public int Between(String dbName, String tblName, int ordinal, Object vt0, Object vt1, CTable t) {
        if (dbName == null) {
            dbName = "";
        }
        if (tblName == null || tblName.length() == 0) {
            return CTable.NO_TABLE_NAME_GIVEN;
        }
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                return tbl.Between(ordinal, vt0, vt1, t, true);
            }
        }
        return CTable.NO_TABLE_FOUND;
    }

    private int FindKeyColIndex(CDBColumnInfoArray meta) {
        int index = 0;
        for (CDBColumnInfo col : meta) {
            if ((col.Flags & (CDBColumnInfo.FLAG_PRIMARY_KEY | CDBColumnInfo.FLAG_AUTOINCREMENT)) > 0) {
                return index;
            }
            ++index;
        }
        return INVALID_VALUE;
    }

    public int DeleteARow(String dbName, String tblName, Object vtKey) {
        int deleted = 0;
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                int key = FindKeyColIndex(meta);
                if (key == INVALID_VALUE) {
                    return INVALID_VALUE;
                }
                java.util.ArrayList<CDBVariantArray> vRow = tbl.m_vRow;
                int rows = vRow.size();
                for (int r = 0; r < rows; ++r) {
                    Object vtKey0 = vRow.get(r).get(key);
                    if (tbl.eq(vtKey0, vtKey) > 0) {
                        vRow.remove(r);
                        deleted = 1;
                        break;
                    }
                }
            }
        }
        return deleted;
    }

    private int FindKeyColIndex(CDBColumnInfoArray meta, Integer key1) {
        int index = 0;
        int key0 = INVALID_VALUE;
        key1 = INVALID_VALUE;
        for (CDBColumnInfo col : meta) {
            if ((col.Flags & (CDBColumnInfo.FLAG_PRIMARY_KEY | CDBColumnInfo.FLAG_AUTOINCREMENT)) > 0) {
                if (key0 == INVALID_VALUE) {
                    key0 = index;
                } else {
                    key1 = index;
                    break;
                }
            }
            ++index;
        }
        return INVALID_VALUE;
    }

    public int DeleteARow(String dbName, String tblName, Object vtKey0, Object vtKey1) {
        int deleted = 0;
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                Integer key1 = INVALID_VALUE;
                int key = FindKeyColIndex(meta, key1);
                if (key == INVALID_VALUE || key1 == INVALID_VALUE) {
                    return INVALID_VALUE;
                }
                java.util.ArrayList<CDBVariantArray> vRow = tbl.m_vRow;
                int rows = vRow.size();
                for (int r = 0; r < rows; ++r) {
                    Object vtKey = vRow.get(r).get(key);
                    Object vt2 = vRow.get(r).get(key1);
                    if (tbl.eq(vtKey0, vtKey) > 0 && tbl.eq(vt2, vtKey1) > 0) {
                        vRow.remove(r);
                        deleted = 1;
                        break;
                    }
                }
            }
        }
        return deleted;
    }

    public int Find(String dbName, String tblName, int ordinal, CTable.Operator op, Object vt, CTable t) {
        if (dbName == null) {
            dbName = "";
        }
        if (tblName == null || tblName.length() == 0) {
            return CTable.NO_TABLE_NAME_GIVEN;
        }
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                return tbl.Find(ordinal, op, vt, t, true);
            }
        }
        return CTable.NO_TABLE_FOUND;
    }

    private ArrayList<Object> FindARowInternal(CTable tbl, int ordinal, Object key) {
        java.util.ArrayList<CDBVariantArray> vRow = tbl.m_vRow;
        int rows = vRow.size();
        for (int r = 0; r < rows; ++r) {
            Object vtKey = vRow.get(r).get(ordinal);
            if (tbl.eq(key, vtKey) > 0) {
                return vRow.get(r);
            }
        }
        return null;
    }

    private ArrayList<Object> FindARowInternal(CTable tbl, int f0, int f1, Object key0, Object key1) {
        java.util.ArrayList<CDBVariantArray> vRow = tbl.m_vRow;
        int rows = vRow.size();
        for (int r = 0; r < rows; ++r) {
            Object vtKey = vRow.get(r).get(f0);
            Object vtKey1 = vRow.get(r).get(f1);
            if (tbl.eq(key0, vtKey) > 0 && tbl.eq(key1, vtKey1) > 0) {
                return vRow.get(r);
            }
        }
        return null;
    }

    public int UpdateARow(String dbName, String tblName, java.util.ArrayList<Object> pvt) {
        if (pvt == null || pvt.isEmpty()) {
            return INVALID_VALUE;
        }
        int count = pvt.size();
        if ((count % 2) > 0) {
            return INVALID_VALUE;
        }
        int updated = 0;
        synchronized (m_cs) {
            for (CTable tbl : m_ds) {
                if (!Is(tbl, dbName, tblName)) {
                    continue;
                }
                CDBColumnInfoArray meta = tbl.getMeta();
                int col_count = meta.size();
                if ((count % col_count) > 0 || 2 * col_count != count) {
                    return INVALID_VALUE;
                }
                ArrayList<Object> row = null;
                Integer key1 = 0;
                int key0 = FindKeyColIndex(meta, key1);
                if (key0 == INVALID_VALUE && key1 == INVALID_VALUE) {
                    return INVALID_VALUE;
                } else if (key1 == INVALID_VALUE) {
                    row = FindARowInternal(tbl, key0, pvt.get(key0 * 2));
                } else {
                    row = FindARowInternal(tbl, key0, key1, pvt.get(key0 * 2), pvt.get(key1 * 2));
                }
                if (row != null) {
                    for (int n = 0; n < col_count; ++n) {
                        short vtTarget = meta.get(n).DataType;
                        Object vt = pvt.get(2 * n + 1);
                        row.set(n, vt);
                    }
                    updated = 1;
                }
                break;
            }
        }
        return updated;
    }

    public int FindNull(String dbName, String tblName, int ordinal, CTable tbl) {
        Object vt = null;
        return Find(dbName, tblName, ordinal, CTable.Operator.is_null, vt, tbl);
    }

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
            eq = col.TablePath.equals(tblName);
        } else {
            eq = col.TablePath.equalsIgnoreCase(tblName);
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

    public void setDBServerName(String s) {
        synchronized (m_cs) {
            m_strHostName = s;
            if (s == null) {
                m_strHostName = "";
            }
        }
    }

    public void setUpdater(String s) {
        synchronized (m_cs) {
            m_strUpdater = s;
            if (s == null) {
                m_strUpdater = "";
            }
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
            CTable tbl = new CTable(meta, m_bFieldNameCaseSensitive, m_bDataCaseSensitive);
            java.util.HashMap<Integer, CDBColumnInfo> keys = tbl.getKeys();
            if (keys.isEmpty() || keys.size() > 2) {
                return;
            }
            m_ds.add(tbl);
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
