
from functools import total_ordering
from dataset import CTable

@total_ordering
class MinType(object):
    def __le__(self, other):
        return True
    def __eq__(self, other):
        return self is other

Min = MinType()

tbl = CTable(1)
tbl_copy = tbl.Copy()

obj = -0.35
obj0 = -1

ok = obj0 > obj

def my_key(a):
    if a is None:
        return Min
    return a
arr = ['4', u'', None, u'ABC', 'abc']

arr.sort(key=my_key, reverse=True)

tbl_copy = None