using System;
using System.Collections.Generic;
#if USQLSERVER
#else
using SocketProAdapter.ClientSide;
#endif
using System.Data;

namespace SocketProAdapter {
    namespace UDB {
        public enum tagTransactionIsolation {
            tiUnspecified = -1,
            tiChaos = 0,
            tiReadUncommited = 1,
            tiBrowse = 2,
            tiCursorStability = 3,
            tiReadCommited = tiCursorStability,
            tiRepeatableRead = 4,
            tiSerializable = 5,
            tiIsolated = 6
        };

        public enum tagRollbackPlan {
            /// <summary>
            /// Manual transaction will rollback whenever there is an error by default
            /// </summary>
            rpDefault = 0,

            /// <summary>
            /// Manual transaction will rollback whenever there is an error by default
            /// </summary>
            rpRollbackErrorAny = rpDefault,

            /// <summary>
            /// Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
            /// </summary>
            rpRollbackErrorLess = 1,

            /// <summary>
            /// Manual transaction will rollback as long as the number of errors is less or equal than the number of ok processing statements
            /// </summary>
            rpRollbackErrorEqual = 2,

            /// <summary>
            /// Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
            /// </summary>
            rpRollbackErrorMore = 3,

            /// <summary>
            /// Manual transaction will rollback only if all the processing statements are failed
            /// </summary>
            rpRollbackErrorAll = 4,

            /// <summary>
            /// Manual transaction will rollback always no matter what happens.
            /// </summary>
            rpRollbackAlways = 5
        };

        public enum tagUpdateEvent {
            ueUnknown = -1,

            /// <summary>
            /// An event for inserting a record into a table
            /// </summary>
            ueInsert = 0,

            /// <summary>
            /// An event for updating a record of a table
            /// </summary>
            ueUpdate = 1,

            /// <summary>
            /// An event for deleting a record from a table
            /// </summary>
            ueDelete = 2,
        };

        public enum tagManagementSystem {
            msUnknown = -1,
            msSqlite = 0,
            msMysql = 1,
            msODBC = 2,
            msMsSQL = 3,
            msOracle = 4,
            msDB2 = 5,
            msPostgreSQL = 6,
            msMongoDB = 7
        };

        public class CDBVariantArray : List<object>, IUSerializer {
            #region IUSerializer Members

            public void LoadFrom(CUQueue UQueue) {
                int size;
                Clear();
                UQueue.Load(out size);
                while (size > 0) {
                    object obj;
                    UQueue.Load(out obj);
                    Add(obj);
                    --size;
                }
            }

            public void SaveTo(CUQueue UQueue) {
                int size = Count;
                UQueue.Save(size);
                foreach (object data in this) {
                    UQueue.Save(data);
                }
            }

            #endregion
        }

        public class CDBColumnInfo : IUSerializer {
            public const uint FLAG_NOT_NULL = 0x1;
            public const uint FLAG_UNIQUE = 0x2;
            public const uint FLAG_PRIMARY_KEY = 0x4;
            public const uint FLAG_AUTOINCREMENT = 0x8;
            public const uint FLAG_NOT_WRITABLE = 0x10;
            public const uint FLAG_ROWID = 0x20;
            public const uint FLAG_XML = 0x40;
            public const uint FLAG_JSON = 0x80;
            public const uint FLAG_CASE_SENSITIVE = 0x100;
            public const uint FLAG_IS_ENUM = 0x200;
            public const uint FLAG_IS_SET = 0x400;
            public const uint FLAG_IS_UNSIGNED = 0x800;
            public const uint FLAG_IS_BIT = 0x1000;

            public string DBPath = "";
            public string TablePath = "";
            public string DisplayName = "";
            public string OriginalName = "";
            public string DeclaredType = "";
            public string Collation = "";
            public uint ColumnSize = 0;
            public uint Flags = 0;
            public tagVariantDataType DataType = tagVariantDataType.sdVT_NULL;
            public byte Precision = 0;
            public byte Scale = 0;

            #region IUSerializer Members

            public void LoadFrom(CUQueue UQueue) {
                UQueue.Load(out DBPath).Load(out TablePath).Load(out DisplayName).Load(out OriginalName).Load(out DeclaredType).Load(out Collation).Load(out ColumnSize).Load(out Flags);
                ushort dt;
                UQueue.Load(out dt);
                DataType = (tagVariantDataType)(dt);
                UQueue.Load(out Precision).Load(out Scale);
            }

            public void SaveTo(CUQueue UQueue) {
                UQueue.Save(DBPath).Save(TablePath).Save(DisplayName).Save(OriginalName).Save(DeclaredType).Save(Collation).Save(ColumnSize).Save(Flags);
                UQueue.Save((ushort)DataType);
                UQueue.Save(Precision).Save(Scale);
            }

            #endregion
        }

        public class CDBColumnInfoArray : List<CDBColumnInfo>, IUSerializer {
            #region IUSerializer Members

            public void LoadFrom(CUQueue UQueue) {
                int size;
                Clear();
                UQueue.Load(out size);
                while (size > 0) {
                    CDBColumnInfo info = new CDBColumnInfo();
                    info.LoadFrom(UQueue);
                    Add(info);
                    --size;
                }
            }

            public void SaveTo(CUQueue UQueue) {
                int size = Count;
                UQueue.Save(size);
                foreach (CDBColumnInfo info in this) {
                    info.SaveTo(UQueue);
                }
            }

            #endregion
        }

        public enum tagParameterDirection {
            pdUnknown = 0,
            pdInput = 1,
            pdOutput = 2,
            pdInputOutput = 3,
            pdReturnValue = 4
        };

        public class CParameterInfo : IUSerializer {
            public tagParameterDirection Direction = tagParameterDirection.pdInput; //required
            public tagVariantDataType DataType = tagVariantDataType.sdVT_NULL; //required! for example, VT_I4, VT_BSTR, VT_I1|VT_ARRAY (UTF8 string), ....
            public uint ColumnSize; //-1 BLOB, string len or binary bytes; ignored for other data types
            public byte Precision; //datetime, decimal or numeric only
            public byte Scale; //datetime, decimal or numeric only
            public string ParameterName = ""; //may be optional, which depends on remote database system

            #region IUSerializer Members

