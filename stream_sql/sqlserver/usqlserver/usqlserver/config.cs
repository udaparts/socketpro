using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;

public static class SQLConfig
{
    private static void CreateTableConfig(SqlConnection conn)
    {
        //table doesn't exist
        SqlCommand cmd = new SqlCommand("CREATE TABLE sp_streaming_db.dbo.config(mykey nvarchar(32)primary key,value nvarchar(max)not null)", conn);
        cmd.ExecuteNonQuery();

        string sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "disable_ipv6", m_bNoV6 ? 1 : 0);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "main_threads", m_Param);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "enable_http_websocket", m_bWebSocket ? 1 : 0);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "port", m_nPort);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "services", m_services);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "store_or_pfx", m_store_or_pfx);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "subject_or_password", m_subject_or_password);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();

        sqlInsert = string.Format("INSERT INTO sp_streaming_db.dbo.config VALUES('{0}','{1}')", "read_only", m_readOnly ? 1 : 0);
        cmd.CommandText = sqlInsert;
        cmd.ExecuteNonQuery();
    }

    private static void SetConfig(SqlDataReader reader)
    {
        while (reader.Read())
        {
            string key = reader.GetString(0);
            string value = reader.GetString(1);
            key = key.ToLower();
            switch (key)
            {
                case "disable_ipv6":
                    try
                    {
                        m_bNoV6 = ((int.Parse(value) == 0) ? false : true);
                    }
                    catch
                    {
                        m_bNoV6 = false;
                    }
                    break;
                case "read_only":
                    try
                    {
                        m_readOnly = ((int.Parse(value) == 0) ? false : true);
                    }
                    catch
                    {
                        m_readOnly = true;
                    }
                    break;
                case "main_threads":
                    try
                    {
                        m_Param = int.Parse(value);
                    }
                    catch
                    {
                        m_Param = 1;
                    }
                    break;
                case "enable_http_websocket":
                    try
                    {
                        m_bWebSocket = ((int.Parse(value) == 0) ? false : true);
                    }
                    catch
                    {
                        m_bWebSocket = false;
                    }
                    break;
                case "port":
                    try
                    {
                        m_nPort = uint.Parse(value);
                    }
                    catch
                    {
                        m_nPort = 20903;
                    }
                    break;
                case "services":
                    m_services = value;
                    break;
                case "store_or_pfx":
                    m_store_or_pfx = value;
                    break;
                case "subject_or_password":
                    m_subject_or_password = value;
                    break;
                default:
                    break; //ignored
            }
        }
    }

    static SQLConfig()
    {
        using (SqlConnection conn = new SqlConnection("context connection=true"))
        {
            SqlCommand cmd = null;
            SqlDataReader reader = null;
            try
            {
                conn.Open();
                cmd = new SqlCommand("SELECT * from sp_streaming_db.dbo.config", conn);
                reader = cmd.ExecuteReader();
                SetConfig(reader);
            }
            finally
            {
                conn.Close();
            }
        }
    }

    private static bool m_bNoV6 = false;
    public static bool NoV6
    {
        get
        {
            return m_bNoV6;
        }
    }

    private static bool m_readOnly = true;
    public static bool ReadOnly
    {
        get
        {
            return m_readOnly;
        }
    }

    private static bool m_bWebSocket = false;
    public static bool HttpWebSocket
    {
        get
        {
            return m_bWebSocket;
        }
    }

    private static uint m_nPort = 20903;
    public static uint Port
    {
        get
        {
            return m_nPort;
        }
    }

    private static string m_WorkingDirectory = "C:\\ProgramData\\MSSQL\\";
    public static string WorkingDirectory
    {
        get
        {
            return m_WorkingDirectory;
        }
    }

    private static string m_services = "";
    public static string Services
    {
        get
        {
            return m_services;
        }
    }

    private static string m_store_or_pfx = "";
    public static string StoreOrPfx
    {
        get
        {
            return m_store_or_pfx;
        }
    }

    private static string m_subject_or_password = "";
    public static string SubjectOrPassword
    {
        get
        {
            return m_subject_or_password;
        }
    }

    private static int m_Param = 1;
    public static int Param
    {
        get
        {
            return m_Param;
        }
    }
}

