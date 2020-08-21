#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from spa.udb import *
from spa import CServerError as Se
from spa import Pair
from spa.clientside import CSocketPool, CConnectionContext, CMysql, CUQueue, CSocketError
import datetime

# prepare two large texts
g_wstr = u''
while len(g_wstr) < 128 * 1024:
    g_wstr += u'近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？'
g_astr = ''
while len(g_astr) < 256 * 1024:
    g_astr += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.'

with CSocketPool(CMysql) as spMysql:
    print('Remote async mysql server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20902, 'root', 'Smash123')
    ok = spMysql.StartSocketPool(cc, 1)
    mysql = spMysql.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(mysql.Socket.ErrorCode))
        spMysql.ShutdownPool()
        exit(0)

    def TestCreateTables():
        return [mysql.execute('CREATE DATABASE IF NOT EXISTS mysqldb character set utf8 collate utf8_general_ci;USE mysqldb'),
                mysql.execute('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)'),
                mysql.execute('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(18,2),FOREIGN KEY(CompanyId)REFERENCES company(id))'),
                mysql.execute('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(17,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now() into p_last_dt;END')]

    def TestPreparedStatements():
        mysql.prepare(u'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)')
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
        return mysql.execute(vData)

    ra = []

    def cbRows(mysql, lstData):
        index = len(ra) - 1
        ra[index].second.append(lstData)

    def cbRowHeader(mysql):
        vColInfo = mysql.ColumnInfo
        ra.append(Pair(vColInfo, []))

    def cbBatchHeader(mysql):
        print('Batch header come here')

    def TestBatch():
        # sql with delimiter '|'
        sql='delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from company;select * from employee;select curtime()|call sp_TestProc(?,?,?)'
        vData = []
        sbBlob = CUQueue()
        vData.append(1)
        vData.append("Google Inc.")
        vData.append("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA")
        vData.append(66000000000.0)
        vData.append(1)  # google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(g_wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_wstr)
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
        sbBlob.SaveAString(g_astr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_astr)
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
        sbBlob.SaveString(g_wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_wstr)
        vData.append(6254000.0)
        vData.append(0)
        vData.append(4.5)
        vData.append(0)
        # first start a manual transaction with isolation level ReadCommited
        # second, execute delete from employee;delete from company
        # third, prepare and execute three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        # fourth, prepare and execute three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        # fifth, SELECT * from company;select * from employee;select curtime()
        # sixth, three sets of call sp_TestProc(?,?,?)
        # last, commit deletes and inserts if no error happens or rollback if there is an error
        return mysql.executeBatch(tagTransactionIsolation.tiReadCommited, sql, vData, cbRows, cbRowHeader, '|', cbBatchHeader), vData

    def InsertBLOBByPreparedStatement():

        mysql.prepare(u'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)')

        vData = []
        sbBlob = CUQueue()

        # first set of data
        vData.append(1)  # google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(g_wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_wstr)
        vData.append(254000.0)

        # second set of data
        vData.append(1)  # google company id
        vData.append("Donald Trump")
        vData.append(datetime.datetime.now())
        sbBlob.SetSize(0)
        sbBlob.SaveAString(g_astr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_astr)
        vData.append(20254000.0)

        # third set of data
        vData.append(2)  # Microsoft company id
        vData.append("Hillary Clinton")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(g_wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(g_wstr)
        vData.append(6254000.0)
        return mysql.execute(vData)

    def TestStoredProcedure():
        # two sets (2 * 3) of parameter data
        # 1st set -- 1, 0, 0
        # 2nd set -- 2, 0, 0
        vData = [1, 1.5, 0, 2, 1.8, 0]

        mysql.prepare('call sp_TestProc(?,?,?)')
        # send multiple sets of parameter data in one shot
        return mysql.execute(vData, cbRows, cbRowHeader), vData
    try:
        # stream all SQL requests with in-line batching for the best network efficiency
        fOpen = mysql.open(u'')
        vF = TestCreateTables()
        fD = mysql.execute('delete from employee;delete from company')
        fP0 = TestPreparedStatements()
        fP1 = InsertBLOBByPreparedStatement()
        fS = mysql.execute('SELECT * from company;select * from employee;select curtime()', cbRows, cbRowHeader)
        fStore, vPData = TestStoredProcedure()
        fStore1, vData = TestBatch()

        print('SQL requests streamed, and waiting for results ......')
        print(fOpen.result())
        for f in vF:
            print(f.result())
        print(fP0.result())
        print(fP1.result())
        print(fS.result())
        print(fStore.result())
        print('vPData: ' + str(vPData))
        print('')
        print('There are ' + str(mysql.Outputs * 2) + ' output data returned')
        print(fStore1.result())
        # print('vPData: ' + str(vData))
        print('There are ' + str(mysql.Outputs * 3) + ' output data returned')
        print('')  # put debug point here and see what happens to ra and vData
    except Se as ex:  # an exception from remote server
        print(ex)
    except CSocketError as ex:  # a communication error
        print(ex)
    except Exception as ex:
        print('Unexpected error: ' + str(ex))  # invalid parameter, bad de-serialization, and so on
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
