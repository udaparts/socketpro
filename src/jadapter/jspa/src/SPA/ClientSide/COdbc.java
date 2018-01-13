package SPA.ClientSide;

import SPA.*;

public class COdbc extends CAsyncDBHandler {

    public final static int sidOdbc = SPA.BaseServiceID.sidODBC; //asynchronous ODBC service id

    public COdbc() {
        super(sidOdbc);
    }

    /**
     * You may use the protected constructor when extending this class
     *
     * @param sid a service id
     */
    protected COdbc(int sid) {
        super(sid);
    }

    //meta recordsets
    public static final short idSQLColumnPrivileges = 0x7f00 + 100;
    public static final short idSQLColumns = 0x7f00 + 101;
    public static final short idSQLForeignKeys = 0x7f00 + 102;
    public static final short idSQLPrimaryKeys = 0x7f00 + 103;
    public static final short idSQLProcedureColumns = 0x7f00 + 104;
    public static final short idSQLProcedures = 0x7f00 + 105;
    public static final short idSQLSpecialColumns = 0x7f00 + 106;
    public static final short idSQLStatistics = 0x7f00 + 107;
    public static final short idSQLTablePrivileges = 0x7f00 + 108;
    public static final short idSQLTables = 0x7f00 + 109;
    public static final short idSQLGetInfo = 0x7f00 + 110;

    public final static int ER_SUCCESS = 0;
    public final static int ER_ERROR = -1;

    //error codes from async ODBC server library
    public final static int ER_NO_DB_OPENED_YET = -1981;
    public final static int ER_BAD_END_TRANSTACTION_PLAN = -1982;
    public final static int ER_NO_PARAMETER_SPECIFIED = -1983;
    public final static int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
    public final static int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
    public final static int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
    public final static int ER_NO_DB_NAME_SPECIFIED = -1987;
    public final static int ER_ODBC_ENVIRONMENT_NOT_INITIALIZED = -1988;
    public final static int ER_BAD_MANUAL_TRANSACTION_STATE = -1989;
    public final static int ER_BAD_INPUT_PARAMETER_DATA_TYPE = -1990;
    public final static int ER_BAD_PARAMETER_DIRECTION_TYPE = -1991;
    public final static int ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = -1992;

    private final java.util.HashMap<Short, Object> m_mapInfo = new java.util.HashMap<>();

