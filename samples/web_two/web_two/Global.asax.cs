﻿using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace web_two {
    public class Global : System.Web.HttpApplication {
        public static CDataSet Cache;
        public static CSqlMasterPool<CMysql, CDataSet> Master;
        public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Slave;
        public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Master_No_Backup;
        protected void Application_Start(object sender, System.EventArgs e) {
            //suppose sp_config.json inside C:\Program Files\IIS Express
            //CSpConfig config = SpManager.SetConfig(false,
            //@"D:\cyetest\socketpro\samples\web_two\web_two\sp_config.json");
            Master = SpManager.GetPool("masterdb") as CSqlMasterPool<CMysql, CDataSet>;
            CMysql handler = Master.Seek();
            if (handler != null) { //create a test database
                string sql = @"CREATE DATABASE IF NOT EXISTS mysample character set
                utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS
                COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE
                IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,
                CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)
                DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;
                INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),
                (2,'Microsoft Inc.'),(3,'Amazon Inc.')";
                handler.Execute(sql);
            }
            Cache = Master.Cache;
            Slave = SpManager.GetPool("slavedb0") as
                CSqlMasterPool<CMysql, CDataSet>.CSlavePool;
            Master_No_Backup = SpManager.GetPool("db_no_backup") as
                CSqlMasterPool<CMysql, CDataSet>.CSlavePool;
        }
    }
}
