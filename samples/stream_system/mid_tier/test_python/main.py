
from functools import total_ordering
from spa import CDataSet, CTable, CMasterSlaveBase
from decimal import Decimal
import datetime, uuid
from spa.udb import *
from spa.clientside import CSqlite, CConnectionContext
from sqlpool import CSqlMasterPool

slave = CSqlMasterPool.CSlavePool(CSqlite, 'sakila.db')

with CMasterSlaveBase(CSqlite, 'sakila.db') as sp:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    ok = sp.StartSocketPool(cc, 1, 1)
    hw = sp.Seek()
    hw = None


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