' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter.ServerSide

'server implementation for service HelloWorld
Public Class HelloWorldPeer
    Inherits CClientPeer

    <RequestAttr(hwConst.idSayHelloHelloWorld)> _
    Private Function SayHello(ByVal firstName As String, ByVal lastName As String) As String
        Dim res As String = "Hello " & firstName & " " & lastName
        Console.WriteLine(res)
        Return res
    End Function

    <RequestAttr(hwConst.idSleepHelloWorld, True)> _
    Private Sub Sleep(ByVal ms As Integer) 'true -- slow request
        System.Threading.Thread.Sleep(ms)
    End Sub

    <RequestAttr(hwConst.idEchoHelloWorld)> _
    Private Function Echo(ByVal ms As CMyStruct) As CMyStruct
        Return ms
    End Function
End Class
