using System.Collections.Generic;
using System.Configuration;
using SocketProAdapter.ClientSide;
namespace SPA {
    public class CPoolConfig {
        public string DefaultDB = "";
        public uint Sessions_Per_Host = 2;
        public uint Threads = 1;
        public List<CConnectionContext> Hosts = new List<CConnectionContext>();
        public uint RecvTimeout = 30; //A default SQL request timeout in ms
    }
    public class CConfig {
        public string Work_Dir = ""; //a directory containing local request backup files
        public CPoolConfig Master = new CPoolConfig();
        public CPoolConfig Slave = new CPoolConfig();
        public void SetConfig(string path) {
            Configuration root = System.Web.Configuration.WebConfigurationManager.OpenWebConfiguration(path);
            KeyValueConfigurationElement setting = root.AppSettings.Settings["work_dir"];
            if (setting != null) {
                CClientSocket.QueueConfigure.WorkDirectory = setting.Value;
                Work_Dir = setting.Value;
            }
            setting = root.AppSettings.Settings["master"];
            if (setting != null) SetPoolConfig(Master, setting.Value);
            setting = root.AppSettings.Settings["slave"];
            if (setting != null) SetPoolConfig(Slave, setting.Value);
        }
        private void SetPoolConfig(CPoolConfig config, string setting) {
            string[] vItem = setting.Split('|');
            foreach (string s in vItem) {
                string temp = s.Trim();
                if (temp.IndexOf("hosts=") == 0) {
                    string str = temp.Substring(6);
                    string[] vHost = str.Split('+');
                    foreach (string h in vHost) {
                        CConnectionContext cc = new CConnectionContext();
                        string[] entries = h.Split(';');
                        foreach (string item in entries) {
                            string[] v = item.Split('=');
                            if (v.Length == 2) {
                                switch (v[0].Trim()) {
                                    case "host":
                                        cc.Host = v[1].Trim();
                                        break;
                                    case "port":
                                        uint.TryParse(v[1].Trim(), out cc.Port);
                                        break;
                                    case "uid":
                                        cc.UserId = v[1].Trim();
                                        break;
                                    case "pwd":
                                        cc.Password = v[1].Trim();
                                        break;
                                    case "zip":
                                        cc.Zip = true;
                                        cc.AnyData = v[1].Trim();
                                        break;
                                    case "ca":
                                        cc.EncrytionMethod = SocketProAdapter.tagEncryptionMethod.TLSv1;
                                        CClientSocket.SSL.SetVerifyLocation(v[1].Trim());
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        config.Hosts.Add(cc);
                    }
                } else {
                    string[] v = s.Split('=');
                    if (v.Length == 2) {
                        switch (v[0].Trim()) {
                            case "threads":
                                uint.TryParse(v[1].Trim(), out config.Threads);
                                break;
                            case "sessions_per_host":
                                uint.TryParse(v[1].Trim(), out config.Sessions_Per_Host);
                                break;
                            case "default_db":
                                config.DefaultDB = v[1].Trim();
                                break;
                            case "timeout":
                                uint.TryParse(v[1].Trim(), out config.RecvTimeout);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
}
