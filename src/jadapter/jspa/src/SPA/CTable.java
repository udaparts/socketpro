package SPA;

import SPA.*;
import SPA.UDB.*;

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
        is_null(5);

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
    final private CDBColumnInfoArray m_meta = new CDBColumnInfoArray();
    private java.util.ArrayList<CDBVariantArray> m_vRow = new java.util.ArrayList<>();

    public CTable() {

    }

    public CTable(CDBColumnInfoArray meta, boolean bFieldNameCaseSensitive, boolean bDataCaseSensitive) {
        if (meta != null) {
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

    private Object ChangeType(Object vt0, short vtTarget) {
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
                    return (int)(short) vt0;
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
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_I8:
            case tagVariantDataType.sdVT_UI8:
                if (vt0 instanceof Integer) {
                    return (long)(int) vt0;
                } else if (vt0 instanceof Long) {
                    return (long) vt0;
                } else if (vt0 instanceof Short) {
                    return (long)(short) vt0;
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
                } else {
                    return null;
                }
            case tagVariantDataType.sdVT_R4:
                if (vt0 instanceof Integer) {
                    return (float)(int) vt0;
                } else if (vt0 instanceof Long) {
                    return (float)(long) vt0;
                } else if (vt0 instanceof Short) {
                    return (float)(short) vt0;
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
                } else {
                    return null;
                }
            default:
                break;
        }
        return null;
    }

    private int eq(Object vt0, Object vt1) {
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
