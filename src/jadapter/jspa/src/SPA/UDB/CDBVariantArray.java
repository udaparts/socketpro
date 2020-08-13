package SPA.UDB;

import java.util.ArrayList;

public class CDBVariantArray extends ArrayList<Object> implements SPA.IUSerializer, Comparable<CDBVariantArray> {

    @Override
    public SPA.CUQueue LoadFrom(SPA.CUQueue UQueue) {
        clear();
        int size = UQueue.LoadInt();
        while (size > 0) {
            Object obj = UQueue.LoadObject();
            add(obj);
            --size;
        }
        return UQueue;
    }

    @Override
    public SPA.CUQueue SaveTo(SPA.CUQueue UQueue) {
        int size = size();
        UQueue.Save(size);
        for (int n = 0; n < size; ++n) {
            UQueue.Save(get(n));
        }
        return UQueue;
    }

    @Override
    public int compareTo(CDBVariantArray arr) {
        Object vt0 = get(Ordinal);
        Object vt1 = arr.get(Ordinal);
        if (vt0 == null && vt1 == null) {
            return 0;
        } else if (vt1 == null) {
            return 1;
        } else if (vt0 == null) {
            return -1;
        }
        if (vt0 instanceof Integer) {
            return (int) vt0 - (int) vt1;
        } else if (vt0 instanceof Long) {
            return (int) ((long) vt0 - (long) vt1);
        } else if (vt0 instanceof Short) {
            return (short) vt0 - (short) vt1;
        } else if (vt0 instanceof Double) {
            double d = (double) vt0 - (double) vt1;
            if (d == 0) {
                return 0;
            } else if (d > 0.0) {
                return 1;
            }
            return -1;
        } else if (vt0 instanceof String) {
            String s0 = (String) vt0;
            String s1 = (String) vt1;
            if (CaseSenstive) {
                return s0.compareTo(s1);
            } else {
                return s0.compareToIgnoreCase(s1);
            }
        } else if (vt0 instanceof Boolean) {
            int n0 = (boolean) vt0 ? 1 : 0;
            int n1 = (boolean) vt1 ? 1 : 0;
            return n0 - n1;
        } else if (vt0 instanceof java.sql.Timestamp) {
            java.sql.Timestamp n0 = (java.sql.Timestamp) vt0;
            java.sql.Timestamp n1 = (java.sql.Timestamp) vt1;
            return n0.compareTo(n1);
        } else if (vt0 instanceof java.math.BigDecimal) {
            java.math.BigDecimal n0 = (java.math.BigDecimal) vt0;
            java.math.BigDecimal n1 = (java.math.BigDecimal) vt1;
            return n0.compareTo(n1);
        } else if (vt0 instanceof Float) {
            float f = (float) vt0 - (float) vt1;
            if (f == 0) {
                return 0;
            } else if (f > 0) {
                return 1;
            }
            return -1;
        } else if (vt0 instanceof java.util.UUID) {
            java.util.UUID n0 = (java.util.UUID) vt0;
            java.util.UUID n1 = (java.util.UUID) vt1;
            return n0.compareTo(n1);
        } else if (vt0 instanceof Character) {
            return (char) vt0 - (char) vt1;
        } else if (vt0 instanceof Byte) {
            return (byte) vt0 - (byte) vt1;
        } else if (vt0 instanceof java.util.Date) {
            java.util.Date n0 = (java.util.Date) vt0;
            java.util.Date n1 = (java.util.Date) vt1;
            return n0.compareTo(n1);
        }
        return 0;
    }
    public int Ordinal = 0;
    public boolean CaseSenstive = false;
}
