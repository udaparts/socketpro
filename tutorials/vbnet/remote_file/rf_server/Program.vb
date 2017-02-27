Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
    Inherits CSocketProServer

    <ServiceAttr(RemFileConst.sidRemotingFile)> _
    Private m_RemotingFile As New CSocketProService(Of RemotingFilePeer)()
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

End Class