            public void LoadFrom(CUQueue UQueue) {
                int data;
                UQueue.Load(out data);
                Direction = (tagParameterDirection)data;
                ushort dt;
                UQueue.Load(out dt);
                if ((dt & (ushort)tagVariantDataType.sdVT_ARRAY) == (ushort)tagVariantDataType.sdVT_ARRAY) {
                    if ((dt & (ushort)tagVariantDataType.sdVT_UI1) == (ushort)tagVariantDataType.sdVT_UI1) {
                        DataType = tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY;
                    } else {
                        //will convert all ASCII/UTF8 string into Unicode string at run time
                        DataType = tagVariantDataType.sdVT_BSTR;
                    }
                } else {
                    DataType = (tagVariantDataType)(dt);
                }
                UQueue.Load(out ColumnSize).Load(out Precision).Load(out Scale).Load(out ParameterName);
            }

            public void SaveTo(CUQueue UQueue) {
                UQueue.Save((int)Direction);
                if (DataType == tagVariantDataType.sdVT_BYTES) {
                    UQueue.Save((ushort)tagVariantDataType.sdVT_ARRAY | (ushort)tagVariantDataType.sdVT_UI1);
                } else {
                    UQueue.Save((ushort)DataType);
                }
                UQueue.Save(ColumnSize).Save(Precision).Save(Scale).Save(ParameterName);
            }

            #endregion
        }

        public class CParameterInfoArray : List<CParameterInfo>, IUSerializer {
            #region IUSerializer Members

            public void LoadFrom(CUQueue UQueue) {
                int count;
                Clear();
                UQueue.Load(out count);
                while (count > 0) {
                    CParameterInfo info = new CParameterInfo();
                    info.LoadFrom(UQueue);
                    Add(info);
                    --count;
                }
            }

            public void SaveTo(CUQueue UQueue) {
                UQueue.Save(this.Count);
                foreach (CParameterInfo info in this) {
                    info.SaveTo(UQueue);
                }
            }

            #endregion
        }

        public static class DB_CONSTS {
            /// <summary>
            /// Async database client/server just requires the following request identification numbers 
            /// </summary>
            public const ushort idOpen = 0x7E7F;
            public const ushort idClose = idOpen + 1;
            public const ushort idBeginTrans = idClose + 1;
            public const ushort idEndTrans = idBeginTrans + 1;
            public const ushort idExecute = idEndTrans + 1;
            public const ushort idPrepare = idExecute + 1;
            public const ushort idExecuteParameters = idPrepare + 1;

            /// <summary>
            /// the request identification numbers used for message push from server to client
            /// </summary>
            public const ushort idDBUpdate = idExecuteParameters + 1; //server ==> client only
            public const ushort idRowsetHeader = idDBUpdate + 1; //server ==> client only
            public const ushort idOutputParameter = idRowsetHeader + 1; //server ==> client only

            /// <summary>
            /// Internal request/response identification numbers used for data communication between client and server
            /// </summary>
            public const ushort idBeginRows = idOutputParameter + 1;
            public const ushort idTransferring = idBeginRows + 1;
            public const ushort idStartBLOB = idTransferring + 1;
            public const ushort idChunk = idStartBLOB + 1;
            public const ushort idEndBLOB = idChunk + 1;
            public const ushort idEndRows = idEndBLOB + 1;
            public const ushort idCallReturn = idEndRows + 1;

            public const ushort idGetCachedTables = idCallReturn + 1;

            public const ushort idSqlBatchHeader = idGetCachedTables + 1;
            public const ushort idExecuteBatch = idSqlBatchHeader + 1;
            public const ushort idParameterPosition = idExecuteBatch + 1;

            /// <summary>
            /// Whenever a data size in bytes is about twice larger than the defined value,
            /// the data will be treated in large object and transferred in chunks for reducing memory foot print
            /// </summary>
            public const uint DEFAULT_BIG_FIELD_CHUNK_SIZE = 16 * 1024; //16k

            /// <summary>
            /// A record data size in bytes is approximately equal to or slightly larger than the defined constant
            /// </summary>
            public const uint DEFAULT_RECORD_BATCH_SIZE = 16 * 1024; //16k

            /// <summary>
            /// A flag used with idOpen for tracing database table update events
            /// </summary>
            public const uint ENABLE_TABLE_UPDATE_MESSAGES = 0x1;

            /// <summary>
            /// A chat group id used at SocketPro server side for notifying database events from server to connected clients
            /// </summary>
            public const uint STREAMING_SQL_CHAT_GROUP_ID = 0x1fffffff;

            public const uint CACHE_UPDATE_CHAT_GROUP_ID = STREAMING_SQL_CHAT_GROUP_ID + 1;
        }
    }

#if USQLSERVER

#else
    namespace ClientSide {
        using UDB;
        public class CAsyncDBHandler : CAsyncServiceHandler {
            private const uint ONE_MEGA_BYTES = 0x100000;
            private const uint BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0;

            public delegate void DResult(CAsyncDBHandler dbHandler, int res, string errMsg);
            public delegate void DExecuteResult(CAsyncDBHandler dbHandler, int res, string errMsg, long affected, ulong fail_ok, object vtId);
            public delegate void DRowsetHeader(CAsyncDBHandler dbHandler);
            public delegate void DRows(CAsyncDBHandler dbHandler, CDBVariantArray lstData);

            protected CAsyncDBHandler(uint ServiceId)
                : base(ServiceId) {

            }

            protected object m_csDB = new object();
            private object m_csOneSending = new object();
            protected CDBColumnInfoArray m_vColInfo = new CDBColumnInfoArray();

            private string m_strConnection;
            protected long m_affected = -1;
            protected int m_dbErrCode = 0;
            protected string m_dbErrMsg = "";
            protected ushort m_lastReqId = 0;
            protected Dictionary<ulong, KeyValuePair<DRowsetHeader, DRows>> m_mapRowset = new Dictionary<ulong, KeyValuePair<DRowsetHeader, DRows>>();
            private Dictionary<ulong, CDBVariantArray> m_mapParameterCall = new Dictionary<ulong, CDBVariantArray>();
            private Dictionary<ulong, DRowsetHeader> m_mapHandler = new Dictionary<ulong, DRowsetHeader>();
            private ulong m_indexRowset = 0;
            private CUQueue m_Blob = new CUQueue();
            private CDBVariantArray m_vData = new CDBVariantArray();
            private tagManagementSystem m_ms = tagManagementSystem.msUnknown;
            private uint m_flags = 0;
            private uint m_parameters = 0;
            private uint m_indexProc = 0;
            private uint m_output = 0;
            private bool m_bCallReturn = false;
            private bool m_queueOk = false;
            private uint m_nParamPos = 0;
            public ushort LastDBRequestId {
                get {
                    lock (m_csDB) {
                        return m_lastReqId;
                    }
                }
            }

