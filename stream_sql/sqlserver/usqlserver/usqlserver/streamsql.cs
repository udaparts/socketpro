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

    private static List<string> Split(string sql, string delimiter)
    {
        List<string> v = new List<string>();
        int d_len = delimiter.Length;
        if (d_len > 0)
        {
            const char quote = '\'', slash = '\\';
            char done = delimiter[0];
            int ps = 0, len = sql.Length;
            bool b_slash = false, balanced = true;
            for (int n = 0; n < len; ++n)
            {
                char c = sql[n];
                if (c == slash)
                {
                    b_slash = true;
                    continue;
                }
                if (c == quote && b_slash)
                {
                    b_slash = false;
                    continue; //ignore a quote if there is a slash ahead
                }
                b_slash = false;
                if (c == quote)
                {
                    balanced = (!balanced);
                    continue;
                }
                if (balanced && c == done)
                {
                    int pos = sql.IndexOf(delimiter, n);
                    if (pos == n)
                    {
                        v.Add(sql.Substring(ps, n - ps));
                        n += d_len;
                        ps = n;
                    }
                }
            }
            v.Add(sql.Substring(ps));
        }
        else
        {
            v.Add(sql);
        }
        return v;
    }

    private static uint ComputeParameters(string sql)
    {
        const char quote = '\'', slash = '\\', question = '@';
        bool b_slash = false, balanced = true;
        int ps = 0, len = sql.Length;
        for (int n = 0; n < len; ++n)
        {
            char c = sql[n];
            if (c == slash)
            {
                b_slash = true;
                continue;
            }
            if (c == quote && b_slash)
            {
                b_slash = false;
                continue; //ignore a quote if there is a slash ahead
            }
            b_slash = false;
            if (c == quote)
            {
                balanced = (!balanced);
                continue;
            }
            if (balanced)
            {
                ps += ((c == question) ? 1 : 0);
            }
        }
        return (uint)ps;
    }

    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        switch (reqId)
        {
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
                        if (m_sqlPrepare != null)
                            m_sqlPrepare.Transaction = null;
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
            case DB_CONSTS.idExecuteBatch:
                m_vParam.Clear();
                ResetMemories();
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
        string errMsg = CloseDb(out res);
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

    private bool PushRowsetHeader(SqlDataReader reader, bool meta, out CDBColumnInfoArray vCol)
    {
        bool b;
        DataTable dt = null;
        DataRow dr = null;
        vCol = new CDBColumnInfoArray();
        if (meta)
            dt = reader.GetSchemaTable();
        int cols = reader.FieldCount;
        for (int n = 0; n < cols; ++n)
        {
            CDBColumnInfo info = new CDBColumnInfo();
            if (meta)
            {
                dr = dt.Rows[n];
                b = (bool)dr["AllowDBNull"];
                if (!b)
                    info.Flags = CDBColumnInfo.FLAG_NOT_NULL;
                string dbName = dr["BaseCatalogName"].ToString();
                if (dbName.Length == 0)
                    dbName = m_conn.Database;
                info.DBPath = dbName;
                string schema = dr["BaseSchemaName"].ToString();
                if (schema.Length == 0)
                    schema = "dbo";
                info.TablePath = schema + "." + dr["BaseTableName"];
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
                    if (b)
                        info.Flags |= CDBColumnInfo.FLAG_PRIMARY_KEY;
                }
                b = (bool)dr["IsUnique"];
                if (b)
                    info.Flags |= CDBColumnInfo.FLAG_UNIQUE;
                b = (bool)dr["IsRowVersion"];
                if (b)
                    info.Flags |= CDBColumnInfo.FLAG_ROWID;
                info.DisplayName = dr["ColumnName"].ToString();
                info.OriginalName = dr["BaseColumnName"].ToString();
            }
            string data_type = reader.GetDataTypeName(n);
            info.DeclaredType = data_type;
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
                    if (meta)
                        info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    break;
                case "nchar":
                case "char":
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    if (meta)
                        info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    else
                        info.ColumnSize = 255;
                    break;
                case "smalldatetime":
                case "date":
                case "datetime":
                    info.DataType = tagVariantDataType.sdVT_DATE;
                    break;
                case "datetime2":
                    info.DataType = tagVariantDataType.sdVT_DATE;
                    if (meta)
                        info.Precision = byte.Parse(dr["NumericPrecision"].ToString());
                    break;
                case "datetimeoffset":
                    //!!!! use string instead GetDateTimeOffset()
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    info.ColumnSize = 64;
                    break;
                case "smallmoney":
                case "numeric":
                case "money":
                case "decimal":
                    info.DataType = tagVariantDataType.sdVT_DECIMAL;
                    if (meta)
                    {
                        info.Precision = byte.Parse(dr["NumericPrecision"].ToString());
                        info.Precision = byte.Parse(dr["NumericScale"].ToString());
                    }
                    break;
                case "float":
                    info.DataType = tagVariantDataType.sdVT_R8;
                    break;
                case "image":
                    //!!!! FILESTREAM attribute 
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
                    if (meta)
                    {
                        b = (bool)dr["IsLong"];
                        if (b)
                            info.ColumnSize = uint.MaxValue;
                        else
                            info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    }
                    else
                        info.ColumnSize = uint.MaxValue;
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
                    if (meta)
                    {
                        b = (bool)dr["IsLong"];
                        if (b)
                            info.ColumnSize = uint.MaxValue;
                        else
                            info.ColumnSize = uint.Parse(dr["ColumnSize"].ToString());
                    }
                    else
                        info.ColumnSize = uint.MaxValue;
                    break;
                case "xml":
                    //!!!! use string instead GetSqlXml
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    info.ColumnSize = uint.MaxValue;
                    info.Flags |= CDBColumnInfo.FLAG_XML;
                    break;
                case "sql_variant":
                    info.DataType = tagVariantDataType.sdVT_VARIANT;
                    break;
                case "time":
                default:
                    //!!!! use string instead GetValue
                    info.DataType = tagVariantDataType.sdVT_BSTR;
                    info.ColumnSize = 0; //default
                    break;
            }
            vCol.Add(info);
        }
        uint res = SendResult(DB_CONSTS.idRowsetHeader, vCol, m_indexCall);
        if (res == SOCKET_NOT_FOUND || res == REQUEST_CANCELED)
            return false;
        return true;
    }

    private bool PushToClient(SqlDataReader reader, bool meta)
    {
        CDBColumnInfoArray vCol;
        if (!PushRowsetHeader(reader, meta, out vCol))
            return false;
        return PushRows(reader, vCol);
    }

    private bool SendRows(CUQueue q, bool transferring)
    {
        uint ret;
        bool batching = (BytesBatched >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE);
        if (batching)
            CommitBatching();
        ret = SendResult(transferring ? DB_CONSTS.idTransferring : DB_CONSTS.idEndRows, q.IntenalBuffer, q.GetSize());
        if (batching)
            StartBatching();
        if (ret != q.GetSize())
            return false; //socket closed or request canceled
        q.SetSize(0);
        return true;
    }

    private bool PushText(string text)
    {
        using (CScopeUQueue sb = new CScopeUQueue())
        {
            CUQueue q = sb.UQueue;
            q.Push(text);
            return PushBlob(q.IntenalBuffer, q.GetSize(), (ushort)tagVariantDataType.sdVT_BSTR);
        }
    }

    private bool PushBlob(byte[] buffer, uint len, ushort data_type = (ushort)(tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI1))
    {
        bool batching = Batching;
        try
        {
            if (batching)
                CommitBatching();
            uint bytes = len;
            //extra 4 bytes for string null termination
            uint ret = SendResult(DB_CONSTS.idStartBLOB, bytes + sizeof(ushort) + sizeof(uint) + sizeof(uint), data_type, bytes);
            if (ret == SOCKET_NOT_FOUND || ret == REQUEST_CANCELED)
                return false;
            uint offset = 0;
            while (bytes > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
            {
                ret = SendResult(DB_CONSTS.idChunk, buffer, DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE, offset);
                if (ret != DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                    return false;
                offset += ret;
                bytes -= ret;
            }
            ret = SendResult(DB_CONSTS.idEndBLOB, buffer, bytes, offset);
            return (ret == bytes);
        }
        finally
        {
            if (batching)
                StartBatching();
        }
    }

    private bool PushRows(SqlDataReader reader, CDBColumnInfoArray vCol)
    {
        using (CScopeUQueue sb = new CScopeUQueue())
        {
            CUQueue q = sb.UQueue;
            while (reader.Read())
            {
                if (q.GetSize() >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE && !SendRows(q, false))
                    return false;
                int col = 0;
                foreach (CDBColumnInfo info in vCol)
                {
                    if (reader.IsDBNull(col))
                    {
                        q.Save((ushort)tagVariantDataType.sdVT_NULL);
                        ++col;
                        continue;
                    }
                    switch (info.DataType)
                    {
                        case tagVariantDataType.sdVT_BSTR:
                            if (info.DeclaredType == "xml")
                            {
                                string xml = reader.GetSqlXml(col).Value;
                                if (xml.Length <= DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                                {
                                    q.Save((ushort)info.DataType).Save(xml);
                                }
                                else
                                {
                                    if (q.GetSize() != 0 && !SendRows(q, true))
                                        return false;
                                    if (!PushText(xml))
                                        return false;
                                }
                            }
                            else if (info.DeclaredType == "datetimeoffset")
                            {
                                DateTimeOffset dto = reader.GetDateTimeOffset(col);
                                q.Save((ushort)info.DataType).Save(dto.ToString());
                            }
                            else if (info.ColumnSize == 0)
                            {
                                //for example, case "time"
                                object obj = reader.GetValue(col);
                                q.Save((ushort)info.DataType).Save(obj.ToString());
                            }
                            else
                            {
                                string s = reader.GetString(col);
                                if (s.Length <= DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                                {
                                    q.Save((ushort)info.DataType).Save(s);
                                }
                                else
                                {
                                    //text, ntext, varchar(max), nvarchar(max)
                                    if (q.GetSize() != 0 && !SendRows(q, true))
                                        return false;
                                    if (!PushText(s))
                                        return false;
                                }
                            }
                            break;
                        case (tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY):
                            {
                                SqlBinary bytes = reader.GetSqlBinary(col);
                                if (bytes.Length <= 2 * DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                                {
                                    q.Save((ushort)info.DataType).Save(bytes.Value);
                                }
                                else
                                {
                                    //image, varbinary(max) or file?
                                    if (q.GetSize() != 0 && !SendRows(q, true))
                                        return false;
                                    if (!PushBlob(bytes.Value, (uint)bytes.Length))
                                        return false;
                                }
                            }
                            break;
                        case tagVariantDataType.sdVT_I8:
                            q.Save((ushort)info.DataType).Save(reader.GetInt64(col));
                            break;
                        case tagVariantDataType.sdVT_I4:
                        case tagVariantDataType.sdVT_INT:
                            q.Save((ushort)info.DataType).Save(reader.GetInt32(col));
                            break;
                        case tagVariantDataType.sdVT_I2:
                            q.Save((ushort)info.DataType).Save(reader.GetInt16(col));
                            break;
                        case tagVariantDataType.sdVT_UI1:
                            q.Save((ushort)info.DataType).Save(reader.GetByte(col));
                            break;
                        case tagVariantDataType.sdVT_R4:
                            q.Save((ushort)info.DataType).Save(reader.GetFloat(col));
                            break;
                        case tagVariantDataType.sdVT_R8:
                            q.Save((ushort)info.DataType).Save(reader.GetDouble(col));
                            break;
                        case tagVariantDataType.sdVT_BOOL:
                            q.Save((ushort)info.DataType);
                            if (reader.GetBoolean(col))
                                q.Save((short)-1);
                            else
                                q.Save((short)0);
                            break;
                        case tagVariantDataType.sdVT_DATE:
                            q.Save((ushort)info.DataType).Save(reader.GetDateTime(col));
                            break;
                        case tagVariantDataType.sdVT_DECIMAL:
                            q.Save((ushort)info.DataType).Save(reader.GetDecimal(col));
                            break;
                        case tagVariantDataType.sdVT_CLSID:
                            q.Save((ushort)info.DataType).Save(reader.GetGuid(col));
                            break;
                        case tagVariantDataType.sdVT_VARIANT:
                            q.Save(reader.GetValue(col));
                            break;
                        default:
                            break;
                    }
                    ++col;
                }
            }
            uint ret = SendResult(DB_CONSTS.idEndRows, q.IntenalBuffer, q.GetSize());
            if (ret != q.GetSize())
                return false; //socket closed or request canceled
        }
        return true;
    }

    private string GenerateSqlForCachedTables()
    {
        string sqlCache = "";
        string current_db = m_conn.Database;
        List<string> vDB = new List<string>();
        SqlDataReader reader = null;
        string sql = "SELECT name FROM master.dbo.sysdatabases where name NOT IN('master','tempdb','model','msdb')";
        try
        {
            SqlCommand cmd = new SqlCommand(sql, m_conn, m_trans);
            reader = cmd.ExecuteReader();
            while (reader.Read())
            {
                vDB.Add(reader.GetString(0));
            }
            reader.Close();
            foreach (string db in vDB)
            {
                cmd.CommandText = "USE " + db;
                cmd.ExecuteNonQuery();
                sql = "select object_schema_name(parent_id),OBJECT_NAME(parent_id)from sys.assembly_modules as am,sys.triggers as t where t.object_id=am.object_id and assembly_method like 'PublishDMLEvent%' and assembly_class='USqlStream'";
                cmd.CommandText = sql;
                reader = cmd.ExecuteReader();
                while (reader.Read())
                {
                    if (sqlCache.Length > 0)
                        sqlCache += ";";
                    string schema = reader.GetString(0);
                    string tblName = reader.GetString(1);
                    sqlCache += string.Format("SELECT * FROM [{0}].[{1}].[{2}]", db, schema, tblName);
                }
                reader.Close();
            }
        }
        finally
        {
            if (reader != null)
                reader.Close();
            try
            {
                SqlCommand cmd = new SqlCommand("USE " + current_db, m_conn, m_trans);
                cmd.ExecuteNonQuery();
            }
            finally { }
        }
        return sqlCache;
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
            if ((sql == null || sql.Length == 0) && m_EnableMessages)
            {
                sql = GenerateSqlForCachedTables();
            }
            SqlCommand cmd = new SqlCommand(sql, m_conn, m_trans);
            if (rowset)
            {
                reader = cmd.ExecuteReader(meta ? CommandBehavior.KeyInfo : CommandBehavior.Default);
                while (reader.FieldCount > 0 && ok)
                {
                    ok = PushToClient(reader, meta);
                    HeaderSent = true;
                    if (!ok || !reader.NextResult())
                        break;
                }
                if (reader.RecordsAffected > 0)
                    affected += reader.RecordsAffected;
                reader.Close();
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

    private void SetVParam(CDBVariantArray vAll, uint parameters, uint pos, uint ps)
    {
        m_vParam.Clear();
        uint rows = (uint)vAll.Count / parameters;
        for (uint r = 0; r < rows; ++r)
        {
            for (uint p = 0; p < ps; ++p)
            {
                object vt = vAll[(int)(parameters * r + pos + p)];
                m_vParam.Add(vt);
            }
        }
    }

    private CParameterInfoArray GetVInfo(CParameterInfoArray vPInfo, uint pos, uint ps)
    {
        CParameterInfoArray v = new CParameterInfoArray();
        int count = vPInfo.Count;
        for (int n = 0; n < (int)ps; ++n)
        {
            v.Add(vPInfo[(int)pos + n]);
        }
        return v;
    }

    [RequestAttr(DB_CONSTS.idExecuteBatch, true)]
    private ulong ExecuteBatch(string sql, string delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, string dbConn, uint flags, ulong callIndex, out long affected, out int res, out string errMsg, out object vtId)
    {
        if (sql == null)
            sql = "";
        if (dbConn == null)
            dbConn = "";
        if (delimiter == null)
            delimiter = "";
        m_indexCall = callIndex;
        affected = 0;
        res = 0;
        ulong fail_ok = 0;
        CParameterInfoArray vPInfo = new CParameterInfoArray();
        vPInfo.LoadFrom(UQueue);
        vtId = null;
        do
        {
            if (m_defaultDB.Length == 0)
            {
                int ms = Open(dbConn, flags, out res, out errMsg);
                if (res == 0)
                    errMsg = m_conn.Database;
            }
            else
            {
                errMsg = m_conn.Database;
            }
            uint parameters = 0;
            List<string> vSql = Split(sql, delimiter);
            foreach (string s in vSql)
            {
                parameters += ComputeParameters(s);
            }
            if (m_defaultDB == "")
            {
                fail_ok = (uint)vSql.Count;
                fail_ok <<= 32;
                SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
                break;
            }
            uint rows = 0;
            if (parameters > 0)
            {
                if (m_vParam.Count == 0)
                {
                    res = -2;
                    errMsg = "No parameter specified";
                    m_fails += (uint)vSql.Count;
                    fail_ok = (uint)vSql.Count;
                    fail_ok <<= 32;
                    SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
                    break;
                }
                if (((uint)m_vParam.Count % parameters) > 0)
                {
                    res = -2;
                    errMsg = "Bad parameter data array size";
                    m_fails += (uint)vSql.Count;
                    fail_ok = (uint)vSql.Count;
                    fail_ok <<= 32;
                    SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
                    break;
                }
                rows = (uint)m_vParam.Count / parameters;
                if ((uint)vPInfo.Count != parameters)
                {
                    res = -2;
                    errMsg = "No proper parameter information array not provided";
                    m_fails += (uint)vSql.Count;
                    fail_ok = (uint)vSql.Count;
                    fail_ok <<= 32;
                    SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
                    break;
                }
            }
            if (isolation != (int)tagTransactionIsolation.tiUnspecified)
            {
                int ms = BeginTrans(isolation, dbConn, flags, out res, out errMsg);
                if (res != 0)
                {
                    m_fails += (uint)vSql.Count;
                    fail_ok = (uint)vSql.Count;
                    fail_ok <<= 32;
                    SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
                    break;
                }
                else if (IsCanceled || !Connected)
                    return fail_ok;
            }
            uint ret = SendResult(DB_CONSTS.idSqlBatchHeader, res, errMsg, (int)tagManagementSystem.msMsSQL, parameters, callIndex);
            if (ret == CClientPeer.SOCKET_NOT_FOUND || ret == CClientPeer.REQUEST_CANCELED)
                return fail_ok;
            errMsg = "";
            CDBVariantArray vAll = m_vParam;
            m_vParam = new CDBVariantArray();
            long aff = 0;
            int r = 0;
            string err = "";
            ulong fo = 0;
            uint pos = 0;
            foreach (string it in vSql)
            {
                uint ps = ComputeParameters(it);
                string s = it.Trim(' ', '\t', '\r', '\n', ';', '@');
                if (ps > 0)
                {
                    CParameterInfoArray vP = GetVInfo(vPInfo, pos, ps);
                    //prepared statements
                    uint my_ps = Prepare(s, vP, out r, out err);
                    if (IsCanceled || !Connected)
                        return fail_ok;
                    if (r != 0)
                    {
                        fail_ok += (((ulong)rows) << 32);
                    }
                    else
                    {
                        SetVParam(vAll, parameters, pos, ps);
                        uint nParamPos = (pos << 16) + ps;
                        ret = SendResult(DB_CONSTS.idParameterPosition, nParamPos);
                        if (ret == CClientPeer.SOCKET_NOT_FOUND || ret == CClientPeer.REQUEST_CANCELED)
                            return fail_ok;
                        fo = ExecuteParameters(rowset, meta, lastInsertId, callIndex, out aff, out r, out err, out vtId);
                    }
                    pos += ps;
                }
                else
                {
                    fo = Execute(s, rowset, meta, lastInsertId, callIndex, out aff, out r, out err, out vtId);
                }
                if (r != 0 && res == 0)
                {
                    res = r;
                    errMsg = err;
                }
                affected += aff;
                fail_ok += fo;
                if (IsCanceled || !Connected)
                    return fail_ok;
                if (r != 0 && isolation != (int)tagTransactionIsolation.tiUnspecified && plan == (int)tagRollbackPlan.rpDefault)
                    break;
            }
            if (isolation != (int)tagTransactionIsolation.tiUnspecified)
            {
                err = EndTrans(plan, out r);
                if (r != 0 && res == 0)
                {
                    res = r;
                    errMsg = err;
                }
            }
        } while (false);
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
        bool ok = true;
        bool HeaderSent = false;
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
            if (m_trans != null)
                m_sqlPrepare.Transaction = m_trans;
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
                            ok = PushToClient(reader, meta);
                            HeaderSent = true;
                            if (reader.RecordsAffected > 0)
                                affected += reader.RecordsAffected;
                            if (!ok || !reader.NextResult())
                                break;
                        }
                        reader.Close();
                    }
                    else
                    {
                        int ret = m_sqlPrepare.ExecuteNonQuery();
                        if (ret > 0)
                            affected += ret;
                    }
                    if (ok && m_outputs > 0)
                    {
                        CDBColumnInfoArray v = new CDBColumnInfoArray();
                        uint ret = SendResult(DB_CONSTS.idRowsetHeader, v, index, (uint)m_outputs);
                        ok = (ret != SOCKET_NOT_FOUND && ret != REQUEST_CANCELED);
                        HeaderSent = true;
                        if (ok)
                        {
                            using (CScopeUQueue sb = new CScopeUQueue())
                            {
                                CUQueue q = sb.UQueue;
                                foreach (SqlParameter p in m_sqlPrepare.Parameters)
                                {
                                    if (p.Direction != ParameterDirection.Input)
                                    {
                                        q.Save(p.Value);
                                    }
                                }
                                ok = (SendResult(DB_CONSTS.idOutputParameter, q.IntenalBuffer, q.GetSize()) == q.GetSize());
                            }
                        }
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
                if (!ok)
                    break;
            }
        } while (false);
        if (!HeaderSent && ok)
        {
            CDBColumnInfoArray v = new CDBColumnInfoArray();
            SendResult(DB_CONSTS.idRowsetHeader, v, index);
        }
        fail_ok = ((m_fails - fails) << 32);
        fail_ok += (m_oks - oks);
        m_vParam.Clear();
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
            m_sqlPrepare = new SqlCommand(sql, m_conn, m_trans);
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
                            {
                                if (info.Direction == tagParameterDirection.pdInput)
                                    param = new SqlParameter(info.ParameterName, SqlDbType.Image, int.MaxValue);
                                else
                                    param = new SqlParameter(info.ParameterName, SqlDbType.Image, (int)DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
                            }
                            else
                                param = new SqlParameter(info.ParameterName, SqlDbType.VarBinary, (int)info.ColumnSize);
                            break;
                        case tagVariantDataType.sdVT_STR:
                        case tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_I1:
                            if (info.ColumnSize == uint.MaxValue)
                            {
                                if (info.Direction == tagParameterDirection.pdInput)
                                    param = new SqlParameter(info.ParameterName, SqlDbType.Text, int.MaxValue);
                                else
                                    param = new SqlParameter(info.ParameterName, SqlDbType.Text, (int)DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
                            }
                            else
                                param = new SqlParameter(info.ParameterName, SqlDbType.VarChar, (int)info.ColumnSize);
                            break;
                        case tagVariantDataType.sdVT_WSTR:
                        case tagVariantDataType.sdVT_BSTR:
                            if (info.ColumnSize == uint.MaxValue)
                            {
                                if (info.Direction == tagParameterDirection.pdInput)
                                    param = new SqlParameter(info.ParameterName, SqlDbType.NText, int.MaxValue);
                                else
                                    param = new SqlParameter(info.ParameterName, SqlDbType.NText, (int)DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
                            }
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
                        case tagVariantDataType.sdVT_UI1:
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
                        case tagVariantDataType.sdVT_VARIANT:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Variant);
                            break;
                        case tagVariantDataType.sdVT_XML:
                            param = new SqlParameter(info.ParameterName, SqlDbType.Xml);
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
            if (m_outputs > 0)
                m_sqlPrepare.CommandType = CommandType.StoredProcedure;
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
                if (m_sqlPrepare != null)
                    m_sqlPrepare.Transaction = null;
            }
        } while (false);
        return errMsg;
    }

    [RequestAttr(DB_CONSTS.idBeginTrans, true)]
    private int BeginTrans(int isolation, string strConnection, uint flags, out int res, out string errMsg)
    {
        res = 0;
        errMsg = m_conn.Database;
        if (strConnection == null)
            strConnection = "";
        int ms = (int)tagManagementSystem.msMsSQL;
        if (m_defaultDB.Length == 0 && strConnection.Length > 0)
        {
            ms = Open(strConnection, flags, out res, out errMsg);
            if (res != 0)
                return ms;
            errMsg = strConnection;
        }
        if (m_trans != null)
        {
            res = -2;
            errMsg = "Transaction already started";
            return ms;
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
        m_defaultDB = m_conn.Database;
        if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            m_EnableMessages = Push.Subscribe(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
        if (strConnection != null && strConnection.Length > 0)
        {
            try
            {
                string sql = "USE " + strConnection;
                SqlCommand cmd = new SqlCommand(sql, m_conn);
                int affected = cmd.ExecuteNonQuery();
                errMsg = strConnection;
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
        else
        {
            errMsg = "";
            errMsg = m_defaultDB;
        }
        return (int)tagManagementSystem.msMsSQL;
    }

    [RequestAttr(DB_CONSTS.idStartBLOB)]
    private void StartBLOB(uint lenExpected)
    {
        m_Blob.SetSize(0);
        if (lenExpected > m_Blob.MaxBufferSize)
            m_Blob.Realloc(lenExpected);
        m_Blob.Push(UQueue.IntenalBuffer, UQueue.HeadPosition, UQueue.GetSize());
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

    [RequestAttr(DB_CONSTS.idClose)]
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
            }
        }
        ResetMemories();
        if (m_EnableMessages)
        {
            Push.Unsubscribe();
            m_EnableMessages = false;
        }
        m_defaultDB = "";
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
