using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Runtime.InteropServices;
using System.Web.Script.Serialization;
using System.Collections.Generic;
using System.IO;

public class UConfig
{
    public static readonly uint DEFAULT_PORT = 20903;
    public static readonly string DEFAULT_DRIVER = "{ODBC Driver 17 for SQL Server}";
    public uint port = DEFAULT_PORT;
    public int main_threads = 1;
    public bool disable_ipv6 = false;
    public string cert_root_store = "";
    public string cert_subject_cn = "";
    public string monitored_tables = "";
    public string services = "";
    public string working_dir = "";
    public Dictionary<string, Dictionary<string, object>> services_config = new Dictionary<string, Dictionary<string, object>>();
    public string odbc_driver = DEFAULT_DRIVER;
    public string default_db = "";
};

public class CSqlPlugin : CSocketProServer
{
    private UConfig m_Config;
    private bool m_changed = false;
    public static readonly string STREAM_DB_CONFIG_FILE = "sp_streaming_db_config.json";
    public static readonly string STREAM_DB_LOG_FILE = "streaming_db.log";
    private object m_cs = new object();
    private StreamWriter m_sw = null; //protected by m_cs

    public void LogMsg(string lineText, string file = "", int fileLineNumber = 0)
    {
        lock (m_cs)
        {
            OpenLogFile();
            if (m_sw != null)
            {
                string message = string.Format("{0} - {1}:{2} - {3}", DateTime.Now.ToString(), file, fileLineNumber, lineText);
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

    private void OpenLogFile()
    {
        try
        {
            if (m_sw == null)
            {
                m_sw = new StreamWriter(CSqlPlugin.STREAM_DB_LOG_FILE, true, System.Text.Encoding.UTF8);
            }
        }
        catch (Exception)
        {
            m_sw = null;
        }
    }

    public void UpdateLog()
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

    public CSqlPlugin(int param = 0)
        : base(param)
    {
        try
        {
            string json = File.ReadAllText(STREAM_DB_CONFIG_FILE);
            JavaScriptSerializer serializer = new JavaScriptSerializer();
            m_Config = serializer.Deserialize<UConfig>(json);
            if (m_Config.main_threads <= 0)
            {
                m_Config.main_threads = 1;
                m_changed = true;
            }
            if (m_Config.port == 0 || m_Config.port > 0xffff)
            {
                m_Config.port = UConfig.DEFAULT_PORT;
                m_changed = true;
            }
            string str = m_Config.odbc_driver.Trim();
            if (m_Config.odbc_driver != str || str.Length == 0)
            {
                m_Config.odbc_driver = UConfig.DEFAULT_DRIVER;
                m_changed = true;
            }
            str = m_Config.default_db.Trim();
            if (str != m_Config.default_db)
            {
                m_Config.default_db = str;
                m_changed = true;
            }
            str = m_Config.services.Trim();
            if (str != m_Config.services)
            {
                m_Config.services = str;
                m_changed = true;
            }
            str = m_Config.cert_root_store.Trim();
            if (str != m_Config.cert_root_store)
            {
                m_Config.cert_root_store = str;
                m_changed = true;
            }
            str = m_Config.cert_subject_cn.Trim();
            if (str != m_Config.cert_subject_cn)
            {
                m_Config.cert_subject_cn = str;
                m_changed = true;
            }
        }
        catch (Exception ex)
        {
            LogMsg(ex.Message, "myserver.cs", 112); //line 112
            m_Config = new UConfig();
            m_changed = true;
        }
        finally
        {
            UpdateConfigFile();
            UpdateLog();
        }
    }

    protected override void OnIdle(ulong milliseconds)
    {
        UpdateLog();
    }

    private void UpdateConfigFile()
    {
        if (m_changed)
        {
            try
            {
                JavaScriptSerializer serializer = new JavaScriptSerializer();
                string json = serializer.Serialize(m_Config);
                File.WriteAllText(STREAM_DB_CONFIG_FILE, json);
                m_changed = false;
            }
            catch (Exception ex)
            {
                LogMsg(ex.Message, "myserver.cs", 112); //line 135
            }
            finally
            {
            }
        }
    }

    [DllImport("sodbc")]
    private static extern int DoSPluginAuthentication(ulong hSocket, [MarshalAs(UnmanagedType.LPWStr)] string userId, [MarshalAs(UnmanagedType.LPWStr)] string password, uint nSvsId, [MarshalAs(UnmanagedType.LPWStr)] string dsn);

    public override bool Run(uint port, uint maxBacklog, bool v6Supported)
    {
        IntPtr p = CSocketProServer.DllManager.AddALibrary("sodbc");
#if PLUGIN_DEV

#else
        string[] vService = SQLConfig.Services.Split(';');
        foreach (string s in vService)
        {
            if (s.Length > 0)
                DllManager.AddALibrary(s);
        }
#endif
        PushManager.AddAChatGroup(SocketProAdapter.UDB.DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
        return base.Run(port, maxBacklog, v6Supported);
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        string driver = "DRIVER=" + m_Config.odbc_driver + ";Server=(local)";
        if (m_Config.default_db != null && m_Config.default_db.Length > 0)
        {
            driver += (";database=" + m_Config.default_db);
        }
        int res = DoSPluginAuthentication(hSocket, userId, password, nSvsID, driver);
        if (res <= 0)
        {
            string message = "Authentication failed for user " + userId + ", res: " + res;
            LogMsg(message, "myserver.cs", 179); //line 179
        }
        return (res > 0);
    }
}