            public int LastDBErrorCode {
                get {
                    lock (m_csDB) {
                        return m_dbErrCode;
                    }
                }
            }

            public uint Outputs {
                get {
                    lock (m_csDB) {
                        return m_output;
                    }
                }
            }

            public tagManagementSystem DBManagementSystem {
                get {
                    lock (m_csDB) {
                        return m_ms;
                    }
                }
            }

            public bool Opened {
                get {
                    lock (m_csDB) {
                        return (m_strConnection != null && m_strConnection.Length > 0 && m_lastReqId > 0);
                    }
                }
            }

            public CDBColumnInfoArray ColumnInfo {
                get {
                    lock (m_csDB) {
                        return m_vColInfo;
                    }
                }
            }

            public long LastAffected {
                get {
                    lock (m_csDB) {
                        return m_affected;
                    }
                }
            }

            public string LastDBErrorMessage {
                get {
                    lock (m_csDB) {
                        return m_dbErrMsg;
                    }
                }
            }

            public string Connection {
                get {
                    lock (m_csDB) {
                        return m_strConnection;
                    }
                }
            }

            public override uint CleanCallbacks() {
                lock (m_csDB) {
                    Clean();
                }
                return base.CleanCallbacks();
            }

            public uint Parameters {
                get {
                    lock (m_csDB) {
                        return m_parameters;
                    }
                }
            }

            public bool CallReturn {
                get {
                    lock (m_csDB) {
                        return m_bCallReturn;
                    }
                }
            }

