
using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using SocketProAdapter.UDB;
using SocketProAdapter.ServerSide;
using System.IO;

public static class USqlStream
{
    private static CSqlPlugin Plugin = null;
    private static object m_cs = new object();

    static USqlStream()
    {

    }

    public static CSocketProServer Server
    {
        get
        {
            lock (m_cs)
            {
                return Plugin;
            }
        }
    }

    [SqlProcedure()]
    public static void StopSPServer(out SqlInt32 res)
    {
        res = 0;
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            try
            {
                conn.Open();
                lock (m_cs)
                {
                    if (CSqlPlugin.Running)
                    {
                        Plugin.StopSocketProServer();
                        res += 10;
                    }
                    Plugin = null;
                }
                res += 1;
            }
            catch (Exception err)
            {
                LogError(conn, err.Message);
            }
            finally
            {
                conn.Close();
            }
        }
    }

    [SqlProcedure()]
    public static void StartSPServer(out SqlInt32 res)
    {
        res = 0;
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            try
            {
                lock (m_cs)
                {
                    if (Plugin == null)
                    {
                        res += 100;
                        if (!Directory.Exists(SQLConfig.WorkingDirectory))
                            Directory.CreateDirectory(SQLConfig.WorkingDirectory);
                        Directory.SetCurrentDirectory(SQLConfig.WorkingDirectory);
                        Plugin = new CSqlPlugin(SQLConfig.Param);
                    }
                    if (!CSqlPlugin.Running)
                    {
                        res += 10;
                        if (SQLConfig.StoreOrPfx != null && SQLConfig.SubjectOrPassword != null && SQLConfig.StoreOrPfx.Length > 0 && SQLConfig.SubjectOrPassword.Length > 0)
                        {
                            if (SQLConfig.StoreOrPfx.IndexOf(".pfx") == -1)
                            {
                                //load cert and private key from windows system cert store
                                Plugin.UseSSL(SQLConfig.StoreOrPfx/*"my"*/, SQLConfig.SubjectOrPassword, "");
                            }
                            else
                            {
                                Plugin.UseSSL(SQLConfig.StoreOrPfx, "", SQLConfig.SubjectOrPassword);
                            }
                        }
                        if (Plugin.Run(SQLConfig.Port, 16, !SQLConfig.NoV6))
                            res += 1;
                    }
                }
            }
            catch (Exception err)
            {
                LogError(conn, err.Message);
            }
            finally
            {
                conn.Close();
            }
        }
    }

    private static object[] PublishInsert(SqlConnection conn, out DataTable dt)
    {
        dt = null;
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            dt = reader.GetSchemaTable();
            int count = reader.FieldCount;
            int total = 5 + count;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueInsert;
            if (!reader.Read())
                return null;
            for (int n = 5; n < total; ++n)
            {
                msg[n] = GetData(reader, n - 5);
            }
            return msg;
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    private static object GetData(SqlDataReader reader, int ordinal)
    {
        if (reader.IsDBNull(ordinal))
            return null;
        string native = reader.GetDataTypeName(ordinal);
        if (native.IndexOf(".") != -1)
            return reader.GetValue(ordinal).ToString(); //customer defined data types, geometry, and geography
        else if (native == "xml")
            return reader.GetSqlXml(ordinal).ToString();
        else if (native == "datetimeoffset")
            return reader.GetDateTimeOffset(ordinal).ToString();
        return reader.GetValue(ordinal);
    }

    private static object[] PublishUpdate(SqlConnection conn, out DataTable dt)
    {
        dt = null;
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED;SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            dt = reader.GetSchemaTable();
            int count = reader.FieldCount;
            int total = 5 + count * 2;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueUpdate;
            if (!reader.Read())
                return null;
            for (int ordinal = 0; ordinal < count; ++ordinal)
            {
                int n = 5 + ordinal * 2;
                msg[n] = GetData(reader, ordinal);
            }
            reader.NextResult();
            if (!reader.Read())
                return null;
            for (int ordinal = 0; ordinal < count; ++ordinal)
            {
                int n = 6 + ordinal * 2;
                msg[n] = GetData(reader, ordinal);
            }
            return msg;
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    private static object[] PublishDelete(SqlConnection conn, out DataTable dt)
    {
        dt = null;
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            dt = reader.GetSchemaTable();
            int count = reader.FieldCount;
            int total = 5 + count;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueDelete;
            if (!reader.Read())
                return null;
            for (int n = 5; n < total; ++n)
            {
                msg[n] = GetData(reader, n - 5);
            }
            return msg;
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    private static string[] GetUSqlServerKeys(SqlConnection conn)
    {
        Exception ex = null;
        if (conn == null || conn.State != ConnectionState.Open)
            throw new InvalidOperationException("An opened connection required");
        string[] v = null;
        SqlDataReader dr = null;
        string sqlCmd = "select SUSER_NAME(),DB_NAME()";
        try
        {
            SqlCommand cmd = new SqlCommand(sqlCmd, conn);
            dr = cmd.ExecuteReader();
            if (dr.Read())
            {
                v = new string[2];
                v[0] = dr.GetString(0);
                v[1] = dr.GetString(1);
            }
        }
        catch (Exception err)
        {
            ex = err;
        }
        finally
        {
            if (dr != null)
                dr.Close();
        }
        if (ex != null)
            throw ex;
        return v;
    }

    private static string GuessTablePath(SqlConnection conn, DataTable dt)
    {
        SqlDataReader dr = null;
        string sql = "select OBJECT_NAME(parent_id),object_schema_name(parent_id)from sys.assembly_modules as am,sys.triggers as t where t.object_id=am.object_id and assembly_method like 'PublishDMLEvent%' and assembly_class='USqlStream'";
        try
        {
            List<KeyValuePair<string, string>> v = new List<KeyValuePair<string, string>>();
            SqlCommand cmd = new SqlCommand(sql, conn);
            dr = cmd.ExecuteReader();
            while (dr.Read())
            {
                KeyValuePair<string, string> p = new KeyValuePair<string, string>(dr.GetString(0), dr.GetString(1));
                v.Add(p);
            }
            dr.Close();
            dr = null;
            foreach (KeyValuePair<string, string> p in v)
            {
                cmd.CommandText = string.Format("select COUNT(TABLE_NAME)as cols from information_schema.columns where TABLE_NAME='{0}' and TABLE_SCHEMA='{1}' group by TABLE_SCHEMA", p.Key, p.Value);
                object obj = cmd.ExecuteScalar();
                if (obj == null || obj is DBNull)
                    continue;
                int cols = int.Parse(obj.ToString());
                if (cols != dt.Rows.Count)
                    continue;
                cmd.CommandText = string.Format("select COLUMN_NAME,DATA_TYPE from information_schema.columns where TABLE_NAME='{0}' and TABLE_SCHEMA='{1}' order by ordinal_position", p.Key, p.Value);
                dr = cmd.ExecuteReader();
                int row = 0;
                bool eq = true;
                while (dr.Read())
                {
                    string col_name = dr.GetString(0);
                    string data_type = dr.GetString(1);
                    DataRow myrow = dt.Rows[row];
                    string cName = (string)myrow["ColumnName"];
                    string dType = (string)myrow["DataTypeName"];
                    if (col_name != cName || data_type != dType)
                    {
                        eq = false;
                        break;
                    }
                    ++row;
                }
                if (eq)
                    return p.Value + "." + p.Key;
            }
        }
        finally
        {
            if (dr != null)
                dr.Close();
        }
        return "";
    }

    public static int LogError(SqlConnection conn, string errMsg)
    {
        if (errMsg == null)
            errMsg = "";
        if (conn == null || conn.State != ConnectionState.Open)
            return -1;
        try
        {
            string sql = string.Format("UPDATE sp_streaming_db.dbo.config set value='{0}' WHERE mykey='{1}'", errMsg, "usql_streaming_last_error");
            SqlCommand cmd = new SqlCommand(sql, conn);
            return cmd.ExecuteNonQuery();
        }
        finally
        {
        }
    }

    public static void PublishDMLEvent()
    {
        SqlTriggerContext tc = SqlContext.TriggerContext;
        if (!SqlContext.IsAvailable || tc == null)
            return;
        string errMsg = "";
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            try
            {
                if (CSocketProServer.Running)
                {
                    conn.Open();
                    string[] v = GetUSqlServerKeys(conn);
                    object[] msg = null;
                    DataTable dt = null;
                    switch (tc.TriggerAction)
                    {
                        case TriggerAction.Update:
                            msg = PublishUpdate(conn, out dt);
                            if (dt == null)
                                errMsg = "DELETED schema table not available";
                            break;
                        case TriggerAction.Delete:
                            msg = PublishDelete(conn, out dt);
                            if (dt == null)
                                errMsg = "DELETED schema table not available";
                            break;
                        case TriggerAction.Insert:
                            msg = PublishInsert(conn, out dt);
                            if (dt == null)
                                errMsg = "INSERTED schema table not available";
                            break;
                        default:
                            errMsg = "Unknown DML event";
                            break;
                    }
                    if (msg == null && errMsg.Length == 0)
                        errMsg = "Trigger record not obtained";
                    if (dt != null && msg != null)
                    {
                        msg[1] = SQLConfig.Server;
                        msg[2] = v[0];
                        msg[3] = v[1];
                        string tblName = GuessTablePath(conn, dt);
                        if (tblName != null && tblName.Length > 0)
                        {
                            msg[4] = tblName;
                            lock (m_cs)
                            {
                                if (!CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID))
                                    errMsg = "Message publishing failed";
                            }
                        }
                        else
                        {
                            errMsg = "Triggered table not guessed out";
                        }
                    }
                }
                else
                    errMsg = "MS SQL streaming plugin not running";
            }
            catch (Exception err)
            {
                errMsg = err.Message;
            }
            finally
            {
                //LogError(conn, errMsg);
                conn.Close();
            }
        }
        SqlContext.Pipe.Send(errMsg);
    }
}

