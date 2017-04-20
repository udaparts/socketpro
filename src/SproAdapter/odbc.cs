
using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class COdbc : CAsyncDBHandler
        {
            public const uint sidOdbc = SocketProAdapter.BaseServiceID.sidODBC; //asynchronous ODBC service id
            public COdbc()
                : base(sidOdbc)
            {
            }

            //meta recordsets
            public const ushort idSQLColumnPrivileges = 0x7f00 + 100;
            public const ushort idSQLColumns = 0x7f00 + 101;
            public const ushort idSQLForeignKeys = 0x7f00 + 102;
            public const ushort idSQLPrimaryKeys = 0x7f00 + 103;
            public const ushort idSQLProcedureColumns = 0x7f00 + 104;
            public const ushort idSQLProcedures = 0x7f00 + 105;
            public const ushort idSQLSpecialColumns = 0x7f00 + 106;
            public const ushort idSQLStatistics = 0x7f00 + 107;
            public const ushort idSQLTablePrivileges = 0x7f00 + 108;
            public const ushort idSQLTables = 0x7f00 + 109;
            public const ushort idSQLGetInfo = 0x7f00 + 110;

            public const int ER_SUCCESS = 0;
            public const int ER_ERROR = -1;

            //error codes from async ODBC server library
            public const int ER_NO_DB_OPENED_YET = -1981;
            public const int ER_BAD_END_TRANSTACTION_PLAN = -1982;
            public const int ER_NO_PARAMETER_SPECIFIED = -1983;
            public const int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
            public const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
            public const int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
            public const int ER_NO_DB_NAME_SPECIFIED = -1987;
            public const int ER_ODBC_ENVIRONMENT_NOT_INITIALIZED = -1988;
            public const int ER_BAD_MANUAL_TRANSACTION_STATE = -1989;
            public const int ER_BAD_INPUT_PARAMETER_DATA_TYPE = -1990;
            public const int ER_BAD_PARAMETER_DIRECTION_TYPE = -1991;
            public const int ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = -1992;

            private Dictionary<ushort, object> m_mapInfo = new Dictionary<ushort, object>();

            public object GetInfo(ushort infoType)
            {
                lock (m_csDB)
                {
                    if (m_mapInfo.ContainsKey(infoType))
                        return m_mapInfo[infoType];
                    return null;
                }
            }

            public virtual bool ColumnPrivileges(string CatalogName, string SchemaName, string TableName, string ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh);
            }

            public virtual bool Columns(string CatalogName, string SchemaName, string TableName, string ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh);
            }

            public virtual bool ProcedureColumns(string CatalogName, string SchemaName, string ProcName, string ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh);
            }

            public virtual bool PrimaryKeys(string CatalogName, string SchemaName, string TableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh);
            }

            public virtual bool TablePrivileges(string CatalogName, string SchemaName, string TableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh);
            }

            public virtual bool Procedures(string CatalogName, string SchemaName, string ProcName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh);
            }

            public virtual bool SpecialColumns(short identifierType, string CatalogName, string SchemaName, string TableName, short scope, short nullable, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh);
            }

            public virtual bool Statistics(string CatalogName, string SchemaName, string TableName, ushort unique, ushort reserved, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLStatistics, CatalogName, SchemaName, TableName, unique, reserved, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLStatistics;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, -1);
                }))
                {
                    lock (m_csDB)
                    {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }

            public virtual bool Tables(string CatalogName, string SchemaName, string TableName, string TableType, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh);
            }

            public virtual bool ForeignKeys(string PKCatalogName, string PKSchemaName, string PKTableName, string FKCatalogName, string FKSchemaName, string FKTableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                return DoMeta(idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh);
            }

            private bool DoMeta(ushort id, string s0, string s1, string s2, string s3, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(id, s0, s1, s2, s3, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = id;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, -1);
                }))
                {
                    lock (m_csDB)
                    {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }

            private bool DoMeta(ushort id, string s0, string s1, string s2, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(id, s0, s1, s2, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = id;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, -1);
                }))
                {
                    lock (m_csDB)
                    {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }

            private bool DoMeta<T0, T1, T2>(ushort id, T0 t0, string s0, string s1, string s2, T1 t1, T2 t2, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(id, t0, s0, s1, s2, t1, t2, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = id;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, -1);
                }))
                {
                    lock (m_csDB)
                    {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }

            protected override void OnResultReturned(ushort reqId, CUQueue mc)
            {
                switch (reqId)
                {
                    case idSQLGetInfo:
                        lock (m_csDB)
                        {
                            m_mapInfo.Clear();
                            while (mc.GetSize() > 0)
                            {
                                ushort infoType;
                                object infoValue;
                                mc.Load(out infoType).Load(out infoValue);
                                m_mapInfo[infoType] = infoValue;
                            }
                        }
                        break;
                    default:
                        base.OnResultReturned(reqId, mc);
                        break;
                }
            }
        }
    }
}
