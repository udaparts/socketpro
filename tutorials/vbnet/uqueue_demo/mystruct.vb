Imports SocketProAdapter

Public Class CMyStruct
    Implements IUSerializer

    Private NullString As String = Nothing
    Private ObjectNull As Object = Nothing
    Private ADateTime As Date = Date.Now
    Public ADouble As Double
    Public ABool As Boolean
    Public UnicodeString As String
    Public AsciiString() As Byte
    Public ObjBool As Object = True
    Public ObjString As Object
    Public objArrString As Object
    Public objArrInt As Object

    Public Shared Function MakeOne() As CMyStruct
        Dim msOriginal As New CMyStruct()
        Dim arrInt() As Integer = {1, 76890}
        msOriginal.objArrInt = arrInt
        Dim arrString() As String = {"Hello", "world"}
        msOriginal.objArrString = arrString
        msOriginal.ObjBool = True
        msOriginal.ObjString = "test"
        msOriginal.UnicodeString = "Unicode"
        msOriginal.ABool = True
        msOriginal.ADouble = 1234.567
        msOriginal.AsciiString = System.Text.ASCIIEncoding.ASCII.GetBytes("ASCII")
        Return msOriginal
    End Function

    'make sure both serialization and de-serialization match against each other.
    Public Sub LoadFrom(ByVal UQueue As CUQueue) Implements IUSerializer.LoadFrom
        UQueue.Load(NullString).Load(ObjectNull).Load(ADateTime).Load(ADouble).Load(ABool).Load(UnicodeString).Load(AsciiString).Load(ObjBool).Load(ObjString).Load(objArrString).Load(objArrInt)
    End Sub

    'make sure both serialization and de-serialization match against each other.
    Public Sub SaveTo(ByVal UQueue As CUQueue) Implements IUSerializer.SaveTo
        UQueue.Save(NullString).Save(ObjectNull).Save(ADateTime).Save(ADouble).Save(ABool).Save(UnicodeString).Save(AsciiString).Save(ObjBool).Save(ObjString).Save(objArrString).Save(objArrInt)
    End Sub
End Class

