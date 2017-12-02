
using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using SocketProAdapter.UDB;
using SocketProAdapter.ServerSide;

public static class SQLPlugin
{
    private static CSqlPlugin Plugin = null;
    static SQLPlugin()
    {
        
    }

    public static CSocketProServer Server
    {
        get
        {
            return Plugin;
        }
    }

    public static SqlInt32 StartSPServer(int param, string store_or_pfx = "", string subject_or_password = "")
    {
        int n = 1000;
        if (Plugin == null)
        {
            n += 100;
            Plugin = new CSqlPlugin(param);
        }
        if (!CSqlPlugin.Running)
        {
            n += 10;
            if (store_or_pfx != null && subject_or_password != null && store_or_pfx.Length > 0 && subject_or_password.Length > 0) {
                if (store_or_pfx.IndexOf(".pfx") == -1)
                {
                    //load cert and private key from windows system cert store
                    Plugin.UseSSL(store_or_pfx/*"my"*/, subject_or_password, "");
                }
                else
                {
                    Plugin.UseSSL(store_or_pfx, "", subject_or_password);
                }
            }
            if (Plugin.Run(20903))
                n += 1;
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
                CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
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
                CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
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

    /// <summary>
    /// Call this method from your trigger code to push data of table events (DELETE, INSERT and UPDATE) onto clients
    /// </summary>
    /// <param name="tableName">A string for table name</param>
    /// <param name="schema">A string for schema which defaults to dbo</param>
    /// <returns></returns>
    public static string PublishDBEvent(string tableName, string schema="dbo")
    {
        if (schema == null || schema.Length == 0)
            schema = "dbo";
        if (Plugin == null || !CSqlPlugin.Running)
            return "SocketPro server not started yet";
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

