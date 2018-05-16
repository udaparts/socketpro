using SocketProAdapter.ClientSide;
namespace web_two {
    using CSql = CMysql; //point to one of CMysql, CSqlServer and CSQLite
    public class Global : System.Web.HttpApplication {
        public static SPA.CMyMaster Master = null;
        public static SPA.CMySlave Slave = null;
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
        }
        private static void StartPools() {
            StartPool(true); //start master pool
            StartPool(false); //start slave pool
            CSql handler = Master.SeekByQueue();
            if (handler != null) { //create a test database
                string sql = @"CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;
                USE mysample;CREATE TABLE COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);
                CREATE TABLE EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)
                NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;
                INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
                bool ok = handler.Execute(sql);
            }
        }
        private static bool StartPool(bool master) {
            uint threads, sessions_per_host; bool ok = false;
            System.Collections.Generic.List<CConnectionContext> Hosts; CSocketPool<CSql> pool;
            if (master) {
                Master = new SPA.CMyMaster(Config.Master.DefaultDB, false, Config.Master.RecvTimeout);
                Master.QueueName = "qmaster"; pool = Master; threads = Config.Master.Threads;
                sessions_per_host = Config.Master.Sessions_Per_Host; Hosts = Config.Master.Hosts;
            } else {
                Slave = new SPA.CMySlave(Config.Slave.DefaultDB, Config.Slave.RecvTimeout);
                Slave.QueueName = "qslave"; pool = Slave; threads = Config.Slave.Threads;
                sessions_per_host = Config.Slave.Sessions_Per_Host; Hosts = Config.Slave.Hosts;
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
                //no automatcally merging requests saved in local/client message queue files in case master or one host
                if (Hosts.Count < 2 || master) pool.QueueAutoMerge = false;
            }
            return ok;
        }
    }
}
