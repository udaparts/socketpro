using System;
using System.Collections.Generic;
using TinyJson;
using System.IO;

public class UConfig
{
    public static readonly int DEFAULT_MAIN_THREADS = 1;
    public static readonly uint DEFAULT_PORT = 20903;
    public static readonly string DEFAULT_DRIVER = "{ODBC Driver 17 for SQL Server}";
    public static readonly string STREAM_DB_CONFIG_FILE = "sp_streaming_db_config.json";
    public static readonly string STREAM_DB_LOG_FILE = "streaming_db.log";
    public static readonly string DEFAULT_WORKING_DIRECTORY = "C:\\ProgramData\\MSSQL\\";
    public static readonly string DEFAULT_CA_ROOT = "root";

    public uint port = DEFAULT_PORT;
    public int main_threads = DEFAULT_MAIN_THREADS;
    public bool disable_ipv6 = false;
    public string cert_root_store = DEFAULT_CA_ROOT;
    public string cert_subject_cn = "";
    public string services = "";
    public Dictionary<string, Dictionary<string, object>> services_config = new Dictionary<string, Dictionary<string, object>>(StringComparer.OrdinalIgnoreCase);
    public string odbc_driver = DEFAULT_DRIVER;
    public string default_db = "";

    public UConfig()
    {

    }

    public UConfig(string json)
    {
        bool changed = false;
        try
        {
            UConfig config = json.FromJson<UConfig>();
            CopyFrom(config);
            if (main_threads <= 0)
            {
                main_threads = DEFAULT_MAIN_THREADS;
                changed = true;
            }
            if (port == 0 || port > 0xffff)
            {
                port = UConfig.DEFAULT_PORT;
                changed = true;
            }
            string str = odbc_driver.Trim();
            if (odbc_driver != str || str.Length == 0)
            {
                odbc_driver = UConfig.DEFAULT_DRIVER;
                changed = true;
            }
            str = default_db.Trim();
            if (str != default_db)
            {
                default_db = str;
                changed = true;
            }
            str = services.Trim();
            if (str != services)
            {
                services = str;
                changed = true;
            }
            str = cert_root_store.Trim();
            if (str != cert_root_store)
            {
                cert_root_store = str;
                changed = true;
            }
            str = cert_subject_cn.Trim();
            if (str != cert_subject_cn)
            {
                cert_subject_cn = str;
                changed = true;
            }
        }
        catch (Exception ex)
        {
            LogMsg(ex.Message, "UConfig::UConfig(string json)", 84); //line 80
            changed = true;
        }
        finally
        {
            if (changed)
            {
                UpdateConfigFile(this);
                UpdateLog();
            }
        }
    }

    private static object m_cs = new object();
    private static StreamWriter m_sw = null; //protected by m_cs
    private static void OpenLogFile()
    {
        try
        {
            if (m_sw == null)
            {
                m_sw = new StreamWriter(DEFAULT_WORKING_DIRECTORY + STREAM_DB_LOG_FILE, true, System.Text.Encoding.UTF8);
            }
        }
        catch (Exception)
        {
            m_sw = null;
        }
    }

    public static void UpdateConfigFile(UConfig config)
    {
        try
        {
            string json = config.ToJson();
            File.WriteAllText(DEFAULT_WORKING_DIRECTORY + STREAM_DB_CONFIG_FILE, json);
        }
        catch (Exception ex)
        {
            LogMsg(ex.Message, "UConfig::UpdateConfigFile", 119); //line 119
        }
    }

    public void CopyFrom(UConfig config)
    {
        port = config.port;
        main_threads = config.main_threads;
        disable_ipv6 = config.disable_ipv6;
        cert_root_store = config.cert_root_store;
        cert_subject_cn = config.cert_subject_cn;
        services = config.services;
        services_config = config.services_config;
        odbc_driver = config.odbc_driver;
        default_db = config.default_db;
    }

    public static void LogMsg(string lineText, string file = "", int fileLineNumber = 0)
    {
        lock (m_cs)
        {
            OpenLogFile();
            if (m_sw != null)
            {
                string message = string.Format("{0} - {1}:{2} - {3}", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.ffffff"), file, fileLineNumber, lineText);
                try
                {
                    m_sw.WriteLine(message);
                }
                finally
                {
                }
            }
        }
    }
    public static void UpdateLog()
    {
        lock (m_cs)
        {
            if (m_sw != null)
            {
                try
                {
                    m_sw.Flush();
                    m_sw.Close();
                }
                finally
                {
                    m_sw = null;
                }
            }
        }
    }
}