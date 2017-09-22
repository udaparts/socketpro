import sys
from spa.udb import *
from spa.clientside import CSocketPool, CConnectionContext, CSqlite, CUQueue
import datetime
import threading

sample_database = "mysample.db"


def Demo_Cross_Request_Dead_Lock(sqlite):
    # uncomment the following call to remove potential cross SendRequest dead lock
    # sqlite.AttachedClientSocket.ClientQueue.StartQueue("cross_locking_0", 3600)
    count = 1000
    def cb(sqlite, res, errMsg):
        if res != 0:
            print('Open: res = ' + str(res) + ', errMsg: ' + errMsg)
    while count > 0:
        sqlite.Open(sample_database, cb)
        count = count - 1

def TestCreateTables(sqlite):
    def cb(sqlite, res, errMsg, affected, fail_ok, lastRowId):
        if res != 0:
            print('affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg)
    sqlite.ExecuteSql("CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,NAME CHAR(64)NOT NULL)", cb)
    sqlite.ExecuteSql("CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))", cb)

m_csConsole = threading.Lock()
def StreamSQLsWithManualTransaction(sqlite):
    def cb(sqlite, res, errMsg):
        if res != 0:
            with m_csConsole:
                print('MANUAL TRANSACTION: res = ' + str(res) + ', errMsg: ' + errMsg)
    def cbExecute(sqlite, res, errMsg, affected, fail_ok, lastRowId):
        if res != 0:
            with m_csConsole:
                print('StreamSQL: affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg)

    sqlite.BeginTrans(tagTransactionIsolation.tiReadCommited, cb)
    sqlite.ExecuteSql("delete from EMPLOYEE;delete from COMPANY", cbExecute)
    sqlite.Prepare("INSERT INTO COMPANY(ID,NAME)VALUES(?,?)")
    vData = []
    vData.append(1)
    vData.append("Google Inc.")

    vData.append(2)
    vData.append("Microsoft Inc.")
    # send two sets of parameter data in one shot for processing
    sqlite.ExecuteParameters(vData, cbExecute)

    ok = sqlite.Prepare("INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)")
    vData = []
    # first set of data
    vData.append(1)
    vData.append(1) # google company id
    vData.append("Ted Cruz")
    vData.append(datetime.datetime.now())

    # second set of data
    vData.append(2)
    vData.append(1) # google company id
    vData.append("Donald Trump")
    vData.append(datetime.datetime.now())

    # third set of data
    vData.append(3)
    vData.append(2) # Microsoft company id
    vData.append("Hillary Clinton")
    vData.append(datetime.datetime.now())
    # send three sets of parameterized data in one shot for processing
    sqlite.ExecuteParameters(vData, cbExecute)

    sqlite.EndTrans(tagRollbackPlan.rpDefault, cb)

m_cycle = 100
def Demo_Multiple_SendRequest_MultiThreaded_Wrong(sp):
    cycle = m_cycle
    while cycle > 0:
        # Seek an async handler on the min number of requests queued in memory and its associated socket connection
        sqlite = sp.Seek()
        StreamSQLsWithManualTransaction(sqlite)
        cycle = cycle - 1
    vSqlite = sp.AsyncHandlers
    for s in vSqlite:
        s.WaitAll()


def Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(sp):
    cycle = m_cycle
    while cycle > 0:
        # Take an async handler infinitely from socket pool for sending multiple requests from current thread
        sqlite = sp.Lock()
        StreamSQLsWithManualTransaction(sqlite)
        # Put back a previously locked async handler to pool for reuse
        sp.Unlock(sqlite)
        cycle = cycle - 1
    vSqlite = sp.AsyncHandlers
    for s in vSqlite:
        s.WaitAll()

with CSocketPool(CSqlite) as spSqlite:
    print('Remote async sqlite server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'PythonUser', 'TooMuchSecret')
    # start socket pool having 1 worker which hosts 2 non-block sockets
    ok = spSqlite.StartSocketPool(cc, 2, 1)
    if not ok:
        print('No connection to sqlite server and press any key to close the demo ......')
        sys.stdin.readline()
        exit(0)
    sqlite = spSqlite.AsyncHandlers[0]
    # Use the above bad implementation to replace original spa.clientside.CAsyncDBHandler.Open method
    # at file socketpro/bin/spa/clientside/asyncdbhandler.py for cross SendRequest dead lock demonstration
    print('Doing Demo_Cross_Request_Dead_Lock ......')
    Demo_Cross_Request_Dead_Lock(sqlite)

    # create two tables, COMPANY and EMPLOYEE
    TestCreateTables(sqlite)
    sqlite.WaitAll()
    print(sample_database + ' created, opened and shared by multiple sessions')
    print('')

    # make sure all other handlers/sockets to open the same database mysample.db
    vSqlite = spSqlite.AsyncHandlers
    for s in vSqlite:
        def cb(s, res, errMsg):
            if res != 0:
                print('Open: res = ' + str(res) + ', errMsg: ' + errMsg)
        s.Open(sample_database, cb)
        s.WaitAll()

    # execute manual transactions concurrently with transaction overlapping on the same session
    Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite)
    print('Demo_Multiple_SendRequest_MultiThreaded_Wrong completed')
    print('')

    # execute manual transactions concurrently without transaction overlapping on the same session by lock/unlock
    Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite)
    print('Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock completed')
    print('')

    print('Press any key to close the application ......')
    sys.stdin.readline()