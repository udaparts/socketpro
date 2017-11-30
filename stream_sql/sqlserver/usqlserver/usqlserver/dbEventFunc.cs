using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using SocketProAdapter;
using SocketProAdapter.UDB;
using SocketProAdapter.ServerSide;

public static class DBEvents
{
    private static readonly string NOT_SUPPORTED;
    
    static DBEvents()
    {
        NOT_SUPPORTED = "Not supported";
    }

    private static void PublishInsert(string[] v, SqlConnection conn)
    {
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            int count = reader.FieldCount;
            int total = 2 + v.Length + count;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueInsert;
            msg[1] = v[0];
            msg[2] = v[1];
            msg[3] = v[2];
            DataTable dt = reader.GetSchemaTable();
            msg[4] = dt.Rows[0]["BaseSchemaName"] + "." + dt.Rows[0]["BaseTableName"];
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

    private static void PublishUpdate(string[] v, SqlConnection conn)
    {
        SqlDataReader reader = null;
        try
        {
            SqlCommand cmd = new SqlCommand("SELECT * FROM DELETED;SELECT * FROM INSERTED", conn);
            reader = cmd.ExecuteReader(CommandBehavior.KeyInfo);
            int count = reader.FieldCount;
            int total = 2 + v.Length + count * 2;
            object[] msg = new object[total];
            msg[0] = (int)tagUpdateEvent.ueUpdate;
            msg[1] = v[0];
            msg[2] = v[1];
            msg[3] = v[2];
            DataTable dt = reader.GetSchemaTable();
            msg[4] = dt.Rows[0]["BaseSchemaName"] + "." + dt.Rows[0]["BaseTableName"];
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

    private static void PublishDelete(string[] v, SqlConnection conn)
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
                msg[4] = dt.Rows[0]["BaseSchemaName"] + "." + dt.Rows[0]["BaseTableName"];
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

    [Microsoft.SqlServer.Server.SqlFunction(DataAccess = DataAccessKind.Read)]
    public static string PublishDBEvent()
    {
        SqlTriggerContext tc = SqlContext.TriggerContext;
        if (!SqlContext.IsAvailable || tc == null)
            return NOT_SUPPORTED;
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
                        PublishUpdate(v, conn);
                        break;
                    case TriggerAction.Delete:
                        PublishDelete(v, conn);
                        break;
                    case TriggerAction.Insert:
                        PublishInsert(v, conn);
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

