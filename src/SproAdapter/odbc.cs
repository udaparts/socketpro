
using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class COdbc : CAsyncDBHandler
        {
            public const uint sidOdbc = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2; //asynchronous ODBC service id
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
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLColumnPrivileges;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool Columns(string CatalogName, string SchemaName, string TableName, string ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLColumns;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool ProcedureColumns(string CatalogName, string SchemaName, string ProcName, string ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLProcedureColumns;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool PrimaryKeys(string CatalogName, string SchemaName, string TableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLPrimaryKeys, CatalogName, SchemaName, TableName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLPrimaryKeys;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool TablePrivileges(string CatalogName, string SchemaName, string TableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLTablePrivileges, CatalogName, SchemaName, TableName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLTablePrivileges;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool Procedures(string CatalogName, string SchemaName, string ProcName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLProcedures, CatalogName, SchemaName, ProcName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLProcedures;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool SpecialColumns(short identifierType, string CatalogName, string SchemaName, string TableName, short scope, short nullable, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLSpecialColumns;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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
                        handler(this, res, errMsg, 0, fail_ok, null);
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
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLTables, CatalogName, SchemaName, TableName, TableType, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLTables;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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

            public virtual bool ForeignKeys(string PKCatalogName, string PKSchemaName, string PKTableName, string FKCatalogName, string FKSchemaName, string FKTableName, DExecuteResult handler, DRows row, DRowsetHeader rh)
            {
                ulong index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB)
                {
                    index = ++m_nCall;
                    m_mapRowset[m_nCall] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, index, (ar) =>
                {
                    ulong fail_ok;
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out fail_ok);
                    lock (m_csDB)
                    {
                        m_lastReqId = idSQLForeignKeys;
                        m_affected = 0;
                        m_dbErrCode = res;
                        m_dbErrMsg = errMsg;
                        m_mapRowset.Remove(m_indexRowset);
                        if (m_mapRowset.Count == 0)
                            m_nCall = 0;
                    }
                    if (handler != null)
                        handler(this, res, errMsg, 0, fail_ok, null);
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
                        m_mapInfo.Clear();
                        while (mc.GetSize() > 0)
                        {
                            ushort infoType;
                            object infoValue;
                            mc.Load(out infoType).Load(out infoValue);
                            lock (m_csDB)
                            {
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
