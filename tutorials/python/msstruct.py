
from spa import IUSerializer
from datetime import datetime

class CMyStruct(IUSerializer):
    def __init__(self):
        self.NullString = None
        self.ObjectNull = None
        self.ADateTime = datetime.now()

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

    def SaveTo(self, q):
        q.SaveString(self.NullString).SaveObject(self.ObjectNull).SaveDate(self.ADateTime).SaveDouble(self.ADouble)
        q.SaveBool(self.ABool).SaveString(self.UnicodeString).SaveAString(self.AsciiString).SaveObject(self.ObjBool)
        q.SaveObject(self.ObjString).SaveObject(self.objArrString).SaveObject(self.objArrInt)

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