Imports SocketProAdapter.ClientSide

Public Class HelloWorld
    Inherits CAsyncServiceHandler

    Public Sub New()
        MyBase.New(hwConst.sidHelloWorld)
    End Sub

    Public Function SayHello(ByVal firstName As String, ByVal lastName As String) As String
        Dim SayHelloRtn As String = Nothing
        Dim bProcessRy As Boolean = ProcessR1(hwConst.idSayHelloHelloWorld, firstName, lastName, SayHelloRtn)
        Return SayHelloRtn
    End Function

    Public Sub Sleep(ByVal ms As Integer)
        Dim bProcessRy As Boolean = ProcessR0(hwConst.idSleepHelloWorld, ms)
    End Sub

    Public Function Echo(ByVal ms As CMyStruct) As CMyStruct
        Dim EchoRtn As CMyStruct = Nothing
        Dim bProcessRy As Boolean = ProcessR1(hwConst.idEchoHelloWorld, ms, EchoRtn)
        Return EchoRtn
    End Function
End Class
