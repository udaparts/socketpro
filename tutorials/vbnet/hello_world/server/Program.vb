Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
    Inherits CSocketProServer

    <ServiceAttr(hwConst.sidHelloWorld)> _
    Private m_HelloWorld As New CSocketProService(Of HelloWorldPeer)()
    'One SocketPro server supports any number of services. You can list them here!

    Shared Sub Main(ByVal args() As String)
        Dim MySocketProServer As New CMySocketProServer()
        If Not MySocketProServer.Run(20901) Then
            Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
        End If
        Console.WriteLine("Input a line to close the application ......")
        Console.ReadLine()
        MySocketProServer.StopSocketProServer() 'or MySocketProServer.Dispose();
    End Sub

    Protected Overrides Function OnIsPermitted(ByVal hSocket As ULong, ByVal userId As String, ByVal password As String, ByVal nSvsID As UInteger) As Boolean
        Console.WriteLine("Ask for a service " & nSvsID & " from user " & userId & " with password = " & password)
        Return True
    End Function

    Protected Overrides Sub OnClose(ByVal hSocket As ULong, ByVal nError As Integer)
        Dim bs As CBaseService = CBaseService.SeekService(hSocket)
        If bs IsNot Nothing Then
            Dim sp As CSocketPeer = bs.Seek(hSocket)
            ' ......
        End If
    End Sub
End Class
