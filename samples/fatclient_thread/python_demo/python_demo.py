import sys
from spa.udb import *
from spa.clientside import CSocketPool, CConnectionContext, CSqlite, CUQueue
import datetime

sample_database = "mysample.db"


def Demo_Cross_Request_Dead_Lock(sqlite):
    # uncomment the following call to remove potential cross SendRequest dead lock
    # sqlite.AttachedClientSocket.ClientQueue.StartQueue("cross_locking_0", 3600)
    count = 1000000
    def cb(sqlite, res, errMsg):
        if res != 0:
            print('Open: res = ' + str(res) + ', errMsg: ' + errMsg)
    while count > 0:
        sqlite.Open(sample_database, cb)
        --count

def TestCreateTables(sqlite):
    def cb(sqlite, res, errMsg, affected, fail_ok, lastRowId):
        if res != 0:
            return
        print('affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg)
    sqlite.Execute("CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,NAME CHAR(64)NOT NULL)", cb)
    sqlite.Execute("CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))", cb)

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
    for sqlite in vSqlite:
        def cb(sqlite, res, errMsg):
            if res != 0:
                print('Open: res = ' + str(res) + ', errMsg: ' + errMsg)
        sqlite.Open(sample_database, cb)
        sqlite.WaitAll()

    # execute manual transactions concurrently with transaction overlapping on the same session


    # execute manual transactions concurrently without transaction overlapping on the same session by lock/unlock

    print('Press any key to close the application ......')
    sys.stdin.readline()