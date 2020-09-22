from spa import IUSerializer
from datetime import datetime


class CMyStruct(IUSerializer):
    def __init__(self):
        self.NullString = None
        self.ObjectNull = None
        self.ADateTime = datetime.now()

    @staticmethod
    def MakeOne():
        ms = CMyStruct()
        ms.objArrInt = [1, 76890]
        ms.objArrString = [u'Hello', u'world']
        ms.ObjBool = True
        ms.ObjString = u'test'
        ms.UnicodeString = u'Unicode'
        ms.ABool = True
        ms.ADouble = 1234.567
        ms.AsciiString = 'ASCII'
        return ms

    def __str__(self):
        return str(self.__repr__())

    def __repr__(self):
        return {'NullString': self.NullString, 'ObjectNull': self.ObjectNull, 'ADateTime': self.ADateTime,
                'ADouble': self.ADouble, 'ABool': self.ABool, 'UnicodeString': self.UnicodeString,
                'AsciiString': self.AsciiString, 'ObjBool': self.ObjBool, 'ObjString': self.ObjString,
                'objArrString': self.objArrString, 'objArrInt': self.objArrInt}

    def LoadFrom(self, q):
        self.NullString = q.LoadString()
        self.ObjectNull = q.LoadObject()
        self.ADateTime = q.LoadDate()
        self.ADouble = q.LoadDouble()
        self.ABool = q.LoadBool()
        self.UnicodeString = q.LoadString()
        self.AsciiString = q.LoadAString()
        self.ObjBool = q.LoadObject()
        self.ObjString = q.LoadObject()
        self.objArrString = q.LoadObject()
        self.objArrInt = q.LoadObject()
        return q

    def SaveTo(self, q):
        q.SaveString(self.NullString).SaveObject(self.ObjectNull).SaveDate(self.ADateTime).SaveDouble(self.ADouble)
        q.SaveBool(self.ABool).SaveString(self.UnicodeString).SaveAString(self.AsciiString).SaveObject(self.ObjBool)
        q.SaveObject(self.ObjString).SaveObject(self.objArrString).SaveObject(self.objArrInt)
        return q
