from spa import CSqlMasterPool as CMaster
from spa.serverside import CSocketProServer, tagAuthenticationMethod, CSocketProService
from spa.clientside import CAsyncDBHandler, CSqlite
from config import CConfig
from sharedstruct import *
from yourpeer import CYourPeer


class CYourServer(CSocketProServer):
    def __init__(self, param=0):
        super(CYourServer, self).__init__(param)
        self.YourPeerService = None

    def OnSettingServer(self):
        CYourServer.Config.AuthenticationMethod = tagAuthenticationMethod.amOwn
        self.SetChatGroups()
        return True

    def OnIsPermitted(self, hSocket, userId, password, nSvsID):
        print('"Ask for a service %d from user %s with password = %s' % (nSvsID, userId, password))
        return True  # True -- permitted; False -- denied

    def Run(self, port, maxBacklog, v6Supported):
        mapIdMethod = {
            idGetMasterSlaveConnectedSessions: 'GetMasterSlaveConnectedSessions',
            idGetRentalDateTimes: ['GetRentalDateTimes', True],  # or ('GetRentalDateTimes', True)
            CAsyncDBHandler.idGetCachedTables: ('GetCachedTables', True)  # True -- slow request
        }
        self.YourPeerService = CSocketProService(CYourPeer, sidStreamSystem, mapIdMethod)
        ok = super(CYourServer, self).Run(port, maxBacklog, v6Supported)
        self.YourPeerService.ReturnRandom = True
        return ok

    def SetChatGroups(self):
        CYourServer.PushManager.AddAChatGroup(CAsyncDBHandler.STREAMING_SQL_CHAT_GROUP_ID, 'Subscribe/publish for front clients')
        CYourServer.PushManager.AddAChatGroup(CAsyncDBHandler.CACHE_UPDATE_CHAT_GROUP_ID, 'Cache update notification from middle tier to front')

    @staticmethod
    def CreateTestDB():
        handler = CYourPeer.Master.Seek()
        if handler:
            ok = handler.ExecuteSql("ATTACH DATABASE 'mysample.db' as mysample", None)
            sql = "CREATE TABLE mysample.COMPANY(ID INT8 PRIMARY KEY NOT NULL,Name CHAR(64)NOT NULL);CREATE TABLE mysample.EMPLOYEE(EMPLOYEEID INTEGER PRIMARY KEY AUTOINCREMENT,CompanyId INT8 not null,Name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))"
            ok = handler.ExecuteSql(sql)
            sql = "INSERT INTO mysample.COMPANY(ID,Name)VALUES(1,'Google Inc.'),(2,'Microsoft Inc.'),(3,'Amazon Inc.')"
            ok = handler.ExecuteSql(sql)

    @staticmethod
    def StartMySQLPools():
        config = CConfig.getConfig()
        CYourPeer.Master = CMaster(CSqlite, config.m_master_default_db, True)

        # These case-sensitivities depends on your DB running platform and sensitivity settings.
        # All of them are false or case-insensitive by default
        CYourPeer.Master.Cache.FieldNameCaseSensitive = False
        CYourPeer.Master.Cache.TableNameCaseSensitive = False
        CYourPeer.Master.Cache.DBNameCaseSensitive = False

        ok = CYourPeer.Master.StartSocketPool(config.m_ccMaster, config.m_nMasterSessions, 1)  # one thread enough

        # compute threads and sockets_per_thread
        sockets_per_thread = len(config.m_vccSlave)
        threads = int(config.m_nSlaveSessions / sockets_per_thread)

        CYourPeer.Slave = CMaster.CSlavePool(CSqlite, config.m_slave_default_db)
        mcc = [[0 for i in range(sockets_per_thread)] for i in range(threads)]
        while threads >= 0:
            threads -= 1
            j = 0
            for cc in config.m_vccSlave:
                mcc[threads][j] = cc
                j += 1
        ok = CYourPeer.Slave.StartSocketPoolEx(mcc)

        # Wait until all data of cached tables are brought from backend database server to this middle server application cache
        ok = CYourPeer.Master.AsyncHandlers[0].WaitAll()
