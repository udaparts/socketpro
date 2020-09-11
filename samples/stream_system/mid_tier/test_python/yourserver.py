from spa import CSqlMasterPool as CMaster
from spa.serverside import CSocketProServer, tagAuthenticationMethod, CSocketProService
from spa.clientside import CAsyncDBHandler, CMysql
from spa.udb import DB_CONSTS
from shared.sharedstruct import *
from mid_tier.test_python.yourpeer import CYourPeer

class CYourServer(CSocketProServer):
    def __init__(self, param=0):
        super(CYourServer, self).__init__(param)
        self.YourPeerService = None

    def OnSettingServer(self):
        CYourServer.Config.AuthenticationMethod = tagAuthenticationMethod.amOwn
        self.SetChatGroups()
        mapIdMethod = {
            idGetMasterSlaveConnectedSessions: 'GetMasterSlaveConnectedSessions',
            DB_CONSTS.idGetCachedTables: ('GetCachedTables', True)  # True -- slow request
        }
        self.YourPeerService = CSocketProService(CYourPeer, sidStreamSystem, mapIdMethod)
        self.YourPeerService.ReturnRandom = True
        return True

    def OnIsPermitted(self, hSocket, userId, password, nSvsID):
        print('"Ask for a service %d from user: %s with password: %s' % (nSvsID, userId, password))
        return True  # True -- permitted; False -- denied

    def SetChatGroups(self):
        CYourServer.PushManager.AddAChatGroup(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, 'Subscribe/publish for front clients')
        CYourServer.PushManager.AddAChatGroup(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, 'Cache update notification from middle tier to front')

    @staticmethod
    def CreateTestDB():
        handler = CYourPeer.Master.Seek()
        if handler:
            sql = "CREATE DATABASE IF NOT EXISTS mysample character set utf8 collate utf8_general_ci;USE mysample;CREATE TABLE IF NOT EXISTS COMPANY(ID BIGINT PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID BIGINT PRIMARY KEY AUTO_INCREMENT,CompanyId BIGINT NOT NULL,Name NCHAR(64)NOT NULL,JoinDate DATETIME(6)DEFAULT NULL,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id));USE sakila;INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')"
            ok = handler.Execute(sql)
