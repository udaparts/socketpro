using System;
using System.Collections.Generic;


public class CConfig
{
    private CConfig() { }

    //master
    public string m_master_default_db = "";
    public uint m_nMasterSessions = 2;
    public SocketProAdapter.ClientSide.CConnectionContext m_ccMaster = new SocketProAdapter.ClientSide.CConnectionContext();
    public string m_master_queue_name = "";

    //slave
    public string m_slave_default_db = "";
    public uint m_nSlaveSessions = 0;
    public List<SocketProAdapter.ClientSide.CConnectionContext> m_vccSlave = new List<SocketProAdapter.ClientSide.CConnectionContext>();
    public string m_slave_queue_name = "";

    //middle tier server
    public byte m_main_threads = 1;
    public uint m_nPort = 20911;
    public bool m_bNoIpV6 = false;

    //secure communication between front and middle tier
#if WIN32_64
    public string m_store_or_pfx;
#else
        public string m_cert; //in PEM
        public string m_key; //in PEM
#endif
    public string m_password_or_subject;

    public List<string> m_vFrontCachedTable = new List<string>();

    public static CConfig GetConfig()
    {
        if (m_config != null)
            return m_config;
        m_config = new CConfig();

        //load the following settings from a configuration file
        m_config.m_main_threads = 4;

        //master
#if USE_SQLITE
        m_config.m_ccMaster.Port = 20901;
        m_config.m_master_default_db = "sakila.db";
        m_config.m_nMasterSessions = 1; //one session enough
#else
        m_config.m_ccMaster.Port = 20902;
        m_config.m_master_default_db = "sakila";
        m_config.m_nMasterSessions = 2; //two sessions enough
#endif
        m_config.m_ccMaster.Host = "localhost";
        m_config.m_ccMaster.UserId = "root";
        m_config.m_ccMaster.Password = "Smash123";

        SocketProAdapter.ClientSide.CConnectionContext cc = new SocketProAdapter.ClientSide.CConnectionContext();
#if USE_SQLITE
        m_config.m_slave_default_db = "sakila.db";
        cc.Port = 20901;
#else
        m_config.m_slave_default_db = "sakila";
        cc.Port = 20902;
#endif
        m_config.m_slave_queue_name = "db_sakila";
        cc.Host = "192.168.1.111";
        cc.UserId = "root";
        cc.Password = "Smash123";
        m_config.m_vccSlave.Add(cc);

        //treat master as last salve
        m_config.m_vccSlave.Add(m_config.m_ccMaster);
        m_config.m_nSlaveSessions = 4;

        //middle tier
        //test certificate and private key files are located at the directory ../socketpro/bin
#if WIN32_64
        m_config.m_store_or_pfx = "intermediate.pfx";
#else
        m_config.m_cert = "intermediate.cert.pem";
	    m_config.m_key = "intermediate.key.pem";
#endif
        m_config.m_password_or_subject = "mypassword";

        //cached tables on front applications
#if USE_SQLITE
        m_config.m_vFrontCachedTable.Add("actor");
        m_config.m_vFrontCachedTable.Add("language");
        m_config.m_vFrontCachedTable.Add("country");
#else
        m_config.m_vFrontCachedTable.Add("sakila.actor");
        m_config.m_vFrontCachedTable.Add("sakila.language");
        m_config.m_vFrontCachedTable.Add("sakila.country");
#endif
        return m_config;
    }

    private static CConfig m_config = null;
}