    public final boolean ColumnPrivileges(String CatalogName, String SchemaName, String TableName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, null);
    }

    public final boolean ColumnPrivileges(String CatalogName, String SchemaName, String TableName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded);
    }

    public final boolean Columns(String CatalogName, String SchemaName, String TableName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, null);
    }

    public final boolean Columns(String CatalogName, String SchemaName, String TableName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded);
    }

    public final boolean ProcedureColumns(String CatalogName, String SchemaName, String ProcName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, null);
    }

    public final boolean ProcedureColumns(String CatalogName, String SchemaName, String ProcName, String ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, discarded);
    }

    public final boolean PrimaryKeys(String CatalogName, String SchemaName, String TableName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, null);
    }

    public final boolean PrimaryKeys(String CatalogName, String SchemaName, String TableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, discarded);
    }

    public final boolean TablePrivileges(String CatalogName, String SchemaName, String TableName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, null);
    }

    public final boolean TablePrivileges(String CatalogName, String SchemaName, String TableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, discarded);
    }

    public final boolean Procedures(String CatalogName, String SchemaName, String ProcName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, null);
    }

    public final boolean Procedures(String CatalogName, String SchemaName, String ProcName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, discarded);
    }

    public final boolean Tables(String CatalogName, String SchemaName, String TableName, String TableType, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return DoMeta(idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, null);
    }

    public final boolean Tables(String CatalogName, String SchemaName, String TableName, String TableType, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        return DoMeta(idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, discarded);
    }

    public final boolean Statistics(String CatalogName, String SchemaName, String TableName, short unique, short reserved, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return Statistics(CatalogName, SchemaName, TableName, unique, reserved, handler, row, rh, null);
    }

    public final boolean Statistics(String CatalogName, String SchemaName, String TableName, short unique, short reserved, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        MyCallback<DExecuteResult> cb = new MyCallback<>(idSQLStatistics, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(CatalogName);
        sb.Save(SchemaName);
        sb.Save(TableName);
        sb.Save(unique);
        sb.Save(reserved);
        final long index = GetCallIndex();
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_mapRowset.put(index, new Pair<>(rh, row));
                m_deqExecuteResult.add(cb);
            }
            sb.Save(index);
            if (!SendRequest(idSQLStatistics, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    m_mapRowset.remove(index);
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    public final boolean SpecialColumns(short identifierType, String CatalogName, String SchemaName, String TableName, short scope, short nullable, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return SpecialColumns(identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh, null);
    }

    public final boolean SpecialColumns(short identifierType, String CatalogName, String SchemaName, String TableName, short scope, short nullable, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        MyCallback<DExecuteResult> cb = new MyCallback<>(idSQLSpecialColumns, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(identifierType);
        sb.Save(CatalogName);
        sb.Save(SchemaName);
        sb.Save(TableName);
        sb.Save(scope);
        sb.Save(nullable);
        final long index = GetCallIndex();
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_mapRowset.put(index, new Pair<>(rh, row));
                m_deqExecuteResult.add(cb);
            }
            sb.Save(index);
            if (!SendRequest(idSQLSpecialColumns, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    m_mapRowset.remove(index);
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    public final boolean ForeignKeys(String PKCatalogName, String PKSchemaName, String PKTableName, String FKCatalogName, String FKSchemaName, String FKTableName, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return ForeignKeys(PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh, null);
    }

    public final boolean ForeignKeys(String PKCatalogName, String PKSchemaName, String PKTableName, String FKCatalogName, String FKSchemaName, String FKTableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        MyCallback<DExecuteResult> cb = new MyCallback<>(idSQLForeignKeys, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(PKCatalogName);
        sb.Save(PKSchemaName);
        sb.Save(PKTableName);
        sb.Save(FKCatalogName);
        sb.Save(FKSchemaName);
        sb.Save(FKTableName);
        final long index = GetCallIndex();
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_mapRowset.put(index, new Pair<>(rh, row));
                m_deqExecuteResult.add(cb);
            }
            sb.Save(index);
            if (!SendRequest(idSQLForeignKeys, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    m_mapRowset.remove(index);
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    private boolean DoMeta(short id, String s0, String s1, String s2, String s3, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        MyCallback<DExecuteResult> cb = new MyCallback<>(id, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(s0);
        sb.Save(s1);
        sb.Save(s2);
        sb.Save(s3);
        final long index = GetCallIndex();
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_mapRowset.put(index, new Pair<>(rh, row));
                m_deqExecuteResult.add(cb);
            }
            sb.Save(index);
            if (!SendRequest(id, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    m_mapRowset.remove(index);
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    private boolean DoMeta(short id, String s0, String s1, String s2, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
        MyCallback<DExecuteResult> cb = new MyCallback<>(id, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(s0);
        sb.Save(s1);
        sb.Save(s2);
        final long index = GetCallIndex();
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_mapRowset.put(index, new Pair<>(rh, row));
                m_deqExecuteResult.add(cb);
            }
            sb.Save(index);
            if (!SendRequest(id, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    m_mapRowset.remove(index);
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    public Object GetInfo(short infoType) {
        synchronized (m_csDB) {
            if (m_mapInfo.containsKey(infoType)) {
                return m_mapInfo.get(infoType);
            }
            return null;
        }
    }

    @Override
    protected void OnResultReturned(short reqId, CUQueue mc) {
        if (reqId >= idSQLColumnPrivileges && reqId <= idSQLTables) {
            long fail_ok = mc.LoadLong();
            int res = mc.LoadInt();
            String errMsg = mc.LoadString();
            MyCallback<DExecuteResult> t = GetExecuteResultHandler(reqId);
            synchronized (m_csDB) {
                m_lastReqId = reqId;
                m_affected = 0;
                m_dbErrCode = res;
                m_dbErrMsg = errMsg;
                if (m_mapRowset.containsKey(m_indexRowset)) {
                    m_mapRowset.remove(m_indexRowset);
                }
            }
            if (t != null && t.Callback != null) {
                t.Callback.invoke(this, res, errMsg, 0, fail_ok, null);
            }
        } else if (reqId == idSQLGetInfo) {
            synchronized (m_csDB) {
                m_mapInfo.clear();
                while (mc.GetSize() > 0) {
                    short infoType = mc.LoadShort();
                    Object infoValue = mc.LoadObject();
                    m_mapInfo.put(infoType, infoValue);
                }
            }
        } else {
            super.OnResultReturned(reqId, mc);
        }
    }
}
