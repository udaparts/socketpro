﻿using System;
using System.Collections.Generic;
using TinyJson;
using System.IO;

public class MsSqlConfig
{
    public uint manual_batching = 0;
    public uint max_queries_batched = 0;
    public uint max_sql_size = 0;
    public string ca_for_mssql;
}

public class UConfig
{
    public static readonly string MY_VERSION = "1.7.0.3";
    public static readonly string DEFAULT_CONNECTION_STRING = "server=localhost;timeout=30";
    public static readonly int DEFAULT_MAIN_THREADS = 1;
    public static readonly uint DEFAULT_MAX_SQL_SIZE = 4 * 1024 * 1024;
    public static readonly uint DEFAULT_MIN_SQL_SIZE = 4 * 1024;
    public static readonly uint DEFAULT_PORT = 20903;
    public static readonly string STREAM_DB_CONFIG_FILE = "sp_streaming_db_config.json";
    public static readonly string STREAM_DB_LOG_FILE = "streaming_db.log";
    public static readonly string DEFAULT_WORKING_DIRECTORY = "C:\\ProgramData\\MSSQL\\";
    public static readonly string DEFAULT_CA_ROOT = "root";
    public string cert_root_store = DEFAULT_CA_ROOT;
    public string cert_subject_cn = "";
    public string default_connection_string = DEFAULT_CONNECTION_STRING;
    public bool disable_ipv6 = false;
    public int main_threads = DEFAULT_MAIN_THREADS;
    public uint manual_batching = 1;
    public uint max_queries_batched = 8;
    public uint max_sql_size = DEFAULT_MAX_SQL_SIZE;
    public string mssql_plugin_version = "";
    public uint port = DEFAULT_PORT;
    public string services = "";
    public Dictionary<string, Dictionary<string, object>> services_config = new Dictionary<string, Dictionary<string, object>>();
    public string sp_server_core_version = "";
    public string version = "";
    public uint service_id = 0;
    //
    //

    public UConfig()
    {
        version = MY_VERSION;
        //asynchronous MS SQL service id
        service_id = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2;
    }

    public UConfig(string json)
    {
        bool changed = false;
        try
        {
            UConfig config = json.FromJson<UConfig>();
            CopyFrom(config);
            if (version != MY_VERSION)
            {
                version = MY_VERSION;
                changed = true;
            }
            if (service_id != SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2)
            {
                service_id = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2;
                changed = true;
            }
            if (main_threads <= 0)
            {
                main_threads = DEFAULT_MAIN_THREADS;
                changed = true;
            }
            if (port == 0 || port > 0xffff)
            {
                port = DEFAULT_PORT;
                changed = true;
            }
            if (max_sql_size < DEFAULT_MIN_SQL_SIZE)
            {
                max_sql_size = DEFAULT_MIN_SQL_SIZE;
                changed = true;
            }
            string str = default_connection_string.Trim();
            if (str != default_connection_string)
            {
                default_connection_string = str;
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
            LogMsg(ex.Message, "UConfig::UConfig(string json)", 110); //line 110
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
            LogMsg(ex.Message, "UConfig::UpdateConfigFile", 149); //line 149
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
        default_connection_string = config.default_connection_string;
        version = config.version;
        mssql_plugin_version = config.mssql_plugin_version;
        sp_server_core_version = config.sp_server_core_version;
        service_id = config.service_id;
        manual_batching = config.manual_batching;
        max_queries_batched = config.max_queries_batched;
        max_sql_size = config.max_sql_size;
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