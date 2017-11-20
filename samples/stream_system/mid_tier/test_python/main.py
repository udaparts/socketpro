
from functools import total_ordering
from spa import CDataSet, CTable, CSqlMasterPool, CMasterPool
from decimal import Decimal
import datetime, uuid, sys
from spa.udb import *
from spa.clientside import CSqlite, CConnectionContext

slave = CSqlMasterPool.CSlavePool(CSqlite, 'sakila.db')

with CSqlMasterPool(CSqlite, 'sakila.db', False) as sp:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    ok = sp.StartSocketPool(cc, 1, 1)
    cache = sp.Cache
    tbl = CTable()
    res = cache.Find('main', 'mynulls', 0, CTable.Operator.great, -1, tbl)
    res = tbl.Sort(0, True)
    res = tbl.Sort(0, False)
    res = tbl.Sort(1, True)
    res = tbl.Sort(1, False)
    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()
    res = 0

var = [(1, 'str'), (2, 'ye'), (3, 'char')]
for (myd, mys) in var:
    myd = 0

meta = CDBColumnInfoArray()
meta.append(CDBColumnInfo())
meta.append(CDBColumnInfo())

ds = CDataSet()

tbl = CTable(meta)
keys = tbl.Keys

n = tbl._eq_(u' ', ' ')

tbl_copy = tbl.Copy()

obj = -0.35
obj0 = -1

ok = obj0 > obj

def my_key(a):
    if a is None:
        return CTable._MIN_
    return a
arr = ['4', u'', None, u'ABC', 'abc']

arr.sort(key=my_key, reverse=True)

tbl_copy = None