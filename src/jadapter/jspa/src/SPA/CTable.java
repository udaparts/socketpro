package SPA;

import SPA.UDB.*;
import java.util.ArrayList;

public class CTable {

    public final static int BAD_ORDINAL = -1;
    public final static int BAD_DATA_TYPE = -2;
    public final static int OPERATION_NOT_SUPPORTED = -3;
    public final static int COMPARISON_NOT_SUPPORTED = -4;
    public final static int NO_TABLE_NAME_GIVEN = -5;
    public final static int NO_TABLE_FOUND = -6;

    public final static int INVALID_ORDINAL = -1;

    public enum Operator {

        equal(0),
        great(1),
        less(2),
        great_equal(3),
        less_equal(4),
        not_equal(5),
        is_null(6);

        private final int intValue;
        private static java.util.HashMap<Integer, Operator> mappings;

        private static java.util.HashMap<Integer, Operator> getMappings() {
            synchronized (Operator.class) {
                if (mappings == null) {
                    mappings = new java.util.HashMap<>();
                }
            }
            return mappings;
        }

        @SuppressWarnings("LeakingThisInConstructor")
        private Operator(int value) {
            intValue = value;
            getMappings().put(value, this);
        }

        public int getValue() {
            return intValue;
        }

        public static Operator forValue(int value) {
            return getMappings().get(value);
        }
    }

    private boolean m_bFieldNameCaseSensitive = false;
    private boolean m_bDataCaseSensitive = false;
    private CDBColumnInfoArray m_meta = new CDBColumnInfoArray();
    protected java.util.ArrayList<CDBVariantArray> m_vRow = new java.util.ArrayList<>();

    public CTable() {
    }

    public CTable(CDBColumnInfoArray meta, boolean bFieldNameCaseSensitive, boolean bDataCaseSensitive) {
        if (meta != null) {
            //deep copy
            CUQueue q = CScopeUQueue.Lock();
            meta.SaveTo(q);
            m_meta.LoadFrom(q);
            CScopeUQueue.Unlock(q);
        }
        m_bFieldNameCaseSensitive = bFieldNameCaseSensitive;
        m_bDataCaseSensitive = bDataCaseSensitive;
    }

    public CDBColumnInfoArray getMeta() {
        return m_meta;
    }

    public java.util.ArrayList<CDBVariantArray> getDataMatrix() {
        return m_vRow;
    }

    public java.util.HashMap<Integer, CDBColumnInfo> getKeys() {
        int index = 0;
        java.util.HashMap<Integer, CDBColumnInfo> map = new java.util.HashMap<>();
        for (CDBColumnInfo col : m_meta) {
            if ((col.Flags & CDBColumnInfo.FLAG_PRIMARY_KEY) == CDBColumnInfo.FLAG_PRIMARY_KEY || (col.Flags & CDBColumnInfo.FLAG_AUTOINCREMENT) == CDBColumnInfo.FLAG_AUTOINCREMENT) {
                map.put(index, col);
            }
            ++index;
        }
        return map;
    }

    public CTable Copy() {
        CTable tbl = new CTable(m_meta, m_bFieldNameCaseSensitive, m_bDataCaseSensitive);
        CUQueue q = CScopeUQueue.Lock();
        for (CDBVariantArray v : m_vRow) {
            //deep copy
            v.SaveTo(q);
            CDBVariantArray r = new CDBVariantArray();
            r.LoadFrom(q);
            tbl.m_vRow.add(r);
        }
        CScopeUQueue.Unlock(q);
        return tbl;
    }

    public int Append(CTable tbl) {
        if (tbl == null) {
            return 1;
        }
        if (tbl.m_meta.size() != m_meta.size()) {
            return OPERATION_NOT_SUPPORTED;
        }
        int index = 0;
        for (CDBColumnInfo col : m_meta) {
            if (col.DataType != tbl.m_meta.get(index).DataType) {
                return OPERATION_NOT_SUPPORTED;
            }
            ++index;
        }
        for (CDBVariantArray v : tbl.m_vRow) {
            m_vRow.add(v);
        }
        return 1;
    }

    public int Sort(int ordinal, boolean desc) {
        if (ordinal < 0 || ordinal >= m_meta.size()) {
            return BAD_ORDINAL;
        }
        for (CDBVariantArray r : m_vRow) {
            r.CaseSenstive = this.m_bDataCaseSensitive;
            r.Ordinal = ordinal;
        }
        if (desc) {
            java.util.Collections.sort(m_vRow, java.util.Collections.reverseOrder());
        } else {
            java.util.Collections.sort(m_vRow);
        }
        return 1;
    }

