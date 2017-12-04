
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

    [SqlFunction(DataAccess=DataAccessKind.Read)]
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

    private static void PublishInsert(string tablepath, string[] v, SqlConnection conn)
    {
        SqlDataReader reader = null;
        try
        {
            int index = 0;
            SqlCommand cmd = new SqlCommand("SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            DataTable dt = reader.GetSchemaTable();
            List<int> vKeyOrdinal = new List<int>();
            foreach (DataRow dr in dt.Rows)
            {
                bool isKey = (bool)dr["IsKey"];
                if (!isKey)
                    isKey = (bool)dr["IsAutoIncrement"];
                if (isKey)
                    vKeyOrdinal.Add(index);
                ++index;
            }
            if (vKeyOrdinal.Count == 0)
                return;
            int count = reader.FieldCount;
            int total = 2 + v.Length + count;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueInsert;
            msg[1] = v[0];
            msg[2] = v[1];
            msg[3] = v[2];
            msg[4] = tablepath;
            if (reader.Read())
            {
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
                lock (m_cs)
                {
                    CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
                }
            }
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    private static void PublishUpdate(string tablepath, string[] v, SqlConnection conn)
    {
        SqlDataReader reader = null;
        try
        {
            int index = 0;
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED;SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            DataTable dt = reader.GetSchemaTable();
            List<int> vKeyOrdinal = new List<int>();
            foreach (DataRow dr in dt.Rows)
            {
                bool isKey = (bool)dr["IsKey"];
                if (!isKey)
                    isKey = (bool)dr["IsAutoIncrement"];
                if (isKey)
                    vKeyOrdinal.Add(index);
                ++index;
            }
            if (vKeyOrdinal.Count == 0)
                return;
            int count = reader.FieldCount;
            int total = 2 + v.Length + count * 2;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueUpdate;
            msg[1] = v[0];
            msg[2] = v[1];
            msg[3] = v[2];
            msg[4] = tablepath;
            do
            {
                if (!reader.Read())
                    break;
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
                    break;
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
                lock (m_cs)
                {
                    CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
                }
            } while (false);
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    private static void PublishDelete(string tablepath, string[] v, SqlConnection conn)
    {
        SqlDataReader reader = null;
        try
        {
            int index = 0;
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            DataTable dt = reader.GetSchemaTable();
            List<int> vKeyOrdinal = new List<int>();
            foreach (DataRow dr in dt.Rows)
            {
                bool isKey = (bool)dr["IsKey"];
                if (!isKey)
                    isKey = (bool)dr["IsAutoIncrement"];
                if (isKey)
                    vKeyOrdinal.Add(index);
                ++index;
            }
            do
            {
                if (vKeyOrdinal.Count == 0)
                    break;
                int total = 2 + v.Length + vKeyOrdinal.Count;
                object[] msg = new object[total];
                msg[0] = (int)tagUpdateEvent.ueDelete;
                msg[1] = v[0];
                msg[2] = v[1];
                msg[3] = v[2];
                msg[4] = tablepath;
                if (reader.Read())
                {
                    index = 0;
                    foreach (int ordinal in vKeyOrdinal)
                    {
                        Object data = reader.GetValue(ordinal);
                        int n = 5 + index;
                        if (data is DateTimeOffset)
                            msg[n] = data.ToString();
                        else if (data is TimeSpan)
                            msg[n] = data.ToString();
                        else if (data is SqlXml)
                            msg[n] = data.ToString();
                        else
                            msg[n] = data;
                        ++index;
                    }
                    lock (m_cs)
                    {
                        CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
                    }
                }
            } while (false);
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
    }

    /// <summary>
    /// Call this method from your trigger code to push data of table events (DELETE, INSERT and UPDATE) onto clients
    /// </summary>
    /// <param name="tableName">A string for table name</param>
    /// <param name="schema">A string for schema which defaults to dbo</param>
    /// <returns>An enmpty string if successful. Otherwise, an error message returned</returns>
    public static string PublishDBEvent(string tableName, string schema = "dbo")
    {
        if (schema == null || schema.Length == 0)
            schema = "dbo";
        if (tableName == null || tableName.Length == 0)
            return "Non empty table name string required";
        string tablepath = string.Format("[{0}].[{1}]", schema, tableName);
        SqlTriggerContext tc = SqlContext.TriggerContext;
        if (!SqlContext.IsAvailable || tc == null)
            return "Trigger context not available";
        string errMsg = "";
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            try
            {
                conn.Open();
                string[] v = MsSql.Utilities.GetUSqlServerKeys(conn);
                switch (tc.TriggerAction)
                {
                    case TriggerAction.Update:
                        PublishUpdate(tablepath, v, conn);
                        break;
                    case TriggerAction.Delete:
                        PublishDelete(tablepath, v, conn);
                        break;
                    case TriggerAction.Insert:
                        PublishInsert(tablepath, v, conn);
                        break;
                    default:
                        break;
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
        return errMsg;
    }
}

