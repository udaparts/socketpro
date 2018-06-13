#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from spa.udb import *
from spa import Pair
from spa.clientside import CSocketPool, CConnectionContext, COdbc, CUQueue
from spa.udb import CParameterInfo
import datetime
from decimal import Decimal

with CSocketPool(COdbc) as spOdbc:
    print('Remote async odbc server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'odbcUser', 'TooMuchSecret')
    ok = spOdbc.StartSocketPool(cc, 1, 1)
    odbc = spOdbc.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(odbc.AttachedClientSocket.ErrorCode))
        spOdbc.ShutdownPool()
        exit(0)

    def cb(odbc, res, errMsg):
        print('res = ' + str(res) + ', errMsg: ' + errMsg)

    def cbExecute(odbc, res, errMsg, affected, fail_ok, lastRowId):
        print('affected = ' + str(affected) + ', fails = ' + str(fail_ok >> 32) + ', oks = ' + str(fail_ok & 0xffffffff) + ', res = ' + str(res) + ', errMsg: ' + errMsg)

    def TestCreateTables():
        ok = odbc.ExecuteSql('CREATE DATABASE IF NOT EXISTS mysqldb character set utf8 collate utf8_general_ci;USE mysqldb', cbExecute)
        ok = odbc.ExecuteSql('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)', cbExecute)
        ok = odbc.ExecuteSql('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))', cbExecute)
        ok = odbc.ExecuteSql('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(15,2),out p_last_dt datetime) BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END', cbExecute)

    ra = []

    def cbRows(odbc, lstData):
        index = len(ra) - 1
        ra[index].second.append(lstData)

    def cbRowHeader(odbc):
        vColInfo = odbc.ColumnInfo
        ra.append(Pair(vColInfo, []))

    def TestPreparedStatements():
        sql_insert_parameter = u'INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)'
        ok = odbc.Prepare(sql_insert_parameter, cb)

        vData = []
        vData.append(1)
        vData.append("Google Inc.")
        vData.append("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA")
        vData.append(66000000000.12)

        vData.append(2)
        vData.append("Microsoft Inc.")
        vData.append("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA")
        vData.append(93600000000.16)

        vData.append(3)
        vData.append("Apple Inc.")
        vData.append("1 Infinite Loop, Cupertino, CA 95014, USA")
        vData.append(234000000000.04)
        return odbc.ExecuteParameters(vData, cbExecute)

    def InsertBLOBByPreparedStatement():
        wstr = ""
        while len(wstr) < 128 * 1024:
            wstr += u'近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？'
        str = ""
        while len(str) < 256 * 1024:
            str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.'
        sqlInsert = u'insert into employee(CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?)'
        ok = odbc.Prepare(sqlInsert, cb)

        vData = []
        sbBlob = CUQueue()

        #first set of data
        vData.append(1) #google company id
        vData.append("Ted Cruz")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(254000.12)

        #second set of data
        vData.append(1) #google company id
        vData.append("Donald Trump")
        vData.append(datetime.datetime.now())
        sbBlob.SetSize(0)
        sbBlob.SaveAString(str)
        vData.append(sbBlob.GetBuffer())
        vData.append(str)
        vData.append(20254000.24)

        #third set of data
        vData.append(2) #Microsoft company id
        vData.append("Hillary Clinton")
        vData.append(datetime.datetime.now())
        sbBlob.SaveString(wstr)
        vData.append(sbBlob.GetBuffer())
        vData.append(wstr)
        vData.append(6254000.12)
        return odbc.ExecuteParameters(vData, cbExecute)

    def TestStoredProcedure(vData):
        vInfo = []

        info = CParameterInfo()
        info.DataType = tagVariantDataType.sdVT_I4
        vInfo.append(info)

        info = CParameterInfo()
        info.DataType = tagVariantDataType.sdVT_DECIMAL
        info.Direction = tagParameterDirection.pdInputOutput
        info.Scale = 2
        vInfo.append(info)

        info = CParameterInfo()
        info.DataType = tagVariantDataType.sdVT_DATE
        info.Direction = tagParameterDirection.pdOutput
        vInfo.append(info)

        ok = odbc.Prepare('call sp_TestProc(?, ?, ?)', cb, vInfo)
        #send multiple sets of parameter data in one shot
        return odbc.ExecuteParameters(vData, cbExecute, cbRows, cbRowHeader)

    ok = odbc.Open(u'dsn=ToMySQL;uid=root;pwd=Smash123', cb)
    ok = TestCreateTables()
    ok = odbc.ExecuteSql('delete from employee;delete from company', cbExecute)
    ok = TestPreparedStatements()
    ok = InsertBLOBByPreparedStatement()
    ok = odbc.ExecuteSql('SELECT * from company;select * from employee;select curtime()', cbExecute, cbRows, cbRowHeader)

    #two sets (2 * 3) of parameter data
    # 1st set -- 1, 2.35, 0
    # 2nd set -- 2, 1.22, 0
    vPData = [1, Decimal('2.35'), 0, 2, Decimal('1.22'), 0]
    TestStoredProcedure(vPData)
    ok = odbc.WaitAll()
    print('')
    print('Parameters: ' + str(vPData))
    print('')
    ok = odbc.Tables(u'sakila', u'', u'%', u'TABLE', cbExecute, cbRows, cbRowHeader)
    ok = odbc.WaitAll()

    ok = odbc.ExecuteSql(u'use sakila', cbExecute)
    row_index = len(ra) - 1
    p = ra[row_index]
    cols = len(p.first)
    data = p.second[0]
    tables = len(data)/cols
    row = 0
    while row < tables:
        sql = u'select * from ' + data[row * cols + 2]
        ok = odbc.ExecuteSql(sql, cbExecute, cbRows, cbRowHeader)
        row += 1
    ok = odbc.WaitAll()

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