    public int Between(int ordinal, Object vt0, Object vt1, CTable tbl) {
        return Between(ordinal, vt0, vt1, tbl, false);
    }

    private boolean In(Object v, ArrayList<Object> vArray) {
        for (Object o : vArray) {
            if (eq(o, v) > 0) {
                return true;
            }
        }
        return false;
    }

    public int In(int ordinal, ArrayList<Object> vArray, CTable tbl) {
        return In(ordinal, vArray, tbl, false);
    }

    public int In(int ordinal, ArrayList<Object> vArray, CTable tbl, boolean copyData) {
        if (tbl == null) {
            return OPERATION_NOT_SUPPORTED;
        } else {
            tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
            tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
            tbl.m_vRow.clear();
            if (copyData) {
                CUQueue q = CScopeUQueue.Lock();
                m_meta.SaveTo(q);
                tbl.m_meta.LoadFrom(q);
                CScopeUQueue.Unlock(q);
            } else {
                tbl.m_meta = m_meta;
            }
        }
        if (ordinal < 0 || ordinal >= m_meta.size()) {
            return BAD_ORDINAL;
        }
        if (vArray == null) {
            return OPERATION_NOT_SUPPORTED;
        }
        short type = m_meta.get(ordinal).DataType;
        if (type == (tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY)) {
            type = tagVariantDataType.sdVT_BSTR; //Table string is always unicode string 
        }
        int index = 0;
        for (Object o : vArray) {
            Object vt = ChangeType(o, type);
            if (vt == null) {
                return BAD_DATA_TYPE;
            }
            vArray.set(index, vt);
            ++index;
        }
        int rows = m_vRow.size();
        for (int r = 0; r < rows; ++r) {
            CDBVariantArray prow = m_vRow.get(r);
            Object v0 = prow.get(ordinal);
            if (In(v0, vArray)) {
                if (copyData) {
                    CDBVariantArray p = new CDBVariantArray();
                    CUQueue q = CScopeUQueue.Lock();
                    prow.SaveTo(q);
                    p.LoadFrom(q);
                    CScopeUQueue.Unlock(q);
                    tbl.m_vRow.add(p);
                } else {
                    tbl.m_vRow.add(prow);
                }
            }
        }
        return 1;
    }

    public int NotIn(int ordinal, ArrayList<Object> vArray, CTable tbl) {
        return NotIn(ordinal, vArray, tbl, false);
    }

    public int NotIn(int ordinal, ArrayList<Object> vArray, CTable tbl, boolean copyData) {
        if (tbl == null) {
            return OPERATION_NOT_SUPPORTED;
        } else {
            tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
            tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
            tbl.m_vRow.clear();
            if (copyData) {
                CUQueue q = CScopeUQueue.Lock();
                m_meta.SaveTo(q);
                tbl.m_meta.LoadFrom(q);
                CScopeUQueue.Unlock(q);
            } else {
                tbl.m_meta = m_meta;
            }
        }
        if (ordinal < 0 || ordinal >= m_meta.size()) {
            return BAD_ORDINAL;
        }
        if (vArray == null) {
            return OPERATION_NOT_SUPPORTED;
        }
        short type = m_meta.get(ordinal).DataType;
        if (type == (tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY)) {
            type = tagVariantDataType.sdVT_BSTR; //Table string is always unicode string 
        }
        int index = 0;
        for (Object o : vArray) {
            Object vt = ChangeType(o, type);
            if (vt == null) {
                return BAD_DATA_TYPE;
            }
            vArray.set(index, vt);
            ++index;
        }
        int rows = m_vRow.size();
        for (int r = 0; r < rows; ++r) {
            CDBVariantArray prow = m_vRow.get(r);
            Object v0 = prow.get(ordinal);
            if (v0 != null && !In(v0, vArray)) {
                if (copyData) {
                    CDBVariantArray p = new CDBVariantArray();
                    CUQueue q = CScopeUQueue.Lock();
                    prow.SaveTo(q);
                    p.LoadFrom(q);
                    CScopeUQueue.Unlock(q);
                    tbl.m_vRow.add(p);
                } else {
                    tbl.m_vRow.add(prow);
                }
            }
        }
        return 1;
    }

