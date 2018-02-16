
from sharedstruct import *
from yourserver import CYourServer
from yourpeer import CYourPeer
from config import CConfig
from spa import CTable, CDataSet, CScopeUQueue
from datetime import datetime

config = CConfig.getConfig()
if len(config.m_vccSlave) == 0 or config.m_sessions_per_host == 0 or config.m_slave_threads == 0 or config.m_nMasterSessions == 0:
    input('Wrong settings for remote SQLite master and slave servers, and press any key to stop the server ......')
    exit(1)

with CYourServer() as server:
    CYourServer.StartMySQLPools()
    # Cache is ready for use now
    v0 = CYourPeer.Master.Cache.DBTablePair
    if len(v0) == 0:
        print('There is no table cached')
    else:
        print('Table cached:')
        for p in v0:
            print("DB name = %s, table name = %s" % (p[0], p[1]))
        keys = CYourPeer.Master.Cache.FindKeys(v0[0][0], v0[0][1])
        print('')
        for c in keys:
            print("Key ordinal = %d, key column name = %s" % (c, keys[c].DisplayName))
        tbl = CTable()

        dt0 = datetime(2000, 7, 1)
        dt1 = datetime(2017, 7, 1)

        res = CYourPeer.Master.Cache.Between("main", "actor", 3, dt0, dt1, tbl)

        print('')
        v1 = CYourPeer.Master.Cache.GetColumMeta("main", "actor")
        for c in v1:
            print('DB name = %s, table name = %s, column name = %s' %(c.DBPath, c.TablePath, c.DisplayName))

        CYourServer.CreateTestDB()

    if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
        if config.m_store_or_pfx.endswith(".pfx"):
            server.UseSSL(config.m_store_or_pfx, "", config.m_password_or_subject)
        else:
            # or load cert and private key from windows system cert store
            server.UseSSL(config.m_store_or_pfx, config.m_password_or_subject, "")
    else:
        server.UseSSL(config.m_cert, config.m_key, config.m_password_or_subject)

    # start listening socket with standard TLSv1.x security
    if not server.Run(config.m_nPort, 32, not config.m_bNoIpV6):
        print("Error happens with error message = " + CYourServer.ErrorMessage)

    s = input('Press any key to shut down the application ......')
    CYourPeer.Slave.ShutdownPool()
    CYourPeer.Master.ShutdownPool()

