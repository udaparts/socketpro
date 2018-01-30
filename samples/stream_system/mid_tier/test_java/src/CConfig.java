
import SPA.ClientSide.CConnectionContext;
import java.util.ArrayList;

public class CConfig {

    private CConfig() {
    }
    //master
    public String m_master_default_db = "";
    public int m_nMasterSessions = 2;
    public CConnectionContext m_ccMaster = new CConnectionContext();
    public String m_master_queue_name = "";

    //slave
    public String m_slave_default_db = "";
    public int m_nSlaveSessions = 0;
    public ArrayList<CConnectionContext> m_vccSlave = new ArrayList<>();
    public String m_slave_queue_name = "";

    //middle tier server
    public byte m_main_threads = 1;
    public int m_nPort = 20911;
    public boolean m_bNoIpV6 = false;

    //windows
    public String m_store_or_pfx = "";
    //Linux
    public String m_cert = ""; //in PEM
    public String m_key = ""; //in PEM

    //both windows and Linux
    public String m_password_or_subject;

    public ArrayList<String> m_vFrontCachedTable = new ArrayList<>();

    private static CConfig m_config = null;

    public static CConfig getConfig() {
        if (m_config != null) {
            return m_config;
        }
        m_config = new CConfig();

        //load the following settings from a configuration file
        m_config.m_main_threads = 4;

        //master
        m_config.m_ccMaster.Port = 20901;
        m_config.m_master_default_db = "sakila.db";
        m_config.m_nMasterSessions = 1; //one session enough

        //MySQL plugin
        //m_config.m_ccMaster.Port = 20902;
        //m_config.m_master_default_db = "sakila";
        //m_config.m_nMasterSessions = 2; //two sessions enough
        m_config.m_ccMaster.Host = "localhost";
        m_config.m_ccMaster.UserId = "root";
        m_config.m_ccMaster.Password = "Smash123";

        m_config.m_slave_default_db = "sakila.db";
        m_config.m_slave_queue_name = "db_sakila";
        //MySQL plugin
        //m_config.m_slave_default_db = "sakila";

        CConnectionContext cc = new CConnectionContext();
        cc.Host = "35.202.62.6";
        cc.Port = 20901;
        cc.UserId = "root";
        cc.Password = "Smash123";
        m_config.m_vccSlave.add(cc);

        //treat master as last salve
        m_config.m_vccSlave.add(m_config.m_ccMaster);
        m_config.m_nSlaveSessions = 4;

        //middle tier
        //test certificate and private key files are located at the directory ../socketpro/bin
        if (SPA.CUQueue.DEFAULT_OS == SPA.tagOperationSystem.osWin) {
            m_config.m_store_or_pfx = "intermediate.pfx";
        } else {
            m_config.m_cert = "intermediate.cert.pem";
            m_config.m_key = "intermediate.key.pem";
        }
        m_config.m_password_or_subject = "mypassword";

        //cached tables on front applications
        m_config.m_vFrontCachedTable.add("actor");
        m_config.m_vFrontCachedTable.add("language");
        m_config.m_vFrontCachedTable.add("country");

        //MySQL plugin
        //m_config.m_vFrontCachedTable.add("sakila.actor");
        //m_config.m_vFrontCachedTable.add("sakila.language");
        //m_config.m_vFrontCachedTable.add("sakila.country");
        return m_config;
    }
}