    public int Between(int ordinal, Object vt0, Object vt1, CTable tbl, boolean copyData) {
        if (tbl == null) {
            return OPERATION_NOT_SUPPORTED;
        } else {
            tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
            tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
            tbl.m_vRow.clear();
            if (copyData) {
                CUQueue q = CScopeUQueue.Lock();
                m_meta.SaveTo(q);
                tbl.m_meta.LoadFrom(q);
                CScopeUQueue.Unlock(q);
            } else {
                tbl.m_meta = m_meta;
            }
        }
        if (ordinal < 0 || ordinal >= m_meta.size()) {
            return BAD_ORDINAL;
        }
        if (vt0 == null || vt1 == null) {
            return OPERATION_NOT_SUPPORTED;
        }

        short type = m_meta.get(ordinal).DataType;
        if (type == (tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY)) {
            type = tagVariantDataType.sdVT_BSTR; //Table string is always unicode string 
        }
        Object v0, v1;
        v0 = ChangeType(vt0, type);
        if (v0 == null) {
            return BAD_DATA_TYPE;
        }
        v1 = ChangeType(vt1, type);
        if (v1 == null) {
            return BAD_DATA_TYPE;
        }
        Object small_vt = v0;
        Object large_vt = v1;
        int res = gt(v0, v1);
        if (res < 0) {
            return res;
        } else if (res > 0) {
            small_vt = v1;
            large_vt = v0;
        }
        int cols = m_meta.size();
        int rows = m_vRow.size();
        for (int r = 0; r < rows; ++r) {
            CDBVariantArray prow = m_vRow.get(r);
            Object vt = prow.get(ordinal);
            res = ge(vt, small_vt);
            if (res == 0) {
                continue;
            }
            if (res < 0) {
                return res;
            }
            res = le(vt, large_vt);
            if (res == 0) {
                continue;
            }
            if (res < 0) {
                return res;
            }
            if (copyData) {
                CDBVariantArray p = new CDBVariantArray();
                CUQueue q = CScopeUQueue.Lock();
                prow.SaveTo(q);
                p.LoadFrom(q);
                CScopeUQueue.Unlock(q);
                tbl.m_vRow.add(p);
            } else {
                tbl.m_vRow.add(prow);
            }
        }
        return 1;
    }

    public int FindNull(int ordinal, CTable tbl) {
        return Find(ordinal, Operator.is_null, null, tbl, false);
    }

    public int FindNull(int ordinal, CTable tbl, boolean copyData) {
        return Find(ordinal, Operator.is_null, null, tbl, copyData);
    }

    public int Find(int ordinal, Operator op, Object vt, CTable tbl) {
        return Find(ordinal, op, vt, tbl, false);
    }

    public int Find(int ordinal, Operator op, Object vt, CTable tbl, boolean copyData) {
        if (tbl == null) {
            return OPERATION_NOT_SUPPORTED;
        } else {
            tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
            tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
            tbl.m_vRow.clear();
            if (copyData) {
                CUQueue q = CScopeUQueue.Lock();
                m_meta.SaveTo(q);
                tbl.m_meta.LoadFrom(q);
                CScopeUQueue.Unlock(q);
            } else {
                tbl.m_meta = m_meta;
            }
        }
        if (ordinal < 0 || ordinal >= m_meta.size()) {
            return BAD_ORDINAL;
        }
        if (vt == null && op != Operator.is_null) {
            return COMPARISON_NOT_SUPPORTED;
        }
        short type = m_meta.get(ordinal).DataType;
        if (type == (tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY)) {
            type = tagVariantDataType.sdVT_BSTR; //Table string is always unicode string 
        }
        int cols = m_meta.size();
        int rows = m_vRow.size();
        for (int r = 0; r < rows; ++r) {
            boolean ok;
            CDBVariantArray prow = m_vRow.get(r);
            if (op == Operator.is_null) {
                ok = (prow.get(ordinal) == null);
            } else {
                int res;
                Object v = ChangeType(vt, type);
                if (v == null) {
                    return BAD_DATA_TYPE;
                }
                Object v0 = prow.get(ordinal);
                switch (op) {
                    case equal:
                        res = eq(v0, v);
                        break;
                    case not_equal:
                        res = neq(v0, v);
                        break;
                    case great:
                        res = gt(v0, v);
                        break;
                    case great_equal:
                        res = ge(v0, v);
                        break;
                    case less:
                        res = lt(v0, v);
                        break;
                    case less_equal:
                        res = le(v0, v);
                        break;
                    default:
                        return OPERATION_NOT_SUPPORTED;
                }
                if (res < 0) {
                    return res;
                }
                ok = (res > 0);
            }
            if (ok) {
                if (copyData) {
                    CDBVariantArray row = new CDBVariantArray();
                    CUQueue q = CScopeUQueue.Lock();
                    prow.SaveTo(q);
                    row.LoadFrom(q);
                    CScopeUQueue.Unlock(q);
                    tbl.m_vRow.add(row);
                } else {
                    tbl.m_vRow.add(prow);
                }
            }
        }
        return 1;
    }

