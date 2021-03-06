using System;
using SocketProAdapter.ServerSide;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using TinyJson;

public class CSqlPlugin : CSocketProServer
{
    private UConfig m_Config = new UConfig();

    public CSqlPlugin(UConfig config) : base(config.main_threads)
    {
        m_Config.CopyFrom(config);
    }

    protected override void OnIdle(ulong milliseconds)
    {
        UConfig.UpdateLog();
    }

    [DllImport("sodbc")]
    private static extern IntPtr GetSPluginVersion();

    [DllImport("sodbc")]
    private static extern int DoSPluginAuthentication(ulong hSocket, [MarshalAs(UnmanagedType.LPWStr)] string userId, [MarshalAs(UnmanagedType.LPWStr)] string password, uint nSvsId, [MarshalAs(UnmanagedType.LPWStr)] string dsn);

    [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
    static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

    public override bool Run(uint port, uint maxBacklog, bool v6Supported)
    {
        IntPtr p = CSocketProServer.DllManager.AddALibrary("sodbc");
        if (p.ToInt64() == 0)
        {
            UConfig.LogMsg("Cannot load SocketPro ODBC plugin!", "CSqlPlugin::Run", 35); //line 35
            UConfig.UpdateLog();
            return false;
        }
        ConfigServices();
        PushManager.AddAChatGroup(SocketProAdapter.UDB.DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
        bool ok = base.Run(port, maxBacklog, v6Supported);
        if (!ok)
        {
            string errMsg = string.Format("Starting listening socket failed (errCode={0}; errMsg={1})", CSocketProServer.LastSocketError, CSocketProServer.ErrorMessage);
            UConfig.LogMsg(errMsg, "CSqlPlugin::Run", 45); //line 45
            UConfig.UpdateLog();
        }
        return ok;
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
            UConfig.LogMsg(message, "CSqlPlugin::OnIsPermitted", 62); //line 62
        }
        return (res > 0);
    }

    //typedef bool (WINAPI *PSetSPluginGlobalOptions)(const char *jsonUtf8Options);
    private delegate bool DSetSPluginGlobalOptions([MarshalAs(UnmanagedType.LPStr)] string jsonUtf8Options);

    //typedef unsigned int (WINAPI *PGetSPluginGlobalOptions)(char *jsonUtf8, unsigned int buffer_size);
    private delegate uint DGetSPluginGlobalOptions([MarshalAs(UnmanagedType.LPArray)] byte[] jsonUtf8, int buffer_size);

    //typedef const char* const (WINAPI *PGetSPluginVersion)();
    private delegate IntPtr DGetSPluginVersion();

    private void ConfigServices()
    {
        bool changed = false;
        string odbc_plugin_version = Marshal.PtrToStringAnsi(GetSPluginVersion());
        if (m_Config.odbc_plugin_version != odbc_plugin_version)
        {
            m_Config.odbc_plugin_version = odbc_plugin_version;
            changed = true;
        }
        string server_core_version = Version;
        if (m_Config.sp_server_core_version != server_core_version)
        {
            m_Config.sp_server_core_version = server_core_version;
            changed = true;
        }
        string[] vService = m_Config.services.Split(';');
        List<string> vP = new List<string>();
        foreach (string s in vService)
        {
            string p_name = s.Trim();
            if (p_name.Length == 0)
            {
                continue;
            }
            if (p_name.Equals("sodbc", StringComparison.OrdinalIgnoreCase) || p_name.Equals("sodbc.dll", StringComparison.OrdinalIgnoreCase))
            {
                continue; //cannot load odbc plugin again
            }
            do
            {
                IntPtr h = DllManager.AddALibrary(p_name);
                if (h.ToInt64() == 0)
                {
                    string message = "Not able to load server plugin " + p_name;
                    UConfig.LogMsg(message, "CSqlPlugin::ConfigServices", 110); //line 110
                    break;
                }
                vP.Add(p_name);
                bool having = m_Config.services_config.ContainsKey(p_name);
                if (!having)
                {
                    changed = true;
                }
                IntPtr addr = GetProcAddress(h, "GetSPluginGlobalOptions");
                if (addr.ToInt64() != 0)
                {
                    try
                    {
                        DGetSPluginGlobalOptions GetSPluginGlobalOptions = (DGetSPluginGlobalOptions)Marshal.GetDelegateForFunctionPointer(addr, typeof(DGetSPluginGlobalOptions));
                        byte[] bytes = new byte[65536];
                        uint res = GetSPluginGlobalOptions(bytes, bytes.Length);
                        string jsonutf8 = System.Text.Encoding.UTF8.GetString(bytes, 0, (int)res);
                        Dictionary<string, object> v = jsonutf8.FromJson<Dictionary<string, object>>();
                        if (m_Config.services_config.ContainsKey(p_name))
                        {
                            m_Config.services_config[p_name] = v;
                        }
                        else
                        {
                            m_Config.services_config.Add(p_name, v);
                        }
                    }
                    catch (Exception ex)
                    {
                        UConfig.LogMsg(ex.Message, "CSqlPlugin::ConfigServices/GetSPluginGlobalOptions", 140); //line 140
                        m_Config.services_config.Add(p_name, new Dictionary<string, object>());
                    }
                }
                else
                {
                    m_Config.services_config.Add(p_name, new Dictionary<string, object>());
                }
                Dictionary<string, object> jsonDic = m_Config.services_config[p_name];
                if (having && jsonDic.Count > 0)
                {
                    addr = GetProcAddress(h, "SetSPluginGlobalOptions");
                    if (addr.ToInt64() != 0)
                    {
                        try
                        {
                            DSetSPluginGlobalOptions func = (DSetSPluginGlobalOptions)Marshal.GetDelegateForFunctionPointer(addr, typeof(DSetSPluginGlobalOptions));
                            if (!func(jsonDic.ToJson(false)))
                            {
                                UConfig.LogMsg("Not able to set global options for plugin " + p_name, "CSqlPlugin::ConfigServices", 159); //line 159
                            }
                        }
                        catch (Exception ex)
                        {
                            UConfig.LogMsg(ex.Message, "CSqlPlugin::ConfigServices/SetSPluginGlobalOptions", 164); //line 164
                        }
                    }
                }
                //DGetSPluginVersion
                addr = GetProcAddress(h, "GetSPluginVersion");
                if (addr.ToInt64() != 0)
                {
                    try
                    {
                        DGetSPluginVersion func = (DGetSPluginVersion)Marshal.GetDelegateForFunctionPointer(addr, typeof(DGetSPluginVersion));
                        string v = Marshal.PtrToStringAnsi(func());
                        string vOld = null;
                        if (jsonDic.ContainsKey("version") && jsonDic["version"] is string)
                        {
                            vOld = (string)jsonDic["version"];
                        }
                        if (v == null || vOld == null || v != vOld)
                        {
                            jsonDic["version"] = v;
                            changed = true;
                        }
                    }
                    catch (Exception ex)
                    {
                        UConfig.LogMsg(ex.Message, "CSqlPlugin::ConfigServices/GetSPluginVersion", 189); //line 189
                    }
                }
            } while (false);
        }
        if (vP.Count != vService.Length)
        {
            string str = "";
            foreach (string item in vP)
            {
                if (str.Length > 0)
                {
                    str += ";";
                }
                str += item;
            }
            m_Config.services = str;
            changed = true;
        }
        List<string> unused = new List<string>();
        foreach (var entry in m_Config.services_config)
        {
            if (!vP.Contains(entry.Key))
            {
                unused.Add(entry.Key);
            }
        }
        foreach (string key in unused)
        {
            m_Config.services_config.Remove(key);
            changed = true;
        }
        if (changed)
        {
            UConfig.UpdateConfigFile(m_Config);
        }
    }
}