            private void Clean() {
                m_strConnection = "";
                m_mapRowset.Clear();
                m_mapParameterCall.Clear();
                m_mapHandler.Clear();
                m_vColInfo.Clear();
                m_lastReqId = 0;
                m_Blob.SetSize(0);
                if (m_Blob.MaxBufferSize > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                    m_Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
                m_vData.Clear();
            }

            private bool Send(CScopeUQueue sb, ref bool firstRow) {
                CUQueue q = sb.UQueue;
                if (q.GetSize() > 0) {
                    if (firstRow) {
                        firstRow = false;
                        if (!SendRequest(DB_CONSTS.idBeginRows, q.IntenalBuffer, q.GetSize(), null)) {
                            return false;
                        }
                    } else {
                        if (!SendRequest(DB_CONSTS.idTransferring, q.IntenalBuffer, q.GetSize(), null)) {
                            return false;
                        }
                    }
                    q.SetSize(0);
                } else if (firstRow) {
                    firstRow = false;
                    if (!SendRequest(DB_CONSTS.idBeginRows, null)) {
                        return false;
                    }
                }
                return true;
            }

            private bool SendBlob(CScopeUQueue sb) {
                CUQueue q = sb.UQueue;
                if (q.GetSize() > 0) {
                    byte[] bytes = null;
                    bool start = true;
                    while (q.GetSize() > 0) {
                        uint send = (q.GetSize() >= DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) ? DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE : q.GetSize();
                        q.Pop(send, ref bytes);
                        if (start) {
                            if (!SendRequest(DB_CONSTS.idStartBLOB, bytes, send, null)) {
                                return false;
                            }
                            start = false;
                        } else {
                            if (!SendRequest(DB_CONSTS.idChunk, bytes, send, null)) {
                                return false;
                            }
                        }
                        if (q.GetSize() < DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                            break;
                        }
                    }
                    if (!SendRequest(DB_CONSTS.idEndBLOB, q.GetBuffer(), q.GetSize(), null)) {
                        return false;
                    }
                    q.SetSize(0);
                }
                return true;
            }

            private bool SendParametersData(CDBVariantArray vParam) {
                if (vParam == null)
                    return true;
                int size = vParam.Count;
                if (size == 0)
                    return true;
                bool firstRow = true;
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    for (int n = 0; n < size; ++n) {
                        object vt = vParam[n];
                        if (vt is string/*UNICODE string*/) {
                            string s = (string)vt;
                            uint len = (uint)(s.Length);
                            if (len < DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                                sb.Save(vt);
                            else {
                                if (!Send(sb, ref firstRow))
                                    return false;
                                uint bytes = len;
                                bytes *= sizeof(char);
                                bytes += 6; //sizeof(ushort) + sizeof(uint)
                                sb.Save(bytes).Save(vt);
                                if (!SendBlob(sb)) {
                                    return false;
                                }
                            }
                        } else if (vt is byte[]) {
                            byte[] bytes = (byte[])vt;
                            uint len = (uint)bytes.Length;
                            if (len < 2 * DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                                sb.Save(vt);
                            else {
                                if (!Send(sb, ref firstRow))
                                    return false;
                                len += 6; //sizeof(ushort) + sizeof(uint)
                                sb.Save(len).Save(vt);
                                if (!SendBlob(sb)) {
                                    return false;
                                }
                            }
                        } else {
                            sb.Save(vt);
                        }
                        if (sb.UQueue.GetSize() >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE) {
                            if (!Send(sb, ref firstRow))
                                return false;
                        }
                    }
                    if (!Send(sb, ref firstRow))
                        return false;
                }
                return true;
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously, and don't expect any data returned
            /// </summary>
            /// <param name="vParam"></param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam) {
                return Execute(vParam, null, null, null, false, false, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously, and don't expect any rowsets or outputs returned 
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam, DExecuteResult handler) {
                return Execute(vParam, handler, null, null, true, true, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously, and don't expect any rowsets returned
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for tracking output parameter returned data</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row) {
                return Execute(vParam, handler, row, null, true, true, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for tracking record or output parameter returned data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh) {
                return Execute(vParam, handler, row, rh, true, true, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for tracking record or output parameter returned data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta) {
                return Execute(vParam, handler, row, rh, meta, true, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for tracking record or output parameter returned data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <param name="lastInsertId">a boolean value for last insert record identification number. It defaults to true</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId) {
                return Execute(vParam, handler, row, rh, meta, lastInsertId, null);
            }

            /// <summary>
            /// Process one or more sets of prepared statements with an array of parameter data asynchronously
            /// </summary>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for tracking record or output parameter returned data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <param name="lastInsertId">a boolean value for last insert record identification number. It defaults to true</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId, DDiscarded discarded) {
                bool rowset = (rh != null || row != null) ? true : false;
                if (!rowset)
                    meta = false;
                bool queueOk = false;
                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
                lock (m_csOneSending) {
                    if (vParam != null && vParam.Count > 0) {
                        queueOk = AttachedClientSocket.ClientQueue.StartJob();
                        if (!SendParametersData(vParam)) {
                            lock (m_csDB) {
                                Clean();
                            }
                            return false;
                        }
                    }
                    ulong callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    lock (m_csDB) {
                        if (rowset) {
                            m_mapRowset[callIndex] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                        }
                        m_mapParameterCall[callIndex] = vParam;
                    }
                    if (!SendRequest(DB_CONSTS.idExecuteParameters, rowset, meta, lastInsertId, callIndex, (ar) => {
                        Process(handler, ar, DB_CONSTS.idExecuteParameters, callIndex);
                    }, discarded, null)) {
                        lock (m_csDB) {
                            m_mapParameterCall.Remove(callIndex);
                            if (rowset) {
                                m_mapRowset.Remove(callIndex);
                            }
                        }
                        return false;
                    }
                    if (queueOk)
                        AttachedClientSocket.ClientQueue.EndJob();
                    return true;
                }
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql) {
                return ExecuteBatch(isolation, sql, new CDBVariantArray(), null, null, null, null, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam) {
                return ExecuteBatch(isolation, sql, vParam, null, null, null, null, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler) {
                return ExecuteBatch(isolation, sql, vParam, handler, null, null, null, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, null, null, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, null, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, null, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, vPInfo, tagRollbackPlan.rpDefault, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <param name="plan">a value for computing how included transactions should be rollback</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo, tagRollbackPlan plan) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, vPInfo, plan, null, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <param name="plan">a value for computing how included transactions should be rollback</param>
            /// <param name="discarded">a callback for tracking socket closed or request canceled event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo, tagRollbackPlan plan, DDiscarded discarded) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, vPInfo, plan, discarded, ";", true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <param name="plan">a value for computing how included transactions should be rollback</param>
            /// <param name="discarded">a callback for tracking socket closed or request canceled event</param>
            /// <param name="delimiter">a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo, tagRollbackPlan plan, DDiscarded discarded, string delimiter) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, vPInfo, plan, discarded, delimiter, true, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <param name="plan">a value for computing how included transactions should be rollback</param>
            /// <param name="discarded">a callback for tracking socket closed or request canceled event</param>
            /// <param name="delimiter">a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing</param>
            /// <param name="meta">a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo, tagRollbackPlan plan, DDiscarded discarded, string delimiter, bool meta) {
                return ExecuteBatch(isolation, sql, vParam, handler, row, rh, batchHeader, vPInfo, plan, discarded, delimiter, meta, true);
            }

            /// <summary>
            /// Execute a batch of SQL statements on one single call
            /// </summary>
            /// <param name="isolation">a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified</param>
            /// <param name="sql">a SQL statement having a batch of individual SQL statements</param>
            /// <param name="vParam">an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="batchHeader">a callback for tracking returning batch start error messages</param>
            /// <param name="vPInfo">a given array of parameter informations which may be empty to some of database management systems</param>
            /// <param name="plan">a value for computing how included transactions should be rollback</param>
            /// <param name="discarded">a callback for tracking socket closed or request canceled event</param>
            /// <param name="delimiter">a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing</param>
            /// <param name="meta">a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on</param>
            /// <param name="lastInsertId">a boolean for last insert record identification number</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool ExecuteBatch(tagTransactionIsolation isolation, string sql, CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, DRowsetHeader batchHeader, CParameterInfo[] vPInfo, tagRollbackPlan plan, DDiscarded discarded, string delimiter, bool meta, bool lastInsertId) {
                if (vPInfo == null)
                    vPInfo = new CParameterInfo[0];
                bool rowset = (rh != null || row != null) ? true : false;
                if (!rowset)
                    meta = false;
                using (CScopeUQueue sub = new CScopeUQueue()) {
                    bool queueOk = false;
                    CUQueue sb = sub.UQueue;
                    sb.Save(sql).Save(delimiter).Save((int)isolation).Save((int)plan).Save(rowset).Save(meta).Save(lastInsertId);
                    //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                    //to avoid possible request sending overlapping within multiple threading environment
                    lock (m_csOneSending) {
                        if (vParam != null && vParam.Count > 0) {
                            queueOk = AttachedClientSocket.ClientQueue.StartJob();
                            if (!SendParametersData(vParam)) {
                                lock (m_csDB) {
                                    Clean();
                                }
                                return false;
                            }
                        }
                        ulong callIndex = GetCallIndex();
                        //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                        //in case a client asynchronously sends lots of requests without use of client side queue.
                        lock (m_csDB) {
                            if (rowset) {
                                m_mapRowset[callIndex] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                            }
                            m_mapParameterCall[callIndex] = vParam;
                            m_mapHandler[callIndex] = batchHeader;
                            sb.Save(m_strConnection).Save(m_flags);
                        }
                        sb.Save(callIndex);
                        sb.Save(vPInfo.Length);
                        foreach (CParameterInfo info in vPInfo) {
                            info.SaveTo(sb);
                        }
                        if (!SendRequest(DB_CONSTS.idExecuteBatch, sb.IntenalBuffer, sb.GetSize(), (ar) => {
                            Process(handler, ar, DB_CONSTS.idExecuteBatch, callIndex);
                        }, discarded, null)) {
                            lock (m_csDB) {
                                m_mapHandler.Remove(callIndex);
                                m_mapParameterCall.Remove(callIndex);
                                if (rowset) {
                                    m_mapRowset.Remove(callIndex);
                                }
                            }
                            return false;
                        }
                        if (queueOk)
                            AttachedClientSocket.ClientQueue.EndJob();
                        return true;
                    }
                }

            }


            /// <summary>
            /// Asynchronously process a complex SQL statement which may be combined with multiple basic SQL statements, and don't expect any data returned
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql) {
                return Execute(sql, null, null, null, true, true, null);
            }

            /// <summary>
            /// Asynchronously process a complex SQL statement which may be combined with multiple basic SQL statements, and don't expect any records returned 
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql, DExecuteResult handler) {
                return Execute(sql, handler, null, null, true, true, null);
            }

            /// <summary>
            /// Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql, DExecuteResult handler, DRows row) {
                return Execute(sql, handler, row, null, true, true, null);
            }

            /// <summary>
            /// Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql, DExecuteResult handler, DRows row, DRowsetHeader rh) {
                return Execute(sql, handler, row, rh, true, true, null);
            }

            /// <summary>
            /// Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta) {
                return Execute(sql, handler, row, rh, meta, true, null);
            }

            /// <summary>
            /// Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <param name="lastInsertId">a boolean value for last insert record identification number. It defaults to true</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Execute(string sql, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId) {
                return Execute(sql, handler, row, rh, meta, lastInsertId, null);
            }

            /// <summary>
            /// Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously
            /// </summary>
            /// <param name="sql">a complex SQL statement which may be combined with multiple basic SQL statements</param>
            /// <param name="handler">a callback for tracking final result</param>
            /// <param name="row">a callback for receiving records of data</param>
            /// <param name="rh">a callback for tracking row set of header column informations</param>
            /// <param name="meta">a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true</param>
            /// <param name="lastInsertId">a boolean value for last insert record identification number. It defaults to true</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool Execute(string sql, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId, DDiscarded discarded) {
                bool rowset = (rh != null || row != null) ? true : false;
                if (!rowset)
                    meta = false;
                ulong index = GetCallIndex();
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                //in case a client asynchronously sends lots of requests without use of client side queue.
                if (rowset) {
                    lock (m_csDB) {
                        m_mapRowset[index] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                    }
                }
                if (!SendRequest(DB_CONSTS.idExecute, sql, rowset, meta, lastInsertId, index, (ar) => {
                    Process(handler, ar, DB_CONSTS.idExecute, index);
                }, discarded, null)) {
                    lock (m_csDB) {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }

            private void Process(DExecuteResult handler, CAsyncResult ar, ushort reqId, ulong index) {
                long affected;
                ulong fail_ok;
                int res;
                string errMsg;
                object vtId;
                ar.Load(out affected).Load(out res).Load(out errMsg).Load(out vtId).Load(out fail_ok);
                CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                lock (adb.m_csDB) {
                    adb.m_lastReqId = reqId;
                    adb.m_affected = affected;
                    adb.m_dbErrCode = res;
                    adb.m_dbErrMsg = errMsg;
                    adb.m_indexProc = 0;
                    adb.m_mapRowset.Remove(index);
                    adb.m_mapParameterCall.Remove(index);
                    adb.m_mapHandler.Remove(index);
                }
                if (handler != null)
                    handler(adb, res, errMsg, affected, fail_ok, vtId);
            }

            /// <summary>
            /// Open a database connection at server side asynchronously
            /// </summary>
            /// <param name="strConnection">a database connection string. The database connection string can be an empty string if its server side supports global database connection string</param>
            /// <param name="handler">a callback for database connecting result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Open(string strConnection, DResult handler) {
                return Open(strConnection, handler, 0, null);
            }

            /// <summary>
            /// Open a database connection at server side asynchronously
            /// </summary>
            /// <param name="strConnection">a database connection string. The database connection string can be an empty string if its server side supports global database connection string</param>
            /// <param name="handler">a callback for database connecting result</param>
            /// <param name="flags">a set of flags transferred to server to indicate how to build database connection at server side. It defaults to zero</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Open(string strConnection, DResult handler, uint flags) {
                return Open(strConnection, handler, flags, null);
            }