    private int gt(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 > (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 > (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 > (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 > (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) > 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) > 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 > n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return n0.after(n1) ? 1 : 0;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) > 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 > (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) > 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 > (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 > (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return n0.after(n1) ? 1 : 0;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    private int ge(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 >= (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 >= (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 >= (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 >= (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) >= 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) >= 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 >= n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return (n0.after(n1) || n0.equals(n1)) ? 1 : 0;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) >= 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 >= (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) >= 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 >= (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 >= (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return (n0.after(n1) || n0.equals(n1)) ? 1 : 0;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    private int lt(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 < (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 < (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 < (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 < (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) < 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) < 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 < n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return n0.before(n1) ? 1 : 0;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) < 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 < (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) < 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 < (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 < (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return n0.before(n1) ? 1 : 0;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    private int le(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 <= (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 <= (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 <= (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 <= (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) <= 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) <= 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 <= n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return (n0.before(n1) || n0.equals(n1)) ? 1 : 0;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) <= 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 <= (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) <= 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 <= (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 <= (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return (n0.before(n1) || n0.equals(n1)) ? 1 : 0;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    int eq(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 == (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 == (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 == (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 == (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) == 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) == 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 == n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return n0.equals(n1) ? 1 : 0;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) == 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 == (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) == 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 == (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 == (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return n0.equals(n1) ? 1 : 0;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    private int neq(Object vt0, Object vt1) {
        if (vt0 == null) {
            return 0;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 != (int) vt1 ? 1 : 0;
        } else if (vt0 instanceof Long) {
            return (long) vt0 != (long) vt1 ? 1 : 0;
        } else if (vt0 instanceof Short) {
            return (short) vt0 != (short) vt1 ? 1 : 0;
        } else if (vt0 instanceof Double) {
            return (double) vt0 != (double) vt1 ? 1 : 0;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (this.m_bDataCaseSensitive) {
                return s0.compareTo(s1) != 0 ? 1 : 0;
            } else {
                return s0.compareToIgnoreCase(s1) != 0 ? 1 : 0;
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 != n1 ? 1 : 0;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return n0.equals(n1) ? 0 : 1;
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1) != 0 ? 1 : 0;
        } else if (vt0 instanceof Float) {
            return (float) vt0 != (float) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1) != 0 ? 1 : 0;
        } else if (vt0 instanceof Character) {
            return (char) vt0 != (char) vt1 ? 1 : 0;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 != (byte) vt1 ? 1 : 0;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return n0.equals(n1) ? 0 : 1;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    Object ChangeType(Object vt0, short vtTarget) {
        if (vt0 == null) {
            return null;
        }
        switch (vtTarget) {
            case tagVariantDataType.sdVT_I1:
            case tagVariantDataType.sdVT_UI1:
                if (vt0 instanceof Integer) {
                    return (byte) (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (byte) (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (byte) (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (byte) (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (byte) (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (byte) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.byteValue();
                } else if (vt0 instanceof String) {
                    return Byte.parseByte((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_I2:
            case tagVariantDataType.sdVT_UI2:
                if (vt0 instanceof Integer) {
                    return (short) (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (short) (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (short) (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (short) (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (short) (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (short) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return (short) n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.shortValue();
                } else if (vt0 instanceof String) {
                    return Short.parseShort((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_I4:
            case tagVariantDataType.sdVT_UI4:
            case tagVariantDataType.sdVT_INT:
            case tagVariantDataType.sdVT_UINT:
                if (vt0 instanceof Integer) {
                    return (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (int) (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (int) (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (int) (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (int) (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (int) (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (int) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return (int) n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.intValue();
                } else if (vt0 instanceof String) {
                    return Integer.parseInt((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_I8:
            case tagVariantDataType.sdVT_UI8:
                if (vt0 instanceof Integer) {
                    return (long) (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (long) (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (long) (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (long) (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (long) (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (long) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return (long) n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.longValue();
                } else if (vt0 instanceof String) {
                    return Long.parseLong((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_R4:
                if (vt0 instanceof Integer) {
                    return (float) (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (float) (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (float) (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (float) (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (float) (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (float) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return (float) n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.floatValue();
                } else if (vt0 instanceof String) {
                    return Float.parseFloat((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_R8:
                if (vt0 instanceof Integer) {
                    return (double) (int) vt0;
                } else if (vt0 instanceof Long) {
                    return (double) (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (double) (short) vt0;
                } else if (vt0 instanceof Double) {
                    return (double) vt0;
                } else if (vt0 instanceof Byte) {
                    return (double) (byte) vt0;
                } else if (vt0 instanceof Float) {
                    return (double) (float) vt0;
                } else if (vt0 instanceof Character) {
                    return (double) (char) vt0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return (double) n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.doubleValue();
                } else if (vt0 instanceof String) {
                    return Double.parseDouble((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_DECIMAL:
                if (vt0 instanceof Integer) {
                    return new java.math.BigDecimal((int) vt0);
                } else if (vt0 instanceof Long) {
                    return new java.math.BigDecimal((long) vt0);
                } else if (vt0 instanceof Short) {
                    return new java.math.BigDecimal((short) vt0);
                } else if (vt0 instanceof Double) {
                    return new java.math.BigDecimal((double) vt0);
                } else if (vt0 instanceof Byte) {
                    return new java.math.BigDecimal((byte) vt0);
                } else if (vt0 instanceof Float) {
                    return new java.math.BigDecimal((float) vt0);
                } else if (vt0 instanceof Character) {
                    return new java.math.BigDecimal((char) vt0);
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return new java.math.BigDecimal(n0);
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0;
                } else if (vt0 instanceof String) {
                    java.math.BigDecimal n0 = new java.math.BigDecimal((String) vt0);
                    return n0;
                } else {
                    return null;
                }
            case (tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_I1):
            case tagVariantDataType.sdVT_BSTR:
                return vt0.toString();
            case (tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI1):
                if (vt0 instanceof byte[]) {
                    return (byte[]) vt0;
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_CLSID:
                if (vt0 instanceof java.util.UUID) {
                    return (java.util.UUID) vt0;
                } else if (vt0 instanceof String) {
                    return java.util.UUID.fromString((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_DATE:
                if (vt0 instanceof java.sql.Timestamp) {
                    return (java.sql.Timestamp) vt0;
                } else if (vt0 instanceof String) {
                    return java.sql.Timestamp.valueOf((String) vt0);
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_BOOL:
                if (vt0 instanceof Integer) {
                    return (int) vt0 != 0;
                } else if (vt0 instanceof Long) {
                    return (long) vt0 != 0;
                } else if (vt0 instanceof Short) {
                    return (short) vt0 != 0;
                } else if (vt0 instanceof Double) {
                    return (double) vt0 != 0;
                } else if (vt0 instanceof Byte) {
                    return (byte) vt0 != 0;
                } else if (vt0 instanceof Float) {
                    return (float) vt0 != 0;
                } else if (vt0 instanceof Character) {
                    return (char) vt0 != 0;
                } else if (vt0 instanceof Boolean) {
                    byte n0 = (byte) ((boolean) vt0 ? 1 : 0);
                    return n0;
                } else if (vt0 instanceof java.math.BigDecimal) {
                    java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
                    return n0.doubleValue() != 0;
                } else if (vt0 instanceof String) {
                    return Boolean.parseBoolean((String) vt0);
                } else {
                    return null;
                }
            default:
                break;
        }
        return null;
    }

    public int FindOrdinal(String colName) {
        if (colName == null) {
            return INVALID_ORDINAL;
        }
        int ordinal = 0;
        for (CDBColumnInfo col : m_meta) {
            boolean b;
            if (m_bFieldNameCaseSensitive) {
                b = colName.equals(col.OriginalName);
            } else {
                b = colName.equalsIgnoreCase(col.OriginalName);
            }
            if (b) {
                return ordinal;
            }
            ++ordinal;
        }
        return INVALID_ORDINAL;
    }
}
