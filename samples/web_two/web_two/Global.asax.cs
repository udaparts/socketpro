using SocketProAdapter;
using SocketProAdapter.ClientSide;
namespace web_two {
    public class Global : System.Web.HttpApplication {
        public static CDataSet Cache = null;
        public static CSqlMasterPool<CMysql, CDataSet> Master = null;
        public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Slave = null;
        public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Master_No_Backup = null;
        protected void Application_Start(object sender, System.EventArgs e) {
            //CSpConfig config = SpManager.SetConfig(false, @"D:\mydev\socketpro\samples\web_two\web_two\sp_config.json");
            Master = SpManager.GetPool("masterdb") as CSqlMasterPool<CMysql, CDataSet>;
            CMysql handler = Master.Seek();
            if (handler != null) { //create a test database
                string sql = @"CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;
                USE mysample;CREATE TABLE COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);
                CREATE TABLE EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)
                NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;
                INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
                handler.Execute(sql);
            }
            Cache = Master.Cache;
            Slave = SpManager.GetPool("slavedb0") as CSqlMasterPool<CMysql, CDataSet>.CSlavePool;
            Master_No_Backup = SpManager.GetPool("db_no_backup") as CSqlMasterPool<CMysql, CDataSet>.CSlavePool;
        }
    }
}
