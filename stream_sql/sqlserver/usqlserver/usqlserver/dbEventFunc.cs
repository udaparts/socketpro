
using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using SocketProAdapter.UDB;
using SocketProAdapter.ServerSide;
using System.IO;

public static class SQLPlugin
{
    private static CSqlPlugin Plugin = null;
    private static object m_cs = new object();

    static SQLPlugin()
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

    [SqlFunction(DataAccess = DataAccessKind.Read)]
    public static SqlInt32 StopSPServer()
    {
        int n = 100;
        do
        {
            lock (m_cs)
            {
                if (Plugin == null)
                    break;
                if (CSqlPlugin.Running)
                {
                    Plugin.StopSocketProServer();
                    n += 10;
                }
                Plugin.Dispose();
                Plugin = null;
            }
            n += 1;
        } while (false);
        return n;
    }

    [SqlFunction(DataAccess = DataAccessKind.Read)]
    public static SqlInt32 StartSPServer()
    {
        int n = 1000;
        lock (m_cs)
        {
            if (Plugin == null)
            {
                n += 100;
                if (!Directory.Exists(SQLConfig.WorkingDirectory))
                    Directory.CreateDirectory(SQLConfig.WorkingDirectory);
                Directory.SetCurrentDirectory(SQLConfig.WorkingDirectory);
                Plugin = new CSqlPlugin(SQLConfig.Param);
            }
            if (!CSqlPlugin.Running)
            {
                n += 10;
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
                    n += 1;
            }
        }
        return n;
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
            if (reader.Read())
                return null;
            for (int n = 5; n < total; ++n)
            {
                Object data = reader.GetValue(n - 5);
                if (data is DateTimeOffset)
                    msg[n] = data.ToString();
                else if (data is TimeSpan)
                    msg[n] = data.ToString();
                else if (data is SqlXml)
                    msg[n] = data.ToString();
                else
                    msg[n] = data;
            }
            return msg;
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
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
                Object data = reader.GetValue(ordinal);
                if (data is DateTimeOffset)
                    msg[n] = data.ToString();
                else if (data is TimeSpan)
                    msg[n] = data.ToString();
                else if (data is SqlXml)
                    msg[n] = data.ToString();
                else
                    msg[n] = data;
            }
            reader.NextResult();
            if (!reader.Read())
                return null;
            for (int ordinal = 0; ordinal < count; ++ordinal)
            {
                int n = 6 + ordinal * 2;
                Object data = reader.GetValue(ordinal);
                if (data is DateTimeOffset)
                    msg[n] = data.ToString();
                else if (data is TimeSpan)
                    msg[n] = data.ToString();
                else if (data is SqlXml)
                    msg[n] = data.ToString();
                else
                    msg[n] = data;
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
            if (reader.Read())
                return null;
            for (int n = 5; n < total; ++n)
            {
                Object data = reader.GetValue(n - 5);
                if (data is DateTimeOffset)
                    msg[n] = data.ToString();
                else if (data is TimeSpan)
                    msg[n] = data.ToString();
                else if (data is SqlXml)
                    msg[n] = data.ToString();
                else
                    msg[n] = data;
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
        string sqlCmd = "select SUSER_NAME(), DB_NAME()";
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

    private static string GuessTablePath(SqlConnection conn, string dbName, DataTable dt)
    {
        SqlDataReader dr = null;
        string sql = string.Format("select object_name(parent_id), object_schema_name(parent_id) from [{0}].sys.triggers where type='TA' and is_disabled=0 and is_instead_of_trigger=0 and is_ms_shipped=0 and parent_class=1", dbName);
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
                cmd.CommandText = string.Format("select COUNT(TABLE_NAME) as cols from information_schema.columns where TABLE_NAME='{0}' and TABLE_SCHEMA='{1}' group by TABLE_SCHEMA", p.Key, p.Value);
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

    public static void PublishDBEventEx(string tablePath)
    {
        SqlTriggerContext tc = SqlContext.TriggerContext;
        if (!SqlContext.IsAvailable || tc == null || Plugin == null || !CSocketProServer.Running)
            return;
        string errMsg = "";
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            try
            {
                conn.Open();
                string[] v = GetUSqlServerKeys(conn);
                object[] msg = null;
                DataTable dt = null;
                switch (tc.TriggerAction)
                {
                    case TriggerAction.Update:
                        msg = PublishUpdate(conn, out dt);
                        break;
                    case TriggerAction.Delete:
                        msg = PublishDelete(conn, out dt);
                        break;
                    case TriggerAction.Insert:
                        msg = PublishInsert(conn, out dt);
                        break;
                    default:
                        errMsg = "Unknown DML event";
                        break;
                }
                errMsg = "Unknown error";
                if (dt != null && msg != null)
                {
                    msg[1] = SQLConfig.Server;
                    msg[2] = v[0];
                    msg[3] = v[1];
                    if (tablePath == null || tablePath.Length == 0)
                        tablePath = GuessTablePath(conn, v[1], dt);
                    if (tablePath != null && tablePath.Length > 0)
                    {
                        msg[4] = tablePath;
                        lock (m_cs)
                        {
                            CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
                            errMsg = "";
                        }
                    }
                }
            }
            catch (Exception err)
            {
                errMsg = err.Message;
            }
            finally
            {
                conn.Close();
            }
        }
        SqlContext.Pipe.Send(errMsg);
    }

    public static void PublishDBEvent()
    {
        PublishDBEventEx("");
    }
}

