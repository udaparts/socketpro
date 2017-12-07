using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.UDB;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;

class CStreamSql : CClientPeer
{
    public const uint sidMsSql = BaseServiceID.sidReserved + 0x6FFFFFF2;
    private string m_defaultDB = "";
    private CUQueue m_Blob = new CUQueue();
    private SqlConnection m_conn = null;
    private SqlCommand m_sqlPrepare = null;
    private ulong m_indexCall = 0;
    private ushort m_outputs = 0;
    internal static object m_csPeer = new object();
    internal static Dictionary<ulong, SqlConnection> m_mapConnection = new Dictionary<ulong, SqlConnection>(); //protected by m_csPeer

    protected CDBVariantArray m_vParam = new CDBVariantArray();
    protected bool m_EnableMessages = false;
    protected ulong m_oks = 0;
    protected ulong m_fails = 0;
    protected SqlTransaction m_trans = null;

    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        switch (reqId)
        {
            case tagBaseRequestID.idPing:
                ResetMemories();
                break;
            case tagBaseRequestID.idCancel:
                if (m_trans != null)
                {
                    try
                    {
                        m_trans.Rollback();
                    }
                    finally
                    {
                        m_fails = 0;
                        m_oks = 0;
                        m_trans = null;
                    }
                }
                break;
            default:
                break;
        }
    }

    protected override void OnSlowRequestProcessed(ushort reqId)
    {
        switch (reqId)
        {
            case DB_CONSTS.idExecuteParameters:
                m_vParam.Clear();
                break;
            default:
                break;
        }
    }

    protected override void OnSwitchFrom(uint oldServiceId)
    {
        m_oks = 0;
        m_fails = 0;
        ulong socket = Handle;
        lock (m_csPeer)
        {
            if (m_mapConnection.ContainsKey(socket))
            {
                m_conn = m_mapConnection[socket];
                m_mapConnection.Remove(socket);
            }
        }
    }

    protected override void OnReleaseResource(bool bClosing, uint info)
    {
        int res;
        CloseDb(out res);
        if (m_conn != null)
        {
            //we close a database session when a socket is closed
            try
            {
                m_conn.Close();
                m_conn.Dispose();
            }
            finally
            {
                m_conn = null;
            }
        }
    }

    private bool PushRowsetHeader(SqlDataReader reader, out CDBColumnInfoArray vCol)
    {
        vCol = new CDBColumnInfoArray();
        DataTable dt = reader.GetSchemaTable();
        int cols = reader.FieldCount;
        for (int n = 0; n < cols; ++n)
        {
            DataRow dr = dt.Rows[n];
            CDBColumnInfo info = new CDBColumnInfo();
            bool b = (bool)dr["AllowDBNull"];
            if (!b)
                info.Flags = CDBColumnInfo.FLAG_NOT_NULL;
            info.DBPath = m_conn.DataSource + "." + m_conn.Database;
            info.TablePath = dr["BaseSchemaName"] + "." + dr["BaseTableName"];
            b = (bool)dr["IsAutoIncrement"];
            if (b)
            {
                info.Flags |= CDBColumnInfo.FLAG_PRIMARY_KEY;
                info.Flags |= CDBColumnInfo.FLAG_UNIQUE;
                info.Flags |= CDBColumnInfo.FLAG_AUTOINCREMENT;
            }
            object isKey = dr["IsKey"];
            if (!(isKey is DBNull))
            {
                b = (bool)isKey;
                info.Flags |= CDBColumnInfo.FLAG_PRIMARY_KEY;
            }
            b = (bool)dr["IsUnique"];
            if (b)
                info.Flags |= CDBColumnInfo.FLAG_UNIQUE;
            b = (bool)dr["IsRowVersion"];
            if (b)
                info.Flags |= CDBColumnInfo.FLAG_ROWID;
            string data_type = reader.GetDataTypeName(n);
            info.DeclaredType = data_type;
            info.DisplayName = dr["ColumnName"].ToString();
            info.OriginalName = dr["BaseColumnName"].ToString();
            switch (data_type)
            {
                case "bigint":
                    info.DataType = tagVariantDataType.sdVT_I8;
                    break;
                case "bit":
                    info.DataType = tagVariantDataType.sdVT_BOOL;
                    info.Flags |= CDBColumnInfo.FLAG_IS_BIT;
                    break;
                case "timestamp":
                case "rowversion":
                case "binary":
                    info.DataType = (tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY);
                    info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    break;
                case "nchar":
                case "char":
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    break;
                case "time":
                case "smalldatetime":
                case "date":
                case "datetime":
                    info.DataType = tagVariantDataType.sdVT_DATE;
                    break;
                case "datetime2":
                    info.DataType = tagVariantDataType.sdVT_DATE;
                    info.Precision = byte.Parse(dr["NumericPrecision"].ToString());
                    break;
                case "datetimeoffset":
                    info.DataType = tagVariantDataType.sdVT_BSTR; //!!!! use string instead GetDateTimeOffset()
                    info.ColumnSize = 64;
                    break;
                case "smallmoney":
                case "numeric":
                case "money":
                case "decimal":
                    info.DataType = tagVariantDataType.sdVT_DECIMAL;
                    info.Precision = byte.Parse(dr["NumericPrecision"].ToString());
                    info.Precision = byte.Parse(dr["NumericScale"].ToString());
                    break;
                case "float":
                    info.DataType = tagVariantDataType.sdVT_R8;
                    break;
                case "image": //!!!! FILESTREAM attribute 
                    info.DataType = (tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY);
                    info.ColumnSize = uint.MaxValue;
                    break;
                case "int":
                    info.DataType = tagVariantDataType.sdVT_INT;
                    break;
                case "text":
                case "ntext":
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    info.ColumnSize = uint.MaxValue;
                    break;
                case "varchar":
                case "nvarchar":
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    b = (bool)dr["IsLong"];
                    if (b)
                        info.ColumnSize = uint.MaxValue;
                    else
                        info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    break;
                case "real":
                    info.DataType = tagVariantDataType.sdVT_R4;
                    break;
                case "smallint":
                    info.DataType = tagVariantDataType.sdVT_I2;
                    break;
                case "tinyint":
                    info.DataType = tagVariantDataType.sdVT_UI1;
                    break;
                case "uniqueidentifier":
                    info.DataType = tagVariantDataType.sdVT_CLSID;
                    break;
                case "varbinary":
                    info.DataType = (tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY);
                    b = (bool)dr["IsLong"];
                    if (b)
                        info.ColumnSize = uint.MaxValue;
                    else
                        info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    break;
                case "xml":
                    info.DataType = tagVariantDataType.sdVT_BSTR; //!!!! use string instead GetSqlXml
                    info.ColumnSize = uint.MaxValue;
                    info.Flags |= CDBColumnInfo.FLAG_XML;
                    break;
                case "sql_variant":
                    info.DataType = tagVariantDataType.sdVT_VARIANT;
                    break;
                default:
                    info.DataType = tagVariantDataType.sdVT_BSTR; //!!!! use string instead GetValue
                    info.ColumnSize = 4 * 1024;
                    break;
            }
            vCol.Add(info);
        }
        uint res = SendResult(DB_CONSTS.idRowsetHeader, vCol, m_indexCall);
        if (res == SOCKET_NOT_FOUND || res == REQUEST_CANCELED)
            return false;
        return true;
    }

    private bool PushToClient(SqlDataReader reader)
    {
        CDBColumnInfoArray vCol;
        if (!PushRowsetHeader(reader, out vCol))
            return false;
        return PushRows(reader, vCol);
    }

    private bool PushRows(SqlDataReader reader, CDBColumnInfoArray vCol)
    {
        using (CScopeUQueue sb = new CScopeUQueue())
        {
            while (reader.Read())
            {

            }
        }
        return true;
    }

    [RequestAttr(DB_CONSTS.idExecute, true)]
    private ulong Execute(string sql, bool rowset, bool meta, bool lastInsertId, ulong index, out long affected, out int res, out string errMsg, out object vtId)
    {
        ulong fail_ok = 0;
        m_indexCall = index;
        affected = 0;
        res = 0;
        errMsg = "";
        vtId = null;
        ulong fails = m_fails;
        ulong oks = m_oks;
        bool HeaderSent = false;
        bool ok = true;
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand(sql, m_conn);
            if (rowset)
            {
                reader = cmd.ExecuteReader(meta ? CommandBehavior.KeyInfo : CommandBehavior.Default);
                while (reader.FieldCount > 0 && ok)
                {
                    ok = PushToClient(reader);
                    HeaderSent = true;
                    if (!reader.NextResult())
                        break;
                }
                if (reader.RecordsAffected > 0)
                    affected += reader.RecordsAffected;
            }
            else
            {
                int ret = cmd.ExecuteNonQuery();
                if (ret > 0)
                    affected += ret;
            }
            ++m_oks;
        }
        catch (SqlException err)
        {
            if (res == 0)
            {
                res = err.ErrorCode;
                errMsg = err.Message;
            }
            ++m_fails;
        }
        catch (Exception err)
        {
            if (res == 0)
            {
                res = -1;
                errMsg = err.Message;
            }
            ++m_fails;
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
        if (!HeaderSent && ok)
        {
            CDBColumnInfoArray v = new CDBColumnInfoArray();
            SendResult(DB_CONSTS.idRowsetHeader, v, index);
        }
        fail_ok = ((m_fails - fails) << 32);
        fail_ok += (m_oks - oks);
        return fail_ok;
    }

    [RequestAttr(DB_CONSTS.idExecuteParameters, true)]
    private ulong ExecuteParameters(bool rowset, bool meta, bool lastInsertId, ulong index, out long affected, out int res, out string errMsg, out object vtId)
    {
        ulong fail_ok = 0;
        m_indexCall = index;
        affected = 0;
        res = 0;
        errMsg = "";
        vtId = null;
        ulong fails = m_fails;
        ulong oks = m_oks;
        do
        {
            if (m_sqlPrepare == null || m_sqlPrepare.Parameters.Count == 0 || m_vParam.Count == 0)
            {
                res = -2;
                errMsg = "No parameter specified";
                ++m_fails;
                break;
            }
            int cols = m_sqlPrepare.Parameters.Count;
            if ((m_vParam.Count % cols) != 0)
            {
                res = -2;
                errMsg = "Bad parameter data array size";
                ++m_fails;
                break;
            }
            int rows = m_vParam.Count / cols;
            for (int r = 0; r < rows; ++r)
            {
                try
                {
                    int c = 0;
                    foreach (SqlParameter p in m_sqlPrepare.Parameters)
                    {
                        p.Value = m_vParam[r * cols + c];
                        ++c;
                    }
                    if (rowset)
                    {
                        SqlDataReader reader = m_sqlPrepare.ExecuteReader(meta ? CommandBehavior.KeyInfo : CommandBehavior.Default);
                        while (reader.FieldCount > 0)
                        {
                            if (reader.RecordsAffected > 0)
                                affected += reader.RecordsAffected;

                        }
                    }
                    else
                    {
                        int ret = m_sqlPrepare.ExecuteNonQuery();
                        if (ret > 0)
                            affected += ret;
                    }
                    ++m_oks;
                }
                catch (SqlException err)
                {
                    if (res == 0)
                    {
                        res = err.ErrorCode;
                        errMsg = err.Message;
                    }
                    ++m_fails;
                }
                catch (Exception err)
                {
                    if (res == 0)
                    {
                        res = -1;
                        errMsg = err.Message;
                    }
                    ++m_fails;
                }
            }
        } while (false);
        fail_ok = ((m_fails - fails) << 32);
        fail_ok += (m_oks - oks);
        return fail_ok;
    }

    [RequestAttr(DB_CONSTS.idPrepare, true)]
    private uint Prepare(string sql, CParameterInfoArray vColInfo, out int res, out string errMsg)
    {
        res = 0;
        errMsg = "";
        m_outputs = 0;
        uint parameters = 0;
        try
        {
            m_sqlPrepare = new SqlCommand(sql, m_conn);
            if (vColInfo != null)
            {
                SqlParameter param = null;
                foreach (CParameterInfo info in vColInfo)
                {
                    switch (info.DataType)
                    {
                        case tagVariantDataType.sdVT_BYTES:
                        case tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI1:
                            if (info.ColumnSize == uint.MaxValue)
                                param = new SqlParameter(info.ParameterName, SqlDbType.Image);
                            else
                                param = new SqlParameter(info.ParameterName, SqlDbType.VarBinary, (int)info.ColumnSize);
                            break;
                        case tagVariantDataType.sdVT_STR:
                        case tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_I1:
                            if (info.ColumnSize == uint.MaxValue)
                                param = new SqlParameter(info.ParameterName, SqlDbType.Text);
                            else
                                param = new SqlParameter(info.ParameterName, SqlDbType.VarChar, (int)info.ColumnSize);
                            break;
                        case tagVariantDataType.sdVT_WSTR:
                        case tagVariantDataType.sdVT_BSTR:
                            if (info.ColumnSize == uint.MaxValue)
                                param = new SqlParameter(info.ParameterName, SqlDbType.NText);
                            else
                                param = new SqlParameter(info.ParameterName, SqlDbType.NVarChar, (int)info.ColumnSize);
                            break;
                        case tagVariantDataType.sdVT_BOOL:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Bit);
                            break;
                        case tagVariantDataType.sdVT_CLSID:
                            param = new SqlParameter(info.ParameterName, SqlDbType.UniqueIdentifier);
                            break;
                        case tagVariantDataType.sdVT_DECIMAL:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Decimal);
                            param.Precision = info.Precision;
                            param.Scale = info.Scale;
                            break;
                        case tagVariantDataType.sdVT_I1:
                            param = new SqlParameter(info.ParameterName, SqlDbType.TinyInt);
                            break;
                        case tagVariantDataType.sdVT_I2:
                            param = new SqlParameter(info.ParameterName, SqlDbType.SmallInt);
                            break;
                        case tagVariantDataType.sdVT_I4:
                        case tagVariantDataType.sdVT_INT:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Int);
                            break;
                        case tagVariantDataType.sdVT_I8:
                            param = new SqlParameter(info.ParameterName, SqlDbType.BigInt);
                            break;
                        case tagVariantDataType.sdVT_R4:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Real);
                            break;
                        case tagVariantDataType.sdVT_R8:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Float);
                            break;
                        case tagVariantDataType.sdVT_DATE:
                            if (info.Precision == 0)
                                param = new SqlParameter(info.ParameterName, SqlDbType.DateTime);
                            else
                            {
                                param = new SqlParameter(info.ParameterName, SqlDbType.DateTime2);
                                param.Precision = info.Precision;
                            }
                            break;
                        case tagVariantDataType.sdVT_XML:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Xml);
                            break;
                        case tagVariantDataType.sdVT_VARIANT:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Variant);
                            break;
                        case tagVariantDataType.sdVT_TIMESPAN:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Time);
                            break;
                        case tagVariantDataType.sdVT_DATETIMEOFFSET:
                            param = new SqlParameter(info.ParameterName, SqlDbType.DateTimeOffset);
                            param.Precision = info.Precision;
                            break;
                        default:
                            res = -2;
                            errMsg = "Unsupported data type: " + info.DataType.ToString();
                            m_sqlPrepare = null;
                            parameters = 0;
                            return parameters;
                    }
                    switch (info.Direction)
                    {
                        case tagParameterDirection.pdInput:
                            param.Direction = ParameterDirection.Input;
                            break;
                        case tagParameterDirection.pdInputOutput:
                            param.Direction = ParameterDirection.InputOutput;
                            ++m_outputs;
                            break;
                        case tagParameterDirection.pdOutput:
                            param.Direction = ParameterDirection.Output;
                            ++m_outputs;
                            break;
                        case tagParameterDirection.pdReturnValue:
                            param.Direction = ParameterDirection.ReturnValue;
                            ++m_outputs;
                            break;
                        default:
                            res = -2;
                            errMsg = "Unsupported direction: " + info.Direction.ToString();
                            m_sqlPrepare = null;
                            parameters = 0;
                            return parameters;
                    }
                    m_sqlPrepare.Parameters.Add(param);
                }
                parameters = m_outputs;
                parameters <<= 16;
                parameters += (ushort)vColInfo.Count;
            }
            m_sqlPrepare.Prepare();
        }
        catch (SqlException err)
        {
            res = err.ErrorCode;
            errMsg = err.Message;
            m_sqlPrepare = null;
            parameters = 0;
            m_outputs = 0;
        }
        catch (Exception err)
        {
            res = -1;
            errMsg = err.Message;
            m_sqlPrepare = null;
            parameters = 0;
            m_outputs = 0;
        }
        return parameters;
    }

    [RequestAttr(DB_CONSTS.idEndTrans, true)]
    private string EndTrans(int plan, out int res)
    {
        res = 0;
        string errMsg = "";
        do
        {
            if (plan < 0 || plan > (int)tagRollbackPlan.rpRollbackAlways)
            {
                res = -2;
                errMsg = "Bad end transaction plan";
                break;
            }
            if (m_trans == null)
            {
                res = -2;
                errMsg = "Transaction not started yet";
                break;
            }
            bool rollback = false;
            tagRollbackPlan rp = (tagRollbackPlan)plan;
            switch (rp)
            {
                case tagRollbackPlan.rpRollbackErrorAny:
                    rollback = (m_fails > 0) ? true : false;
                    break;
                case tagRollbackPlan.rpRollbackErrorLess:
                    rollback = (m_fails < m_oks && m_fails > 0) ? true : false;
                    break;
                case tagRollbackPlan.rpRollbackErrorEqual:
                    rollback = (m_fails >= m_oks) ? true : false;
                    break;
                case tagRollbackPlan.rpRollbackErrorMore:
                    rollback = (m_fails > m_oks) ? true : false;
                    break;
                case tagRollbackPlan.rpRollbackErrorAll:
                    rollback = (m_oks > 0) ? false : true;
                    break;
                case tagRollbackPlan.rpRollbackAlways:
                    rollback = true;
                    break;
                default:
                    break;
            }
            try
            {
                if (rollback)
                    m_trans.Rollback();
                else
                    m_trans.Commit();
            }
            catch (SqlException err)
            {
                res = err.ErrorCode;
                errMsg = err.Message;
            }
            catch (Exception err)
            {
                res = -1;
                errMsg = err.Message;
            }
            finally
            {
                m_fails = 0;
                m_oks = 0;
                m_trans = null;
            }
        } while (false);
        return errMsg;
    }

    [RequestAttr(DB_CONSTS.idBeginTrans, true)]
    private int BeginTrans(int isolation, string strConnection, uint flags, out int res, out string errMsg)
    {
        int ms = (int)tagManagementSystem.msMsSQL;
        if (m_conn.State != ConnectionState.Open)
        {
            m_trans = null;
            try
            {
                m_conn.Open();
                res = 0;
                errMsg = "";
            }
            catch (SqlException err)
            {
                res = err.ErrorCode;
                errMsg = err.Message;
            }
            catch (Exception err)
            {
                res = -1;
                errMsg = err.Message;
            }
            if (res != 0)
                return ms;
            ms = Open(strConnection, flags, out res, out errMsg);
            if (res != 0)
                return ms;
        }
        else if (m_trans != null)
        {
            res = -2;
            errMsg = "Transaction already started";
        }
        else
        {
            res = 0;
            errMsg = m_defaultDB;
        }
        try
        {
            switch ((tagTransactionIsolation)isolation)
            {
                case tagTransactionIsolation.tiReadCommited:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.ReadCommitted);
                    break;
                case tagTransactionIsolation.tiBrowse:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.Snapshot);
                    break;
                case tagTransactionIsolation.tiReadUncommited:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.ReadUncommitted);
                    break;
                case tagTransactionIsolation.tiChaos:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.Chaos);
                    break;
                case tagTransactionIsolation.tiRepeatableRead:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.RepeatableRead);
                    break;
                case tagTransactionIsolation.tiSerializable:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.Serializable);
                    break;
                case tagTransactionIsolation.tiUnspecified:
                    m_trans = m_conn.BeginTransaction(IsolationLevel.Unspecified);
                    break;
                default:
                    res = -2;
                    errMsg = "Unknown isolation level";
                    break;
            }
        }
        catch (SqlException err)
        {
            res = err.ErrorCode;
            errMsg = err.Message;
        }
        catch (Exception err)
        {
            res = -1;
            errMsg = err.Message;
        }
        finally
        {
            if (m_trans != null)
            {
                m_fails = 0;
                m_oks = 0;
            }
        }
        return ms;
    }

    [RequestAttr(DB_CONSTS.idOpen, true)]
    private int Open(string strConnection, uint flags, out int res, out string errMsg)
    {
        res = 0;
        errMsg = "";
        if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            m_EnableMessages = Push.Subscribe(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
        SqlCommand cmd = new SqlCommand("select DB_NAME()", m_conn);
        m_defaultDB = m_conn.Database;
        if (strConnection != null && strConnection.Length > 0)
        {
            string sql = "USE " + strConnection;
            try
            {
                cmd = new SqlCommand(sql, m_conn);
                int affected = cmd.ExecuteNonQuery();
            }
            catch (SqlException err)
            {
                res = err.ErrorCode;
                errMsg = err.Message;
            }
            catch (Exception err)
            {
                res = -1;
                errMsg = err.Message;
            }
        }
        if (res == 0)
            errMsg = m_defaultDB;
        return (int)tagManagementSystem.msMsSQL;
    }


    [RequestAttr(DB_CONSTS.idStartBLOB)]
    private void StartBLOB(uint lenExpected)
    {
        m_Blob.SetSize(0);
        if (lenExpected > m_Blob.MaxBufferSize)
            m_Blob.Realloc(lenExpected);
        m_Blob.Push(UQueue.IntenalBuffer, UQueue.GetSize());
        UQueue.SetSize(0);
    }

    [RequestAttr(DB_CONSTS.idChunk)]
    private void Chunk()
    {
        CUQueue q = UQueue;
        if (q.GetSize() > 0)
        {
            m_Blob.Push(q.IntenalBuffer, q.GetSize());
            q.SetSize(0);
        }
    }

    [RequestAttr(DB_CONSTS.idEndBLOB)]
    private void EndBLOB()
    {
        Chunk();
        object vt;
        m_Blob.Load(out vt);
        m_vParam.Add(vt);
    }

    [RequestAttr(DB_CONSTS.idTransferring)]
    private void Transferring()
    {
        CUQueue q = UQueue;
        while (q.GetSize() > 0)
        {
            object vt;
            q.Load(out vt);
            m_vParam.Add(vt);
        }
    }

    [RequestAttr(DB_CONSTS.idBeginRows)]
    private void BeginRows()
    {
        Transferring();
    }

    [RequestAttr(DB_CONSTS.idEndRows)]
    private void EndRows()
    {
        Transferring();
    }

    [RequestAttr(DB_CONSTS.idClose, true)]
    private string CloseDb(out int res)
    {
        //we don't close a database session when a client call the method but close it when a socket is closed, which we believe the approach is fit with client socket pool architecture
        string errMsg = "";
        res = 0;
        m_vParam.Clear();
        try
        {
            if (m_trans != null)
                m_trans.Rollback();
        }
        finally
        {
            m_trans = null;
        }
        if (m_conn != null && m_conn.State != ConnectionState.Closed)
        {
            if (m_defaultDB != null && m_defaultDB.Length > 0)
            {
                try
                {
                    //reset back to original default database
                    SqlCommand cmd = new SqlCommand("USE " + m_defaultDB, m_conn);
                    int ret = cmd.ExecuteNonQuery();
                }
                catch (SqlException err)
                {
                    errMsg = err.Message;
                    res = err.ErrorCode;
                }
                catch (Exception err)
                {
                    errMsg = err.Message;
                    res = -1;
                }
                finally { }
            }
        }
        ResetMemories();
        if (m_EnableMessages)
        {
            Push.Unsubscribe();
            m_EnableMessages = false;
        }
        return errMsg;
    }

    private void ResetMemories()
    {
        m_Blob.SetSize(0);
        if (m_Blob.MaxBufferSize > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
        {
            m_Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
        }
    }
}
