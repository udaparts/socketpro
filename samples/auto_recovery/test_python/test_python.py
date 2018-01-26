#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, copy
from spa.clientside import CSocketPool, CConnectionContext, CSqlite

class MyStruct(object):
    def __init__(self):
        self.reset()

    def reset(self):
        self.dmax = 0.0
        self.dmin = 0.0
        self.davg = 0.0
        self.returned = 0
        self.row = []

with CSocketPool(CSqlite) as sp:
    cycles = 10000
    sessions_per_host = 2
    threads = 1
    vHost = ['localhost', 'ws-yye-1']
    channels = sessions_per_host * len(vHost)
    sp.QueueName = 'ar_python'
    mcc = [[0 for i in range(channels)] for i in range(threads)]
    m = 0
    while m < threads:
        n = 0
        while n < sessions_per_host:
            j = 0
            while j < len(vHost):
                mcc[m][n * sessions_per_host + j] = CConnectionContext(vHost[j], 20901, 'root', 'Smash123')
                j += 1
            n += 1
        m += 1
    ok = sp.StartSocketPoolEx(mcc)
    if not ok:
        s = input('No connection to SQL stream server and press ENTER key to shut down the application ......')
        exit(-1)
    handler = sp.SeekByQueue()
    sql = 'SELECT max(amount), min(amount), avg(amount) FROM payment'
    filter = input('Input a filter for payment_id')
    if len(filter) > 0:
        sql += (' WHERE ' + filter)
    v = sp.AsyncHandlers
    def dr(h, res, errMsg):
        if res:
            print('Error code: %d, error message: %s' % (res, errMsg))
    for h in v:
        ok = h.Open('sakila.db', dr)

    mystruct = MyStruct()
    def r(h, vData):
        mystruct.row = copy.deepcopy(vData)

    def er(h, res, errMsg, affected, fail_ok, lastId):
        if res != 0:
            print('Error code: %d, error message: %s' % (res, errMsg))
        else:
            mystruct.dmax += mystruct.row[0]
            mystruct.dmin += mystruct.row[1]
            mystruct.davg += mystruct.row[2]
        mystruct.returned += 1
    h = sp.SeekByQueue()
    ok = h.Execute(sql, er, r)
    ok = h.WaitAll()
    print('Result: max = %f, min = %f, avg = %f' % ( mystruct.dmax, mystruct.dmin, mystruct.davg))
    mystruct.reset()
    n = 0
    while n < cycles:
        h = sp.SeekByQueue()
        ok = h.Execute(sql, er, r)
        n += 1
    for h in v:
        ok = h.WaitAll()
    print('Returned = %d, max = %f, min = %f, avg = %f' % (mystruct.returned, mystruct.dmax, mystruct.dmin, mystruct.davg))
    s = input('Press ENTER key to shut down the application ......')