            /// <summary>
            /// Open a database connection at server side asynchronously
            /// </summary>
            /// <param name="strConnection">a database connection string. The database connection string can be an empty string if its server side supports global database connection string</param>
            /// <param name="handler">a callback for database connecting result</param>
            /// <param name="flags">a set of flags transferred to server to indicate how to build database connection at server side. It defaults to zero</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool Open(string strConnection, DResult handler, uint flags, DDiscarded discarded) {
                string s = null;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                //in case a client asynchronously sends lots of requests without use of client side queue.
                lock (m_csDB) {
                    m_flags = flags;
                    if (strConnection != null) {
                        s = m_strConnection;
                        m_strConnection = strConnection;
                    }
                }
                if (SendRequest(DB_CONSTS.idOpen, strConnection, flags, (ar) => {
                    int res, ms;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg).Load(out ms);
                    CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                    lock (adb.m_csDB) {
                        adb.m_dbErrCode = res;
                        adb.m_lastReqId = DB_CONSTS.idOpen;
                        if (res == 0) {
                            adb.m_strConnection = errMsg;
                            errMsg = "";
                        } else {
                            adb.m_strConnection = "";
                        }
                        adb.m_dbErrMsg = errMsg;
                        adb.m_ms = (tagManagementSystem)ms;
                        adb.m_parameters = 0;
                        adb.m_indexProc = 0;
                        adb.m_output = 0;
                    }
                    if (handler != null) {
                        handler(adb, res, errMsg);
                    }
                }, discarded, null)) {
                    return true;
                }
                lock (m_csDB) {
                    if (strConnection != null) {
                        m_strConnection = s;
                    }
                }
                return false;
            }

