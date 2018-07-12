using SocketProAdapter.ClientSide;
namespace web_two {
    using CSql = CMysql; //point to one of CMysql, COdbc+MSSQL and CSQLite
    enum MyCase {
        mcSlaveWithClientQueue = 0,
        mcMasterWithClientQueue = 1,
        mcMasterWithoutClientQueue = 2
    }
    public class Global : System.Web.HttpApplication {
        public static SPA.CMyMaster Master = null;
        public static SPA.CMyMaster.CSlavePool Slave = null;
        public static SPA.CMyMaster.CSlavePool MasterNotQueued = null;
        public static SPA.CConfig Config = new SPA.CConfig();
        public static SocketProAdapter.CDataSet Cache {
            get {
                if (Master == null) return null;
                return Master.Cache; //real-time update cache
            }
        }
        protected void Application_Start(object sender, System.EventArgs e) {
            Config.SetConfig("/web_two"); StartPools();
        }
        protected void Application_End(object sender, System.EventArgs e) {
            if (Slave != null) Slave.ShutdownPool();
            if (Master != null) Master.ShutdownPool();
            if (MasterNotQueued != null) MasterNotQueued.ShutdownPool();
        }
        private static void StartPools() {
            bool ok = StartPool(MyCase.mcMasterWithClientQueue);
            ok = StartPool(MyCase.mcSlaveWithClientQueue);
            ok = StartPool(MyCase.mcMasterWithoutClientQueue);
            CSql handler = Master.SeekByQueue();
            if (handler != null) { //create a test database
                string sql = @"CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;
                USE mysample;CREATE TABLE COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);
                CREATE TABLE EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)
                NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;
                INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
                ok = handler.Execute(sql);
            }
        }
        private static bool StartPool(MyCase mc) {
            uint threads, sessions_per_host; bool ok = false;
            System.Collections.Generic.List<CConnectionContext> Hosts; CSocketPool<CSql> pool = null;
            switch (mc) {
                case MyCase.mcMasterWithClientQueue:
                    Master = new SPA.CMyMaster(Config.Master.DefaultDB, false, Config.Master.RecvTimeout);
                    Master.QueueName = "qmaster"; pool = Master; threads = Config.Master.Threads;
                    sessions_per_host = Config.Master.Sessions_Per_Host; Hosts = Config.Master.Hosts;
                    break;
                case MyCase.mcSlaveWithClientQueue:
                    Slave = new SPA.CMyMaster.CSlavePool(Config.Slave.DefaultDB, Config.Slave.RecvTimeout);
                    Slave.QueueName = "qslave"; pool = Slave; threads = Config.Slave.Threads;
                    sessions_per_host = Config.Slave.Sessions_Per_Host; Hosts = Config.Slave.Hosts;
                    break;
                case MyCase.mcMasterWithoutClientQueue:
                    MasterNotQueued = new SPA.CMyMaster.CSlavePool(Config.Master.DefaultDB, Config.Master.RecvTimeout);
                    pool = MasterNotQueued; threads = Config.Master.Threads;
                    sessions_per_host = Config.Master.Sessions_Per_Host; Hosts = Config.Master.Hosts;
                    break;
                default:
                    throw new System.NotImplementedException("Not implemented");
            }
            pool.DoSslServerAuthentication += (sender, cs) => {
                int errCode; string res = cs.UCert.Verify(out errCode);
                return (errCode == 0); //true -- user id and password will be sent to server
            };
            uint sockets_per_thread = sessions_per_host * (uint)Hosts.Count;
            if (sockets_per_thread > 0 && threads > 0) {
                CConnectionContext[,] ppCC = new CConnectionContext[threads, sockets_per_thread];
                for (uint i = 0; i < threads; ++i)
                    for (uint j = 0; j < (uint)Hosts.Count; ++j)
                        for (uint n = 0; n < sessions_per_host; ++n)
                            ppCC[i, j * sessions_per_host + n] = Hosts[(int)j];
                ok = pool.StartSocketPool(ppCC);
                //not automatcally merge requests saved in local/client message queue files in case there is one host only
                if (Hosts.Count < 2) pool.QueueAutoMerge = false;
            }
            return ok;
        }
    }
}
