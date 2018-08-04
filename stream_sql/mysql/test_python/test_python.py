#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from spa.udb import *
from spa import Pair
from spa.clientside import CSocketPool, CConnectionContext, CMysql, CUQueue
import datetime

with CSocketPool(CMysql) as spMysql:
    print('Remote async mysql server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20902, 'root', 'Smash123')
    ok = spMysql.StartSocketPool(cc, 1, 1)
    mysql = spMysql.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(mysql.AttachedClientSocket.ErrorCode))
        spMysql.ShutdownPool()
        exit(0)
    def cb(mysql, res, errMsg):
        print('res = ' + str(res) + ', errMsg: ' + errMsg)

    def cbExecute(mysql, res, errMsg, affected, fail_ok, lastRowId):
        print('affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg + ', last insert id = ' + str(lastRowId))

    def TestCreateTables():
        ok = mysql.ExecuteSql('CREATE DATABASE IF NOT EXISTS mysqldb character set utf8 collate utf8_general_ci;USE mysqldb', cbExecute)
        ok = mysql.ExecuteSql('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)', cbExecute)
        ok = mysql.ExecuteSql('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(18,2),FOREIGN KEY(CompanyId)REFERENCES company(id))', cbExecute)
        ok = mysql.ExecuteSql('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(17,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now() into p_last_dt;END', cbExecute)

    ra = []

    def cbRows(mysql, lstData):
        index = len(ra) - 1
        ra[index].second.append(lstData)

    def cbRowHeader(mysql):
        vColInfo = mysql.ColumnInfo
        ra.append(Pair(vColInfo, []))

    def TestPreparedStatements():
        sql_insert_parameter = u'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)'
        ok = mysql.Prepare(sql_insert_parameter, cb)

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
        return mysql.ExecuteParameters(vData, cbExecute)


    def cbBatchHeader(mysql):
        # called before rh, r and er
        pass

    def cbDiscarded(mysql):
        # called when canceling or socket closed if client queue is NOT used
        pass

    def TestBatch():
        # sql with delimiter '|'
        sql="delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from company;select * from employee;select curtime()|call sp_TestProc(?,?,?)"
        wstr = ""
        while len(wstr) < 128 * 1024:
            wstr += u'近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？'
        astr = ""
        while len(astr) < 256 * 1024:
            astr += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.'
        vData = []
        sbBlob = CUQueue()
        vData.append(1)
        vData.append("Google Inc.")
        vData.append("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA")
        vData.append(66000000000.0)
        vData.append(1)  # google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(254000.0)
        vData.append(1)
        vData.append(1.4)
        vData.append(0)

        vData.append(2)
        vData.append("Microsoft Inc.")
        vData.append("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA")
        vData.append(93600000000.0)
        vData.append(1)  # google company id
        vData.append("Donald Trump")
        vData.append(datetime.datetime.now())
        sbBlob.SetSize(0)
        sbBlob.SaveAString(astr)
        vData.append(sbBlob.GetBuffer())
        vData.append(astr)
        vData.append(20254000.0)
        vData.append(2)
        vData.append(2.5)
        vData.append(0)

        vData.append(3)
        vData.append("Apple Inc.")
        vData.append("1 Infinite Loop, Cupertino, CA 95014, USA")
        vData.append(234000000000.0)
        vData.append(2)  # Microsoft company id
        vData.append("Hillary Clinton")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(6254000.0)
        vData.append(0)
        vData.append(4.5)
        vData.append(0)
        # first, execute delete from employee;delete from company
        # second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        # third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        # fourth, SELECT * from company;select * from employee;select curtime()
        # last, three sets of call sp_TestProc(?,?,?)
        ok = mysql.ExecuteBatch(tagTransactionIsolation.tiUnspecified, sql, vData, cbExecute, cbRows, cbRowHeader, cbBatchHeader, None, tagRollbackPlan.rpDefault, cbDiscarded, '|')
        return vData

    def InsertBLOBByPreparedStatement():
        wstr = ""
        while len(wstr) < 128 * 1024:
            wstr += u'近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？'
        str = ""
        while len(str) < 256 * 1024:
            str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.'
        sqlInsert = u'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)'
        ok = mysql.Prepare(sqlInsert, cb)

        vData = []
        sbBlob = CUQueue()

        #first set of data
        vData.append(1) #google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(254000.0)

        #second set of data
        vData.append(1) #google company id
        vData.append("Donald Trump")
        vData.append(datetime.datetime.now())
        sbBlob.SetSize(0)
        sbBlob.SaveAString(str)
        vData.append(sbBlob.GetBuffer())
        vData.append(str)
        vData.append(20254000.0)

        #third set of data
        vData.append(2) #Microsoft company id
        vData.append("Hillary Clinton")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(6254000.0)
        return mysql.ExecuteParameters(vData, cbExecute)

    def TestStoredProcedure():
        # two sets (2 * 3) of parameter data
        # 1st set -- 1, 0, 0
        # 2nd set -- 2, 0, 0
        vData = [1, 1.5, 0, 2, 1.8, 0]

        ok = mysql.Prepare('call sp_TestProc(?,?,?)', cb)
        #send multiple sets of parameter data in one shot
        ok = mysql.ExecuteParameters(vData, cbExecute, cbRows, cbRowHeader)
        return vData

    ok = mysql.Open(u'', cb)
    ok = TestCreateTables()
    ok = mysql.ExecuteSql('delete from employee;delete from company', cbExecute)
    ok = TestPreparedStatements()
    ok = InsertBLOBByPreparedStatement()
    ok = mysql.ExecuteSql('SELECT * from company;select * from employee;select curtime()', cbExecute, cbRows, cbRowHeader)

    vPData = TestStoredProcedure()
    ok = mysql.WaitAll()
    print('vPData: ' + str(vPData))
    print('')
    print('There are ' + str(mysql.Outputs * 2) + ' output data returned')

    vData = TestBatch()
    ok = mysql.WaitAll()
    # print('vData: ' + str(vData))
    print('')  # put debug point here and see what happens to ra and vData
    print('There are ' + str(mysql.Outputs * 3) + ' output data returned')
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