            /// <summary>
            /// Send a parameterized SQL statement for preparing asynchronously
            /// </summary>
            /// <param name="sql">a parameterized SQL statement</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Prepare(string sql) {
                return Prepare(sql, null, null, null);
            }

            /// <summary>
            /// Send a parameterized SQL statement for preparing asynchronously
            /// </summary>
            /// <param name="sql">a parameterized SQL statement</param>
            /// <param name="handler">a callback for SQL preparing result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Prepare(string sql, DResult handler) {
                return Prepare(sql, handler, null, null);
            }

            /// <summary>
            /// Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
            /// </summary>
            /// <param name="sql">a parameterized SQL statement</param>
            /// <param name="handler">a callback for SQL preparing result</param>
            /// <param name="vParameterInfo">a given array of parameter informations</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Prepare(string sql, DResult handler, CParameterInfo[] vParameterInfo) {
                return Prepare(sql, handler, vParameterInfo, null);
            }

            /// <summary>
            /// Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
            /// </summary>
            /// <param name="sql">a parameterized SQL statement</param>
            /// <param name="handler">a callback for SQL preparing result</param>
            /// <param name="vParameterInfo">a given array of parameter informations</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool Prepare(string sql, DResult handler, CParameterInfo[] vParameterInfo, DDiscarded discarded) {
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(sql);
                    int count = 0;
                    if (vParameterInfo != null) {
                        count = vParameterInfo.Length;
                    }
                    sb.Save(count);
                    if (count > 0) {
                        foreach (CParameterInfo info in vParameterInfo) {
                            info.SaveTo(sb.UQueue);
                        }
                    }
                    if (!SendRequest(DB_CONSTS.idPrepare, sb.UQueue.IntenalBuffer, sb.UQueue.GetSize(), (ar) => {
                        int res;
                        string errMsg;
                        uint parameters;
                        ar.Load(out res).Load(out errMsg).Load(out parameters);
                        CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                        lock (adb.m_csDB) {
                            adb.m_bCallReturn = false;
                            adb.m_lastReqId = DB_CONSTS.idPrepare;
                            adb.m_dbErrCode = res;
                            adb.m_dbErrMsg = errMsg;
                            adb.m_parameters = (parameters & 0xffff);
                            adb.m_indexProc = 0;
                            adb.m_output = (parameters >> 16);
                        }
                        if (handler != null) {
                            handler(adb, res, errMsg);
                        }
                    }, discarded, null)) {
                        return false;
                    }
                }
                return true;
            }

            /// <summary>
            /// End a manual transaction with a given rollback plan tagRollbackPlan.rpDefault. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool EndTrans() {
                return EndTrans(tagRollbackPlan.rpDefault, null, null);
            }

            /// <summary>
            /// End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="plan">a value for computing how included transactions should be rollback at server side. It defaults to tagRollbackPlan.rpDefault</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool EndTrans(tagRollbackPlan plan) {
                return EndTrans(plan, null, null);
            }

            /// <summary>
            /// End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="plan">a value for computing how included transactions should be rollback at server side. It defaults to tagRollbackPlan.rpDefault</param>
            /// <param name="handler">a callback for tracking its response result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool EndTrans(tagRollbackPlan plan, DResult handler) {
                return EndTrans(plan, handler, null);
            }

            /// <summary>
            /// End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="plan">a value for computing how included transactions should be rollback at server side. It defaults to tagRollbackPlan.rpDefault</param>
            /// <param name="handler">a callback for tracking its response result</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool EndTrans(tagRollbackPlan plan, DResult handler, DDiscarded discarded) {
                //make sure EndTrans sending and underlying client persistent message queue as one combination sending
                //to avoid possible request sending/client message writing overlapping within multiple threading environment
                lock (m_csOneSending) {
                    if (SendRequest(DB_CONSTS.idEndTrans, (int)plan, (ar) => {
                        int res;
                        string errMsg;
                        ar.Load(out res).Load(out errMsg);
                        CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                        lock (adb.m_csDB) {
                            adb.m_lastReqId = DB_CONSTS.idEndTrans;
                            adb.m_dbErrCode = res;
                            adb.m_dbErrMsg = errMsg;
                        }
                        if (handler != null) {
                            handler(adb, res, errMsg);
                        }
                    }, discarded, null)) {
                        if (m_queueOk) {
                            //associate end transaction with underlying client persistent message queue
                            AttachedClientSocket.ClientQueue.EndJob();
                            m_queueOk = false;
                        }
                        return true;
                    }
                    return false;
                }
            }

            /// <summary>
            /// Start a manual transaction with a given isolation tagTransactionIsolation.tiReadCommited asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool BeginTrans() {
                return BeginTrans(tagTransactionIsolation.tiReadCommited, null, null);
            }

            /// <summary>
            /// Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="isolation">a value for transaction isolation. It defaults to tagTransactionIsolation.tiReadCommited</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool BeginTrans(tagTransactionIsolation isolation) {
                return BeginTrans(isolation, null, null);
            }

            /// <summary>
            /// Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="isolation">a value for transaction isolation. It defaults to tagTransactionIsolation.tiReadCommited</param>
            /// <param name="handler">a callback for tracking its response result</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool BeginTrans(tagTransactionIsolation isolation, DResult handler) {
                return BeginTrans(isolation, handler, null);
            }

            /// <summary>
            /// Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
            /// </summary>
            /// <param name="isolation">a value for transaction isolation. It defaults to tagTransactionIsolation.tiReadCommited</param>
            /// <param name="handler">a callback for tracking its response result</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool BeginTrans(tagTransactionIsolation isolation, DResult handler, DDiscarded discarded) {
                uint flags;
                string connection;
                //make sure BeginTrans sending and underlying client persistent message queue as one combination sending
                //to avoid possible request sending/client message writing overlapping within multiple threading environment
                lock (m_csOneSending) {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    lock (m_csDB) {
                        connection = m_strConnection;
                        flags = m_flags;
                    }
                    //associate begin transaction with underlying client persistent message queue
                    m_queueOk = AttachedClientSocket.ClientQueue.StartJob();
                    return SendRequest(DB_CONSTS.idBeginTrans, (int)isolation, connection, flags, (ar) => {
                        int res, ms;
                        string errMsg;
                        ar.Load(out res).Load(out errMsg).Load(out ms);
                        CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                        lock (adb.m_csDB) {
                            if (res == 0) {
                                adb.m_strConnection = errMsg;
                                errMsg = "";
                            }
                            adb.m_lastReqId = DB_CONSTS.idBeginTrans;
                            adb.m_dbErrCode = res;
                            adb.m_dbErrMsg = errMsg;
                            adb.m_ms = (tagManagementSystem)ms;
                        }
                        if (handler != null) {
                            handler(adb, res, errMsg);
                        }
                    }, discarded, null);
                }
            }

            /// <summary>
            /// Notify connected remote server to close database connection string asynchronously
            /// </summary>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Close() {
                return Close(null, null);
            }

            /// <summary>
            /// Notify connected remote server to close database connection string asynchronously
            /// </summary>
            /// <param name="handler">a callback for closing result, which should be OK always as long as there is network or queue available </param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public bool Close(DResult handler) {
                return Close(handler, null);
            }

            /// <summary>
            /// Notify connected remote server to close database connection string asynchronously
            /// </summary>
            /// <param name="handler">a callback for closing result, which should be OK always as long as there is network or queue available </param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true if request is successfully sent or queued; and false if request is NOT successfully sent or queued</returns>
            public virtual bool Close(DResult handler, DDiscarded discarded) {
                return SendRequest(DB_CONSTS.idClose, (ar) => {
                    int res;
                    string errMsg;
                    ar.Load(out res).Load(out errMsg);
                    CAsyncDBHandler adb = (CAsyncDBHandler)ar.AsyncServiceHandler;
                    lock (adb.m_csDB) {
                        adb.m_lastReqId = DB_CONSTS.idClose;
                        adb.m_strConnection = "";
                        adb.m_dbErrCode = res;
                        adb.m_dbErrMsg = errMsg;
                    }
                    if (handler != null) {
                        handler(adb, res, errMsg);
                    }
                }, discarded, null);
            }

            protected override void OnAllProcessed() {
                lock (m_csDB) {
                    m_vData.Clear();
                    m_Blob.SetSize(0);
                    if (m_Blob.MaxBufferSize > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                        m_Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
                    }
                }
            }

            protected override void OnMergeTo(CAsyncServiceHandler to) {
                CAsyncDBHandler dbTo = (CAsyncDBHandler)to;
                lock (dbTo.m_csDB) {
                    lock (m_csDB) {
                        foreach (ulong callIndex in m_mapRowset.Keys) {
                            dbTo.m_mapRowset.Add(callIndex, m_mapRowset[callIndex]);
                        }
                        m_mapRowset.Clear();
                        foreach (ulong callIndex in m_mapParameterCall.Keys) {
                            dbTo.m_mapParameterCall.Add(callIndex, m_mapParameterCall[callIndex]);
                        }
                        m_mapParameterCall.Clear();
                        foreach (ulong callIndex in m_mapHandler.Keys) {
                            dbTo.m_mapHandler.Add(callIndex, m_mapHandler[callIndex]);
                        }
                        m_mapHandler.Clear();
                    }
                }
            }

            protected override void OnResultReturned(ushort reqId, CUQueue mc) {
                switch (reqId) {
                    case DB_CONSTS.idParameterPosition:
                        mc.Load(out m_nParamPos);
                        lock (m_csDB) {
                            m_bCallReturn = false;
                            m_indexProc = 0;
                        }
                        break;
                    case DB_CONSTS.idSqlBatchHeader: {
                            ulong callIndex;
                            int res, ms;
                            uint parameters;
                            DRowsetHeader cb = null;
                            string errMsg;
                            mc.Load(out res).Load(out errMsg).Load(out ms).Load(out parameters).Load(out callIndex);
                            lock (m_csDB) {
                                m_indexProc = 0;
                                m_lastReqId = reqId;
                                m_parameters = (parameters & (uint)0xffff);
                                m_output = 0;
                                if (res == 0) {
                                    m_strConnection = errMsg;
                                    errMsg = "";
                                }
                                m_dbErrCode = res;
                                m_dbErrMsg = errMsg;
                                m_ms = (tagManagementSystem)ms;
                                if (m_mapHandler.ContainsKey(callIndex))
                                    cb = m_mapHandler[callIndex];
                            }
                            if (cb != null)
                                cb(this);
                        }
                        break;
                    case DB_CONSTS.idRowsetHeader: {
                            uint output = 0;
                            m_Blob.SetSize(0);
                            if (m_Blob.MaxBufferSize > ONE_MEGA_BYTES) {
                                m_Blob.Realloc(ONE_MEGA_BYTES);
                            }
                            m_vData.Clear();
                            DRowsetHeader header = null;
                            lock (m_csDB) {
                                m_vColInfo = new CDBColumnInfoArray();
                                m_vColInfo.LoadFrom(mc);
                                mc.Load(out m_indexRowset);
                                if (mc.GetSize() > 0) {
                                    mc.Load(out output);
                                }
                                if (output == 0 && m_vColInfo.Count > 0) {
                                    if (m_mapRowset.ContainsKey(m_indexRowset)) {
                                        header = m_mapRowset[m_indexRowset].Key;
                                    }
                                }
                            }
                            if (header != null) {
                                header(this);
                            }
                        }
                        break;
                    case DB_CONSTS.idCallReturn: {
                            object vt;
                            mc.Load(out vt);
                            lock (m_csDB) {
                                if (m_mapParameterCall.ContainsKey(m_indexRowset)) {
                                    CDBVariantArray vParam = m_mapParameterCall[m_indexRowset];
                                    uint pos = m_parameters * m_indexProc + (m_nParamPos >> 16);
                                    vParam[(int)pos] = vt;
                                }
                                m_bCallReturn = true;
                            }
                        }
                        break;
                    case DB_CONSTS.idBeginRows:
                        m_Blob.SetSize(0);
                        m_vData.Clear();
                        if (mc.GetSize() > 0) {
                            lock (m_csDB) {
                                mc.Load(out m_indexRowset);
                            }
                        }
                        break;
                    case DB_CONSTS.idTransferring:
                        while (mc.GetSize() > 0) {
                            object vt;
                            mc.Load(out vt);
                            m_vData.Add(vt);
                        }
                        break;
                    case DB_CONSTS.idOutputParameter:
                    case DB_CONSTS.idEndRows:
                        if (mc.GetSize() > 0 || m_vData.Count > 0) {
                            object vt;
                            while (mc.GetSize() > 0) {
                                mc.Load(out vt);
                                m_vData.Add(vt);
                            }
                            if (reqId == DB_CONSTS.idOutputParameter) {
                                lock (m_csDB) {
                                    if (m_lastReqId == DB_CONSTS.idSqlBatchHeader) {
                                        if (m_indexProc == 0)
                                            m_output += (uint)(m_vData.Count + (m_bCallReturn ? 1 : 0));
                                    } else {
                                        if (m_output == 0)
                                            m_output = (uint)(m_vData.Count + (m_bCallReturn ? 1 : 0));
                                    }
                                    if (m_mapParameterCall.ContainsKey(m_indexRowset)) {
                                        CDBVariantArray vParam = m_mapParameterCall[m_indexRowset];
                                        uint pos;
                                        if (m_lastReqId == DB_CONSTS.idSqlBatchHeader)
                                            pos = m_parameters * m_indexProc + (m_nParamPos & 0xffff) + (m_nParamPos >> 16) - (uint)m_vData.Count;
                                        else
                                            pos = m_parameters * m_indexProc + m_parameters - (uint)m_vData.Count;
                                        foreach (object obj in m_vData) {
                                            vParam[(int)pos] = obj;
                                            ++pos;
                                        }
                                    }
                                }
                                ++m_indexProc;
                            } else {
                                DRows row = null;
                                lock (m_csDB) {
                                    if (m_mapRowset.ContainsKey(m_indexRowset)) {
                                        row = m_mapRowset[m_indexRowset].Value;
                                    }
                                }
                                if (row != null) {
                                    row(this, m_vData);
                                }
                            }
                        }
                        m_vData.Clear();
                        break;
                    case DB_CONSTS.idStartBLOB:
                        if (mc.GetSize() > 0) {
                            m_Blob.SetSize(0);
                            uint len;
                            mc.Load(out len);
                            if (len != uint.MaxValue && len > m_Blob.MaxBufferSize)
                                m_Blob.Realloc(len);
                            m_Blob.Push(mc.IntenalBuffer, mc.HeadPosition, mc.GetSize());
                            mc.SetSize(0);
                        }
                        break;
                    case DB_CONSTS.idChunk:
                        if (mc.GetSize() > 0) {
                            m_Blob.Push(mc.IntenalBuffer, mc.GetSize());
                            mc.SetSize(0);
                        }
                        break;
                    case DB_CONSTS.idEndBLOB:
                        if (mc.GetSize() > 0 || m_Blob.GetSize() > 0) {
                            m_Blob.Push(mc.IntenalBuffer, mc.GetSize());
                            mc.SetSize(0);
                            unsafe {
                                fixed (byte* p = m_Blob.IntenalBuffer) {
                                    uint* len = (uint*)(p + m_Blob.HeadPosition + sizeof(ushort));
                                    if (*len >= BLOB_LENGTH_NOT_AVAILABLE) {
                                        //length should be reset if BLOB length not available from server side at beginning
                                        *len = (m_Blob.GetSize() - sizeof(ushort) - sizeof(uint));
                                    }
                                }
                            }
                            object vt;
                            m_Blob.Load(out vt);
                            m_vData.Add(vt);
                        }
                        break;
                    default:
                        break;
                }
                base.OnResultReturned(reqId, mc);
            }

            private static Type GetType(tagVariantDataType vt) {
                switch (vt) {
                    case tagVariantDataType.sdVT_BOOL:
                        return typeof(bool);
                    case tagVariantDataType.sdVT_WSTR:
                    case tagVariantDataType.sdVT_STR:
                    case tagVariantDataType.sdVT_BSTR:
                        return typeof(string);
                    case tagVariantDataType.sdVT_BYTES:
                        return typeof(byte[]);
                    case tagVariantDataType.sdVT_CLSID:
                        return typeof(Guid);
                    case tagVariantDataType.sdVT_DECIMAL:
                        return typeof(Decimal);
                    case tagVariantDataType.sdVT_DATE:
                        return typeof(DateTime);
                    case tagVariantDataType.sdVT_I1:
                        return typeof(sbyte);
                    case tagVariantDataType.sdVT_I2:
                        return typeof(short);
                    case tagVariantDataType.sdVT_INT:
                    case tagVariantDataType.sdVT_I4:
                        return typeof(int);
                    case tagVariantDataType.sdVT_I8:
                        return typeof(long);
                    case tagVariantDataType.sdVT_R4:
                        return typeof(float);
                    case tagVariantDataType.sdVT_R8:
                        return typeof(double);
                    case tagVariantDataType.sdVT_UI1:
                        return typeof(byte);
                    case tagVariantDataType.sdVT_UI2:
                        return typeof(ushort);
                    case tagVariantDataType.sdVT_UINT:
                    case tagVariantDataType.sdVT_UI4:
                        return typeof(uint);
                    case tagVariantDataType.sdVT_UI8:
                        return typeof(ulong);
                    default:
                        return typeof(object);
                }
            }

            public static DataTable MakeDataTable(CDBColumnInfoArray vColumn, string tableName) {
                DataTable dt = new DataTable(tableName);
                List<DataColumn> lstPrimaryKey = new List<DataColumn>();
                foreach (CDBColumnInfo col in vColumn) {
                    DataColumn dc = new DataColumn(col.OriginalName, GetType(col.DataType));
                    bool b = ((col.Flags & CDBColumnInfo.FLAG_AUTOINCREMENT) == CDBColumnInfo.FLAG_AUTOINCREMENT);
                    dc.AutoIncrement = b;
                    b = ((col.Flags & CDBColumnInfo.FLAG_NOT_NULL) != CDBColumnInfo.FLAG_NOT_NULL);
                    dc.AllowDBNull = b;
                    dc.Caption = col.DisplayName;
                    dc.ReadOnly = ((col.Flags & CDBColumnInfo.FLAG_NOT_WRITABLE) == CDBColumnInfo.FLAG_NOT_WRITABLE);
                    dc.Unique = ((col.Flags & CDBColumnInfo.FLAG_UNIQUE) == CDBColumnInfo.FLAG_UNIQUE || (col.Flags & CDBColumnInfo.FLAG_ROWID) == CDBColumnInfo.FLAG_ROWID || dc.AutoIncrement);
                    if (dc.DataType == typeof(string) || dc.DataType == typeof(byte[])) {
                        dc.MaxLength = (int)col.ColumnSize;
                    }
                    uint flag = (col.Flags & CDBColumnInfo.FLAG_PRIMARY_KEY);
                    if (flag == CDBColumnInfo.FLAG_PRIMARY_KEY) {
                        lstPrimaryKey.Add(dc);
                    }
                    dt.Columns.Add(dc);
                }
                if (lstPrimaryKey.Count > 0) {
                    dt.PrimaryKey = lstPrimaryKey.ToArray();
                }
                return dt;
            }

            public static DataTable MakeDataTable(CDBColumnInfoArray vColumn) {
                return MakeDataTable(vColumn, "");
            }

            public static void AppendRowDataIntoDataTable(List<object> vtData, DataTable dt) {
                int index = 0;
                int cols = dt.Columns.Count;
                int count = vtData.Count;
                object[] row = new object[cols];
                for (int n = 0; n < count; ++n) {
                    if (vtData[n] == null)
                        row[index] = DBNull.Value;
                    else
                        row[index] = vtData[n];
                    ++index;
                    if (index == cols) {
                        dt.Rows.Add(row);
                        index = 0;
                    }
                }
            }
            public static void AppendRowDataIntoDataTable(object[] vtData, DataTable dt) {
                int index = 0;
                int cols = dt.Columns.Count;
                int count = vtData.Length;
                object[] row = new object[cols];
                for (int n = 0; n < count; ++n) {
                    if (vtData[n] == null)
                        row[index] = DBNull.Value;
                    else
                        row[index] = vtData[n];
                    ++index;
                    if (index == cols) {
                        dt.Rows.Add(row);
                        index = 0;
                    }
                }
            }
        }
    } //namespace ClientSide
#endif
}//namespace UDatabase
