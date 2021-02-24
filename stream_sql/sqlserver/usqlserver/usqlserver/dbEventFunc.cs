﻿
using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using SocketProAdapter;
using SocketProAdapter.UDB;
using SocketProAdapter.ServerSide;
using System.IO;

public static class USqlStream
{
    private static CSqlPlugin Plugin = null;
    private static object m_cs = new object();
    private static string ServerHost = null;

    public static CSqlPlugin Server
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
                    if (Plugin != null)
                    {
                        if (ServerCoreLoader.IsRunning())
                        {
                            ServerCoreLoader.SetOnIdle(null);
                            Plugin.StopSocketProServer();
                            res += 1;
                        }
                        Plugin.Dispose();
                        Plugin = null;
                        res += 1;
                    }
                }
            }
            catch (Exception err)
            {
                UConfig.LogMsg(err.Message, "USqlStream::StopSPServer", 57); //line 57
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
                    UConfig config;
                    try
                    {
                        if (!Directory.Exists(UConfig.DEFAULT_WORKING_DIRECTORY))
                            Directory.CreateDirectory(UConfig.DEFAULT_WORKING_DIRECTORY);
                        Directory.SetCurrentDirectory(UConfig.DEFAULT_WORKING_DIRECTORY);
                    }
                    catch (Exception ex)
                    {
                        UConfig.LogMsg(ex.Message, "USqlStream::StartSPServer", 85); //line 85
                    }
                    finally { }

                    try
                    {
                        string json = System.IO.File.ReadAllText(UConfig.DEFAULT_WORKING_DIRECTORY + UConfig.STREAM_DB_CONFIG_FILE);
                        config = new UConfig(json);
                    }
                    catch(Exception ex)
                    {
                        UConfig.LogMsg(ex.Message, "USqlStream::StartSPServer", 96); //line 96
                        config = new UConfig();
                        UConfig.UpdateConfigFile(config);
                        UConfig.UpdateLog();
                    }
                    if (Plugin == null)
                    {
                        Plugin = new CSqlPlugin(config);
                    }
                    res = 1;
                    if (!ServerCoreLoader.IsRunning())
                    {
                        if (config.cert_root_store.Length > 0 && config.cert_subject_cn.Length > 0)
                        {
                            Plugin.UseSSL(config.cert_root_store, config.cert_subject_cn, "");
                        }
                        if (Plugin.Run(config.port, 16, !config.disable_ipv6))
                        {
                            res += 1;
                        }
                        else
                        {
                            res = 0;
                        }
                    }
                }
            }
            catch (Exception err)
            {
                UConfig.LogMsg(err.Message, "USqlStream::StartSPServer", 125); //line 125
                Plugin = null;
            }
            finally
            {
                if (res == 2 && ServerCoreLoader.IsRunning())
                {
                    AppDomain.CurrentDomain.DomainUnload += (sender, args) =>
                    {
                        ServerCoreLoader.StopSocketProServer();
                        Plugin = null;
                    };
                }
                conn.Close();
            }
        }
    }

    private static List<object[]> GetRows(SqlConnection conn, bool delete, out DataTable dt)
    {
        dt = null;
        SqlDataReader reader = null;
        List<object[]> v = new List<object[]>();
        try
        {
            SqlCommand cmd = new SqlCommand(delete ? "SELECT * FROM DELETED" : "SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            dt = reader.GetSchemaTable();
            int count = reader.FieldCount;
            while (reader.Read())
            {
                object[] r = new object[count + 5];
                r[0] = (delete ? (int)tagUpdateEvent.ueDelete : (int)tagUpdateEvent.ueInsert);
                for (int n = 0; n < count; ++n)
                {
                    r[n + 5] = GetData(reader, n);
                }
                v.Add(r);
            }
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
        return v;
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
        else if (native == "time")
            return reader.GetValue(ordinal).ToString();
        else if (native == "datetimeoffset")
            return reader.GetDateTimeOffset(ordinal).ToString();
        return reader.GetValue(ordinal);
    }

    private static List<object[]> GetUpdateRows(SqlConnection conn, out DataTable dt)
    {
        dt = null;
        SqlDataReader reader = null;
        List<object[]> rows = new List<object[]>();
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            dt = reader.GetSchemaTable();
            int count = reader.FieldCount;
            int total = 5 + count * 2;
            while (reader.Read())
            {
                object[] r = new object[total];
                r[0] = (int)tagUpdateEvent.ueUpdate;
                for (int n = 0; n < count; ++n)
                {
                    r[5 + 2 * n] = GetData(reader, n);
                }
                rows.Add(r);
            }
            reader.Close();
            cmd.CommandText = "SELECT * FROM INSERTED";
            reader = cmd.ExecuteReader();
            int index = 0;
            while (reader.Read())
            {
                object[] r = rows[index];
                for (int n = 0; n < count; ++n)
                {
                    r[6 + n * 2] = GetData(reader, n);
                }
                ++index;
            }
        }
        finally
        {
            if (reader != null)
                reader.Close();
        }
        return rows;
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
                    dType = dType.ToLower();
                    if (col_name != cName || dType.IndexOf(data_type) == -1)
                    {
                        eq = false;
                        break;
                    }
                    ++row;
                }
                if (dr != null)
                {
                    dr.Close();
                    dr = null;
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

    private static bool Publish(object Message, params uint[] Groups)
    {
        uint len;
        if (Groups == null)
            len = 0;
        else
            len = (uint)Groups.Length;
        using (CScopeUQueue su = new CScopeUQueue())
        {
            CUQueue q = su.UQueue;
            q.Save(Message);
            unsafe
            {
                fixed (byte* buffer = q.m_bytes)
                {
                    fixed (uint* p = Groups)
                    {
                        return ServerCoreLoader.SpeakPush(buffer, q.GetSize(), p, len);
                    }
                }
            }
        }
    }

    private static string GetServerName(SqlConnection conn)
    {
        if (conn == null || conn.State != ConnectionState.Open)
            throw new InvalidOperationException("An opened connection required");
        string serverName = Environment.MachineName;
        SqlDataReader dr = null;
        string sqlCmd = "SELECT @@servername";
        try
        {
            SqlCommand cmd = new SqlCommand(sqlCmd, conn);
            dr = cmd.ExecuteReader();
            if (dr.Read())
            {
                if (dr.IsDBNull(0))
                {
                    dr.Close();
                    sqlCmd = "SELECT @@SERVICENAME";
                    cmd.CommandText = sqlCmd;
                    dr = cmd.ExecuteReader();
                    if (dr.Read())
                        serverName += ("\\" + dr.GetString(0));
                }
                else
                    serverName = dr.GetString(0);
            }
        }
        finally
        {
            if (dr != null)
                dr.Close();
        }
        return serverName;
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
                DataTable dt = null;
                List<object[]> rows = null;
                string[] v = null;
                lock (m_cs)
                {
                    if (ServerCoreLoader.IsRunning())
                    {
                        conn.Open();
                        v = GetUSqlServerKeys(conn);
                        switch (tc.TriggerAction)
                        {
                            case TriggerAction.Update:
                                rows = GetUpdateRows(conn, out dt);
                                if (dt == null)
                                    errMsg = "DELETED schema table not available";
                                break;
                            case TriggerAction.Delete:
                                rows = GetRows(conn, true, out dt);
                                if (dt == null)
                                    errMsg = "DELETED schema table not available";
                                break;
                            case TriggerAction.Insert:
                                rows = GetRows(conn, false, out dt);
                                if (dt == null)
                                    errMsg = "INSERTED schema table not available";
                                break;
                            default:
                                errMsg = "Unknown DML event";
                                break;
                        }
                    }
                    do
                    {
                        if (dt == null)
                            break;
                        if (ServerHost == null || ServerHost.Length == 0)
                            ServerHost = GetServerName(conn);
                        if (ServerHost == null || ServerHost.Length == 0)
                        {
                            errMsg = "Server not available";
                            break;
                        }
                        string tblName = GuessTablePath(conn, dt);
                        if (tblName == null || tblName.Length == 0)
                        {
                            errMsg = "Table name not available";
                            break;
                        }
                        lock (m_cs)
                        {
                            foreach (object[] msg in rows)
                            {
                                msg[1] = ServerHost;
                                msg[2] = v[0];
                                msg[3] = v[1];
                                msg[4] = tblName;
                                if (!Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID))
                                {
                                    errMsg = "Message publishing failed";
                                    break;
                                }
                            }
                        }
                    } while (false);
                }
            }
            catch (Exception err)
            {
                errMsg = err.Message;
            }
            finally
            {
                conn.Close();
                if (errMsg.Length > 0)
                {
                    UConfig.LogMsg(errMsg, "USqkStream::PublishDMEvent", 470); //line 470
                }
            }
        }
    }
}
