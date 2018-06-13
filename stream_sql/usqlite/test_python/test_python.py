#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from spa import Pair
from spa.udb import *
from spa.clientside import CSocketPool, CConnectionContext, CSqlite, CUQueue
import datetime

with CSocketPool(CSqlite) as spSqlite:
    print('Remote async sqlite server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'PythonUser', 'TooMuchSecret')
    ok = spSqlite.StartSocketPool(cc, 1, 1)
    sqlite = spSqlite.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(sqlite.AttachedClientSocket.ErrorCode))
        spSqlite.ShutdownPool()
        exit(0)
    def cb(sqlite, res, errMsg):
        print('res = ' + str(res) + ', errMsg: ' + errMsg)

    def cbExecute(sqlite, res, errMsg, affected, fail_ok, lastRowId):
        print('affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg + ', last insert id = ' + str(lastRowId))

    def TestCreateTables():
        ok = sqlite.ExecuteSql('CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income float not null)', cbExecute)
        ok = sqlite.ExecuteSql("CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique, CompanyId INT8 not null, name NCHAR(64) NOT NULL, JoinDate DATETIME not null default(datetime('now')), IMAGE BLOB, DESCRIPTION NTEXT, Salary real, FOREIGN KEY(CompanyId) REFERENCES COMPANY(id))", cbExecute)

    ra = []

    def cbRows(sqlite, lstData):
        index = len(ra) - 1
        ra[index].second.append(lstData)

    def cbRowHeader(sqlite):
        vColInfo = sqlite.ColumnInfo
        ra.append(Pair(vColInfo, []))

    def TestPreparedStatements():
        sql_insert_parameter = "Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)"

        ok = sqlite.Prepare(sql_insert_parameter, cb)

        vData = []
        vData.append(1)
        vData.append("Google Inc.")
        vData.append("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA")
        vData.append(66000000000.0)

        vData.append(2)
        vData.append("Microsoft Inc.")
        vData.append("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA")
        vData.append(93600000000.0)

        vData.append(3)
        vData.append("Apple Inc.")
        vData.append("1 Infinite Loop, Cupertino, CA 95014, USA")
        vData.append(234000000000.0)

        return sqlite.ExecuteParameters(vData, cbExecute, cbRows, cbRowHeader)

    def InsertBLOBByPreparedStatement():
        wstr = ""
        while len(wstr) < 128 * 1024:
            wstr += u'近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？'
        str = ""
        while len(str) < 256 * 1024:
            str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.'
        sqlInsert = u'insert or replace into employee(EMPLOYEEID, CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?, ?);select * from employee where employeeid = ?'

        ok = sqlite.Prepare(sqlInsert, cb)

        vData = []
        sbBlob = CUQueue()

        #first set of data
        vData.append(1)
        vData.append(1) #google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(254000.0)
        vData.append(1)

        #second set of data
        vData.append(2)
        vData.append(1) #google company id
        vData.append("Donald Trump")
        vData.append(datetime.datetime.now())
        sbBlob.SetSize(0)
        sbBlob.SaveAString(str)
        vData.append(sbBlob.GetBuffer())
        vData.append(str)
        vData.append(20254000.0)
        vData.append(2)

        #third set of data
        vData.append(3)
        vData.append(2) #Microsoft company id
        vData.append("Hillary Clinton")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(6254000.0)
        vData.append(3)

        return sqlite.ExecuteParameters(vData, cbExecute, cbRows, cbRowHeader)


    ok = sqlite.Open(u'', cb)
    ok = TestCreateTables()
    ok = sqlite.BeginTrans(tagTransactionIsolation.tiReadCommited, cb)
    ok = TestPreparedStatements()
    ok = InsertBLOBByPreparedStatement()
    ok = sqlite.EndTrans(tagRollbackPlan.rpDefault, cb)
    sqlite.WaitAll()
    print('')
    print('+++++ Start rowsets +++')
    index = 0
    for a in ra:
        if len(a.first) > 0:
            print('Statement index = ' + str(index) + ', rowset with columns = ' + str(len(a.first)) + ', records = ' + str(len(a.second)) + '.')
        else:
            print('Statement index = ' + str(index) + ', no rowset received.')
        index += 1
    print('+++++ End rowsets +++')
    print('')
    print('Press any key to close the application ......')
    sys.stdin.readline()
