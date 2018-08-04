
using System;
using System.Data;
using System.Data.SqlClient;

public static class SQLConfig
{
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
                case "working_directory":
                    m_WorkingDirectory = value;
                    break;
                case "odbcdriver":
                    m_odbc = value;
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
                reader.Close();
                m_server = GetServerName(conn);
            }
            finally
            {
                if (reader != null)
                    reader.Close();
                conn.Close();
            }
        } 
    }

    private static string m_server = Environment.MachineName;
    public static string Server
    {
        get
        {
            return m_server;
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

    private static string m_odbc = "{SQL Server Native Client 11.0}";
    public static string ODBCDriver
    {
        get
        {
            return m_odbc;
        }
    }
}
