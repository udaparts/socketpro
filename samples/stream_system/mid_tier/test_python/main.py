
from sharedstruct import *
from yourserver import CYourServer
from yourpeer import CYourPeer
from spa import CTable
from datetime import datetime
from spa.clientside.spmanager import SpManager

sc = SpManager.SetConfig(True, 'D:\\cyetest\\socketpro\\samples\\stream_system\\sp_config.json')
CYourPeer.Master = SpManager.GetPool('masterdb')
Cache = CYourPeer.Master.Cache
CYourPeer.Slave = SpManager.GetPool('slavedb0')

with CYourServer(2) as server:
    # Cache is ready for use now
    v0 = Cache.DBTablePair
    if len(v0) == 0:
        print('There is no table cached')
    else:
        print('Table cached:')
        for p in v0:
            print("DB name = %s, table name = %s" % (p[0], p[1]))
        keys = Cache.FindKeys(v0[0][0], v0[0][1])
        print('')
        for c in keys:
            print("Key ordinal = %d, key column name = %s" % (c, keys[c].DisplayName))
        tbl = CTable()

        dt0 = datetime(2000, 7, 1)
        dt1 = datetime(2017, 7, 1)

        res = Cache.Between("sakila", "actor", 3, dt0, dt1, tbl)

        print('')
        v1 = Cache.GetColumMeta("sakila", "actor")
        for c in v1:
            print('DB name = %s, table name = %s, column name = %s' %(c.DBPath, c.TablePath, c.DisplayName))

        CYourServer.CreateTestDB()

    if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
        server.UseSSL('D:\\cyetest\\socketpro\\bin\\intermediate.pfx', '', 'mypassword')
        # or load cert and private key from windows system cert store
        # server.UseSSL('ROOT', 'UDAParts Intermediate CA', '')
    else:
        server.UseSSL('intermediate.cert.pem', 'intermediate.key.pem', 'mypassword')

    # start listening socket with standard TLSv1.x security
    if not server.Run(20911):
        print("Error happens with error message = " + CYourServer.ErrorMessage)

    s = input('Press any key to shut down the application ......')
    CYourPeer.Slave.ShutdownPool()
    CYourPeer.Master.ShutdownPool()

