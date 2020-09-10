import SPA.*;
import SPA.ServerSide.*;
import SPA.ClientSide.*;
import SPA.UDB.DB_CONSTS;
import java.util.ArrayList;

public class CYourServer extends CSocketProServer {

    public CYourServer(int param) {
        super(param);
    }

    public CYourServer() {
        super(1);
    }

    public static CSqlMasterPool<CMysql> Master = null;
    public static CSqlMasterPool<CMysql>.CSlavePool Slave = null;
    public static ArrayList<String> FrontCachedTables = new ArrayList<>();

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
        //results could be returned randomly and not in order
        m_SSPeer.setReturnRandom(true);
        return true;
    }

    @Override
    protected boolean OnIsPermitted(long hSocket, String userId, String password, int nSvsID) {
        System.out.println("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }

    public static void CreateTestDB() {
        CMysql handler = Master.Seek();
        if (handler != null) {
            String sql = "CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')";
            boolean ok = handler.Execute(sql);
        }
    }
}
