
import SPA.*;
import SPA.ServerSide.*;
import SPA.ClientSide.*;
import SPA.UDB.DB_CONSTS;

public class CYourServer extends CSocketProServer {
    
    public CYourServer(int param) {
        super(param);
    }
    
    public CYourServer() {
        super(1);
    }
    
    public static CSqlMasterPool<CSqlite> Master = null;
    public static CSqlMasterPool<CSqlite>.CSlavePool Slave = null;

    //public static CSqlMasterPool<CMysql> Master = null;
    //public static CSqlMasterPool<CMysql>.CSlavePool Slave = null;
    @ServiceAttr(ServiceID = Consts.sidStreamSystem)
    private final CSocketProService<CYourPeerOne> m_SSPeer = new CSocketProService<>(CYourPeerOne.class);
    
    private void SetChatGroups() {
        PushManager.AddAChatGroup(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for front clients");
        PushManager.AddAChatGroup(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Cache update notification from middle tier to front");
    }
    
    @Override
    protected boolean OnSettingServer() {
        Config.setAuthenticationMethod(tagAuthenticationMethod.amOwn);
        SetChatGroups();
        return true;
    }
    
    @Override
    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        System.out.println("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }
    
    @Override
    public boolean Run(int port, int maxBacklog, boolean v6Supported) {
        boolean ok = super.Run(port, maxBacklog, v6Supported);
        if (ok) {
            m_SSPeer.setReturnRandom(true); //results could be returned randomly and not in order
        }
        return ok;
    }
    
    public static void StartMySQLPools() {
        CConfig config = CConfig.getConfig();
        Master = new CSqlMasterPool<>(CSqlite.class, config.m_master_default_db, true);
        if (config.m_master_queue_name != null && config.m_master_queue_name.length() > 0) {
            Master.setQueueName(config.m_master_queue_name);
        }
        CDataSet Cache = Master.getCache();

        //These case-sensitivities depends on your DB running platform and sensitivity settings.
        //All of them are false or case-insensitive by default
        Cache.setTableNameCaseSensitive(false);
        Cache.setDBNameCaseSensitive(false);

        //start master pool for cache and update accessing
        boolean ok = CYourServer.Master.StartSocketPool(config.m_ccMaster, config.m_nMasterSessions, 1); //one thread enough

        //compute threads and sockets_per_thread
        int threads = config.m_nSlaveSessions / config.m_vccSlave.size();
        int sockets_per_thread = config.m_vccSlave.size();
        
        Slave = Master.new CSlavePool(config.m_slave_default_db);
        if (config.m_slave_queue_name != null && config.m_slave_queue_name.length() > 0) {
            Slave.setQueueName(config.m_slave_queue_name);
        }
        
        CConnectionContext[][] ppCC = new CConnectionContext[threads][sockets_per_thread];
        for (int i = 0; i < threads; ++i) {
            for (int j = 0; j < sockets_per_thread; ++j) {
                ppCC[i][j] = config.m_vccSlave.get(j);
            }
        }
        //start slave pool for query accessing
        ok = Slave.StartSocketPool(ppCC);

        //wait until all data of cached tables are brought from backend database server to this middle server application cache
        ok = Master.getAsyncHandlers()[0].WaitAll();
    }
    
    public static void CreateTestDB() {
        CSqlite handler = Master.Seek();
        if (handler != null) {
            boolean ok = handler.Execute("ATTACH DATABASE 'mysample.db' as mysample", null);
            String sql = "CREATE TABLE mysample.COMPANY(ID INT8 PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE mysample.EMPLOYEE(EMPLOYEEID INTEGER PRIMARY KEY AUTOINCREMENT,CompanyId INT8 not null,Name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
            ok = handler.Execute(sql);
            sql = "INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
            ok = handler.Execute(sql);
        }

        //CMysql handler = Master.Seek();
        //if (handler != null) {
        //    String sql = "CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila";
        //    boolean ok = handler.Execute(sql);
        //    sql = "INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
        //    ok = handler.Execute(sql);
        //}
    }
    
}
