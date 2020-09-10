using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Collections.Generic;

class CYourServer : CSocketProServer
{
    public static CSqlMasterPool<CMysql, CDataSet> Master;
    public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Slave;
    public static List<string> FrontCachedTables = new List<string>();

    [ServiceAttr(ss.Consts.sidStreamSystem)]
    private CSocketProService<CYourPeerOne> m_SSPeer = new CSocketProService<CYourPeerOne>();

    public CYourServer(int param = 1)
        : base(param)
    {
        FrontCachedTables.Add("sakila.actor");
        FrontCachedTables.Add("sakila.language");
        FrontCachedTables.Add("sakila.country");
    }

    protected override bool OnSettingServer()
    {
        //amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;
        SetChatGroups();
        //results could be returned randomly and not in order
        m_SSPeer.ReturnRandom = true;
        return true;
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        Console.WriteLine("Ask for a service {0} from user {1} with password = {2}", nSvsID, userId, password);
        return true;
    }

    void SetChatGroups()
    {
        PushManager.AddAChatGroup(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for front clients");
        PushManager.AddAChatGroup(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Cache update notification from middle tier to front");
    }

    public static void CreateTestDB()
    {
        var handler = Master.Seek();
        if (handler != null)
        {
            string sql = "CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
            bool ok = handler.Execute(sql);
        }
    }
}
