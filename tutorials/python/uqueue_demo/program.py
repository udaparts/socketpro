from msstruct import CMyStruct
from spa import CUQueue
from decimal import Decimal
import uuid
from datetime import datetime

msOriginal = CMyStruct.MakeOne()
q = CUQueue()
msOriginal.SaveTo(q)
ms = CMyStruct()
ms.LoadFrom(q)

with CUQueue() as q:
    q.Empty()
    q.SaveInt(123).SaveInt(-1)
    n = q.LoadInt()
    assert(n == 123)
    n = q.LoadInt()
    assert(n == -1)
    assert(q.Size == 0)

    q.SaveInt(234).SaveAString('test me')
    n = q.LoadInt()
    assert(n == 234)
    s = q.LoadAString()
    assert(s == 'test me')
    assert(q.Size == 0)

    us = u'test'
    q.SaveString(us).SaveString(None)
    s = q.LoadString()
    assert(s == u'test')
    s = q.LoadString()
    assert(s is None)
    assert(q.Size == 0)

    cls_id = uuid.uuid4()
    q.SaveUUID(cls_id)
    s = q.LoadUUID()
    assert(s == cls_id)
    assert(q.Size == 0)

    s = datetime.now()
    q.SaveDate(s)
    s1 = q.LoadDate()
    # assert(s == s1)
    assert(q.Size == 0)

    q.SaveObject([1.25,2.1])
    us = q.LoadObject()
    assert(q.Size == 0)

    dec = Decimal(-10.2525)
    dec_tuple = dec.as_tuple()
    q.SaveDecimal(dec)
    decRes = q.LoadDecimal()
    assert(q.Size == 0)
